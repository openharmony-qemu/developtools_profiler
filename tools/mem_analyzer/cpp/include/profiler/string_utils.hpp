#pragma once

#include <string>
#include <cctype>
#include <ctime>

namespace mem_analyzer {

// Case-insensitive substring search (ASCII): true when `needle` occurs in
// `haystack`. Used e.g. to match user input against device app names/packages
// and to detect permission-related error markers in hdc output.
inline bool contains_ci(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    if (needle.size() > haystack.size()) return false;
    auto lower = [](char c) -> char {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };
    for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
        size_t j = 0;
        while (j < needle.size() && lower(haystack[i + j]) == lower(needle[j])) {
            ++j;
        }
        if (j == needle.size()) return true;
    }
    return false;
}

// Current local time formatted as YYYYMMDD_HHMMSS, used for naming exported
// snapshot files (e.g. smaps_20260809_212310.txt).
inline std::string current_timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm_local{};
#if defined(_WIN32)
    localtime_s(&tm_local, &now);
#else
    localtime_r(&now, &tm_local);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm_local);
    return std::string(buf);
}

} // namespace mem_analyzer
