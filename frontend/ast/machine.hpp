#pragma once
#include "decl.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace flux {

// 事件声明
struct EventDecl {
  std::string name; // event的名字
  std::vector<Param> params;
};

// 定义after后面跟的时间
struct DurationLiteral {
  int64_t duration = 0;
  enum class Unit {
    MilliSeconds,
    Seconds,
    Minutes,
  };
  Unit unit = Unit::Seconds; // 如果没有给单位，默认是秒
};

struct OnEventTrigger {
  std::string event_name;
  std::vector<std::string> bindings; // 这里不是Expr,因为比如Unlock(who)的who只是string,而emit(this,
                                     // unlock(user_input));才是参数，才是Expr,因为user_input要先传递参数
};

struct AfterTrigger {
  DurationLiteral delay;
  std::optional<int64_t> retry; // 间隔多久重试
};

using TransitionTrigger = std::variant<OnEventTrigger, AfterTrigger>;

// 状态转换
struct TransitionDecl {
  // from <src> on|after <trigger> when <guard> -> <dst> do { <actions> }
  std::string src_state;
  // 不同的触发器类型
  TransitionTrigger trigger;
  // when
  std::unique_ptr<Expr> guard;
  // 目标状态
  std::string dst_state;
  // do{}
  std::unique_ptr<BlockStmt> actions;
};

// machine声明
struct MachineDecl {
  // machine名字
  std::string name;
  // machine的状态合集
  std::vector<std::string> states;
  // machine的初始状态
  std::string initial_state;
  // machine定义的events
  std::vector<EventDecl> events;
  // 所有transition语句
  std::vector<TransitionDecl> transitions;
};

struct Program {
  // 一个系统可以有多个machine
  std::vector<MachineDecl> machines;
};

} // namespace flux