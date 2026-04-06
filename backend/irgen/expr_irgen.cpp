#include "expr_irgen.hpp"
#include "irgen_context.hpp"
#include "type_converter.hpp"
#include "value_utils.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace flux {
namespace {

llvm::Value *LoadForValue(IRGenContext &ctx, TypeConverter &types, const LoweredType &type, llvm::Value *address) {
  llvm::Value *loaded = ctx.Builder().CreateLoad(types.ToIRType(type, true), address);
  if (type.kind == LoweredTypeKind::Bool && loaded->getType()->isIntegerTy(8)) {
    return ctx.Builder().CreateTrunc(loaded, types.ToIRType(type, false));
  }
  return loaded;
}

llvm::Value *GetCurrentMachineHeaderPtr(IRGenContext &ctx) {
  const auto *layout = ctx.CurrentMachineLayout();
  llvm::Value *machine_value = ctx.CurrentMachineValue();
  if (layout == nullptr || machine_value == nullptr) {
    return nullptr;
  }
  return ctx.Builder().CreateStructGEP(layout->machine_type, machine_value, 0, "machine.hdr");
}

} // namespace

ExprIRGen::ExprIRGen(IRGenContext &ctx, TypeConverter &types) : _ctx(ctx), _types(types) {}

llvm::Value *ExprIRGen::EmitExpr(const LoweredExpr &expr) {
  switch (expr.kind) {
  case LoweredExprKind::BoolLiteral:
    return EmitBoolLiteral(expr);
  case LoweredExprKind::CharLiteral:
    return EmitCharLiteral(expr);
  case LoweredExprKind::IntLiteral:
    return EmitIntLiteral(expr);
  case LoweredExprKind::StringLiteral:
    return EmitStringLiteral(expr);
  case LoweredExprKind::Var:
    return EmitVar(expr);
  case LoweredExprKind::Unary:
    return EmitUnary(expr);
  case LoweredExprKind::Binary:
    return EmitBinary(expr);
  case LoweredExprKind::Call:
    return EmitCall(expr);
  case LoweredExprKind::Assign:
    return EmitAssign(expr);
  }

  _ctx.Error("unknown lowered expression kind during IR generation");
  return llvm::UndefValue::get(_types.ToIRType(expr.type));
}

llvm::Value *ExprIRGen::EmitAddress(const LoweredExpr &expr) {
  if (expr.kind != LoweredExprKind::Var) {
    _ctx.Error("only variable expressions can be lowered as addresses");
    return nullptr;
  }

  if (expr.symbol.storage != LoweredStorageKind::EnvField) {
    _ctx.Error("only machine environment variables have addressable storage");
    return nullptr;
  }

  const auto *layout = _ctx.CurrentMachineLayout();
  llvm::Value *machine_value = _ctx.CurrentMachineValue();
  if (layout == nullptr || machine_value == nullptr) {
    _ctx.Error("missing current machine layout while lowering variable address");
    return nullptr;
  }

  llvm::Value *env_ptr = _ctx.Builder().CreateStructGEP(layout->machine_type, machine_value, 1, expr.symbol.name + ".env");
  return _ctx.Builder().CreateStructGEP(layout->env_type, env_ptr, expr.symbol.index, expr.symbol.name + ".addr");
}

llvm::Value *ExprIRGen::EmitBoolLiteral(const LoweredExpr &expr) {
  return llvm::ConstantInt::get(_types.ToIRType(expr.type, false), expr.bool_value ? 1 : 0);
}

llvm::Value *ExprIRGen::EmitCharLiteral(const LoweredExpr &expr) {
  return llvm::ConstantInt::get(_types.ToIRType(expr.type, false), expr.char_value);
}

llvm::Value *ExprIRGen::EmitIntLiteral(const LoweredExpr &expr) {
  return llvm::ConstantInt::get(_types.ToIRType(expr.type, false), expr.int_value, true);
}

llvm::Value *ExprIRGen::EmitStringLiteral(const LoweredExpr &expr) {
  auto &llvm_ctx = _ctx.LLVM();
  auto &module = _ctx.Module();
  auto *bytes = llvm::ConstantDataArray::getString(llvm_ctx, expr.string_value, false);
  auto *global = new llvm::GlobalVariable(module, bytes->getType(), true, llvm::GlobalValue::PrivateLinkage, bytes,
                                          "fs.str");
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm_ctx), 0);
  llvm::Value *data_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(bytes->getType(), global, {zero, zero});
  auto *len = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm_ctx), expr.string_value.size());
  return llvm::ConstantStruct::get(_types.GetStringType(), {llvm::cast<llvm::Constant>(data_ptr), len});
}

