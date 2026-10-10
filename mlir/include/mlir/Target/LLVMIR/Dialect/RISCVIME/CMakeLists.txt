add_mlir_translation_library(MLIRRISCVIMEToLLVMIRTranslation
  RISCVIMEToLLVMIRTranslation.cpp

  DEPENDS
  MLIRRISCVIMEOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRLLVMDialect
  MLIRRISCVIME
  MLIRSupport
  MLIRTargetLLVMIRExport
)
