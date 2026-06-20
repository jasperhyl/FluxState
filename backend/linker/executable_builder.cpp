#include "executable_builder.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>

namespace flux {
namespace {

std::string SanitizeIdentifier(std::string_view name) {
  std::string result;
  result.reserve(name.size());
  for (char ch : name) {
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')) {
      result.push_back(ch);
    } else {
      result.push_back('_');
    }
  }
  if (result.empty() || (result[0] >= '0' && result[0] <= '9')) {
    result.insert(result.begin(), '_');
  }
  return result;
}

std::string EscapeCString(std::string_view value) {
  std::string escaped;
  escaped.reserve(value.size() + 8);
  for (char ch : value) {
    switch (ch) {
    case '\\':
      escaped += "\\\\";
      break;
    case '"':
      escaped += "\\\"";
      break;
    case '\n':
      escaped += "\\n";
      break;
    case '\r':
      escaped += "\\r";
      break;
    case '\t':
      escaped += "\\t";
      break;
    default:
      escaped.push_back(ch);
      break;
    }
  }
  return escaped;
}

const LoweredMachine *FindMachine(const LoweredProgram &program, const std::string &name) {
  for (const auto &machine : program.machines) {
    if (machine.name == name) {
      return &machine;
    }
  }
  return nullptr;
}

const LoweredEvent *FindEvent(const LoweredMachine &machine, const std::string &name) {
  for (const auto &event : machine.events) {
    if (event.name == name) {
      return &event;
    }
  }
  return nullptr;
}

std::optional<std::string> MaybeStripQuotes(const std::string &value) {
  if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\''))) {
    return value.substr(1, value.size() - 2);
  }
  return std::nullopt;
}

std::optional<std::string> CFieldType(const LoweredType &type) {
  switch (type.kind) {
  case LoweredTypeKind::Bool:
    return "uint8_t";
  case LoweredTypeKind::Char:
    return "uint32_t";
  case LoweredTypeKind::Short:
    return "int16_t";
  case LoweredTypeKind::Int:
    return "int32_t";
  case LoweredTypeKind::Float:
    return "float";
  case LoweredTypeKind::Double:
    return "double";
  case LoweredTypeKind::String:
    return "FS_String";
  default:
    return std::nullopt;
  }
}

bool IsIntegerText(const std::string &value) {
  if (value.empty()) {
    return false;
  }
  size_t index = 0;
  if (value[0] == '+' || value[0] == '-') {
    index = 1;
  }
  if (index == value.size()) {
    return false;
  }
  for (; index < value.size(); ++index) {
    if (value[index] < '0' || value[index] > '9') {
      return false;
    }
  }
  return true;
}

std::optional<std::string> ConvertArgToCLiteral(const LoweredType &type, const std::string &raw_value, std::string *error) {
  switch (type.kind) {
  case LoweredTypeKind::Bool:
    if (raw_value == "true" || raw_value == "1") {
      return "(uint8_t)1";
    }
    if (raw_value == "false" || raw_value == "0") {
      return "(uint8_t)0";
    }
    break;
  case LoweredTypeKind::Char: {
    if (auto stripped = MaybeStripQuotes(raw_value); stripped.has_value() && stripped->size() == 1) {
      return "(uint32_t)'" + EscapeCString(*stripped) + "'";
    }
    if (raw_value.size() == 1) {
      return "(uint32_t)'" + EscapeCString(raw_value) + "'";
    }
    if (IsIntegerText(raw_value)) {
      return "(uint32_t)(" + raw_value + ")";
    }
    break;
  }
  case LoweredTypeKind::Short:
    if (IsIntegerText(raw_value)) {
      return "(int16_t)(" + raw_value + ")";
    }
    break;
  case LoweredTypeKind::Int:
    if (IsIntegerText(raw_value)) {
      return "(int32_t)(" + raw_value + ")";
    }
    break;
  case LoweredTypeKind::Float:
    return raw_value + "f";
  case LoweredTypeKind::Double:
    return raw_value;
  case LoweredTypeKind::String: {
    const std::string content = MaybeStripQuotes(raw_value).value_or(raw_value);
    return "(FS_String){.data=(char*)\"" + EscapeCString(content) + "\", .len=" + std::to_string(content.size()) + "ull}";
  }
  default:
    break;
  }

  if (error != nullptr) {
    *error = "unsupported or invalid injection argument '" + raw_value + "'";
  }
  return std::nullopt;
}

