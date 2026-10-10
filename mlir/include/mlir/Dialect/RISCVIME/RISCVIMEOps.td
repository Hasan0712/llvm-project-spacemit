#ifndef RISCVIME_OPS
#define RISCVIME_OPS
include "mlir/Dialect/RISCVIME/RISCVIMEDialect.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/OpBase.td"
include "mlir/IR/CommonTypeConstraints.td"
include "mlir/Dialect/LLVMIR/LLVMOpBase.td"

// Base class for all RISCVIME ops that lower via the LLVM IR translation
class RISCVIME_Op<string mnemonic, list<Trait> traits = []>
    : LLVM_OpBase<RISCVIME_Dialect, mnemonic, traits>;

def RISCVIME_VmadotOp : RISCVIME_Op<"vmadot", [Pure]> {
  let summary = "Integer matrix multiply-accumulate (smt.vmadot)";
  let description = [{
    result = acc + lhs * transpose(rhs)
    Lowers to the SpacemiT XSMTVDot instruction via inline asm:
        smt.vmadot  vd, vs1, vs2
    Hardware constraints (X60 / VLEN=256):
      - SEW=8, LMUL<=1
      - vd must be even (VRM2 group)
      - one instruction = 4x4x8 int8->int32 MAC
  }];
  let arguments = (ins
      AnyVectorOfAnyRank:$lhs,
      AnyVectorOfAnyRank:$rhs,
      AnyVectorOfAnyRank:$acc,
      AnyTypeOf<[I32, I64, Index]>:$vl
  );
  let results = (outs AnyVectorOfAnyRank:$result);

  // No llvmBuilder – conversion is done only in
  // RISCVIMEToLLVMIRTranslation.cpp

  let assemblyFormat = [{
    $lhs `,` $rhs `,` $acc `,` $vl attr-dict `:`
    type($lhs) `,` type($rhs) `,` type($acc) `,` type($vl)
    `into` type($result)
  }];
}
#endif // RISCVIME_OPS
