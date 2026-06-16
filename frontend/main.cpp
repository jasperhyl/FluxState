// main.cpp
#include "../backend/irgen/irgen.hpp"
#include "../backend/lowering/lowering.hpp"
#include "../backend/codegen/llvm_codegen.hpp"
#include "../backend/linker/executable_builder.hpp"
#include "../backend/optimizer/llvm_optimizer.hpp"
#include "../backend/visualizer/visualizer.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "sema/diagnostics.hpp"
#include "sema/sema.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace flux;

int main(int argc, char *argv[]);

namespace {

std::string ReadSourceFile(const char *path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("cannot open file: " + std::string(path));
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void PrintSemaDiagnostics(const std::vector<Diagnostic> &diagnostics) {
  for (const auto &diagnostic : diagnostics) {
    const char *level = diagnostic.level == Diagnostic::Level::Error ? "error" : "warning";
    std::cerr << "sema " << level << ": " << diagnostic.msg << '\n';
  }
}

void PrintStringDiagnostics(const char *stage, const std::vector<std::string> &diagnostics) {
  for (const auto &diagnostic : diagnostics) {
    std::cerr << stage << " error: " << diagnostic << '\n';
  }
}

bool VerifyModuleOrPrint(llvm::Module &module, const char *stage) {
  std::string verify_error;
  llvm::raw_string_ostream verify_stream(verify_error);
  if (!llvm::verifyModule(module, &verify_stream)) {
    return true;
  }

  verify_stream.flush();
  std::cerr << "llvm verify error (" << stage << "):\n" << verify_error;
  return false;
}

bool ParseUnsignedOption(const std::string &value, unsigned &output) {
  try {
    size_t pos = 0;
    unsigned long parsed = std::stoul(value, &pos, 10);
    if (pos != value.size()) {
      return false;
    }
    output = static_cast<unsigned>(parsed);
    return true;
  } catch (...) {
    return false;
  }
}

std::filesystem::path ResolveToolRoot(const char *argv0) {
  const std::string executable_path = llvm::sys::fs::getMainExecutable(argv0, reinterpret_cast<void *>(&main));
  return std::filesystem::path(executable_path).parent_path();
}

} // namespace

enum class OutputMode { IR, Dot, Mermaid, Object, Executable };

int main(int argc, char *argv[]) {
  OutputMode output_mode = OutputMode::IR;
  flux::OptimizationLevel optimization_level = flux::OptimizationLevel::O2;
  std::string object_output_path;
  ExecutableBuildOptions executable_options;

  // 简单命令行解析:
  //   ./fluxstate_ir [--dot|--mermaid] [--O0|--O2] <source_file.fs>
  //   ./fluxstate_ir --emit-obj <output.o> [--O0|--O2] <source_file.fs>
  //   ./fluxstate_ir --emit-exe <output> [--inject Machine:Event[:args]] [--idle-timeout-ms N] [--max-runtime-ms N]
  //                  [--O0|--O2] <source_file.fs>
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " [--dot|--mermaid] [--O0|--O2] <source_file.fs>\n"
              << "   or: " << argv[0] << " --emit-obj <output.o> [--O0|--O2] <source_file.fs>\n"
              << "   or: " << argv[0]
              << " --emit-exe <output> [--inject Machine:Event[:args]] [--idle-timeout-ms N] [--max-runtime-ms N]"
                 " [--O0|--O2] <source_file.fs>\n";
    return 1;
  }

  executable_options.idle_timeout_ms = 100;
  executable_options.max_runtime_ms = 1000;