int RunTool(const std::string &program_path, const std::vector<std::string> &args, std::string *error) {
  std::vector<llvm::StringRef> arg_refs;
  arg_refs.reserve(args.size());
  for (const auto &arg : args) {
    arg_refs.emplace_back(arg);
  }

  int exit_code = llvm::sys::ExecuteAndWait(program_path, arg_refs);
  if (exit_code != 0 && error != nullptr) {
    std::ostringstream oss;
    oss << "command failed (" << exit_code << "):";
    for (const auto &arg : args) {
      oss << ' ' << arg;
    }
    *error = oss.str();
  }
  return exit_code;
}

std::string FindCCompiler(std::string *error) {
  const char *env_candidates[] = {"CLANG_BIN", "CC_BIN", "CC"};
  for (const char *env_name : env_candidates) {
    if (const char *value = std::getenv(env_name); value != nullptr && *value != '\0') {
      return value;
    }
  }

  const char *path_candidates[] = {"clang-15", "clang"};
  for (const char *candidate : path_candidates) {
    if (auto found = llvm::sys::findProgramByName(candidate); found) {
      return *found;
    }
  }

  if (error != nullptr) {
    *error = "unable to locate clang; set CLANG_BIN or CC_BIN";
  }
  return "";
}

bool WriteFile(const std::filesystem::path &path, const std::string &contents, std::string *error) {
  std::ofstream stream(path);
  if (!stream.is_open()) {
    if (error != nullptr) {
      *error = "unable to open file '" + path.string() + "' for writing";
    }
    return false;
  }
  stream << contents;
  if (!stream.good()) {
    if (error != nullptr) {
      *error = "failed writing file '" + path.string() + "'";
    }
    return false;
  }
  return true;
}

