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
//  source map v3 的 "mappings" 字段编码规则：
//    - 以 ';' 分隔行，以 ',' 分隔段（segment）
//    - 每个段由若干 VLQ 字段组成（相对值，需累积解码）：
//        [generatedColumn, sourceIndex, originalLine, originalColumn, nameIndex?]
//    - VLQ 编码：每 5 bit 一个 Base64 字符，最高位（0x20）为续位，
//      最低位（0x01）为符号位，数值右移 1 位后按符号取负
// ============================================================================

namespace {

constexpr int VLQ_BASE64_LOWERCASE_OFFSET = 26;
constexpr int VLQ_BASE64_DIGIT_OFFSET = 52;
constexpr int VLQ_BASE64_PLUS_VALUE = 62;
constexpr int VLQ_BASE64_SLASH_VALUE = 63;
constexpr int VLQ_BITS_PER_GROUP = 5;
constexpr int VLQ_MAX_SHIFT = 25;

constexpr size_t SEG_IDX_ORIGINAL_LINE = 2;
constexpr size_t SEG_IDX_ORIGINAL_COLUMN = 3;
constexpr size_t SEG_IDX_NAME_INDEX = 4;

/**
 * Base64 字符 → 6 位数值，非法字符返回 -1。
 */
int Base64Value(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + VLQ_BASE64_LOWERCASE_OFFSET;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + VLQ_BASE64_DIGIT_OFFSET;
    }
    if (c == '+') {
        return VLQ_BASE64_PLUS_VALUE;
    }
    if (c == '/') {
        return VLQ_BASE64_SLASH_VALUE;
    }
    return -1;
}

/**
 * 解码一个 VLQ 值，成功返回 true 并填充 value，失败返回 false。
 */
bool DecodeVlqValue(const std::string& str, size_t& pos, int& value)
{
    int result = 0;
    int shift = 0;
    bool continuation = true;
    while (continuation) {
        if (pos >= str.size()) {
            return false;
        }
        if (shift > VLQ_MAX_SHIFT) { // VLQ 最多 6 段（30 位），防有符号移位 UB
            return false;
        }
        int digit = Base64Value(str[pos++]);
        if (digit < 0) {
            return false;
        }
        continuation = (digit & 0x20) != 0;
        result += (digit & 0x1F) << shift;
        shift += VLQ_BITS_PER_GROUP;
    }
    bool negative = (result & 0x01) != 0;
    value = result >> 1;
    if (negative) {
        value = -value;
    }
    return true;
}

void AssignBestPosition(const SourceMapIndex::Segment& seg, int& originalLine, int& originalColumn)
{
    originalLine = seg[SEG_IDX_ORIGINAL_LINE] + 1;      // originalLine 0-based → 1-based
    originalColumn = seg[SEG_IDX_ORIGINAL_COLUMN] + 1;    // originalColumn 0-based → 1-based
}

} // anonymous namespace

