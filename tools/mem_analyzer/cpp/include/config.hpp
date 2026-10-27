#pragma once
#include <string>

namespace mem_analyzer {

enum class Scope {
    AppOnly,
    SystemOnly,
    AppAndSystem,
    Mixed
};

Scope scope_from_string(const std::string& s);
std::string scope_to_string(Scope s);

// Default values for CLI options
namespace defaults {
    constexpr const char* OUTPUT_DIR = "out";
    constexpr Scope SCOPE = Scope::AppAndSystem;
    constexpr const char* SCOPE_STR = "app-and-system";
    constexpr int TRACE_TOP_N = 10;
    constexpr int TSHANDLE_TOP_N = 50;
    constexpr int JSLEAK_TOP_N = 50;
    constexpr const char* JSLEAK_MODE = "all";
    constexpr const char* HTRACE_TYPE = "nativeheap";
    constexpr int PROF_DURATION = 120;
}

} // namespace mem_analyzer
