#include "utils/file_utils.hpp"
#include <cerrno>
#include <system_error>
#include <iostream>
#include "utils/platform.hpp"

namespace fs = std::filesystem;

namespace mem_analyzer {

std::wstring widen_path(const std::string& path) {
#if defined(_WIN32)
    // Try UTF-8 first with MB_ERR_INVALID_CHARS to reject non-UTF-8 input
    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(), -1, nullptr, 0);
    if (wlen > 0) {
        std::wstring wpath(static_cast<size_t>(wlen) - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], wlen);
        return wpath;
    }
    // Fall back to ANSI code page (e.g., CP936/GBK on Chinese Windows)
    wlen = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, nullptr, 0);
    if (wlen > 0) {
        std::wstring wpath(static_cast<size_t>(wlen) - 1, L'\0');
        MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, &wpath[0], wlen);
        return wpath;
    }
    return std::wstring();
#else
    return std::wstring();
#endif
}

bool ends_with_ignore_case(const std::string &str, const std::string &suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

} // namespace mem_analyzer