bool BuildRunnerSource(const LoweredProgram &program, const ExecutableBuildOptions &options, std::string &output,
                       std::string *error) {
  if (program.machines.empty()) {
    if (error != nullptr) {
      *error = "lowered program has no machines";
    }
    return false;
  }

  std::ostringstream runner;
  runner << "#include \"runtime.h\"\n"
         << "#include <stdbool.h>\n"
         << "#include <stdint.h>\n"
         << "#include <stdlib.h>\n"
         << "#include <time.h>\n\n";

  runner << "extern void fs_module_init(void);\n";
  runner << "extern void fs_module_cleanup(void);\n";
  for (const auto &machine : program.machines) {
    runner << "extern bool fs_step_" << machine.name << "(void);\n";
    runner << "extern void fs_stop_" << machine.name << "(void);\n";
    runner << "extern void *fs_get_machine_" << machine.name << "(void);\n";
  }
  runner << "\n";

  runner << "static uint64_t runner_now_ms(void) {\n"
         << "  struct timespec ts;\n"
         << "  clock_gettime(CLOCK_MONOTONIC, &ts);\n"
         << "  return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;\n"
         << "}\n\n";

  runner << "static void runner_sleep_ms(uint64_t ms) {\n"
         << "  struct timespec ts;\n"
         << "  ts.tv_sec = (time_t)(ms / 1000ull);\n"
         << "  ts.tv_nsec = (long)((ms % 1000ull) * 1000000ull);\n"
         << "  nanosleep(&ts, NULL);\n"
         << "}\n\n";

  runner << "static int inject_initial_events(void) {\n";
  if (options.debug) {
    runner << "  system(\"echo inject_initial_events\");\n";
  }
  for (size_t injection_index = 0; injection_index < options.injections.size(); ++injection_index) {
    const auto &injection = options.injections[injection_index];
    const LoweredMachine *machine = FindMachine(program, injection.machine_name);
    if (machine == nullptr) {
      if (error != nullptr) {
        *error = "unknown machine in injection spec: " + injection.machine_name;
      }
      return false;
    }
    const LoweredEvent *event = FindEvent(*machine, injection.event_name);
    if (event == nullptr) {
      if (error != nullptr) {
        *error = "unknown event '" + injection.event_name + "' for machine '" + injection.machine_name + "'";
      }
      return false;
    }
    if (event->params.size() != injection.args.size()) {
      if (error != nullptr) {
        *error = "event '" + injection.machine_name + ":" + injection.event_name + "' expects " +
                 std::to_string(event->params.size()) + " args but got " + std::to_string(injection.args.size());
      }
      return false;
    }

    const std::string machine_id = SanitizeIdentifier(machine->name);
    const std::string event_id = SanitizeIdentifier(event->name);
    const std::string payload_type_name =
        "RunnerPayload_" + machine_id + "_" + event_id + "_" + std::to_string(injection_index);

    runner << "  {\n";
    runner << "    FS_MachineHeader *machine = (FS_MachineHeader *)fs_get_machine_" << machine->name << "();\n";
    runner << "    FS_Event *event = (FS_Event *)calloc(1, sizeof(FS_Event));\n";
    runner << "    if (event == NULL) {\n"
           << "      return 1;\n"
           << "    }\n";

    if (!event->params.empty()) {
      runner << "    typedef struct " << payload_type_name << " {\n";
      for (const auto &param : event->params) {
        const auto field_type = CFieldType(param.type);
        if (!field_type.has_value()) {
          if (error != nullptr) {
            *error = "unsupported payload type for injected event '" + injection.machine_name + ":" + injection.event_name + "'";
          }
          return false;
        }
        runner << "      " << *field_type << ' ' << param.name << ";\n";
      }
      runner << "    } " << payload_type_name << ";\n";
      runner << "    " << payload_type_name << " *payload = (" << payload_type_name << " *)malloc(sizeof(" << payload_type_name
             << "));\n";
      runner << "    if (payload == NULL) {\n"
             << "      free(event);\n"
             << "      return 1;\n"
             << "    }\n";
      for (size_t arg_index = 0; arg_index < event->params.size(); ++arg_index) {
        std::string literal_error;
        auto literal = ConvertArgToCLiteral(event->params[arg_index].type, injection.args[arg_index], &literal_error);
        if (!literal.has_value()) {
          if (error != nullptr) {
            *error = "invalid argument " + std::to_string(arg_index) + " for injection '" + injection.machine_name + ":" +
                     injection.event_name + "': " + literal_error;
          }
          return false;
        }
        runner << "    payload->" << event->params[arg_index].name << " = " << *literal << ";\n";
      }
      runner << "    event->flags = FS_EVENT_FREE_PAYLOAD;\n";
      runner << "    event->payload_size = (uint32_t)sizeof(" << payload_type_name << ");\n";
      runner << "    event->payload = payload;\n";
    } else {
      runner << "    event->flags = 0u;\n";
      runner << "    event->payload_size = 0u;\n";
      runner << "    event->payload = NULL;\n";
    }

    runner << "    event->tag = (uint32_t)" << event->tag << ";\n";
    runner << "    event->timestamp_ns = 0ull;\n";
    runner << "    fs_enqueue_event(machine, event);\n";
    runner << "  }\n";
  }
  runner << "  return 0;\n";
  runner << "}\n\n";

  runner << "int main(void) {\n";
  if (options.debug) {
    runner << "  system(\"echo main\");\n";
  }
  runner << "  fs_module_init();\n"
         << "  if (inject_initial_events() != 0) {\n"
         << "    fs_module_cleanup();\n"
         << "    return 1;\n"
         << "  }\n"
         << "  const uint64_t start_ms = runner_now_ms();\n"
         << "  uint64_t last_progress_ms = start_ms;\n"
         << "  for (;;) {\n"
         << "    bool progressed = false;\n";
  for (const auto &machine : program.machines) {
    runner << "    progressed = fs_step_" << machine.name << "() || progressed;\n";
  }
  runner << "    const uint64_t now_ms = runner_now_ms();\n"
         << "    if (progressed) {\n"
         << "      last_progress_ms = now_ms;\n"
         << "    } else {\n"
         << "      if (now_ms - last_progress_ms >= " << options.idle_timeout_ms << "u) {\n"
         << "        break;\n"
         << "      }\n"
         << "      runner_sleep_ms(1u);\n"
         << "    }\n"
         << "    if (now_ms - start_ms >= " << options.max_runtime_ms << "u) {\n"
         << "      break;\n"
         << "    }\n"
         << "  }\n";
  for (const auto &machine : program.machines) {
    runner << "  fs_stop_" << machine.name << "();\n";
  }
  runner << "  fs_module_cleanup();\n"
         << "  return 0;\n"
         << "}\n";

  output = runner.str();
  return true;
}

} // namespace

