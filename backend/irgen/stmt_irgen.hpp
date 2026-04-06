#pragma once
#include "../lowering/lowered_model.hpp"

namespace flux {

class IRGenContext;
class TypeConverter;
class ExprIRGen;

class StmtIRGen {
public:
  StmtIRGen(IRGenContext &ctx, TypeConverter &types, ExprIRGen &expr_irgen);

  void EmitStmt(const LoweredStmt &stmt);
  void EmitBlock(const LoweredStmt &stmt);

private:
  void EmitExprStmt(const LoweredStmt &stmt);
  void EmitEmitStmt(const LoweredStmt &stmt);

private:
  IRGenContext &_ctx;
  TypeConverter &_types;
  ExprIRGen &_expr_irgen;
};

} // namespace flux
