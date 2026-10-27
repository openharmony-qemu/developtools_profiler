#pragma once
#include <cctype>
#include <string>

namespace mem_analyzer {

/// 判断字符串是否全为数字（非空且每个字符都是 0-9）。
inline bool all_digits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

/// 判断源码路径是否与 nameCache 顶层 key（原始文件路径）匹配：
/// nameCache 顶层 key 无模块前缀（如 "src/main/ets/web/a.ets"），而还原后的
/// 源码路径常带 "module/" 前缀（如 "entry/src/main/ets/web/a.ets"），按路径后缀匹配。
inline bool source_path_matches(const std::string& src, const std::string& file_path) {
    if (src == file_path) return true;
    if (src.size() <= file_path.size()) return false;
    const size_t sep_pos = src.size() - file_path.size() - 1;
    return src[sep_pos] == '/' && src.compare(sep_pos + 1, file_path.size(), file_path) == 0;
}

} // namespace mem_analyzer
