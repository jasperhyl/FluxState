#pragma once
#include "ast.hpp"
#include <string>
#include <vector>
namespace flux {
struct Param {
  TypeKind type;
  std::string name;
};

// 函数声明
struct FuncDecl {
  TypeKind return_type;
  std::string name;
  std::vector<Param> params;
};

} // namespace flux