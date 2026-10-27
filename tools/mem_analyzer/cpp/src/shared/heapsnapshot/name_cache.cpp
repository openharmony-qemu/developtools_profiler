#include "shared/heapsnapshot/name_cache.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/string_utils.hpp"
#include "utils/file_utils.hpp"  // path_from_utf8/path_to_utf8: Windows wide-path 安全
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace mem_analyzer {

using json = nlohmann::json;

namespace {

/// 解析 nameCache 缓存条目 key 的 ":start:end" 行号后缀。
/// 形如 "方法#成员#变量" / "#类名:start:end" / "方法名:start:end" / 普通变量名。
/// 成功解析后缀时返回 true，name 为去掉后缀的原始名，start_line/end_line 为行号；
/// 无后缀时返回 false，name 原样保留，行号为 -1/-1。
bool parse_line_suffix(const std::string& key, std::string& name, int& start_line, int& end_line) {
    start_line = -1;
    end_line = -1;
    name = key;

    size_t end_colon = key.rfind(':');
    if (end_colon == std::string::npos || end_colon + 1 >= key.size()) return false;
    if (!all_digits(key.substr(end_colon + 1))) return false;

    size_t start_colon = key.rfind(':', end_colon - 1);
    if (start_colon == std::string::npos || start_colon + 1 >= end_colon) return false;
    if (!all_digits(key.substr(start_colon + 1, end_colon - start_colon - 1))) return false;

    try {
        start_line = std::stoi(key.substr(start_colon + 1, end_colon - start_colon - 1));
        end_line = std::stoi(key.substr(end_colon + 1));
    } catch (const std::exception&) {
        start_line = -1;
        end_line = -1;
        return false;
    }
    name = key.substr(0, start_colon);
    return true;
}

/// 将单个缓存对象（IdentifierCache/MemberMethodCache/PropertyCache）的
/// 键值对反查入索引：value（混淆名）→ key（原始名，解析行号后缀）。
/// 同时建立 原始名 → 候选 的正向索引（未开启属性混淆时快照中的方法名即为原始名）。
/// 官方约定：key 中 '#' 代表上下级关系（如 "Class#method#__function"），
/// 匹配时保留最后一级名称，故额外以最后一级名称建立别名索引。
void index_cache_object(const json& obj, const std::string& file_path,
                        std::unordered_map<std::string, std::vector<NameCacheCandidate>>& by_obfuscated,
                        std::unordered_map<std::string, std::vector<NameCacheCandidate>>& by_original) {
    if (!obj.is_object()) return;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (!it.value().is_string()) continue;
        const std::string obf = it.value().get<std::string>();
        if (obf.empty()) continue;

        NameCacheCandidate candidate;
        candidate.file_path = file_path;
        parse_line_suffix(it.key(), candidate.original, candidate.start_line, candidate.end_line);
        by_obfuscated[obf].push_back(candidate);
        if (!candidate.original.empty()) {
            by_original[candidate.original].push_back(candidate);
            const size_t hash_pos = candidate.original.rfind('#');
            if (hash_pos != std::string::npos && hash_pos + 1 < candidate.original.size()) {
                by_original[candidate.original.substr(hash_pos + 1)].push_back(std::move(candidate));
            }
        }
    }
}

} // anonymous namespace

bool NameCacheIndex::load_from_dir(const std::string& dir) {
    std::error_code ec;
    std::vector<std::string> files;
    const fs::path dir_path = mem_analyzer::path_from_utf8(dir);

    // 直接指定 nameCache.json 文件本身
    if (fs::is_regular_file(dir_path, ec)) {
        files.push_back(dir);
    } else {
        ec.clear();
        if (!fs::is_directory(dir_path, ec)) {
            log::logw("Warning: nameCache file or directory not found: ", dir);
            return false;
        }
        // 递归扫描目录：文件名包含 "nameCache" 的 *.json 全部加载合并（兼容多模块产物）
        for (const auto& entry : fs::recursive_directory_iterator(dir_path, ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            const std::string filename = mem_analyzer::path_to_utf8(entry.path().filename());
            if (filename.size() < 8) continue;
            std::string lower = filename;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower.find("namecache") == std::string::npos) continue;
            const std::string ext = mem_analyzer::path_to_utf8(entry.path().extension());
            if (ext != ".json" && ext != ".JSON") continue;
            files.push_back(mem_analyzer::path_to_utf8(entry.path()));
        }
    }

    if (files.empty()) {
        log::logw("Warning: nameCache.json file not found in: ", dir);
        return false;
    }

    size_t loaded = 0;
    for (const auto& f : files) {
        if (index_name_cache_file(f)) ++loaded;
    }
    if (loaded == 0) {
        log::logw("Warning: Failed to parse any nameCache file in: ", dir);
        return false;
    }

    log::logi("  Loaded ", by_obfuscated_.size(), " obfuscated names, ", obf_name_to_path_.size(),
              " obfuscated paths from ", loaded, " nameCache file(s) in: ", dir);
    return true;
}