SourceMapIndex::DecodedMappings SourceMapIndex::DecodeVlqMappings(const std::string& mappings)
{
    // source map v3 的 mappings 采用相对（delta）编码：段内每个字段相对
    // 前一段累积，解码时需按字段顺序逐项累加得到绝对值。解码结果：
    // 行映射列表（vector<vector<vector<int>>>），行数 = 分号数 + 1，
    // 段内字段依次为
    // [generatedColumn, sourceIndex, originalLine, originalColumn, nameIndex?]
    // 注意：segment 的实际字段数（1/4/5 个）无法从编码数据直接判断，
    // 必须按 ';'（行）与 ','（段）先切分出每个 segment 的原始子串，
    // 再解码该子串内的全部 VLQ 字段；若固定按 5 个字段读取，会跨段
    // 误读导致边界错乱、整行映射丢失。
    DecodedMappings decoded;
    decoded.emplace_back();  // 第 0 行

    // 相对值累加器（generatedColumn 换行重置，其余跨行保持）。
    // 用 64 位累加：单个 VLQ 值可达 ~2^30，连续累加会越过 INT_MAX 触发
    // 有符号溢出 UB（表现为行/列号变负、位置还原全乱）。
    auto clampInt = [](int64_t v) -> int {
        if (v > INT32_MAX) {
            return INT32_MAX;
        }
        if (v < INT32_MIN) {
            return INT32_MIN;
        }
        return static_cast<int>(v);
    };
    int64_t generatedColumn = 0;
    int64_t sourceIndex = 0;
    int64_t originalLine = 0;
    int64_t originalColumn = 0;
    int64_t nameIndex = 0;

    size_t pos = 0;
    size_t line = 0;
    while (pos < mappings.size()) {
        // 切出当前 segment 的原始子串（到 ',' / ';' / 末尾）
        const size_t end = mappings.find_first_of(";,", pos);
        const size_t segEnd = (end == std::string::npos) ? mappings.size() : end;
        if (segEnd > pos) {
            // 解码该 segment 内的全部 VLQ 字段（相对值）
            const std::string seg = mappings.substr(pos, segEnd - pos);
            std::vector<int> raw;
            size_t p = 0;
            while (p < seg.size()) {
                int value = 0;
                if (!DecodeVlqValue(seg, p, value)) {
                    break;
                }
                raw.push_back(value);
            }
            if (!raw.empty()) {
                // 将相对值累加为绝对值（64 位累加，输出时收敛到 int 范围）
                std::vector<int> segment;
                generatedColumn += raw[0];
                segment.push_back(clampInt(generatedColumn));
                if (raw.size() > 1) {
                    sourceIndex += raw[1];
                    segment.push_back(clampInt(sourceIndex));
                }
                if (raw.size() > SEG_IDX_ORIGINAL_LINE) {
                    originalLine += raw[SEG_IDX_ORIGINAL_LINE];
                    segment.push_back(clampInt(originalLine));
                }
                if (raw.size() > SEG_IDX_ORIGINAL_COLUMN) {
                    originalColumn += raw[SEG_IDX_ORIGINAL_COLUMN];
                    segment.push_back(clampInt(originalColumn));
                }
                if (raw.size() > SEG_IDX_NAME_INDEX) {
                    nameIndex += raw[SEG_IDX_NAME_INDEX];
                    segment.push_back(clampInt(nameIndex));
                }
                decoded[line].push_back(std::move(segment));
            }
        }
        if (end == std::string::npos) {
            break;
        }
        const char sep = mappings[end];
        pos = end + 1;
        if (sep == ';') {
            // 行结束：行号 +1，generatedColumn 重置
            decoded.emplace_back();
            line++;
            generatedColumn = 0;
        }
    }
    return decoded;
}

const SourceMapIndex::DecodedMappings& SourceMapIndex::GetDecodedMappings(const std::string& mappings) const
{
    auto it = decodedCache.find(mappings);
    if (it != decodedCache.end()) {
        return it->second;
    }
    // unordered_map 的插入/重哈希不会使已存元素的引用失效（仅 erase 失效），
    // 故返回的引用在解码缓存存续期间保持稳定。
    return decodedCache.emplace(mappings, DecodeVlqMappings(mappings)).first->second;
}

bool SourceMapIndex::ResolveOriginalLine(const std::string& mappings, int generatedLine, int& originalLine) const
{
    if (generatedLine < 1 || mappings.empty()) {
        return false;
    }
    const DecodedMappings& decoded = GetDecodedMappings(mappings);
    if (decoded.empty()) {
        return false;
    }

    // 官方算法（"异常堆栈解析原理"）：sourceMap 直接映射记录行，无行号补偿。
    // decoded 为 0 基行索引：decoded[i] 对应生成文件第 i+1 行，故记录行
    // generated_line 对应 decoded[generated_line - 1]；目标行无映射段时，
    // 按 source map v3 段覆盖语义向前回退到最近一个已映射行。
    int line = generatedLine - 1;
    if (line >= static_cast<int>(decoded.size())) {
        return false;
    }

    while (line >= 0 && decoded[line].empty()) {
        --line;
    }
    if (line < 0) {
        return false;
    }
    const LineSegments& segments = decoded[line];
    if (segments.empty() || segments.front().size() <= SEG_IDX_ORIGINAL_LINE) {
        return false;
    }
    originalLine = segments.front()[SEG_IDX_ORIGINAL_LINE] + 1;  // originalLine 0-based → 1-based
    return true;
}

/**
 * Search backward from startLine for the last segment with a valid mapping.
 */