  const char *source_path = nullptr;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--dot") {
      if (output_mode != OutputMode::IR) {
        std::cerr << "output mode error: only one of --dot, --mermaid, --emit-obj, or --emit-exe may be specified\n";
        return 1;
      }
      output_mode = OutputMode::Dot;
    } else if (arg == "--mermaid") {
      if (output_mode != OutputMode::IR) {
        std::cerr << "output mode error: only one of --dot, --mermaid, --emit-obj, or --emit-exe may be specified\n";
        return 1;
      }
      output_mode = OutputMode::Mermaid;
    } else if (arg == "--emit-obj") {
      if (output_mode != OutputMode::IR) {
        std::cerr << "output mode error: only one of --dot, --mermaid, --emit-obj, or --emit-exe may be specified\n";
        return 1;
      }
      if (i + 1 >= argc) {
        std::cerr << "argument error: --emit-obj requires an output path\n";
        return 1;
      }
      output_mode = OutputMode::Object;
      object_output_path = argv[++i];
    } else if (arg == "--emit-exe") {
      if (output_mode != OutputMode::IR) {
        std::cerr << "output mode error: only one of --dot, --mermaid, --emit-obj, or --emit-exe may be specified\n";
        return 1;
      }
      if (i + 1 >= argc) {
        std::cerr << "argument error: --emit-exe requires an output path\n";
        return 1;
      }
      output_mode = OutputMode::Executable;
      executable_options.output_path = argv[++i];
    } else if (arg == "--inject") {
      if (i + 1 >= argc) {
        std::cerr << "argument error: --inject requires a value like Machine:Event[:args]\n";
        return 1;
      }
      std::string spec = argv[++i];
      const size_t first_colon = spec.find(':');
      if (first_colon == std::string::npos) {
        std::cerr << "argument error: invalid inject spec '" << spec << "'\n";
        return 1;
      }
      const size_t second_colon = spec.find(':', first_colon + 1);
      EventInjectionSpec injection;
      injection.machine_name = spec.substr(0, first_colon);
      if (second_colon == std::string::npos) {
        injection.event_name = spec.substr(first_colon + 1);
      } else {
        injection.event_name = spec.substr(first_colon + 1, second_colon - first_colon - 1);
        std::stringstream arg_stream(spec.substr(second_colon + 1));
        std::string item;
        while (std::getline(arg_stream, item, ',')) {
          injection.args.push_back(item);
        }
      }
      executable_options.injections.push_back(std::move(injection));
    } else if (arg == "--idle-timeout-ms") {
      if (i + 1 >= argc || !ParseUnsignedOption(argv[i + 1], executable_options.idle_timeout_ms)) {
        std::cerr << "argument error: --idle-timeout-ms requires an unsigned integer\n";
        return 1;
      }
      ++i;
    } else if (arg == "--max-runtime-ms") {
      if (i + 1 >= argc || !ParseUnsignedOption(argv[i + 1], executable_options.max_runtime_ms)) {
        std::cerr << "argument error: --max-runtime-ms requires an unsigned integer\n";
        return 1;
      }
      ++i;
    } else if (arg == "--O0") {
      optimization_level = flux::OptimizationLevel::O0;
    } else if (arg == "--O2") {
      optimization_level = flux::OptimizationLevel::O2;
    } else {
      if (source_path != nullptr) {
        std::cerr << "Usage: " << argv[0] << " [--dot|--mermaid] [--O0|--O2] <source_file.fs>\n"
                  << "   or: " << argv[0] << " --emit-obj <output.o> [--O0|--O2] <source_file.fs>\n"
                  << "   or: " << argv[0]
                  << " --emit-exe <output> [--inject Machine:Event[:args]] [--idle-timeout-ms N] [--max-runtime-ms N]"
                     " [--O0|--O2] <source_file.fs>\n";
        return 1;
      }
      source_path = argv[i];
    }
  }

  if (!source_path) {
    std::cerr << "Usage: " << argv[0] << " [--dot|--mermaid] [--O0|--O2] <source_file.fs>\n"
              << "   or: " << argv[0] << " --emit-obj <output.o> [--O0|--O2] <source_file.fs>\n"
              << "   or: " << argv[0]
              << " --emit-exe <output> [--inject Machine:Event[:args]] [--idle-timeout-ms N] [--max-runtime-ms N]"
                 " [--O0|--O2] <source_file.fs>\n";
    return 1;
  }

  std::string source;
  try {
    source = ReadSourceFile(source_path);
  } catch (const std::runtime_error &error) {
    std::cerr << "io error: " << error.what() << '\n';
    return 1;
  }

  std::vector<Token> tokens;
  try {
    Lexer lexer(std::move(source));
    tokens = lexer.Tokenize();
  } catch (const std::runtime_error &error) {
    std::cerr << "lex error: " << error.what() << '\n';
    return 1;
  }

  Program program;
  try {
    Parser parser(std::move(tokens));
    program = parser.ParseProgram();
  } catch (const std::runtime_error &error) {
    std::cerr << "parse error: " << error.what() << '\n';
    return 1;
  }

  Sema sema;
  SemaResult sema_result = sema.Analyze(program);
  if (!sema_result.Ok()) {
    PrintSemaDiagnostics(sema_result.diagnostics);
    return 1;
  }

  Lowering lowering;
  LoweringResult lowering_result = lowering.Lower(program, sema_result.semantic_model);
  if (!lowering_result.Ok()) {
    PrintStringDiagnostics("lowering", lowering_result.diagnostics);
    return 1;
  }

  // 可视化模式：在 Lowering 之后直接输出图，不需要 IRGen
  if (output_mode == OutputMode::Dot) {
    Visualizer viz;
    std::cout << viz.ToDot(lowering_result.program);
    return 0;
  }
  if (output_mode == OutputMode::Mermaid) {
    Visualizer viz;
    std::cout << viz.ToMermaid(lowering_result.program);
    return 0;
  }

  IRGen irgen;
  IRGenResult irgen_result = irgen.Generate(lowering_result.program, source_path);
  if (!irgen_result.Ok()) {
    PrintStringDiagnostics("irgen", irgen_result.diagnostics);
    return 1;
  }
  if (!irgen_result.module) {
    std::cerr << "irgen error: generated module is null\n";
    return 1;
  }

  if (!VerifyModuleOrPrint(*irgen_result.module, "before optimization")) {
    return 1;
  }

  OptimizeModule(*irgen_result.module, optimization_level);

  if (!VerifyModuleOrPrint(*irgen_result.module, "after optimization")) {
    return 1;
  }

  if (output_mode == OutputMode::Object) {
    std::string codegen_error;
    if (!EmitObjectFile(*irgen_result.module, object_output_path, &codegen_error)) {
      std::cerr << "codegen error: " << codegen_error << '\n';
      return 1;
    }
    return 0;
  }

  if (output_mode == OutputMode::Executable) {
    executable_options.tool_root = ResolveToolRoot(argv[0]).string();
    const auto output_path = std::filesystem::path(executable_options.output_path);
    const auto build_dir = output_path.parent_path() / (output_path.filename().string() + ".build");
    std::error_code fs_error;
    std::filesystem::create_directories(build_dir, fs_error);
    if (fs_error) {
      std::cerr << "codegen error: cannot create build directory '" << build_dir.string() << "': " << fs_error.message()
                << '\n';
      return 1;
    }

    const auto user_object_path = (build_dir / "program.o").string();
    std::string codegen_error;
    if (!EmitObjectFile(*irgen_result.module, user_object_path, &codegen_error)) {
      std::cerr << "codegen error: " << codegen_error << '\n';
      return 1;
    }

    std::string build_error;
    if (!BuildExecutable(lowering_result.program, user_object_path, executable_options, &build_error)) {
      std::cerr << "link error: " << build_error << '\n';
      return 1;
    }
    return 0;
  }

  irgen_result.module->print(llvm::outs(), nullptr);
  llvm::outs().flush();
  return 0;
}