#include "scope.hpp"
#include <optional>

namespace flux {

Scope::Scope(const Scope *parent) : _parent(parent) {}

bool Scope::Declare(const ResolvedSymbol &symbol) {
  auto it = _symbols.find(symbol.name);
  if (it != _symbols.end()) {
    return false;
  }
  _symbols.emplace(symbol.name, symbol);
  return true;
}

bool Scope::IsDeclLocally(const std::string &name) const { return _symbols.find(name) != _symbols.end(); }

std::optional<ResolvedSymbol> Scope::Lookup(const std::string &name) const {
  auto it = _symbols.find(name);
  if (it != _symbols.end()) {
    return it->second;
  }
  if (_parent != nullptr) {
    return _parent->Lookup(name);
  }
  return std::nullopt;
}

const Scope *Scope::Parent() const { return _parent; }

} // namespace flux