static bool FindFallbackPosition(const SourceMapIndex::DecodedMappings& decoded,
    int startLine, int& originalLine, int& originalColumn, int* outSourceIndex)
{
    for (int l = startLine; l >= 0; --l) {
        if (decoded[l].empty()) {
            continue;
        }
        const SourceMapIndex::Segment* last = nullptr;
        for (const auto& seg : decoded[l]) {
            if (seg.size() > SEG_IDX_ORIGINAL_COLUMN) {
                last = &seg;
            }
        }
        if (last != nullptr) {
            originalLine = (*last)[SEG_IDX_ORIGINAL_LINE] + 1;
            originalColumn = (*last)[SEG_IDX_ORIGINAL_COLUMN] + 1;
            if (outSourceIndex != nullptr) {
                *outSourceIndex = (*last)[1];
            }
            return true;
        }
    }
    return false;
}

bool SourceMapIndex::ResolveOriginalPosition(const std::string& mappings, int generatedLine,
    int generatedColumn, int& originalLine,
    int& originalColumn, int* outSourceIndex) const
{
    if (generatedLine < 1 || mappings.empty()) {
        return false;
    }
    const DecodedMappings& decoded = GetDecodedMappings(mappings);
    if (decoded.empty()) {
        return false;
    }

    int line = generatedLine - 1;
    if (line >= static_cast<int>(decoded.size())) {
        return false;
    }

    const LineSegments* segments = &decoded[line];
    const Segment* best = nullptr;
    for (const auto& seg : *segments) {
        if (seg.size() <= SEG_IDX_ORIGINAL_COLUMN) {
            continue;
        }
        if (seg[0] <= generatedColumn) {
            best = &seg;
        } else {
            break;
        }
    }
    if (best == nullptr) {
        return FindFallbackPosition(decoded, line - 1, originalLine, originalColumn, outSourceIndex);
    }

    AssignBestPosition(*best, originalLine, originalColumn);
    if (outSourceIndex != nullptr) {
        *outSourceIndex = (*best)[1];
    }
    return true;
}

// ============================================================================
//  SourceMapIndex
// ============================================================================