bool BuildExecutable(const LoweredProgram &program, const std::string &user_object_path,
                     const ExecutableBuildOptions &options, std::string *error) {
  if (options.output_path.empty()) {
    if (error != nullptr) {
      *error = "executable output path is empty";
    }
    return false;
  }
  if (options.tool_root.empty()) {
    if (error != nullptr) {
      *error = "tool root is empty";
    }
    return false;
  }

  std::string compiler_error;
  const std::string c_compiler = FindCCompiler(&compiler_error);
  if (c_compiler.empty()) {
    if (error != nullptr) {
      *error = compiler_error;
    }
    return false;
  }

  const auto output_path = std::filesystem::absolute(options.output_path);
  const auto build_dir = output_path.parent_path() / (output_path.filename().string() + ".build");
  std::error_code fs_error;
  std::filesystem::create_directories(build_dir, fs_error);
  if (fs_error) {
    if (error != nullptr) {
      *error = "cannot create build directory '" + build_dir.string() + "': " + fs_error.message();
    }
    return false;
  }

  const auto runtime_dir = std::filesystem::path(options.tool_root) / "runtime";
  const auto runtime_source = runtime_dir / "runtime.c";
  const auto runtime_header = runtime_dir / "runtime.h";
  if (!std::filesystem::exists(runtime_source) || !std::filesystem::exists(runtime_header)) {
    if (error != nullptr) {
      *error = "runtime sources not found under '" + runtime_dir.string() + "'";
    }
    return false;
  }

  std::string runner_source_contents;
  if (!BuildRunnerSource(program, options, runner_source_contents, error)) {
    return false;
  }

  const auto runner_source = build_dir / "runner.c";
  const auto runner_object = build_dir / "runner.o";
  const auto runtime_object = build_dir / "runtime.o";

  if (!WriteFile(runner_source, runner_source_contents, error)) {
    return false;
  }

  std::vector<std::string> compile_runtime_args = {c_compiler,
                                                   "-c",
                                                   runtime_source.string(),
                                                   "-std=c99",
                                                   "-D_POSIX_C_SOURCE=200809L",
                                                   "-I",
                                                   runtime_dir.string(),
                                                   "-o",
                                                   runtime_object.string()};
  if (RunTool(c_compiler, compile_runtime_args, error) != 0) {
    return false;
  }

  std::vector<std::string> compile_runner_args = {c_compiler,
                                                  "-c",
                                                  runner_source.string(),
                                                  "-std=c99",
                                                  "-D_POSIX_C_SOURCE=200809L",
                                                  "-I",
                                                  runtime_dir.string(),
                                                  "-o",
                                                  runner_object.string()};
  if (RunTool(c_compiler, compile_runner_args, error) != 0) {
    return false;
  }

  std::vector<std::string> link_args = {c_compiler,
                                        "-o",
                                        output_path.string(),
                                        runner_object.string(),
                                        runtime_object.string(),
                                        user_object_path,
                                        "-lpthread",
                                        "-lrt"};
  if (RunTool(c_compiler, link_args, error) != 0) {
    return false;
  }

  return true;
}

} // namespace flux
