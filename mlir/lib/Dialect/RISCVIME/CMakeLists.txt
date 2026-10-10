add_subdirectory(Transforms)

add_mlir_dialect_library(MLIRRISCVIME
  IR/RISCVIMEDialect.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/RISCVIME

  DEPENDS
  MLIRRISCVIMEOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRSideEffectInterfaces
  MLIRVectorDialect
  MLIRArithDialect
)