llvm::Value *ExprIRGen::EmitVar(const LoweredExpr &expr) {
  if (expr.symbol.storage == LoweredStorageKind::MachineSymbol) {
    if (expr.symbol.name.empty()) {
      _ctx.Error("machine symbol is missing a name");
      return llvm::UndefValue::get(_types.ToIRType(expr.type));
    }
    if (const auto *layout = _ctx.CurrentMachineLayout();
        layout != nullptr && layout->machine != nullptr && layout->machine->name == expr.symbol.name) {
      if (llvm::Value *header = GetCurrentMachineHeaderPtr(_ctx)) {
        return header;
      }
      _ctx.Error("missing current machine header for machine symbol '" + expr.symbol.name + "'");
      return llvm::UndefValue::get(_types.ToIRType(expr.type));
    }

    const auto *instance = _ctx.FindMachineInstance(expr.symbol.name);
    if (instance == nullptr || instance->global == nullptr) {
      _ctx.Error("missing target machine instance for '" + expr.symbol.name + "'");
      return llvm::UndefValue::get(_types.ToIRType(expr.type));
    }
    return _ctx.Builder().CreateStructGEP(instance->layout.machine_type, instance->global, 0,
                                          expr.symbol.name + ".hdr");
  }

  if (expr.symbol.storage == LoweredStorageKind::TriggerValue) {
    if (llvm::Value *bound = _ctx.LookupValue(expr.symbol.name)) {
      return bound;
    }
    _ctx.Error("missing trigger binding IR value for '" + expr.symbol.name + "'");
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }

  llvm::Value *address = EmitAddress(expr);
  if (address == nullptr) {
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }
  return LoadForValue(_ctx, _types, expr.type, address);
}

llvm::Value *ExprIRGen::EmitUnary(const LoweredExpr &expr) {
  if (expr.operand == nullptr) {
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }

  llvm::Value *operand = EmitExpr(*expr.operand);
  switch (expr.unary_op) {
  case UnaryOp::Plus:
    return operand;
  case UnaryOp::Neg:
    return expr.type.kind == LoweredTypeKind::Float || expr.type.kind == LoweredTypeKind::Double
               ? _ctx.Builder().CreateFNeg(operand)
               : _ctx.Builder().CreateNeg(operand);
  case UnaryOp::AddressOf:
  case UnaryOp::Deref:
    _ctx.Error("pointer unary operators are not supported in IR generation yet");
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }

  return llvm::UndefValue::get(_types.ToIRType(expr.type));
}

