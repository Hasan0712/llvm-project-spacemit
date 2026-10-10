// Intrinsics for Spacemit IME / XSMTVDot
let TargetPrefix = "riscv" in {
  // Same style as Andes
  defm vmadot   : RISCVTernaryWide;   // signed × signed → int32
  defm vmadotu  : RISCVTernaryWide;   // unsigned × unsigned → int32
  defm vmadotus : RISCVTernaryWide;   // unsigned × signed → int32
  defm vmadotsu : RISCVTernaryWide;   // signed × unsigned → int32
// 3-operand wrapper so MLIR contract lowering can call it easily.
// Expanded later to the real 5-operand intrinsic.
def int_riscv_vmadot_wrapper
    : DefaultAttrsIntrinsic<[llvm_anyvector_ty],
                            [llvm_anyvector_ty,   // lhs (i8)
                             llvm_anyvector_ty,   // rhs (i8)
                             llvm_anyvector_ty],  // acc (i32)
                            [IntrNoMem]>;
}
