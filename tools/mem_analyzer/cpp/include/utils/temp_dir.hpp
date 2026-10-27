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
 *   create_directories(tmp.path() + "/subdir");
 *   // ... work ...
 *   tmp.cleanup();           // explicit cleanup before TempDir goes out of scope
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
    const std::string& path() const { return path_; }

    /// Whether the directory still exists (not yet cleaned up).
    bool valid() const { return !path_.empty(); }

    /// Explicitly remove the directory. Safe to call multiple times.
    void cleanup();

    /// Release ownership so the destructor does NOT remove the directory.
    void release() { released_ = true; }

    /// Remove the root mem-analyzer/ directory in the system temp if empty.
    /// Safe to call even if the directory does not exist.
    static void cleanup_root();

    /// 排障开关：为 true 时，非 quiet 的中间产物 cleanup() 只提示路径、不删除；
    /// quiet 的内部缓冲目录不受影响、照常清理。cleanup_root() 保留 root。
    static void set_keep(bool keep);

private:
    std::string path_;
    bool released_ = false;
    bool quiet_ = false;

    static std::string system_temp_root();
    static std::string generate_random_hex();
    static bool s_keep;
};

} // namespace mem_analyzer
