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

static std::string s_root;  // per-process root, cached
bool TempDir::s_keep = false;

void TempDir::set_keep(bool keep) { s_keep = keep; }

std::string TempDir::system_temp_root() {
    if (!s_root.empty()) return s_root;
    // std::filesystem::temp_directory_path() is cross-platform:
    //   Windows : %TEMP%  (typically C:\Users\<user>\AppData\Local\Temp)
    //   Linux   : /tmp
    //   macOS   : /tmp
    std::error_code ec;
    auto tmp = fs::temp_directory_path(ec);
    if (ec) {
        tmp = fs::current_path();
    }
    s_root = path_to_utf8(tmp / ("mem-analyzer." + generate_random_hex()));
    return s_root;
}

static void ensure_root_exists(const std::string& root) {
    platform::create_directories(root);
}

std::string TempDir::generate_random_hex() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFF);
    std::ostringstream oss;
    oss << std::hex << std::setw(6) << std::setfill('0') << dist(rng);
    return oss.str();
}

TempDir::TempDir(const std::string& prefix, bool quiet) : quiet_(quiet) {
    auto root = system_temp_root();
    ensure_root_exists(root);

    for (int attempt = 0; attempt < 10; ++attempt) {
        auto candidate = path_to_utf8(path_from_utf8(root) / (prefix + "." + generate_random_hex()));
        if (platform::create_directory(candidate)) {
            path_ = candidate;
            return;
        }
    }
    // Last resort: append timestamp to avoid collision
    auto ts = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    path_ = path_to_utf8(path_from_utf8(root) / (prefix + "." + ts));
    platform::create_directories(path_);
}

TempDir::~TempDir() {
    cleanup();
}

TempDir& TempDir::operator=(TempDir&& other) noexcept {
    if (this != &other) {
        cleanup();
        path_ = std::move(other.path_);
        released_ = other.released_;
        quiet_ = other.quiet_;
        other.path_.clear();
        other.released_ = false;
        other.quiet_ = false;
    }
    return *this;
}

void TempDir::cleanup() {
    if (path_.empty() || released_) return;
    // 排障模式仅保留有分析价值的中间产物（非 quiet）；quiet 的内部缓冲目录照常清理。
    if (s_keep && !quiet_) {
        log::logi("  Kept temp: ", path_);
        released_ = true;
        return;
    }
    platform::remove_all(path_);
    if (!quiet_) log::logi_minor("  Cleaned up temp: ", path_);
    path_.clear();
}

void TempDir::cleanup_root() {
    auto root = system_temp_root();
    if (s_keep) {
        log::logi("  Kept root temp: ", root);
        return;
    }
    platform::remove_all(root);
    log::logi_minor("  Cleaned up root temp: ", root);
}

} // namespace mem_analyzer
