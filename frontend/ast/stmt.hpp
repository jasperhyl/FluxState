#pragma once
#include "expr.hpp"
#include <memory>
#include <vector>
namespace flux {
struct Stmt {
  virtual ~Stmt() = default;
};

// 有些表达式的返回值我们并不在意，所以把它当作语句执行
struct ExprStmt final : Stmt {
  std::unique_ptr<Expr> expr;
};

// stmts
struct BlockStmt final : Stmt {
  std::vector<std::unique_ptr<Stmt>> stmts;
};

// 事件调用(这是实际调用，不是声明，比如unlock(403))
// 先只支持向一个target发送一个事件
struct EventInvoke {
  std::string event_name;
  std::vector<std::unique_ptr<Expr>> args;
};
// 事件emit
struct EmitStmt final : Stmt {
  std::unique_ptr<Expr> target; // TODO:这里要不要改成string?
  EventInvoke invoke;
};

} // namespace flux