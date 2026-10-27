#pragma once
#include <string>

namespace mem_analyzer {
namespace style {

/// Enable ANSI escape processing on Windows (call once at startup).
void init();

/// True if stdout is a terminal (not piped/redirected).
bool is_tty();

/// True if stderr is a terminal (not piped/redirected).
bool is_stderr_tty();

/// Wrap text with ANSI color/style. Returns raw text if stderr is not a TTY.
std::string green(const std::string& text);
std::string red(const std::string& text);
std::string yellow(const std::string& text);
std::string bold(const std::string& text);
std::string dim(const std::string& text);

} // namespace style
} // namespace mem_analyzer