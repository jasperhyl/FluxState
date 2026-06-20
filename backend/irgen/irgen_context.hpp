#pragma once
#include "../lowering/lowered_model.hpp"
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
class Function;       // llvm ir的函数对象
class Value;          // 基础抽象，几乎能当值用的东西都是他的子类
class StructType;     // 结构体类型描述，告诉如何解释这一段struct内存
class GlobalVariable; // 模块级全局变量，生命周期跟整个module一样长，每个 machine 实例就是放在一个 GlobalVariable里
} // namespace llvm

namespace flux {

// 保存“运行时函数声明句柄”的结构体：
struct RuntimeDecls {
  llvm::Function *enqueue_event = nullptr;   // 入队事件
  llvm::Function *start_timer = nullptr;     // 启动计时器
  llvm::Function *cancel_timer = nullptr;    // 取消计时器
  llvm::Function *init_machine = nullptr;    // 初始化machine
  llvm::Function *cleanup_machine = nullptr; // 清理machine
  llvm::Function *runtime_step = nullptr;    // 驱动runtime执行一步
  llvm::Function *runtime_run = nullptr;     // 驱动runtime持续运行
  llvm::Function *stop_machine = nullptr;    // 停止machine
  llvm::Function *malloc_fn = nullptr;       // 申请内存
  llvm::Function *puts_fn = nullptr;         // 调试输出
};

/*
假设某个 LoweredMachine 大概是这样：
machine.name = "counter"

machine.env.fields = [
  { name: "count", type: int },
  { name: "enabled", type: bool }
]

machine.events = [
  { tag: 1, name: "inc", params: [int delta] },
  { tag: 2, name: "reset", params: [] }
]

那 MachineIRLayout 大概会变成：
machine -> 指向这个 counter machine
header_type -> 统一的 MachineHeader
env_type -> 类似 struct counter.env { i32 count; i1/i8 enabled; }
machine_type -> 类似 struct counter.machine { MachineHeader hdr; counter.env env; }
env_field_indices -> { "count": 0, "enabled": 1 }
payload_types_by_event_tag -> { 1: struct counter.event.inc.payload { i32 delta } }
events_by_tag -> { 1: &inc, 2: &reset }
*/
struct MachineIRLayout {
  const LoweredMachine *machine = nullptr; // 记录来源
  llvm::StructType *header_type = nullptr; // 公共 runtime header 的 LLVM 结构体类型，也就是 FS_MachineHeader
  llvm::StructType *env_type = nullptr;    // 当前 machine 的 env 区域结构体类型
  llvm::StructType *machine_type = nullptr; // 定义成 {header_type, env_type}，也就是一个完整 machine 的内存布局由“公共
                                            // header + 该 machine 的 env”组成。
  // 例如如果 env.fields 里有 count, flag，那这里可能就是： {"count" -> 0, "flag" -> 1}
  // 后面生成取字段地址时，就能按名字找到 struct 第几个成员。
  std::unordered_map<std::string, size_t> env_field_indices; // env 字段名到字段下标的映射
  // 事件 tag 到该事件 payload 结构体类型的映射，key 是 event.tag
  // 如果某个事件带参数，就会为它生成一个 LLVM StructType，表示这个事件 payload 在内存里的布局
  std::unordered_map<int32_t, llvm::StructType *> payload_types_by_event_tag;
  // 事件 tag 到原始 LoweredEvent 的映射
  std::unordered_map<int32_t, const LoweredEvent *> events_by_tag;
};

struct MachineInstanceIRInfo {
  MachineIRLayout layout;                 // 存放type
  llvm::GlobalVariable *global = nullptr; // 单个machine的全局变量,存真实数据
};

class IRGenContext {
public:
  explicit IRGenContext(const std::string &module_name);
  ~IRGenContext();

  llvm::LLVMContext &LLVM();    // 返回LLVMContext
  llvm::Module &Module();       // 返回Module
  llvm::IRBuilder<> &Builder(); // 返回IRBuilder

  void Error(std::string message);
  const std::vector<std::string> &Diagnostics() const;
  std::vector<std::string> TakeDiagnostics();

  // 把已经准备好的 runtime 函数表存进IRGenContext里_runtime = runtime_decls
  void SetRuntimeDecls(RuntimeDecls runtime_decls);
  RuntimeDecls &Runtime();
  const RuntimeDecls &Runtime() const;

  void SetDebug(bool d) { _debug = d; }
  bool Debug() const { return _debug; }

  void SetCurrentFunction(llvm::Function *function);
  llvm::Function *CurrentFunction() const;

  void SetCurrentMachineValue(llvm::Value *value);
  llvm::Value *CurrentMachineValue() const;

  void SetCurrentEventValue(llvm::Value *value);
  llvm::Value *CurrentEventValue() const;

  void SetCurrentMachineLayout(MachineIRLayout layout);
  const MachineIRLayout *CurrentMachineLayout() const;

  void RegisterMachineInstance(std::string name, MachineInstanceIRInfo info);
  const MachineInstanceIRInfo *FindMachineInstance(const std::string &name) const;

  void PushValueScope();
  void PopValueScope();
  void BindValue(const std::string &name, llvm::Value *value);
  llvm::Value *LookupValue(const std::string &name) const;

  std::unique_ptr<llvm::LLVMContext> TakeLLVMContext();
  std::unique_ptr<llvm::Module> TakeModule();

private:
  // 因为 IRGen::Generate(...) 里只创建了一个 IRGenContext，所以以下三个是唯一的
  std::unique_ptr<llvm::LLVMContext> _llvm_context; // 全局唯一
  std::unique_ptr<llvm::Module> _module;            // 全局唯一
  std::unique_ptr<llvm::IRBuilder<>> _builder;      // 全局唯一的
  std::vector<std::string> _diagnostics;
  RuntimeDecls _runtime;
  bool _debug = false;
  llvm::Function *_current_function = nullptr;
  llvm::Value *_current_machine_value = nullptr; // 通常是一个 machine 指针
  llvm::Value *_current_event_value = nullptr;   // 通常是一个 event 指针
  std::optional<MachineIRLayout> _current_machine_layout;
  std::unordered_map<std::string, MachineInstanceIRInfo> _machine_instances;
  std::vector<std::unordered_map<std::string, llvm::Value *>> _value_scopes;
};

} // namespace flux
