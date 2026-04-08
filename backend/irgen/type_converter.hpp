#pragma once
#include "../lowering/lowered_model.hpp"
#include <string>

namespace llvm {
class Type;
class StructType;
} // namespace llvm

namespace flux {

class IRGenContext;

class TypeConverter {
public:
  explicit TypeConverter(IRGenContext &ctx);

  llvm::Type *ToIRType(const LoweredType &type, bool for_memory = false);
  llvm::StructType *GetStringType();
  llvm::StructType *GetEventType();
  llvm::StructType *GetMachineHeaderType();
  llvm::StructType *CreatePayloadType(const std::string &name, const LoweredEvent &event);

private:
  IRGenContext &_ctx;
  llvm::StructType *_string_type = nullptr;
  llvm::StructType *_event_type = nullptr;
  llvm::StructType *_machine_header_type = nullptr; // 这个是动态构造出来的
};

} // namespace flux
