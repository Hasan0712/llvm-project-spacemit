


//Builtins for Riscv spacemit ime vmadot

include "riscv_vector_common.td"
multiclass RVVVMADOT<list<list<string>> i_suffixes_prototypes> {
  let RequiredFeatures = ["xsmtvdot"],
      UnMaskedPolicyScheme = HasPolicyOperand,
      HasMaskedOffOperand = false,
      Log2LMUL = [0],          // adjust to all legal values for VLEN=256
      OverloadedName = NAME in {
    defm NAME : RVVOutOp1Op2BuiltinSet<NAME, "i", i_suffixes_prototypes>;
  }
}

defm vmadot   : RVVVMADOT<[["vv", "v",  "vv(FixedSEW:8)v(FixedSEW:8)v"]]>;
defm vmadotu  : RVVVMADOT<[["vv", "Uv", "UvUv(FixedSEW:8)Uv(FixedSEW:8)Uv"]]>;
defm vmadotus : RVVVMADOT<[["vv", "v",  "vv(FixedSEW:8)Uv(FixedSEW:8)Uv"]]>;
defm vmadotsu : RVVVMADOT<[["vv", "v",  "UvUv(FixedSEW:8)v(FixedSEW:8)v"]]>;
