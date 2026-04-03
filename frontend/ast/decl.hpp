#pragma once
#include "ast.hpp"
#include <string>
#include <vector>
namespace flux {
struct Param {
  ValueType type;
  std::string name;
};

// 函数声明
struct FuncDecl {
  ValueType return_type;
  std::string name;
  std::vector<Param> params;
};

} // namespace flux