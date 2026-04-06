#pragma once
#include "../lowering/lowered_model.hpp"

namespace llvm {
class Value;
}

namespace flux {

class IRGenContext;
class TypeConverter;

class ExprIRGen {
public:
  ExprIRGen(IRGenContext &ctx, TypeConverter &types);

  llvm::Value *EmitExpr(const LoweredExpr &expr);
  llvm::Value *EmitAddress(const LoweredExpr &expr);

private:
  llvm::Value *EmitBoolLiteral(const LoweredExpr &expr);
  llvm::Value *EmitCharLiteral(const LoweredExpr &expr);
  llvm::Value *EmitIntLiteral(const LoweredExpr &expr);
  llvm::Value *EmitStringLiteral(const LoweredExpr &expr);
  llvm::Value *EmitVar(const LoweredExpr &expr);
  llvm::Value *EmitUnary(const LoweredExpr &expr);
  llvm::Value *EmitBinary(const LoweredExpr &expr);
  llvm::Value *EmitCall(const LoweredExpr &expr);
  llvm::Value *EmitAssign(const LoweredExpr &expr);

private:
  IRGenContext &_ctx;
  TypeConverter &_types;
};

} // namespace flux
