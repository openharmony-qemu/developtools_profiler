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
#include "utils/temp_dir.hpp"
#include "utils/file_utils.hpp"  // for path_from_utf8, path_to_utf8
#include "utils/logger.hpp"
#include "utils/platform.hpp"    // for create_directories, remove_all
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>

namespace fs = std::filesystem;

namespace mem_analyzer {

static std::string sRoot;  // per-process root, cached
bool TempDir::sKeep = false;

void TempDir::SetKeep(bool keep) { sKeep = keep; }

std::string TempDir::SystemTempRoot()
{
    if (!sRoot.empty()) {
        return sRoot;
    }
    // std::filesystem::temp_directory_path() is cross-platform:
    //   Windows : %TEMP%  (typically C:\Users\<user>\AppData\Local\Temp)
    //   Linux   : /tmp
    //   macOS   : /tmp
    std::error_code ec;
    auto tmp = fs::temp_directory_path(ec);
    if (ec) {
        tmp = fs::current_path();
    }
    sRoot = PathToUtf8(tmp / ("mem-analyzer." + GenerateRandomHex()));
    return sRoot;
}

static void EnsureRootExists(const std::string& root)
{
    platform::CreateDirectories(root);
}

std::string TempDir::GenerateRandomHex()
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFF);
    std::ostringstream oss;
    oss << std::hex << std::setw(6) << std::setfill('0') << dist(rng);
    return oss.str();
}

TempDir::TempDir(const std::string& prefix, bool quiet) : quiet_(quiet)
{
    auto root = SystemTempRoot();
    EnsureRootExists(root);

    for (int attempt = 0; attempt < 10; ++attempt) {
        auto candidate = PathToUtf8(PathFromUtf8(root) / (prefix + "." + GenerateRandomHex()));
        if (platform::CreateDirectory(candidate)) {
            path_ = candidate;
            return;
        }
    }
    // Last resort: append timestamp to avoid collision
    auto ts = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    path_ = PathToUtf8(PathFromUtf8(root) / (prefix + "." + ts));
    platform::CreateDirectories(path_);
}

TempDir::~TempDir()
{
    Cleanup();
}

TempDir& TempDir::operator=(TempDir&& other) noexcept
{
    if (this != &other) {
        Cleanup();
        path_ = std::move(other.path_);
        released_ = other.released_;
        quiet_ = other.quiet_;
        other.path_.clear();
        other.released_ = false;
        other.quiet_ = false;
    }
    return *this;
}

void TempDir::Cleanup()
{
    if (path_.empty() || released_) {
        return;
    }
    // 排障模式仅保留有分析价值的中间产物（非 quiet）；quiet 的内部缓冲目录照常清理。
    if (sKeep && !quiet_) {
        log::Logi("  Kept temp: ", path_);
        released_ = true;
        return;
    }
    platform::RemoveAll(path_);
    if (!quiet_) {
        log::LogiMinor("  Cleaned up temp: ", path_);
    }
    path_.clear();
}

void TempDir::CleanupRoot()
{
    auto root = SystemTempRoot();
    if (sKeep) {
        log::Logi("  Kept root temp: ", root);
        return;
    }
    platform::RemoveAll(root);
    log::LogiMinor("  Cleaned up root temp: ", root);
}

} // namespace mem_analyzer
