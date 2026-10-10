add_mlir_dialect(RISCVIMEOps riscv_ime)
add_mlir_doc(RISCVIMEDialect RISCVIMEDialect Dialects/ -gen-dialect-doc)
set(LLVM_TARGET_DEFINITIONS RISCVIMEOps.td)          # or whatever .td file contains VmadotOp
mlir_tablegen(RISCVIMEConversions.inc -gen-llvmir-conversions)
add_mlir_dialect_tablegen_target(MLIRRISCVIMEConversionsIncGen)
