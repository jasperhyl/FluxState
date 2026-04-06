#include "stmt_irgen.hpp"
#include "expr_irgen.hpp"
#include "irgen_context.hpp"
#include "type_converter.hpp"
#include "value_utils.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>

namespace flux {
namespace {

llvm::Value *GetMachineHeaderPtr(IRGenContext &ctx, llvm::Value *machine_value) {
  const auto *layout = ctx.CurrentMachineLayout();
  if (layout == nullptr || machine_value == nullptr) {
    return nullptr;
  }
  return ctx.Builder().CreateStructGEP(layout->machine_type, machine_value, 0, "machine.hdr");
}

} // namespace

StmtIRGen::StmtIRGen(IRGenContext &ctx, TypeConverter &types, ExprIRGen &expr_irgen)
    : _ctx(ctx), _types(types), _expr_irgen(expr_irgen) {}

void StmtIRGen::EmitStmt(const LoweredStmt &stmt) {
  switch (stmt.kind) {
  case LoweredStmtKind::Expr:
    EmitExprStmt(stmt);
    return;
  case LoweredStmtKind::Block:
    EmitBlock(stmt);
    return;
  case LoweredStmtKind::Emit:
    EmitEmitStmt(stmt);
    return;
  }
}

void StmtIRGen::EmitBlock(const LoweredStmt &stmt) {
  _ctx.PushValueScope();
  for (const auto &child : stmt.stmts) {
    if (child) {
      EmitStmt(*child);
    }
  }
  _ctx.PopValueScope();
}

void StmtIRGen::EmitExprStmt(const LoweredStmt &stmt) {
  if (stmt.expr) {
    _expr_irgen.EmitExpr(*stmt.expr);
  }
}

void StmtIRGen::EmitEmitStmt(const LoweredStmt &stmt) {
  const auto *layout = _ctx.CurrentMachineLayout();
  if (layout == nullptr) {
    _ctx.Error("emit lowering requires an active machine layout");
    return;
  }

  llvm::Value *target_machine = _ctx.CurrentMachineValue();
  llvm::Value *target_header = GetMachineHeaderPtr(_ctx, target_machine);
  if (stmt.emit_target && stmt.emit_target->type.kind == LoweredTypeKind::MachineRef) {
    target_header = _expr_irgen.EmitExpr(*stmt.emit_target);
  }
  if (target_header == nullptr) {
    _ctx.Error("unable to determine target machine header for emit");
    return;
  }

  llvm::Value *payload_ptr = llvm::ConstantPointerNull::get(llvm::PointerType::get(_ctx.LLVM(), 0));
  llvm::Value *payload_size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), 0);

  const MachineIRLayout *target_layout = layout;
  if (!stmt.emit_target_machine_name.empty()) {
    const auto *target_instance = _ctx.FindMachineInstance(stmt.emit_target_machine_name);
    if (target_instance == nullptr) {
      _ctx.Error("missing IR layout for target machine '" + stmt.emit_target_machine_name + "'");
      return;
    }
    target_layout = &target_instance->layout;
  }

  auto payload_it = target_layout->payload_types_by_event_tag.find(stmt.emit_event_tag);
  if (payload_it != target_layout->payload_types_by_event_tag.end() && !stmt.emit_args.empty()) {
    llvm::StructType *payload_type = payload_it->second;
    llvm::Value *raw_payload = _ctx.Builder().CreateCall(_ctx.Runtime().malloc_fn, {llvm::ConstantExpr::getSizeOf(payload_type)});
    llvm::Value *typed_payload =
        _ctx.Builder().CreateBitCast(raw_payload, llvm::PointerType::get(_ctx.LLVM(), 0), "emit.payload");

    for (size_t i = 0; i < stmt.emit_args.size(); ++i) {
      llvm::Value *arg_value = _expr_irgen.EmitExpr(*stmt.emit_args[i]);
      llvm::Value *field_ptr = _ctx.Builder().CreateStructGEP(payload_type, typed_payload, i);
      LoweredType field_type{};
      auto event_it = target_layout->events_by_tag.find(stmt.emit_event_tag);
      if (event_it != target_layout->events_by_tag.end() && event_it->second != nullptr && i < event_it->second->params.size()) {
        const auto *event = event_it->second;
        field_type = event->params[i].type;
      } else if (i < stmt.emit_args.size()) {
        field_type = stmt.emit_args[i]->type;
      }
      _ctx.Builder().CreateStore(AdjustForStore(_ctx, _types, field_type, stmt.emit_args[i]->type, arg_value), field_ptr);
    }

    payload_ptr = _ctx.Builder().CreateBitCast(typed_payload, llvm::PointerType::get(_ctx.LLVM(), 0));
    payload_size = llvm::ConstantExpr::getSizeOf(payload_type);
    if (payload_size->getType()->isIntegerTy(64)) {
      payload_size = _ctx.Builder().CreateTrunc(payload_size, llvm::Type::getInt32Ty(_ctx.LLVM()));
    }
  }

  llvm::StructType *event_type = _types.GetEventType();
  llvm::Value *raw_event = _ctx.Builder().CreateCall(_ctx.Runtime().malloc_fn, {llvm::ConstantExpr::getSizeOf(event_type)});
  llvm::Value *event_ptr =
      _ctx.Builder().CreateBitCast(raw_event, llvm::PointerType::get(_ctx.LLVM(), 0), "emit.event");

  _ctx.Builder().CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), stmt.emit_event_tag),
                             _ctx.Builder().CreateStructGEP(event_type, event_ptr, 0));
  _ctx.Builder().CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), 0),
                             _ctx.Builder().CreateStructGEP(event_type, event_ptr, 1));
  _ctx.Builder().CreateStore(llvm::ConstantInt::get(llvm::Type::getInt16Ty(_ctx.LLVM()), 0),
                             _ctx.Builder().CreateStructGEP(event_type, event_ptr, 2));
  _ctx.Builder().CreateStore(payload_size, _ctx.Builder().CreateStructGEP(event_type, event_ptr, 3));
  _ctx.Builder().CreateStore(payload_ptr, _ctx.Builder().CreateStructGEP(event_type, event_ptr, 4));
  _ctx.Builder().CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(_ctx.LLVM()), 0),
                             _ctx.Builder().CreateStructGEP(event_type, event_ptr, 5));

  _ctx.Builder().CreateCall(_ctx.Runtime().enqueue_event, {target_header, event_ptr});
}

} // namespace flux