llvm::Value *ExprIRGen::EmitBinary(const LoweredExpr &expr) {
  if (expr.lhs == nullptr || expr.rhs == nullptr) {
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }

  llvm::Value *lhs = EmitExpr(*expr.lhs);
  llvm::Value *rhs = EmitExpr(*expr.rhs);

  switch (expr.binary_op) {
  case BinaryOp::LogicAnd:
    return _ctx.Builder().CreateAnd(lhs, rhs);
  case BinaryOp::LogicOr:
    return _ctx.Builder().CreateOr(lhs, rhs);
  case BinaryOp::Add:
  case BinaryOp::Sub:
  case BinaryOp::Mul:
  case BinaryOp::Div: {
    LoweredType common = CommonNumericType(expr.lhs->type, expr.rhs->type);
    lhs = ConvertValue(_ctx, _types, lhs, expr.lhs->type, common);
    rhs = ConvertValue(_ctx, _types, rhs, expr.rhs->type, common);
    const bool floating = common.kind == LoweredTypeKind::Float || common.kind == LoweredTypeKind::Double;
    switch (expr.binary_op) {
    case BinaryOp::Add:
      return floating ? _ctx.Builder().CreateFAdd(lhs, rhs) : _ctx.Builder().CreateAdd(lhs, rhs);
    case BinaryOp::Sub:
      return floating ? _ctx.Builder().CreateFSub(lhs, rhs) : _ctx.Builder().CreateSub(lhs, rhs);
    case BinaryOp::Mul:
      return floating ? _ctx.Builder().CreateFMul(lhs, rhs) : _ctx.Builder().CreateMul(lhs, rhs);
    case BinaryOp::Div:
      return floating ? _ctx.Builder().CreateFDiv(lhs, rhs) : _ctx.Builder().CreateSDiv(lhs, rhs);
    default:
      break;
    }
    break;
  }
  case BinaryOp::Eq:
    if (expr.lhs->type.kind == LoweredTypeKind::String || expr.rhs->type.kind == LoweredTypeKind::String) {
      _ctx.Error("string equality is not supported in IR generation yet");
      return llvm::ConstantInt::getFalse(_ctx.LLVM());
    }
    if (IsNumericKind(expr.lhs->type.kind) && IsNumericKind(expr.rhs->type.kind)) {
      LoweredType common = CommonNumericType(expr.lhs->type, expr.rhs->type);
      lhs = ConvertValue(_ctx, _types, lhs, expr.lhs->type, common);
      rhs = ConvertValue(_ctx, _types, rhs, expr.rhs->type, common);
      return (common.kind == LoweredTypeKind::Float || common.kind == LoweredTypeKind::Double)
                 ? _ctx.Builder().CreateFCmpOEQ(lhs, rhs)
                 : _ctx.Builder().CreateICmpEQ(lhs, rhs);
    }
    return _ctx.Builder().CreateICmpEQ(lhs, rhs);
  case BinaryOp::Ne:
    if (expr.lhs->type.kind == LoweredTypeKind::String || expr.rhs->type.kind == LoweredTypeKind::String) {
      _ctx.Error("string inequality is not supported in IR generation yet");
      return llvm::ConstantInt::getFalse(_ctx.LLVM());
    }
    if (IsNumericKind(expr.lhs->type.kind) && IsNumericKind(expr.rhs->type.kind)) {
      LoweredType common = CommonNumericType(expr.lhs->type, expr.rhs->type);
      lhs = ConvertValue(_ctx, _types, lhs, expr.lhs->type, common);
      rhs = ConvertValue(_ctx, _types, rhs, expr.rhs->type, common);
      return (common.kind == LoweredTypeKind::Float || common.kind == LoweredTypeKind::Double)
                 ? _ctx.Builder().CreateFCmpONE(lhs, rhs)
                 : _ctx.Builder().CreateICmpNE(lhs, rhs);
    }
    return _ctx.Builder().CreateICmpNE(lhs, rhs);
  case BinaryOp::Lt:
  case BinaryOp::Le:
  case BinaryOp::Gt:
  case BinaryOp::Ge:
  {
    LoweredType common = CommonNumericType(expr.lhs->type, expr.rhs->type);
    lhs = ConvertValue(_ctx, _types, lhs, expr.lhs->type, common);
    rhs = ConvertValue(_ctx, _types, rhs, expr.rhs->type, common);
    const bool floating = common.kind == LoweredTypeKind::Float || common.kind == LoweredTypeKind::Double;
    switch (expr.binary_op) {
    case BinaryOp::Lt:
      return floating ? _ctx.Builder().CreateFCmpOLT(lhs, rhs) : _ctx.Builder().CreateICmpSLT(lhs, rhs);
    case BinaryOp::Le:
      return floating ? _ctx.Builder().CreateFCmpOLE(lhs, rhs) : _ctx.Builder().CreateICmpSLE(lhs, rhs);
    case BinaryOp::Gt:
      return floating ? _ctx.Builder().CreateFCmpOGT(lhs, rhs) : _ctx.Builder().CreateICmpSGT(lhs, rhs);
    case BinaryOp::Ge:
      return floating ? _ctx.Builder().CreateFCmpOGE(lhs, rhs) : _ctx.Builder().CreateICmpSGE(lhs, rhs);
    default:
      break;
    }
    break;
  }
  case BinaryOp::Assign:
    return EmitAssign(expr);
  }

  return llvm::UndefValue::get(_types.ToIRType(expr.type));
}

llvm::Value *ExprIRGen::EmitCall(const LoweredExpr &expr) {
  (void)expr;
  _ctx.Error("call expressions are not supported in IR generation yet");
  return llvm::UndefValue::get(_types.ToIRType(expr.type));
}

llvm::Value *ExprIRGen::EmitAssign(const LoweredExpr &expr) {
  if (expr.lhs == nullptr || expr.rhs == nullptr) {
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }

  llvm::Value *address = EmitAddress(*expr.lhs);
  llvm::Value *rhs = EmitExpr(*expr.rhs);
  if (address == nullptr || rhs == nullptr) {
    return llvm::UndefValue::get(_types.ToIRType(expr.type));
  }

  llvm::Value *stored = AdjustForStore(_ctx, _types, expr.lhs->type, expr.rhs->type, rhs);
  _ctx.Builder().CreateStore(stored, address);
  return ConvertValue(_ctx, _types, stored, expr.lhs->type, expr.lhs->type);
}

} // namespace flux
