#include "mlir/Dialect/RISCVIME/Transforms/Passes.h"
#include "mlir/Dialect/RISCVIME/RISCVIMEDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::riscv_ime;

namespace {

struct LowerContractionToRISCVIMEVMADOTPattern
    : public OpRewritePattern<vector::ContractionOp> {
  using OpRewritePattern<vector::ContractionOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(vector::ContractionOp op,
                                PatternRewriter &rewriter) const override {
    auto lhsType = dyn_cast<VectorType>(op.getLhs().getType());
    auto rhsType = dyn_cast<VectorType>(op.getRhs().getType());
    auto accType = dyn_cast<VectorType>(op.getAcc().getType());
    auto resType = dyn_cast<VectorType>(op.getResultType());
    if (!lhsType || !rhsType || !accType || !resType)
      return failure();
    if (lhsType.getRank() != 2 || rhsType.getRank() != 2 ||
        accType.getRank() != 2 || resType.getRank() != 2)
      return rewriter.notifyMatchFailure(op, "only 2D vectors supported");

    // Peel shape_cast / extsi chains until we reach an i8 vector.
    auto peelToI8 = [&](Value v) -> FailureOr<Value> {
      for (int i = 0; i < 4; ++i) {
        if (auto sc = v.getDefiningOp<vector::ShapeCastOp>()) {
          v = sc.getSource();
          continue;
        }
        if (auto ext = v.getDefiningOp<arith::ExtSIOp>()) {
          v = ext.getIn();
          continue;
        }
        break;
      }
      auto ty = dyn_cast<VectorType>(v.getType());
      if (!ty || !ty.getElementType().isInteger(8))
        return failure();
      return v;
    };

    auto lhsI8Or = peelToI8(op.getLhs());
    auto rhsI8Or = peelToI8(op.getRhs());
    if (failed(lhsI8Or) || failed(rhsI8Or))
      return rewriter.notifyMatchFailure(
          op, "could not peel to i8 vectors (shape_cast/extsi)");

    Value lhsI8 = *lhsI8Or;
    Value rhsI8 = *rhsI8Or;

    auto ensure2D = [&](Value v, int64_t d0, int64_t d1) -> Value {
      auto ty = cast<VectorType>(v.getType());
      if (ty.getRank() == 2 && ty.getDimSize(0) == d0 &&
          ty.getDimSize(1) == d1)
        return v;
      auto newTy = VectorType::get({d0, d1}, ty.getElementType());
      return vector::ShapeCastOp::create(rewriter, op.getLoc(), newTy, v);
    };

    int64_t M = lhsType.getDimSize(0);
    int64_t K = lhsType.getDimSize(1);
    int64_t N = rhsType.getDimSize(1);

    lhsI8 = ensure2D(lhsI8, M, K);
    rhsI8 = ensure2D(rhsI8, K, N);
    auto lhsI8Ty = cast<VectorType>(lhsI8.getType());
    auto rhsI8Ty = cast<VectorType>(rhsI8.getType());

    if (!accType.getElementType().isInteger(32))
      return rewriter.notifyMatchFailure(op, "accumulator must be i32");

    auto iterTypes = op.getIteratorTypesArray();
    if (iterTypes.size() != 3)
      return failure();
    using IT = vector::IteratorType;
    if (iterTypes[0] != IT::parallel || iterTypes[1] != IT::parallel ||
        iterTypes[2] != IT::reduction)
      return rewriter.notifyMatchFailure(
          op, "expected iterators: parallel, parallel, reduction");

    if (lhsI8Ty.getDimSize(1) != rhsI8Ty.getDimSize(0) ||
        lhsI8Ty.getDimSize(0) != accType.getDimSize(0) ||
        rhsI8Ty.getDimSize(1) != accType.getDimSize(1))
      return rewriter.notifyMatchFailure(op, "incompatible shapes");

    Location loc = op.getLoc();
    Value acc = op.getAcc();

    auto flatten = [&](Value v) -> Value {
      auto vTy = cast<VectorType>(v.getType());
      auto flatTy =
          VectorType::get(vTy.getNumElements(), vTy.getElementType());
      return vector::ShapeCastOp::create(rewriter, loc, flatTy, v);
    };

    Value lhs1d = flatten(lhsI8);
    Value rhs1d = flatten(rhsI8);
    Value acc1d = flatten(acc);
    auto res1dType =
        VectorType::get(resType.getNumElements(), resType.getElementType());

    Value vl = arith::ConstantOp::create(rewriter, loc,
                                         rewriter.getI64IntegerAttr(K));

    auto vmadot =
        VmadotOp::create(rewriter, loc, res1dType, lhs1d, rhs1d, acc1d, vl);

    Value result2d = vector::ShapeCastOp::create(rewriter, loc, resType,
                                                 vmadot.getResult());
    rewriter.replaceOp(op, result2d);
    return success();
  }
};

struct LowerVectorContractToRISCVIMEPass
    : public PassWrapper<LowerVectorContractToRISCVIMEPass,
                         OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(
      LowerVectorContractToRISCVIMEPass)

  StringRef getArgument() const override {
    return "lower-vector-contract-to-riscv-ime";
  }
  StringRef getDescription() const override {
    return "Lower vector.contract to riscv_ime.vmadot";
  }
  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<RISCVIMEDialect, arith::ArithDialect,
                    vector::VectorDialect, func::FuncDialect>();
  }
  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<LowerContractionToRISCVIMEVMADOTPattern>(&getContext());
    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<Pass>
mlir::riscv_ime::createLowerVectorContractToRISCVIMEPass() {
  return std::make_unique<LowerVectorContractToRISCVIMEPass>();
}

void mlir::riscv_ime::registerRISCVIMEPasses() {
  PassRegistration<LowerVectorContractToRISCVIMEPass>();
}

namespace {
static PassRegistration<LowerVectorContractToRISCVIMEPass> pass;
}
