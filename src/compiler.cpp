#include "internal.hpp"

#include <utility>

namespace sysy {

CompileResult compile_source(std::string_view source, std::string source_name,
                             const CompileOptions& options) {
  CompileResult result;
  if (source.size() > detail::kMaxSourceBytes) {
    result.diagnostics.push_back(
        Diagnostic{std::move(source_name), 1, 1, "source file exceeds the 64 MiB limit"});
    return result;
  }
  try {
    detail::Parser parser(source);
    detail::Program program = parser.parse_program();
    result.assembly = detail::generate_rv64(program, options);
    result.ok = true;
  } catch (const detail::CompileError& error) {
    const detail::Loc loc = error.where();
    result.diagnostics.push_back(
        Diagnostic{std::move(source_name), loc.line, loc.column, error.what()});
  } catch (const std::bad_alloc&) {
    result.diagnostics.push_back(
        Diagnostic{std::move(source_name), 1, 1, "compiler resource limit exceeded"});
  } catch (const std::exception&) {
    result.diagnostics.push_back(
        Diagnostic{std::move(source_name), 1, 1, "internal compiler error"});
  }
  if (!result.ok) result.assembly.clear();
  return result;
}

}  // namespace sysy
