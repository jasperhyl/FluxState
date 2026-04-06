#include "value_utils.hpp"
#include "irgen_context.hpp"
#include "type_converter.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace flux {

namespace {

bool IsFloatingKind(LoweredTypeKind kind) {
  return kind == LoweredTypeKind::Float || kind == LoweredTypeKind::Double;
}

llvm::Value *ConvertToBool(IRGenContext &ctx, TypeConverter &types, llvm::Value *value, bool for_memory) {
  llvm::Value *bool_value = value;
  if (!value->getType()->isIntegerTy(1)) {
    if (value->getType()->isIntegerTy()) {
      bool_value = ctx.Builder().CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0));
    } else if (value->getType()->isFloatingPointTy()) {
      bool_value = ctx.Builder().CreateFCmpONE(value, llvm::ConstantFP::get(value->getType(), 0.0));
    }
  }

  if (for_memory) {
    return bool_value->getType()->isIntegerTy(8) ? bool_value
                                                 : ctx.Builder().CreateZExt(bool_value, types.ToIRType(LoweredType{
                                                                                             .kind = LoweredTypeKind::Bool},
                                                                                         true));
  }
  return bool_value;
}

} // namespace

llvm::Value *ConvertValue(IRGenContext &ctx, TypeConverter &types, llvm::Value *value, const LoweredType &from,
                          const LoweredType &to, bool for_memory) {
  if (value == nullptr || from.kind == LoweredTypeKind::Invalid || to.kind == LoweredTypeKind::Invalid) {
    return value;
  }

  llvm::Type *target_type = types.ToIRType(to, for_memory);
  if (value->getType() == target_type) {
    return value;
  }

  if (to.kind == LoweredTypeKind::Bool) {
    return ConvertToBool(ctx, types, value, for_memory);
  }

  if (IsNumericKind(from.kind) && IsNumericKind(to.kind)) {
    if (IsFloatingKind(from.kind) && IsFloatingKind(to.kind)) {
      return value->getType()->getPrimitiveSizeInBits() < target_type->getPrimitiveSizeInBits()
                 ? ctx.Builder().CreateFPExt(value, target_type)
                 : ctx.Builder().CreateFPTrunc(value, target_type);
    }
    if (IsFloatingKind(from.kind) && !IsFloatingKind(to.kind)) {
      return ctx.Builder().CreateFPToSI(value, target_type);
    }
    if (!IsFloatingKind(from.kind) && IsFloatingKind(to.kind)) {
      return ctx.Builder().CreateSIToFP(value, target_type);
    }
    if (value->getType()->getIntegerBitWidth() < target_type->getIntegerBitWidth()) {
      return ctx.Builder().CreateSExt(value, target_type);
    }
    if (value->getType()->getIntegerBitWidth() > target_type->getIntegerBitWidth()) {
      return ctx.Builder().CreateTrunc(value, target_type);
    }
    return value;
  }

  if (value->getType()->isPointerTy() && target_type->isPointerTy()) {
    return value;
  }

  return value;
}

llvm::Value *AdjustForStore(IRGenContext &ctx, TypeConverter &types, const LoweredType &target_type,
                            const LoweredType &source_type, llvm::Value *value) {
  return ConvertValue(ctx, types, value, source_type, target_type, true);
}

} // namespace flux