namespace {

bool ParseJsonRoot(const std::string& content, json& root, const std::string& path)
{
    try {
        root = json::parse(content);
    } catch (const std::exception& e) {
        log::Logw("Warning: Failed to parse source map file ", path, ": ", e.what());
        return false;
    }
    return root.is_object();
}

/**
 * 从单个 map 文件解析顶层条目并入索引；失败返回 false。
 * 顶层键即混淆 key（如 "entry|entry|1.0.0|src/main/ets/pages/a.ts"），
 * 值含 sources/file/entry-package-info/package-info 字段。
 */
bool IndexMapFile(const std::string& path, std::unordered_map<std::string, SourceMapEntry>& index)
{
    // 用户路径可能含中文，需按 UTF-8 构造 fs::path（Windows 宽路径安全）
    std::ifstream ifs(mem_analyzer::PathFromUtf8(path), std::ios::binary);
    if (!ifs) {
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    json root;
    if (!ParseJsonRoot(ss.str(), root, path)) {
        return false;
    }

    for (auto it = root.begin(); it != root.end(); ++it) {
        const std::string& key = it.key();
        const json& value = it.value();
        if (!value.is_object()) {
            continue;
        }

        SourceMapEntry entry;
        // sources：可能是数组（source map v3 标准）或单字符串；完整保存数组，
        // 同时用 sources[0] 填充便捷字段 source_path
        if (value.contains("sources")) {
            const json& sources = value["sources"];
            if (sources.is_array()) {
                for (const auto& s : sources) {
                    if (s.is_string()) {
                        entry.sources.push_back(s.get<std::string>());
                    }
                }
                if (!entry.sources.empty()) {
                    entry.sourcePath = entry.sources.front();
                }
            } else if (sources.is_string()) {
                entry.sources.push_back(sources.get<std::string>());
                entry.sourcePath = entry.sources.front();
            }
        }
        if (value.contains("file") && value["file"].is_string()) {
            entry.file = value["file"].get<std::string>();
        }
        if (value.contains("entry-package-info") && value["entry-package-info"].is_string()) {
            entry.entryPackageInfo = value["entry-package-info"].get<std::string>();
        }
        if (value.contains("package-info") && value["package-info"].is_string()) {
            entry.packageInfo = value["package-info"].get<std::string>();
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

/**
 * 重建二级索引：module|version|rel_path → 条目。
 * 混淆 key 格式为 "<bundle>|<module>|<version>|<rel_path>"，
 * 以第 2 段 module（而非第 1 段 bundle）作为匹配键的一部分。
 */
void RebuildModuleIndex(const std::unordered_map<std::string, SourceMapEntry>& index,
    std::unordered_map<std::string, const SourceMapEntry*>& moduleIndex)
{
    moduleIndex.clear();
    for (const auto& kv : index) {
        const std::string& key = kv.first;
        const size_t sep1 = key.find('|');
        if (sep1 == std::string::npos) {
            continue;
        }
        const size_t sep2 = key.find('|', sep1 + 1);
        if (sep2 == std::string::npos) {
            continue;
        }
        const size_t sep3 = key.find('|', sep2 + 1);
        if (sep3 == std::string::npos) {
            continue;
        }
        // 关键修复：第 2 段是真实 module 名（如 @douyin/camera_impl），
        // 第 1 段是 bundle 名（如 entry），参与匹配会导致作用域包失配。
        const std::string mkey = key.substr(sep1 + 1, sep2 - sep1 - 1) + "|" +
                                 key.substr(sep2 + 1, sep3 - sep2 - 1) + "|" +
                                 key.substr(sep3 + 1);
        moduleIndex[mkey] = &kv.second;
    }
}

void LogLoadedEntries(const std::unordered_map<std::string, SourceMapEntry>& index,
    std::unordered_map<std::string, const SourceMapEntry*>& moduleIndex, size_t before,
    const std::string& dir)
{
    log::Logi("  Loaded ", (index.size() - before), " source map entries from: ", dir);
    RebuildModuleIndex(index, moduleIndex);
}

} // anonymous namespace

/**
 * Recursively scan a directory for source map files (*.map / sourceMaps.map).
 */
static bool ScanDirForMapFiles(const fs::path& dirPath,
    std::unordered_map<std::string, SourceMapEntry>& index, bool& foundAny)
{
    bool parseOk = false;
    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(dirPath, ec)) {
        if (ec) {
            break;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::string filename = mem_analyzer::PathToUtf8(entry.path().filename());
        const std::string fullPath = mem_analyzer::PathToUtf8(entry.path());
        bool isMap = filename == "sourceMaps.map";
        if (!isMap && mem_analyzer::PathToUtf8(entry.path().extension()) == ".map") {
            isMap = true;
        }
        if (!isMap) {
            continue;
        }
        foundAny = true;
        if (IndexMapFile(fullPath, index)) {
            parseOk = true;
        }
    }
    return parseOk;
}

bool SourceMapIndex::LoadFromDir(const std::string& dir)
{
    std::error_code ec;
    const fs::path dirPath = mem_analyzer::PathFromUtf8(dir);
    if (fs::is_regular_file(dirPath, ec)) {
        ec.clear();
        if (!IndexMapFile(dir, index_)) {
            log::Logw("Warning: Failed to parse source map file: ", dir);
            return false;
        }
        log::Logi("  Loaded ", index_.size(), " source map entries from: ", dir);
        RebuildModuleIndex(index_, moduleIndex);
        return true;
    }

    if (!fs::is_directory(dirPath, ec)) {
        log::Logw("Warning: Source map file or directory not found: ", dir);
        return false;
    }

    size_t before = index_.size();
    bool foundAny = false;
    bool parseOk = ScanDirForMapFiles(dirPath, index_, foundAny);

    if (!foundAny) {
        log::Logw("Warning: No source map file (sourceMaps.map / *.map) found in: ", dir);
        return false;
    }
    if (!parseOk) {
        log::Logw("Warning: Failed to parse any source map file in: ", dir);
        return false;
    }

    LogLoadedEntries(index_, moduleIndex, before, dir);
    return true;
}

const SourceMapEntry* SourceMapIndex::Lookup(const std::string& key) const
{
    auto it = index_.find(key);
    return (it == index_.end()) ? nullptr : &it->second;
}

const SourceMapEntry* SourceMapIndex::LookupByModule(const std::string& module, const std::string& version,
    const std::string& relPath) const
{
    const std::string mkey = module + "|" + version + "|" + relPath;
    auto it = moduleIndex.find(mkey);
    return (it == moduleIndex.end()) ? nullptr : it->second;
}

bool SourceMapIndex::Empty() const
{
    return index_.empty();
}

} // namespace mem_analyzer