bool NameCacheIndex::index_name_cache_file(const std::string& file_path) {
    // 用户路径可能含中文，需按 UTF-8 构造 fs::path（Windows 宽路径安全）
    std::ifstream ifs(mem_analyzer::path_from_utf8(file_path), std::ios::binary);
    if (!ifs) {
        log::logw("Warning: Failed to open nameCache file: ", file_path);
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();

    json root;
    try {
        root = json::parse(ss.str());
    } catch (const std::exception& e) {
        log::logw("Warning: Failed to parse nameCache file ", file_path, ": ", e.what());
        return false;
    }
    if (!root.is_object()) {
        log::logw("Warning: nameCache file top level is not an object: ", file_path);
        return false;
    }

    // 顶层键：文件原始路径 → { IdentifierCache, MemberMethodCache, PropertyCache, obfName, ... }
    // compileSdkVersion / entryPackageInfo 等顶层字段不是对象，直接跳过。
    for (auto it = root.begin(); it != root.end(); ++it) {
        const std::string& file_original_path = it.key();
        const json& file_obj = it.value();
        if (!file_obj.is_object()) continue;

        static const char* kCaches[] = {"IdentifierCache", "MemberMethodCache", "PropertyCache"};
        for (const char* cache_name : kCaches) {
            auto cache_it = file_obj.find(cache_name);
            if (cache_it != file_obj.end()) {
                index_cache_object(cache_it.value(), file_original_path, by_obfuscated_, by_original_);
            }
        }

        // obfName 混淆路径 → 文件原始路径（顶层 key）
        auto obf_it = file_obj.find("obfName");
        if (obf_it != file_obj.end() && obf_it.value().is_string()) {
            const std::string obf_path = obf_it.value().get<std::string>();
            if (!obf_path.empty()) {
                obf_name_to_path_[obf_path] = file_original_path;
            }
        }
    }

    // 顶层 PropertyCache（全局属性名混淆映射，不隶属于单个文件）
    auto top_property_it = root.find("PropertyCache");
    if (top_property_it != root.end() && top_property_it.value().is_object()) {
        index_cache_object(top_property_it.value(), "", by_obfuscated_, by_original_);
    }
    return true;
}

bool NameCacheIndex::lookup_obfuscated_name(const std::string& obf, std::vector<NameCacheCandidate>& out) const {
    auto it = by_obfuscated_.find(obf);
    if (it == by_obfuscated_.end()) return false;
    out = it->second;
    return true;
}

bool NameCacheIndex::lookup_original_name(const std::string& original, std::vector<NameCacheCandidate>& out) const {
    auto it = by_original_.find(original);
    if (it == by_original_.end()) return false;
    out = it->second;
    return true;
}

bool NameCacheIndex::lookup_obf_name_path(const std::string& obf_path, std::string& original_path) const {
    auto it = obf_name_to_path_.find(obf_path);
    if (it == obf_name_to_path_.end()) return false;
    original_path = it->second;
    return true;
}

bool NameCacheIndex::lookup_unique(const std::string& obf, std::string& original) const {
    auto it = by_obfuscated_.find(obf);
    if (it == by_obfuscated_.end() || it->second.size() != 1) return false;
    original = it->second.front().original;
    return true;
}

bool NameCacheIndex::empty() const {
    return by_obfuscated_.empty() && obf_name_to_path_.empty();
}

} // namespace mem_analyzer
