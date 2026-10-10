#ifndef MLIR_DIALECT_RISCVIME_TRANSFORMS_PASSES_H
#define MLIR_DIALECT_RISCVIME_TRANSFORMS_PASSES_H

#include "mlir/Pass/Pass.h"

namespace mlir {
namespace riscv_ime {

std::unique_ptr<Pass> createLowerVectorContractToRISCVIMEPass();
void registerRISCVIMEPasses();

} // namespace riscv_ime
} // namespace mlir

#endif
