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
#include <fstream>
#include <filesystem>
#include <memory>
#include "utils/temp_dir.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace mem_analyzer {

// Check if the suffixes match.
bool ends_with_ignore_case(const std::string &str, const std::string &suffix);

// ============================================================================
//  Wide-path helpers (Windows)
//  On Windows, convert UTF-8 path to std::wstring for wide-API file operations.
//  On non-Windows, pass through as-is.
// ============================================================================

/// Convert a path string to a wide string on Windows.
/// On Windows, tries CP_UTF8 first, falls back to CP_ACP (current ANSI code page).
/// On non-Windows, returns empty (not used).
std::wstring widen_path(const std::string& path);

// ============================================================================
//  fs::path construction helpers
//  On Windows, fs::path stores wstring internally; constructing from std::string
//  uses the narrow locale (e.g. CP936) and would corrupt UTF-8 input.
//  Use these helpers to go UTF-8 ↔ wide explicitly.
// ============================================================================

/// Construct fs::path from a UTF-8 std::string (wide-path safe on Windows).
inline fs::path path_from_utf8(const std::string& s) {
#if defined(_WIN32)
    return fs::path(widen_path(s));
#else
    return fs::path(s);
#endif
}

/// Convert fs::path to a UTF-8 std::string (wide-path safe on Windows).
inline std::string path_to_utf8(const fs::path& p) {
#if defined(_WIN32)
    auto ws = p.wstring();
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string result(static_cast<size_t>(len) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], len, nullptr, nullptr);
    return result;
#else
    return p.string();
#endif
}

#if defined(_WIN32)

// ============================================================================
//  ifstream / ofstream wrappers (UTF-8 path safe)
//  Uses path_from_utf8() to construct fs::path from UTF-8 input,
//  bypassing the narrow-locale encoding trap on Windows.
// ============================================================================

/// Open an input file stream with UTF-8 path support.
/// The returned stream behaves exactly like std::ifstream.
inline std::ifstream open_ifstream(const std::string& path, std::ios::openmode mode = std::ios::in) {
    std::ifstream ifs(path_from_utf8(path), mode);
    if (!ifs) {
        int err = errno;
        throw std::system_error(err, std::system_category(), "Failed to open: " + path);
    }
    return ifs;
}

/// Open an output file stream with UTF-8 path support.
inline std::ofstream open_ofstream(const std::string& path, std::ios::openmode mode = std::ios::out) {
    std::ofstream ofs(path_from_utf8(path), mode);
    if (!ofs) {
        int err = errno;
        throw std::system_error(err, std::system_category(), "Failed to open for writing: " + path);
    }
    return ofs;
}

/// Open a file with UTF-8 path support, returns raw FILE*.
/// Caller must fclose() the returned pointer.
/// Returns nullptr on failure (check errno).
inline FILE* open_file_raw(const std::string& path, const char* mode_cstr) {
    auto wpath = path_from_utf8(path);
    // Convert narrow mode to wide
    int len = MultiByteToWideChar(CP_UTF8, 0, mode_cstr, -1, nullptr, 0);
    std::wstring wmode(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, mode_cstr, -1, &wmode[0], len);
    return _wfopen(wpath.c_str(), wmode.c_str());
}

#else // not _WIN32

inline std::ifstream open_ifstream(const std::string& path, std::ios::openmode mode = std::ios::in) {
    return std::ifstream(path, mode);
}

inline std::ofstream open_ofstream(const std::string& path, std::ios::openmode mode = std::ios::out) {
    return std::ofstream(path, mode);
}

inline FILE* open_file_raw(const std::string& path, const char* mode) {
    return fopen(path.c_str(), mode);
}

#endif // _WIN32

} // namespace mem_analyzer
