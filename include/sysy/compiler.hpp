#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace sysy {

enum class OptimizationLevel { O0, O1 };

struct Diagnostic {
  std::string file;
  std::size_t line{1};
  std::size_t column{1};
  std::string message;
};

struct CompileOptions {
  OptimizationLevel optimization{OptimizationLevel::O0};
};

struct CompileResult {
  bool ok{false};
  std::string assembly;
  std::vector<Diagnostic> diagnostics;
};

CompileResult compile_source(std::string_view source, std::string source_name,
                             const CompileOptions& options = {});

}  // namespace sysy
