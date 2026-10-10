
#ifndef MLIR_DIALECT_RISCVIME_RISCVIMEDIALECT_H
#define MLIR_DIALECT_RISCVIME_RISCVIMEDIALECT_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/IR/BuiltinTypes.h"

#include "mlir/Dialect/RISCVIME/RISCVIMEOpsDialect.h.inc"

#define GET_OP_CLASSES
#include "mlir/Dialect/RISCVIME/RISCVIMEOps.h.inc"

#endif
