#pragma once
#include "semantic_model.hpp"
#include <optional>
#include <string>
#include <unordered_map>
namespace flux {
// 维护当前作用域和父作用域之间的名字查找
class Scope {
public:
  explicit Scope(const Scope *parent = nullptr);
  // 检查symbol是否已经声明，声明了就报错“重复声明”，否则写入
  bool Declare(const ResolvedSymbol &symbol);
  // 只在局部作用域检查symbol
  bool IsDeclLocally(const std::string &name) const;
  // 查找symbol的类型，用于检查赋值，比较等是否合法
  std::optional<ResolvedSymbol> Lookup(const std::string &name) const;

  // 父作用域
  const Scope *Parent() const;

private:
  std::unordered_map<std::string, ResolvedSymbol> _symbols;
  const Scope *_parent = nullptr;
};
} // namespace flux