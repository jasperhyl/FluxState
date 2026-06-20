#pragma once
#include "../lowering/lowered_model.hpp"
#include <string>
#include <vector>

namespace flux {

struct EventInjectionSpec {
  std::string machine_name;
  std::string event_name;
  std::vector<std::string> args;
};

struct ExecutableBuildOptions {
  std::string output_path;
  std::string tool_root;
  std::vector<EventInjectionSpec> injections;
  unsigned idle_timeout_ms = 100;
  unsigned max_runtime_ms = 1000;
  bool debug = false;
};

bool BuildExecutable(const LoweredProgram &program, const std::string &user_object_path,
                     const ExecutableBuildOptions &options, std::string *error = nullptr);

} // namespace flux
