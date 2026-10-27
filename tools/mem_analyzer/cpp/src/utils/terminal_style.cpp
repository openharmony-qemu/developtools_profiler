#include "utils/terminal_style.hpp"

#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#define ISATTY  _isatty
#define FILENO  _fileno
#else
#include <unistd.h>
#define ISATTY  isatty
#define FILENO  fileno
#endif

namespace mem_analyzer {
namespace style {

static constexpr const char* ANSI_GREEN  = "\033[32m";
static constexpr const char* ANSI_RED    = "\033[31m";
static constexpr const char* ANSI_YELLOW = "\033[33m";
static constexpr const char* ANSI_BOLD   = "\033[1m";
static constexpr const char* ANSI_DIM    = "\033[2m";
static constexpr const char* ANSI_RESET  = "\033[0m";

static bool s_initialized = false;

void init() {
#if defined(_WIN32)
    if (s_initialized) return;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(h, &mode)) {
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
    s_initialized = true;
#else
    (void)s_initialized;
#endif
}

bool is_tty() {
    return ISATTY(FILENO(stdout)) != 0;
}

bool is_stderr_tty() {
    return ISATTY(FILENO(stderr)) != 0;
}

static std::string wrap(const char* code, const std::string& text) {
    if (!is_stderr_tty()) return text;
    return std::string(code) + text + ANSI_RESET;
}

std::string green(const std::string& text)  { return wrap(ANSI_GREEN,  text); }
std::string red(const std::string& text)    { return wrap(ANSI_RED,    text); }
std::string yellow(const std::string& text) { return wrap(ANSI_YELLOW, text); }
std::string bold(const std::string& text)   { return wrap(ANSI_BOLD,   text); }
std::string dim(const std::string& text)    { return wrap(ANSI_DIM,    text); }

} // namespace style
} // namespace mem_analyzer