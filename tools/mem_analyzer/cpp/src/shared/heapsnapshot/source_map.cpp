#include "shared/heapsnapshot/source_map.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/file_utils.hpp"  // path_from_utf8/path_to_utf8: Windows wide-path 安全
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

namespace mem_analyzer {

using json = nlohmann::json;

// ============================================================================
//  Base64 VLQ (Variable Length Quantity) 解码
//
//  source map v3 的 "mappings" 字段编码规则：
//    - 以 ';' 分隔行，以 ',' 分隔段（segment）
//    - 每个段由若干 VLQ 字段组成（相对值，需累积解码）：
//        [generatedColumn, sourceIndex, originalLine, originalColumn, nameIndex?]
//    - VLQ 编码：每 5 bit 一个 Base64 字符，最高位（0x20）为续位，
//      最低位（0x01）为符号位，数值右移 1 位后按符号取负
// ============================================================================

namespace {

/// Base64 字符 → 6 位数值，非法字符返回 -1。
inline int base64_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

/// 解码一个 VLQ 值，成功返回 true 并填充 value，失败返回 false。
inline bool decode_vlq_value(const std::string& str, size_t& pos, int& value) {
    int result = 0;
    int shift = 0;
    bool continuation = true;
    while (continuation) {
        if (pos >= str.size()) return false;
        if (shift > 25) return false;  // VLQ 最多 6 段（30 位），防有符号移位 UB
        int digit = base64_value(str[pos++]);
        if (digit < 0) return false;
        continuation = (digit & 0x20) != 0;
        result += (digit & 0x1F) << shift;
        shift += 5;
    }
    bool negative = (result & 0x01) != 0;
    value = result >> 1;
    if (negative) value = -value;
    return true;
}

} // anonymous namespace

SourceMapIndex::DecodedMappings SourceMapIndex::decode_vlq_mappings(const std::string& mappings) {
    // source map v3 的 mappings 采用相对（delta）编码：段内每个字段相对
    // 前一段累积，解码时需按字段顺序逐项累加得到绝对值。解码结果：
    // 行映射列表（vector<vector<vector<int>>>），行数 = 分号数 + 1，
    // 段内字段依次为
    // [generatedColumn, sourceIndex, originalLine, originalColumn, nameIndex?]
    //
    // 注意：segment 的实际字段数（1/4/5 个）无法从编码数据直接判断，
    // 必须按 ';'（行）与 ','（段）先切分出每个 segment 的原始子串，
    // 再解码该子串内的全部 VLQ 字段；若固定按 5 个字段读取，会跨段
    // 误读导致边界错乱、整行映射丢失。
    DecodedMappings decoded;
    decoded.emplace_back();  // 第 0 行

    // 相对值累加器（generatedColumn 换行重置，其余跨行保持）。
    // 用 64 位累加：单个 VLQ 值可达 ~2^30，连续累加会越过 INT_MAX 触发
    // 有符号溢出 UB（表现为行/列号变负、位置还原全乱）。
    auto clamp_int = [](int64_t v) -> int {
        if (v > INT32_MAX) return INT32_MAX;
        if (v < INT32_MIN) return INT32_MIN;
        return static_cast<int>(v);
    };
    int64_t generated_column = 0;
    int64_t source_index = 0;
    int64_t original_line = 0;
    int64_t original_column = 0;
    int64_t name_index = 0;

    size_t pos = 0;
    size_t line = 0;
    while (pos < mappings.size()) {
        // 切出当前 segment 的原始子串（到 ',' / ';' / 末尾）
        const size_t end = mappings.find_first_of(";,", pos);
        const size_t seg_end = (end == std::string::npos) ? mappings.size() : end;
        if (seg_end > pos) {
            // 解码该 segment 内的全部 VLQ 字段（相对值）
            const std::string seg = mappings.substr(pos, seg_end - pos);
            std::vector<int> raw;
            size_t p = 0;
            while (p < seg.size()) {
                int value = 0;
                if (!decode_vlq_value(seg, p, value)) break;
                raw.push_back(value);
            }
            if (!raw.empty()) {
                // 将相对值累加为绝对值（64 位累加，输出时收敛到 int 范围）
                std::vector<int> segment;
                generated_column += raw[0];
                segment.push_back(clamp_int(generated_column));
                if (raw.size() > 1) {
                    source_index += raw[1];
                    segment.push_back(clamp_int(source_index));
                }
                if (raw.size() > 2) {
                    original_line += raw[2];
                    segment.push_back(clamp_int(original_line));
                }
                if (raw.size() > 3) {
                    original_column += raw[3];
                    segment.push_back(clamp_int(original_column));
                }
                if (raw.size() > 4) {
                    name_index += raw[4];
                    segment.push_back(clamp_int(name_index));
                }
                decoded[line].push_back(std::move(segment));
            }
        }
        if (end == std::string::npos) break;
        const char sep = mappings[end];
        pos = end + 1;
        if (sep == ';') {
            // 行结束：行号 +1，generatedColumn 重置
            decoded.emplace_back();
            line++;
            generated_column = 0;
        }
    }
    return decoded;
}

const SourceMapIndex::DecodedMappings& SourceMapIndex::get_decoded_mappings(const std::string& mappings) const {
    auto it = decoded_cache_.find(mappings);
    if (it != decoded_cache_.end()) return it->second;
    // unordered_map 的插入/重哈希不会使已存元素的引用失效（仅 erase 失效），
    // 故返回的引用在解码缓存存续期间保持稳定。
    return decoded_cache_.emplace(mappings, decode_vlq_mappings(mappings)).first->second;
}

bool SourceMapIndex::resolve_original_line(const std::string& mappings, int generated_line, int& original_line) const {
    if (generated_line < 1 || mappings.empty()) return false;
    const DecodedMappings& decoded = get_decoded_mappings(mappings);
    if (decoded.empty()) return false;

    // 官方算法（"异常堆栈解析原理"）：sourceMap 直接映射记录行，无行号补偿。
    // decoded 为 0 基行索引：decoded[i] 对应生成文件第 i+1 行，故记录行
    // generated_line 对应 decoded[generated_line - 1]；目标行无映射段时，
    // 按 source map v3 段覆盖语义向前回退到最近一个已映射行。
    int line = generated_line - 1;
    if (line >= static_cast<int>(decoded.size())) return false;

    while (line >= 0 && decoded[line].empty()) --line;
    if (line < 0) return false;
    const LineSegments& segments = decoded[line];
    if (segments.empty() || segments.front().size() < 3) return false;
    original_line = segments.front()[2] + 1;  // originalLine 0-based → 1-based
    return true;
}

bool SourceMapIndex::resolve_original_position(const std::string& mappings, int generated_line,
                                               int generated_column, int& original_line,
                                               int& original_column, int* out_source_index) const {
    if (generated_line < 1 || mappings.empty()) return false;
    const DecodedMappings& decoded = get_decoded_mappings(mappings);
    if (decoded.empty()) return false;

    // decoded 为 0 基行索引：decoded[i] 对应生成文件第 i+1 行
    int line = generated_line - 1;
    if (line >= static_cast<int>(decoded.size())) return false;

    // 目标行内取 generatedColumn <= 目标列的最后一个 segment（source map v3
    // 约定：segment 覆盖 [generatedColumn, 下一个 segment 列) 区间）
    const LineSegments* segments = &decoded[line];
    const Segment* best = nullptr;
    for (const auto& seg : *segments) {
        if (seg.size() < 4) continue;
        if (seg[0] <= generated_column) {
            best = &seg;
        } else {
            break;  // segment 按 generatedColumn 递增
        }
    }
    if (best == nullptr) {
        // 目标行无覆盖段：向前回退到最近一个有映射的行，取该行最末覆盖段
        for (int l = line - 1; l >= 0; --l) {
            if (decoded[l].empty()) continue;
            const Segment* last = nullptr;
            for (const auto& seg : decoded[l]) {
                if (seg.size() >= 4) last = &seg;
            }
            if (last != nullptr) {
                original_line = (*last)[2] + 1;
                original_column = (*last)[3] + 1;
                if (out_source_index != nullptr) *out_source_index = (*last)[1];
                return true;
            }
        }
        return false;
    }

    original_line = (*best)[2] + 1;      // originalLine 0-based → 1-based
    original_column = (*best)[3] + 1;    // originalColumn 0-based → 1-based
    if (out_source_index != nullptr) *out_source_index = (*best)[1];  // sourceIndex
    return true;
}

// ============================================================================
//  SourceMapIndex
// ============================================================================

namespace {

/// 从单个 map 文件解析顶层条目并入索引；失败返回 false。
/// 顶层键即混淆 key（如 "entry|entry|1.0.0|src/main/ets/pages/a.ts"），
/// 值含 sources/file/entry-package-info/package-info 字段。
bool index_map_file(const std::string& path, std::unordered_map<std::string, SourceMapEntry>& index) {
    // 用户路径可能含中文，需按 UTF-8 构造 fs::path（Windows 宽路径安全）
    std::ifstream ifs(mem_analyzer::path_from_utf8(path), std::ios::binary);
    if (!ifs) return false;
    std::stringstream ss;
    ss << ifs.rdbuf();
    json root;
    try {
        root = json::parse(ss.str());
    } catch (const std::exception& e) {
        log::logw("Warning: Failed to parse source map file ", path, ": ", e.what());
        return false;
    }
    if (!root.is_object()) return false;

    for (auto it = root.begin(); it != root.end(); ++it) {
        const std::string& key = it.key();
        const json& value = it.value();
        if (!value.is_object()) continue;

        SourceMapEntry entry;
        // sources：可能是数组（source map v3 标准）或单字符串；完整保存数组，
        // 同时用 sources[0] 填充便捷字段 source_path
        if (value.contains("sources")) {
            const json& sources = value["sources"];
            if (sources.is_array()) {
                for (const auto& s : sources) {
                    if (s.is_string()) entry.sources.push_back(s.get<std::string>());
                }
                if (!entry.sources.empty()) entry.source_path = entry.sources.front();
            } else if (sources.is_string()) {
                entry.sources.push_back(sources.get<std::string>());
                entry.source_path = entry.sources.front();
            }
        }
        if (value.contains("file") && value["file"].is_string()) {
            entry.file = value["file"].get<std::string>();
        }
        if (value.contains("entry-package-info") && value["entry-package-info"].is_string()) {
            entry.entry_package_info = value["entry-package-info"].get<std::string>();
        }
        if (value.contains("package-info") && value["package-info"].is_string()) {
            entry.package_info = value["package-info"].get<std::string>();
        }
        if (value.contains("mappings") && value["mappings"].is_string()) {
            entry.mappings = value["mappings"].get<std::string>();
        }
        index[key] = std::move(entry);
    }
    return true;
}

} // anonymous namespace

namespace {

/// 重建二级索引：module|version|rel_path → 条目。
/// 混淆 key 格式为 "<bundle>|<module>|<version>|<rel_path>"，
/// 以第 2 段 module（而非第 1 段 bundle）作为匹配键的一部分。
void rebuild_module_index(const std::unordered_map<std::string, SourceMapEntry>& index,
                          std::unordered_map<std::string, const SourceMapEntry*>& module_index) {
    module_index.clear();
    for (const auto& kv : index) {
        const std::string& key = kv.first;
        const size_t sep1 = key.find('|');
        if (sep1 == std::string::npos) continue;
        const size_t sep2 = key.find('|', sep1 + 1);
        if (sep2 == std::string::npos) continue;
        const size_t sep3 = key.find('|', sep2 + 1);
        if (sep3 == std::string::npos) continue;
        // 关键修复：第 2 段是真实 module 名（如 @douyin/camera_impl），
        // 第 1 段是 bundle 名（如 entry），参与匹配会导致作用域包失配。
        const std::string mkey = key.substr(sep1 + 1, sep2 - sep1 - 1) + "|" +
                                 key.substr(sep2 + 1, sep3 - sep2 - 1) + "|" +
                                 key.substr(sep3 + 1);
        module_index[mkey] = &kv.second;
    }
}

} // anonymous namespace

bool SourceMapIndex::load_from_dir(const std::string& dir) {
    std::error_code ec;
    const fs::path dir_path = mem_analyzer::path_from_utf8(dir);

    // 直接指定单个 source map 文件（sourceMaps.map / *.map）时直接加载
    if (fs::is_regular_file(dir_path, ec)) {
        ec.clear();
        if (!index_map_file(dir, index_)) {
            log::logw("Warning: Failed to parse source map file: ", dir);
            return false;
        }
        log::logi("  Loaded ", index_.size(), " source map entries from: ", dir);
        rebuild_module_index(index_, module_index_);
        return true;
    }

    if (!fs::is_directory(dir_path, ec)) {
        log::logw("Warning: Source map file or directory not found: ", dir);
        return false;
    }

    size_t before = index_.size();
    bool found_any = false;
    bool parse_ok = false;

    // 递归扫描目录：读取 "sourceMaps.map"（merge 文件）及所有 "*.map" 文件
    // （兼容多模块产物位于子目录的场景）
    for (const auto& entry : fs::recursive_directory_iterator(dir_path, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        const std::string filename = mem_analyzer::path_to_utf8(entry.path().filename());
        const std::string full_path = mem_analyzer::path_to_utf8(entry.path());

        bool is_map = filename == "sourceMaps.map";
        if (!is_map && mem_analyzer::path_to_utf8(entry.path().extension()) == ".map") {
            is_map = true;
        }
        if (!is_map) continue;

        found_any = true;
        if (index_map_file(full_path, index_)) {
            parse_ok = true;
        }
    }

    if (!found_any) {
        log::logw("Warning: No source map file (sourceMaps.map / *.map) found in: ", dir);
        return false;
    }
    if (!parse_ok) {
        log::logw("Warning: Failed to parse any source map file in: ", dir);
        return false;
    }

    log::logi("  Loaded ", (index_.size() - before), " source map entries from: ", dir);
    rebuild_module_index(index_, module_index_);
    return true;
}

const SourceMapEntry* SourceMapIndex::lookup(const std::string& key) const {
    auto it = index_.find(key);
    return (it == index_.end()) ? nullptr : &it->second;
}

const SourceMapEntry* SourceMapIndex::lookup_by_module(const std::string& module, const std::string& version,
                                                       const std::string& rel_path) const {
    const std::string mkey = module + "|" + version + "|" + rel_path;
    auto it = module_index_.find(mkey);
    return (it == module_index_.end()) ? nullptr : it->second;
}

bool SourceMapIndex::empty() const {
    return index_.empty();
}

} // namespace mem_analyzer
