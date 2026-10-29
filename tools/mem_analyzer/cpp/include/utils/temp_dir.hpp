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

namespace mem_analyzer {

/**
 * RAII temporary directory.
 *
 * Creates a directory at:
 *   {system_temp}/mem-analyzer/{prefix}.{random_hex}/
 *
 * The directory is automatically removed on destruction.
 * Movable but not copyable.
 *
 * Usage:
 *   TempDir tmp("htrace");
 *   create_directories(tmp.Path() + "/subdir");
 *   // ... work ...
 *   tmp.Cleanup();           // explicit cleanup before TempDir goes out of scope
 *   // Or let the destructor handle it automatically.
 */
class TempDir {
public:
    /// Default constructor: creates an empty (invalid) TempDir.
    /// Call create() or move-assign a valid TempDir to use it.
    TempDir() = default;

    /// Create a temp directory with the given prefix.
    /// `quiet` = 内部缓冲目录：清理时不打印，且 `--keep-temp` 排障模式下也照常清理。
    explicit TempDir(const std::string& prefix, bool quiet = false);

    ~TempDir();

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    TempDir(TempDir&& other) noexcept;
    TempDir& operator=(TempDir&& other) noexcept;

    /// Full path to the temp directory.
    const std::string& Path() const { return path_; }

    /// Whether the directory still exists (not yet cleaned up).
    bool Valid() const { return !path_.empty(); }

    /// Explicitly remove the directory. Safe to call multiple times.
    void Cleanup();

    /// Release ownership so the destructor does NOT remove the directory.
    void Release() { released_ = true; }

    /// Remove the root mem-analyzer/ directory in the system temp if empty.
    /// Safe to call even if the directory does not exist.
    static void CleanupRoot();

    /// 排障开关：为 true 时，非 quiet 的中间产物 cleanup() 只提示路径、不删除；
    /// quiet 的内部缓冲目录不受影响、照常清理。cleanup_root() 保留 root。
    static void SetKeep(bool keep);

private:
    std::string path_;
    bool released_ = false;
    bool quiet_ = false;

    static std::string SystemTempRoot();
    static std::string GenerateRandomHex();
    static bool sKeep;
};

} // namespace mem_analyzer
