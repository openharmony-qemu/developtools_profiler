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
#include "utils/zip_utils.hpp"

#include "utils/file_utils.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "unzip.h"

#if defined(_WIN32)
#include "iowin32.h"
#endif

namespace fs = std::filesystem;

namespace mem_analyzer {
namespace zipUtils {

namespace {
constexpr int WIN_DRIVE_LETTER_MIN_LEN = 2;
constexpr int UNZIP_BUF_SIZE_KB = 64;
constexpr int BYTES_PER_KB = 1024;
} // namespace

namespace {

/**
 * 校验 zip 条目名是否安全（防 zip-slip）：拒绝绝对路径、盘符、`..` 组件。
 */
bool IsSafeEntry(const std::string& name)
{
    if (name.empty()) {
        return false;
    }
    if (name.front() == '/' || name.front() == '\\') {
        return false;
    }
    if (name.size() >= WIN_DRIVE_LETTER_MIN_LEN && name[1] == ':') {
        return false; // Windows 盘符（如 "C:"）
    }
    std::string normalized = name;
    for (auto& c : normalized) {
        if (c == '\\') {
            c = '/';
        }
    }
    const fs::path p = PathFromUtf8(normalized);
    for (const auto& part : p) {
        if (part == "..") {
            return false;
        }
    }
    return true;
}

/**
 * 条目名以 '/' 或 '\' 结尾即视为目录条目。
 */
bool IsDirEntry(const std::string& name)
{
    return !name.empty() && (name.back() == '/' || name.back() == '\\');
}

/**
 * zip 炸弹防护：累计解压字节数上限（4 GiB）。合法 tshandle 数据远小于此，
 * 恶意/损坏 zip 声明超大 uncompressed_size 时中止。
 */
constexpr uint64_t MAX_TOTAL_UNCOMPRESSED_BYTES = 4ULL * 1024 * 1024 * 1024;

} // namespace

/// Process a single zip entry. Returns true to continue, false on error.
static bool ExtractOneEntry(unzFile uf, const std::string& destDir,
    uint64_t& totalWritten, std::string& error)
{
    unz_file_info64 fi;
    if (unzGetCurrentFileInfo64(uf, &fi, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) {
        return true;
    }
    std::vector<char> nameBuf(static_cast<size_t>(fi.size_filename) + 1, '\0');
    if (unzGetCurrentFileInfo64(uf, &fi, nameBuf.data(),
                                static_cast<uLong>(nameBuf.size()),
                                nullptr, 0, nullptr, 0) != UNZ_OK) {
        return true;
    }
    const std::string entryName(nameBuf.data(), static_cast<size_t>(fi.size_filename));

    if (!IsSafeEntry(entryName)) {
        error = "Unsafe entry path in ZIP: " + entryName;
        return false;
    }

    const fs::path outPath = PathFromUtf8(destDir) / PathFromUtf8(entryName);

    if (IsDirEntry(entryName)) {
        std::error_code ec;
        fs::create_directories(outPath, ec);
        return true;
    }

    std::error_code ec;
    fs::create_directories(outPath.parent_path(), ec);

    if (fi.uncompressed_size > MAX_TOTAL_UNCOMPRESSED_BYTES - totalWritten) {
        error = "ZIP entry exceeds decompressed size limit: " + entryName;
        return false;
    }

    if (unzOpenCurrentFile(uf) != UNZ_OK) {
        error = "Failed to open entry in ZIP: " + entryName;
        return false;
    }

    std::ofstream out(outPath, std::ios::binary);
    if (!out) {
        unzCloseCurrentFile(uf);
        error = "Failed to create output file: " + entryName;
        return false;
    }

    std::vector<char> buf(UNZIP_BUF_SIZE_KB * BYTES_PER_KB);
    int n = 0;
    while ((n = unzReadCurrentFile(uf, buf.data(), static_cast<unsigned>(buf.size()))) > 0) {
        out.write(buf.data(), n);
        totalWritten += static_cast<uint64_t>(n);
        if (totalWritten > MAX_TOTAL_UNCOMPRESSED_BYTES) {
            out.close();
            unzCloseCurrentFile(uf);
            error = "ZIP decompressed size exceeds limit: " + entryName;
            return false;
        }
    }
    out.close();
    unzCloseCurrentFile(uf);

    if (n < 0) {
        error = "Failed to decompress entry: " + entryName;
        return false;
    }
    return true;
}

bool ExtractZip(const std::string& zipPath, const std::string& destDir, std::string& error)
{
    unzFile uf = nullptr;
#if defined(_WIN32)
    // Windows：宽字符文件函数 + UTF-8→wstring，正确处理中文路径。
    const fs::path zipP = PathFromUtf8(zipPath);
    zlib_filefunc64_def ff;
    fill_win32_filefunc64W(&ff);
    uf = unzOpen2_64(zipP.c_str(), &ff);
#else
    uf = unzOpen64(zipPath.c_str());
#endif
    if (!uf) {
        error = "Failed to open ZIP file: " + zipPath;
        return false;
    }

    if (unzGoToFirstFile(uf) != UNZ_OK) {
        unzClose(uf);
        return true;  // 空 zip：无条目可解压，视为成功
    }

    uint64_t totalWritten = 0;  // 累计解压字节数（zip 炸弹防护）

    do {
        if (!ExtractOneEntry(uf, destDir, totalWritten, error)) {
            unzClose(uf);
            return false;
        }
    } while (unzGoToNextFile(uf) == UNZ_OK);

    unzClose(uf);
    return true;
}

} // namespace zip_utils
} // namespace mem_analyzer
