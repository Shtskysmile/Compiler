#include "../include/sysy/compiler.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>

namespace {

struct Arguments {
  std::string input;
  std::string output;
  sysy::OptimizationLevel optimization{sysy::OptimizationLevel::O0};
  bool assembly_only{};
};

bool parse_arguments(int argc, char** argv, Arguments& result, std::string& error) {
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "-S") {
      result.assembly_only = true;
    } else if (argument == "-O0") {
      result.optimization = sysy::OptimizationLevel::O0;
    } else if (argument == "-O1") {
      result.optimization = sysy::OptimizationLevel::O1;
    } else if (argument == "-o") {
      if (++i >= argc) {
        error = "-o requires an output path";
        return false;
      }
      result.output = argv[i];
    } else if (!argument.empty() && argument[0] == '-') {
      error = "unknown option: " + argument;
      return false;
    } else if (result.input.empty()) {
      result.input = argument;
    } else {
      error = "multiple input files are not supported";
      return false;
    }
  }
  if (result.input.empty()) error = "missing input file";
  else if (!result.assembly_only) error = "the -S option is required";
  else if (result.output.empty()) error = "missing -o output path";
  return error.empty();
}

enum class ReadStatus { Ok, CannotRead, TooLarge };

ReadStatus read_file(const std::string& path, std::string& contents) {
  constexpr std::size_t kMaxSourceBytes = 64U * 1024U * 1024U;
  std::ifstream input(path, std::ios::binary);
  if (!input) return ReadStatus::CannotRead;
  std::array<char, 64U * 1024U> buffer{};
  while (input) {
    input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    const std::size_t count = static_cast<std::size_t>(input.gcount());
    if (count > kMaxSourceBytes - contents.size()) return ReadStatus::TooLarge;
    contents.append(buffer.data(), count);
  }
  return input.eof() ? ReadStatus::Ok : ReadStatus::CannotRead;
}

void print_diagnostic(const sysy::Diagnostic& diagnostic) {
  std::cerr << diagnostic.file << ':' << diagnostic.line << ':' << diagnostic.column
            << ": error: " << diagnostic.message << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  Arguments arguments;
  std::string argument_error;
  if (!parse_arguments(argc, argv, arguments, argument_error)) {
    std::cerr << "compiler: error: " << argument_error << '\n'
              << "usage: compiler input.sysy -S -o output.s [-O1]\n";
    return 1;
  }

  std::string source;
  const ReadStatus read_status = read_file(arguments.input, source);
  if (read_status == ReadStatus::CannotRead) {
    std::cerr << arguments.input << ":1:1: error: cannot read input file\n";
    return 1;
  }
  if (read_status == ReadStatus::TooLarge) {
    std::cerr << arguments.input << ":1:1: error: source file exceeds the 64 MiB limit\n";
    return 1;
  }
  sysy::CompileOptions options;
  options.optimization = arguments.optimization;
  sysy::CompileResult result = sysy::compile_source(source, arguments.input, options);
  if (!result.ok) {
    for (const auto& diagnostic : result.diagnostics) print_diagnostic(diagnostic);
    return 1;
  }

  std::vector<char> temporary(arguments.output.begin(), arguments.output.end());
  const char suffix[] = ".tmp.XXXXXX";
  temporary.insert(temporary.end(), std::begin(suffix), std::end(suffix));
  const int descriptor = mkstemp(temporary.data());
  if (descriptor < 0) {
    std::cerr << arguments.output << ":1:1: error: cannot create output file\n";
    return 1;
  }
  FILE* output = fdopen(descriptor, "wb");
  bool wrote = false;
  if (output) {
    const std::size_t count =
        std::fwrite(result.assembly.data(), 1, result.assembly.size(), output);
    wrote = std::fclose(output) == 0 && count == result.assembly.size();
  } else {
    close(descriptor);
  }
  if (!wrote) {
    std::remove(temporary.data());
    std::cerr << arguments.output << ":1:1: error: cannot write output file\n";
    return 1;
  }
  if (std::rename(temporary.data(), arguments.output.c_str()) != 0) {
    std::remove(temporary.data());
    std::cerr << arguments.output << ":1:1: error: cannot replace output file\n";
    return 1;
  }
  return 0;
}
