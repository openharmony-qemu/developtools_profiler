#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace mem_analyzer {

/// 一个混淆名候选：原始名及对应的源码行号范围。
struct NameCacheCandidate {
    std::string original;  // 原始名（还原后的名字）
    int start_line = -1;   // 原始起始行号，无行号信息为 -1
    int end_line = -1;     // 原始结束行号，无行号信息为 -1
    std::string file_path; // 该候选所属的原始文件路径（nameCache 顶层 key）
};

/// nameCache 索引：解析工程 nameCache.json（IdentifierCache / MemberMethodCache /
/// PropertyCache / obfName），建立 混淆名 → 原始名 与 obfName 混淆路径 → 原始文件路径
/// 的反查索引，供 heapsnapshot 反混淆使用。
class NameCacheIndex {
public:
    /// 读取目录内 "nameCache.json"（递归扫描子目录，文件名包含 "nameCache" 的
    /// *.json 全部加载合并，兼容多模块产物）。顶层键为文件原始路径，
    /// 值为含 IdentifierCache/MemberMethodCache/PropertyCache/obfName 的对象。
    /// 找不到文件或解析失败返回 false。
    bool load_from_dir(const std::string& dir);

    /// 按混淆名反查所有候选（同名方法可能对应多个原始名）。
    bool lookup_obfuscated_name(const std::string& obf, std::vector<NameCacheCandidate>& out) const;

    /// 按原始名反查所有候选（未开启属性混淆时快照中的方法名即为原始名）。
    bool lookup_original_name(const std::string& original, std::vector<NameCacheCandidate>& out) const;

    /// 按 obfName 混淆路径反查原始文件路径。
    bool lookup_obf_name_path(const std::string& obf_path, std::string& original_path) const;

    /// 仅当候选唯一时返回 true 并填充 original。
    bool lookup_unique(const std::string& obf, std::string& original) const;

    /// 索引是否为空（未加载到任何条目）。
    bool empty() const;

private:
    /// 加载单个 nameCache.json 文件并合并进索引，成功返回 true。
    bool index_name_cache_file(const std::string& file_path);

    std::unordered_map<std::string, std::vector<NameCacheCandidate>> by_obfuscated_;
    std::unordered_map<std::string, std::vector<NameCacheCandidate>> by_original_;
    std::unordered_map<std::string, std::string> obf_name_to_path_;
};

} // namespace mem_analyzer
