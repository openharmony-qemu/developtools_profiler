#pragma once
#include <string>

namespace mem_analyzer {
namespace zip_utils {

/// 用 minizip API 将 zip 解压到 dest_dir，替代 shell 调用 `unzip` / `Expand-Archive`
/// （消除命令注入面）。校验条目路径防 zip-slip：拒绝绝对路径、盘符、`..` 组件。
/// 成功返回 true；失败返回 false 且 `error` 非空。
bool extract_zip(const std::string& zip_path, const std::string& dest_dir,
                 std::string& error);

} // namespace zip_utils
} // namespace mem_analyzer
