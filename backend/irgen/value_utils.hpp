#pragma once

#include "../lowering/lowered_model.hpp"

namespace llvm {
class Value;
}

namespace flux {

class IRGenContext;
class TypeConverter;

inline bool IsNumericKind(LoweredTypeKind kind) {
  switch (kind) {
  case LoweredTypeKind::Char:
  case LoweredTypeKind::Short:
  case LoweredTypeKind::Int:
  case LoweredTypeKind::Float:
  case LoweredTypeKind::Double:
    return true;
  default:
    return false;
  }
}

inline int NumericRank(const LoweredType &type) {
  switch (type.kind) {
  case LoweredTypeKind::Char:
    return 0;
  case LoweredTypeKind::Short:
    return 1;
  case LoweredTypeKind::Int:
    return 2;
  case LoweredTypeKind::Float:
    return 3;
  case LoweredTypeKind::Double:
    return 4;
  default:
    return -1;
  }
}

inline LoweredType CommonNumericType(const LoweredType &lhs, const LoweredType &rhs) {
  return NumericRank(lhs) >= NumericRank(rhs) ? lhs : rhs;
}

llvm::Value *ConvertValue(IRGenContext &ctx, TypeConverter &types, llvm::Value *value, const LoweredType &from,
                          const LoweredType &to, bool for_memory = false);

llvm::Value *AdjustForStore(IRGenContext &ctx, TypeConverter &types, const LoweredType &target_type,
                            const LoweredType &source_type, llvm::Value *value);

} // namespace flux
