add_mlir_dialect_library(MLIRRISCVIMETransforms
  LowerContractionToRISCVIMEVMADOTPattern.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/RISCVIME

  DEPENDS
  MLIRRISCVIMEOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIRTransformUtils
  MLIRVectorDialect
  MLIRArithDialect
  MLIRFuncDialect
  MLIRRISCVIME
)
