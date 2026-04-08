#include "type_converter.hpp"
#include "irgen_context.hpp"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

namespace flux {

TypeConverter::TypeConverter(IRGenContext &ctx) : _ctx(ctx) {}

llvm::Type *TypeConverter::ToIRType(const LoweredType &type, bool for_memory) {
  auto &llvm_ctx = _ctx.LLVM();
  switch (type.kind) {
  case LoweredTypeKind::Void:
    return llvm::Type::getVoidTy(llvm_ctx);
  case LoweredTypeKind::Bool:
    return for_memory ? llvm::Type::getInt8Ty(llvm_ctx) : llvm::Type::getInt1Ty(llvm_ctx);
  case LoweredTypeKind::Char:
    return llvm::Type::getInt32Ty(llvm_ctx);
  case LoweredTypeKind::Short:
    return llvm::Type::getInt16Ty(llvm_ctx);
  case LoweredTypeKind::Int:
    return llvm::Type::getInt32Ty(llvm_ctx);
  case LoweredTypeKind::Float:
    return llvm::Type::getFloatTy(llvm_ctx);
  case LoweredTypeKind::Double:
    return llvm::Type::getDoubleTy(llvm_ctx);
  case LoweredTypeKind::String:
    return GetStringType();
  case LoweredTypeKind::EventRef:
  case LoweredTypeKind::MachineRef:
  case LoweredTypeKind::OpaquePtr:
    return llvm::PointerType::getUnqual(llvm_ctx);
  case LoweredTypeKind::Invalid:
  default:
    return llvm::Type::getVoidTy(llvm_ctx);
  }
}

llvm::StructType *TypeConverter::GetStringType() {
  if (_string_type != nullptr) {
    return _string_type;
  }

  _string_type = llvm::StructType::create(_ctx.LLVM(), "FS_String");
  _string_type->setBody({
      llvm::PointerType::getUnqual(_ctx.LLVM()),
      llvm::Type::getInt64Ty(_ctx.LLVM()),
  });
  return _string_type;
}

llvm::StructType *TypeConverter::GetEventType() {
  if (_event_type != nullptr) {
    return _event_type;
  }

  _event_type = llvm::StructType::create(_ctx.LLVM(), "FS_Event");
  _event_type->setBody({
      llvm::Type::getInt32Ty(_ctx.LLVM()),
      llvm::Type::getInt32Ty(_ctx.LLVM()),
      llvm::Type::getInt16Ty(_ctx.LLVM()),
      llvm::Type::getInt32Ty(_ctx.LLVM()),
      llvm::PointerType::getUnqual(_ctx.LLVM()),
      llvm::Type::getInt64Ty(_ctx.LLVM()),
  });
  return _event_type;
}

llvm::StructType *TypeConverter::GetMachineHeaderType() {
  if (_machine_header_type != nullptr) {
    return _machine_header_type;
  }

  _machine_header_type = llvm::StructType::create(_ctx.LLVM(), "FS_MachineHeader");
  _machine_header_type->setBody({
      llvm::Type::getInt32Ty(_ctx.LLVM()), // i32当前状态current_state
      llvm::Type::getInt32Ty(_ctx.LLVM()), // i32运行时标志位，FS_MACHINE_INITIALIZED或FS_MACHINE_RUNNING
      llvm::Type::getInt64Ty(_ctx.LLVM()), // i64entered_at_ns 进入当前状态的时间戳，单位纳秒。状态发生变化时会更新。
      // 这是ptr类型,queue 事件队列指针，类型是 FS_EventQueue *。machine 收到的事件会先进入这里。
      llvm::PointerType::getUnqual(_ctx.LLVM()),
      // timers 定时器表指针，类型是 FS_TimerTable *。after 之类的超时触发器靠它管理。
      llvm::PointerType::getUnqual(_ctx.LLVM()),
      // dispatch 调度函数指针，类型是 FS_DispatchFn。runtime每取到一个事件，就调用这个函数把事件交给该 machine 的
      // dispatch 逻辑处理。
      llvm::PointerType::getUnqual(_ctx.LLVM()),

  });
  return _machine_header_type;
}

llvm::StructType *TypeConverter::CreatePayloadType(const std::string &name, const LoweredEvent &event) {
  auto *payload_type = llvm::StructType::create(_ctx.LLVM(), name);
  std::vector<llvm::Type *> field_types;
  field_types.reserve(event.params.size());
  for (const auto &param : event.params) {
    field_types.push_back(ToIRType(param.type, true));
  }
  payload_type->setBody(field_types);
  return payload_type;
}

} // namespace flux
