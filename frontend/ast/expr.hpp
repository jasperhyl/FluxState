#pragma once
#include "ast.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
namespace flux {

struct Expr {
  virtual ~Expr() = default;
};
// 字面量
struct BoolLiteralExpr final : Expr {
  bool value = false;
};
struct CharLiteralExpr final : Expr {
  uint32_t value = 0;
};
struct IntLiteralExpr final : Expr {
  int32_t value = 0;
};

struct StringLiteralExpr final : Expr {
  std::string value;
};
// 表达式中使用变量，如x+2
struct VarExpr final : Expr {
  std::string name;
};

// 一元运算符
struct UnaryExpr final : Expr {
  UnaryOp op;
  std::unique_ptr<Expr> operand;
};

// 二元运算符
struct BinaryExpr final : Expr {
  BinaryOp op;
  std::unique_ptr<Expr> lhs; // left hang side左操作数
  std::unique_ptr<Expr> rhs;
};

// 函数调用
struct CallExpr final : Expr {
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> args;
};

// 赋值语句，这里写成Expr,所以是支持链式赋值的
struct AssignStmt final : Expr {
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
};
} // namespace flux