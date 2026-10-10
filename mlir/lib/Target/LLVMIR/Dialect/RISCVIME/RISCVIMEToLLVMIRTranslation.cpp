//===- RISCVIMEToLLVMIRTranslation.cpp - Translate RISCVIME to LLVM IR ----===//
#include "mlir/Target/LLVMIR/Dialect/RISCVIME/RISCVIMEToLLVMIRTranslation.h"
#include "mlir/Dialect/RISCVIME/RISCVIMEDialect.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/LLVMTranslationInterface.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::riscv_ime;

namespace {

class RISCVIMEDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  LogicalResult
  convertOperation(Operation *op, llvm::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {
    auto vmadot = dyn_cast<VmadotOp>(op);
    if (!vmadot)
      return failure();

    llvm::Value *lhs = moduleTranslation.lookupValue(vmadot.getLhs());
    llvm::Value *rhs = moduleTranslation.lookupValue(vmadot.getRhs());
    llvm::Value *acc = moduleTranslation.lookupValue(vmadot.getAcc());
    llvm::Value *vl  = moduleTranslation.lookupValue(vmadot.getVl());
    if (!lhs || !rhs || !acc || !vl)
      return op->emitError("missing translated operands");

    llvm::Type *resultTy =
        moduleTranslation.convertType(vmadot.getResult().getType());
    if (!resultTy)
      return op->emitError("failed to convert result type");

    llvm::Value *policy = builder.getInt64(0);

    // Mangled name: llvm.riscv.vmadot.v16i32.v32i8.v32i8.i64
    auto *resVT = llvm::cast<llvm::VectorType>(resultTy);
    auto *lhsVT = llvm::cast<llvm::VectorType>(lhs->getType());
    auto *rhsVT = llvm::cast<llvm::VectorType>(rhs->getType());

    std::string mangled = "llvm.riscv.vmadot.";
    mangled += "v" + std::to_string(resVT->getElementCount().getKnownMinValue()) +
               "i" + std::to_string(resVT->getElementType()->getIntegerBitWidth());
    mangled += ".v" + std::to_string(lhsVT->getElementCount().getKnownMinValue()) +
               "i" + std::to_string(lhsVT->getElementType()->getIntegerBitWidth());
    mangled += ".v" + std::to_string(rhsVT->getElementCount().getKnownMinValue()) +
               "i" + std::to_string(rhsVT->getElementType()->getIntegerBitWidth());
    mangled += ".i64";

    llvm::FunctionType *fnTy = llvm::FunctionType::get(
        resultTy,
        {acc->getType(), lhs->getType(), rhs->getType(),
         vl->getType(), policy->getType()},
        false);

    llvm::FunctionCallee callee =
        moduleTranslation.getLLVMModule()->getOrInsertFunction(mangled, fnTy);

    // (vd, vs1, vs2, vl, policy)
    llvm::Value *result =
        builder.CreateCall(callee, {acc, lhs, rhs, vl, policy});
    moduleTranslation.mapValue(op->getResult(0), result);
    return success();
  }
};

} // namespace

void mlir::registerRISCVIMEDialectTranslation(DialectRegistry &registry) {
  registry.insert<RISCVIMEDialect>();
  registry.addExtension(+[](MLIRContext *ctx, RISCVIMEDialect *dialect) {
    dialect->addInterfaces<RISCVIMEDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerRISCVIMEDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerRISCVIMEDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
