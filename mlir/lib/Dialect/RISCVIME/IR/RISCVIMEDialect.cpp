#include "mlir/Dialect/RISCVIME/RISCVIMEDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/ImplicitLocOpBuilder.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"   // ← add this line
using namespace mlir;
using namespace mlir::riscv_ime;

#include "mlir/Dialect/RISCVIME/RISCVIMEOpsDialect.cpp.inc"

void RISCVIMEDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/RISCVIME/RISCVIMEOps.cpp.inc"
  >();
}

#define GET_OP_CLASSES
#include "mlir/Dialect/RISCVIME/RISCVIMEOps.cpp.inc"
