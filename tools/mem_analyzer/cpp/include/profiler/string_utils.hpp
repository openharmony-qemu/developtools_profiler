/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <string>
#include <cctype>
#include <ctime>

namespace mem_analyzer {

// Case-insensitive substring search (ASCII): true when `needle` occurs in
// `haystack`. Used e.g. to match user input against device app names/packages
// and to detect permission-related error markers in hdc output.
inline bool ContainsCi(const std::string& haystack, const std::string& needle)
{
    if (needle.empty()) {
        return true;
    }
    if (needle.size() > haystack.size()) {
        return false;
    }
    auto lower = [](char c) -> char {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };
    for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
        size_t j = 0;
        while (j < needle.size() && lower(haystack[i + j]) == lower(needle[j])) {
            ++j;
        }
        if (j == needle.size()) {
            return true;
        }
    }
    return false;
}

// Current local time formatted as YYYYMMDD_HHMMSS, used for naming exported
// snapshot files (e.g. smaps_20260809_212310.txt).
inline std::string CurrentTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm tmLocal{};
#if defined(_WIN32)
    localtime_s(&tmLocal, &now);
#else
    localtime_r(&now, &tmLocal);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tmLocal);
    return std::string(buf);
}

} // namespace mem_analyzer
