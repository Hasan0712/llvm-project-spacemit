
#ifndef RISCVIME_DIALECT
#define RISCVIME_DIALECT

include "mlir/IR/OpBase.td"

def RISCVIME_Dialect : Dialect {
  let name = "riscv_ime";
  let cppNamespace = "::mlir::riscv_ime";
  let summary = "Dialect for the RISC-V Integer Matrix Extension (IME / XSMTVDot)";
  let description = [{
    Models the SpaceMiT XSMTVDot (smt.vmadot) family of integer matrix
    multiply-accumulate instructions.
  }];
  let useDefaultAttributePrinterParser = 0;
}

#endif
