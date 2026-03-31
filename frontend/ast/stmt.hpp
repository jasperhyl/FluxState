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

} // namespace flux