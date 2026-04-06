#pragma once
#include <string>
namespace flux {
struct Diagnostic {
  enum class Level {
    Error,
    Warning,
  };
  Level level = Level::Error;
  std::string msg;
};

} // namespace flux