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
#include "shared/heapsnapshot/heap_snapshot.hpp"
#include "shared/heapsnapshot/node_size_calculator.hpp"
#include "utils/timer.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <regex>
#include <unordered_set>
#include "utils/file_utils.hpp"

using json = nlohmann::ordered_json;

// ============================================================================
//  heap_snapshot.cpp
//
//  V8 heap snapshot (.heapsnapshot) file parser.
//  Uses chunked streaming to avoid loading the entire JSON file into memory.
//  The file format is:
//    { "snapshot": { "meta": {...} }, "nodes": [n0,n1,...], "edges": [...],
//      "strings": ["str1","str2",...], "locations": [...] }
//
//  The nodes and edges arrays can be very large (hundreds of MB), so they
//  are parsed via ChunkedUintParser which processes 16 MB chunks at a time.
//  The strings table is parsed via JSON (nlohmann::json) with fallback
//  sanitization for unescaped control characters and orphan surrogates.
//
//  After parsing, the pipeline continues with:
//    5) Build forward CSR (first_edge_pointers) and node_id_to_idx map
//    6) Compute shortest paths (BFS from GC roots)
//    7) Compute dominator-based retained sizes (Lengauer-Tarjan algorithm)
// ============================================================================

namespace mem_analyzer {

namespace {

// 快速判断字符串是否为纯数字（内联）
inline bool is_numeric_string(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

} // anonymous namespace

// 获取 JSObject 扩展名
std::string HeapSnapshot::get_js_object_ext_name(uint32_t node_index) const {
    uint32_t ctor_node = 0xFFFFFFFF;
    std::string ctor_name;
    std::vector<std::string> prop_names;
    prop_names.reserve(10);
    std::unordered_set<std::string> added;
    added.reserve(16);

    size_t begin = first_edge_pointers[node_index];
    size_t end = first_edge_pointers[node_index + 1];
    for (size_t ptr = begin; ptr < end; ptr += edge_fields_count) {
        uint32_t name_idx = edges[ptr + edge_name_or_index_offset];
        if (name_idx >= strings.size()) continue;
        const std::string& en = strings[name_idx];

        // 查找 constructor
        if (en == "constructor") {
            uint32_t child = edges[ptr + edge_to_node_offset] / node_fields_count;
            if (child < static_cast<uint32_t>(node_count)) {
                ctor_node = child;
                ctor_name = get_node_name(child);
            }
            continue;
        }

        // 跳过系统属性和数字索引
        if (en == "(object elements)" || en == "(object properties)" ||
            en == "ArkInternalHash" || en == "hclass" || en == "__proto__" ||
            en == "Properties" || en == "Elements" || is_numeric_string(en) || 
            en == "HashField") {
            continue;
        }
        if (added.find(en) != added.end()) continue;
        if (prop_names.size() >= 10) break;
        prop_names.push_back(en);
        added.insert(en);
    }

    std::ostringstream oss;
    if (!ctor_name.empty()) {
        oss << "JSObject(ctor=" << ctor_name << ", ";
    } else {
        oss << "JSObject(";
    }
    uint32_t edge_count = get_node_edge_count(node_index);
    oss << "len=" << edge_count;
    if (!prop_names.empty()) { 
         oss << ", prop="; 
         for (size_t i = 0; i < prop_names.size(); ++i) { 
             if (i > 0) oss << ","; 
             oss << prop_names[i]; 
         } 
     }
    oss << ")";
    return oss.str();
}

// 获取 String 扩展名
std::string HeapSnapshot::get_string_ext_name(uint32_t node_index) const {
    static constexpr uint32_t kStringSize10KB = 10 * 1024;
    static constexpr uint32_t kStringSize1MB = 1024 * 1024;

    uint32_t base = node_index * node_fields_count;
    uint32_t size = nodes[base + node_self_size_offset];
    std::string content = get_node_name(node_index);
    if (content.length() > 20) content = content.substr(0, 20);

    // 按大小区间归类展示，避免精确 size 导致字符串对象过于分散
    const char* size_range = "<10KB";
    if (size >= kStringSize1MB) {
        size_range = ">=1MB";
    } else if (size >= kStringSize10KB) {
        size_range = ">=10KB && <1MB";
    }

    std::ostringstream oss;
    oss << "string(size: " << size_range << ", content=" << content << ")";
    return oss.str();
}

// 沿 hclass → Proto 交替上溯，提取 stateMgmt 包装对象背后被包装的类名。
// skip_markers：原型名命中其中任一子串即视为「仍是包装层」，跳过并继续上溯。
// 返回被包装的类名；返回空表示链上未找到可用类名。
std::string HeapSnapshot::resolve_state_mgmt_wrapped_class(
    uint32_t node_index, const std::vector<std::string>& skip_markers) const {
    std::unordered_set<uint32_t> visited;
    visited.reserve(16);
    uint32_t cur = node_index;
    std::string result_proto;
    bool looking_for_hclass = true;

    while (cur < static_cast<uint32_t>(node_count) && visited.insert(cur).second) {
        size_t begin = first_edge_pointers[cur];
        size_t end = first_edge_pointers[cur + 1];
        bool found = false;

        if (looking_for_hclass) {
            for (size_t ptr = begin; ptr < end; ptr += edge_fields_count) {
                uint32_t name_idx = edges[ptr + edge_name_or_index_offset];
                if (name_idx < strings.size() && strings[name_idx] == "hclass") {
                    cur = edges[ptr + edge_to_node_offset] / node_fields_count;
                    found = true;
                    looking_for_hclass = false;
                    break;
                }
            }
        } else {
            for (size_t ptr = begin; ptr < end; ptr += edge_fields_count) {
                uint32_t name_idx = edges[ptr + edge_name_or_index_offset];
                if (name_idx >= strings.size()) continue;
                const std::string& en = strings[name_idx];
                if (en == "Proto" || en == "__proto__") {
                    uint32_t proto = edges[ptr + edge_to_node_offset] / node_fields_count;
                    std::string proto_name = get_node_name(proto);
                    if (proto_name.find('#') != std::string::npos && proto_name.find(".ts") != std::string::npos) {
                        return proto_name;
                    }
                    bool is_wrapper = false;
                    for (const std::string& marker : skip_markers) {
                        if (proto_name.find(marker) != std::string::npos) {
                            is_wrapper = true;
                            break;
                        }
                    }
                    if (is_wrapper) {
                        result_proto.clear();
                    } else if (result_proto.empty()) {
                        result_proto = proto_name;
                    }
                    cur = proto;
                    looking_for_hclass = true;
                    found = true;
                    break;
                }
            }
        }
        if (!found) break;
    }
    return result_proto;
}

// 获取 stateMgmt.js#observedClass 全局对象的扩展名（沿原型链提取 ArkTS 类名）
std::string HeapSnapshot::get_observed_class_ext_name(uint32_t node_index) const {
    std::string result_proto =
        resolve_state_mgmt_wrapped_class(node_index, {"stateMgmt.js#observedClass"});

    if (!result_proto.empty()) {
        return "stateMgmt.js#observedClass(" + result_proto + ")";
    }

    std::string node_name = get_node_name(node_index);
    if (node_name.find('#') != std::string::npos && node_name.find(".js") != std::string::npos) {
        size_t hash = node_name.find('#');
        if (hash != std::string::npos && hash + 1 < node_name.size()) {
            return "stateMgmt.js#observedClass(" + node_name.substr(hash + 1) + ")";
        }
    }
    return node_name;
}

// 获取 declarative_frontend/stateMgmt.js#JSObject 的扩展名：与 observedClass 同一套路，
// 沿 hclass → Proto → hclass → Proto 交替上溯，用被包装的 ArkTS 类名替换掉 JSObject 本身。
// 链上找不到可用类名时，退回到「去掉 declarative_frontend/ 前缀」的旧行为。
std::string HeapSnapshot::get_state_mgmt_js_object_ext_name(uint32_t node_index,
                                                           const std::string& node_name) const {
    const std::string wrapped = resolve_state_mgmt_wrapped_class(
        node_index, {"stateMgmt.js#JSObject", "stateMgmt.js#observedClass"});
    if (!wrapped.empty()) {
        return "stateMgmt.js#JSObject(" + wrapped + ")";
    }
    size_t pos = node_name.find("stateMgmt.js");
    return (pos != std::string::npos) ? node_name.substr(pos) : node_name;
}

// declarative_frontend/stateMgmt.js 对象扩展名的统一入口：
// observedClass 沿原型链取 ArkTS 类名，JSObject 展开被包装的类，其余对象只去掉前缀。
std::string HeapSnapshot::get_state_mgmt_ext_name(uint32_t node_index,
                                                 const std::string& node_name) const {
    if (node_name.find("stateMgmt.js#observedClass") != std::string::npos) {
        return get_observed_class_ext_name(node_index);
    }
    if (node_name.find("stateMgmt.js#JSObject") != std::string::npos) {
        return get_state_mgmt_js_object_ext_name(node_index, node_name);
    }
    // 其余 stateMgmt 对象：去掉 declarative_frontend/ 前缀，保留 stateMgmt.js#后面的字符串
    size_t pos = node_name.find("stateMgmt.js");
    return (pos != std::string::npos) ? node_name.substr(pos) : node_name;
}

// 获取 SourceTextModule 扩展名
std::string HeapSnapshot::get_source_text_module_ext_name(uint32_t node_index) const {
    uint32_t local_export_entries_node = 0xFFFFFFFF;
    uint32_t name_dict_node = 0xFFFFFFFF;

    size_t begin = first_edge_pointers[node_index];
    size_t end = first_edge_pointers[node_index + 1];
    for (size_t ptr = begin; ptr < end; ptr += edge_fields_count) {
        uint32_t name_idx = edges[ptr + edge_name_or_index_offset];
        if (name_idx >= strings.size()) continue;
        const std::string& en = strings[name_idx];
        if (en == "LocalExportEntries") {
            local_export_entries_node = edges[ptr + edge_to_node_offset] / node_fields_count;
        } else if (en == "NameDictionary") {
            name_dict_node = edges[ptr + edge_to_node_offset] / node_fields_count;
        }
        if (local_export_entries_node != 0xFFFFFFFF && name_dict_node != 0xFFFFFFFF) break;
    }

    std::string module_name;
    if (name_dict_node != 0xFFFFFFFF && name_dict_node < static_cast<uint32_t>(node_count)) {
        size_t d_begin = first_edge_pointers[name_dict_node];
        size_t d_end = first_edge_pointers[name_dict_node + 1];
        for (size_t ptr = d_begin; ptr < d_end; ptr += edge_fields_count) {
            uint32_t child = edges[ptr + edge_to_node_offset] / node_fields_count;
            if (child < static_cast<uint32_t>(node_count)) {
                std::string child_name = get_node_name(child);
                if (child_name.find('#') != std::string::npos) {
                    module_name = child_name.substr(0, child_name.find('#'));
                    break;
                }
            }
        }
    }

    uint32_t export_count = 0;
    if (local_export_entries_node != 0xFFFFFFFF && local_export_entries_node < static_cast<uint32_t>(node_count)) {
        export_count = get_node_edge_count(local_export_entries_node);
        if (export_count > 0) export_count -= 1;
    }

    std::vector<std::string> export_names;
    export_names.reserve(4);
    if (local_export_entries_node != 0xFFFFFFFF && local_export_entries_node < static_cast<uint32_t>(node_count)) {
        size_t e_begin = first_edge_pointers[local_export_entries_node];
        size_t e_end = first_edge_pointers[local_export_entries_node + 1];
        for (size_t ptr = e_begin; ptr < e_end && export_names.size() < 4; ptr += edge_fields_count) {
            uint32_t name_idx = edges[ptr + edge_name_or_index_offset];
            if (name_idx < strings.size() && is_numeric_string(strings[name_idx])) {
                uint32_t entry_node = edges[ptr + edge_to_node_offset] / node_fields_count;
                if (entry_node >= static_cast<uint32_t>(node_count)) continue;
                size_t e2_begin = first_edge_pointers[entry_node];
                size_t e2_end = first_edge_pointers[entry_node + 1];
                for (size_t ptr2 = e2_begin; ptr2 < e2_end; ptr2 += edge_fields_count) {
                    uint32_t ename_idx = edges[ptr2 + edge_name_or_index_offset];
                    if (ename_idx < strings.size() && strings[ename_idx] == "ExportName") {
                        uint32_t export_name_node = edges[ptr2 + edge_to_node_offset] / node_fields_count;
                        if (export_name_node < static_cast<uint32_t>(node_count)) {
                            std::string ename = get_node_name(export_name_node);
                            if (ename == "default" || ename.empty()) {
                                if (name_dict_node != 0xFFFFFFFF) {
                                    size_t d_begin2 = first_edge_pointers[name_dict_node];
                                    size_t d_end2 = first_edge_pointers[name_dict_node + 1];
                                    for (size_t ptr3 = d_begin2; ptr3 < d_end2; ptr3 += edge_fields_count) {
                                        uint32_t key_idx = edges[ptr3 + edge_name_or_index_offset];
                                        if (key_idx < strings.size() && (strings[key_idx] == "0" || strings[key_idx] == "1")) {
                                            uint32_t target_node = edges[ptr3 + edge_to_node_offset] / node_fields_count;
                                            if (target_node < static_cast<uint32_t>(node_count)) {
                                                std::string ext_name = get_js_object_ext_name(target_node);
                                                // get_js_object_ext_name 总是返回 "JSObject(...)"（:98/:100），
                                                // 裸 "JSObject" 永远不会出现；按前缀判断才能真正跳过无信息对象。
                                                if (ext_name.rfind("JSObject(", 0) != 0 &&
                                                    ext_name != "JSSharedObject" &&
                                                    ext_name.rfind("JSSharedObject(", 0) != 0) {
                                                    export_names.push_back(ext_name);
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                export_names.push_back(ename);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    std::ostringstream oss;
    oss << "SourceTextModule(nums=" << export_count;
    if (!module_name.empty()) {
        oss << ", module=" << module_name;
    } else {
        oss << ", module=None";
    }
    if (!export_names.empty()) {
        oss << ", names=";
        for (size_t i = 0; i < export_names.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << export_names[i];
        }
    }
    oss << ")";
    return oss.str();
}

// 获取节点的扩展名称
// 计算单个节点的扩展名（不查缓存、不递归调用 get_extended_node_name）。
std::string HeapSnapshot::compute_extended_name(uint32_t node_index) const {
    std::string result;
    std::string node_name = get_node_name(node_index);
    std::string node_type = get_node_type_name(node_index);

    // 先按类型分流：string 节点的 node_name 就是字符串内容本身，若内容恰好含
    // "declarative_frontend/stateMgmt.js"（例如堆里保存的模块 URL），会被误判成
    // stateMgmt 对象，进而改变 property-tag / compare / 引用链的分组键。
    if (node_type != "string" &&
        node_name.find("declarative_frontend/stateMgmt.js") != std::string::npos) {
        result = get_state_mgmt_ext_name(node_index, node_name);
    } else if (node_type == "string") {
        result = get_string_ext_name(node_index);
    } else if (node_type == "object" &&
               (node_name == "JSObject" || node_name == "JSSharedObject" || node_name == "js_shared_object")) {
        result = get_js_object_ext_name(node_index);
    } else if (node_type == "native" &&
               (node_name == "source_text_module_record" || node_name == "SourceTextModule")) {
        result = get_source_text_module_ext_name(node_index);
    } else {
        result = node_name;
    }
    return result;
}

// 获取节点的扩展名称
const std::string& HeapSnapshot::get_extended_node_name(uint32_t node_index) const {
    if (node_index >= static_cast<uint32_t>(node_count)) {
        static const std::string empty;
        return empty;
    }

    // 一次性并行预构建全量扩展名，之后并行阶段无锁只读。
    std::call_once(*ext_names_once_, [this]() {
        const auto t0 = std::chrono::steady_clock::now();
        log::logd("[ref_chain] build ext_names start node_count=", node_count);

        ext_names_.resize(static_cast<size_t>(node_count));
        const int T = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
        const int N = node_count;
        std::vector<std::thread> workers;
        workers.reserve(T);
        for (int t = 0; t < T; ++t) {
            const int b = static_cast<int>((static_cast<long long>(N) * t) / T);
            const int e = static_cast<int>((static_cast<long long>(N) * (t + 1)) / T);
            workers.emplace_back([this, b, e]() {
                for (int i = b; i < e; ++i) {
                    ext_names_[static_cast<size_t>(i)] =
                        compute_extended_name(static_cast<uint32_t>(i));
                }
            });
        }
        for (auto& w : workers) w.join();

        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - t0)
                            .count();
        log::logd("[ref_chain] build ext_names done (", ms, "ms)");
    });
    return ext_names_[node_index];
}

// ============================================================================
//  Constants
// ============================================================================

constexpr size_t CHUNK_SIZE = 16 * 1024 * 1024;   // 16 MB — chunk size for nodes/edges parsing
constexpr size_t META_CHUNK = 64 * 1024;           // 64 KB — chunk size for metadata/string search
constexpr size_t META_LIMIT = 512 * 1024;          // max bytes to scan for file header

// ============================================================================
//  ChunkedUintParser — streaming parser for flat JSON numeric arrays
//
//  Parses JSON arrays like [123,456,789,...] in 16 MB chunks without
//  loading the entire array into memory as a string.  This is critical
//  for large heapsnapshot files where the nodes/edges arrays can be
//  hundreds of millions of integers.
//
//  The parser handles chunk boundaries by saving the partial number at
//  the end of each chunk as "overflow" and prepending it to the next chunk.
//  When the closing ']' is found, the parser finishes.
// ============================================================================

class ChunkedUintParser {
public:
    std::vector<uint32_t> result;   // accumulated parsed integers
    bool done_ = false;             // true when closing ']' has been seen
    std::string overflow_;          // partial number straddling a chunk boundary

    /// Feed one chunk of text. When ']' appears the parser finishes.
    void feed(const std::string& chunk_text) {
        if (done_) return;
        std::string text = overflow_ + chunk_text;
        overflow_.clear();

        // Check if this chunk contains the closing bracket
        size_t r_bracket = text.find(']');
        if (r_bracket != std::string::npos) {
            // Append a trailing comma to flush the last number, then parse
            std::string complete = text.substr(0, r_bracket) + ',';
            parse_numbers_from_string(complete, result);
            done_ = true;
            return;
        }

        // No ']' — save everything after the last ',' as overflow.
        // The number before the last comma is safe to parse because the
        // trailing comma terminates it.  Without the comma, the number
        // would be lost, causing a -1 shift at every chunk boundary.
        size_t last_comma = text.rfind(',');
        if (last_comma != std::string::npos) {
            std::string complete = text.substr(0, last_comma + 1);
            overflow_ = text.substr(last_comma + 1);
            parse_numbers_from_string(complete, result);
        } else {
            // Entire chunk may be part of a single number (unlikely but possible)
            overflow_ = text;
        }
    }

    /// Must be called after the last chunk to flush pending overflow.
    void done() {
        if (done_) return;
        if (!overflow_.empty()) {
            parse_numbers_from_string(overflow_, result);
            overflow_.clear();
        }
        done_ = true;
    }

    bool is_done() const { return done_; }

private:
    /// Parse comma-separated unsigned integers from a string.
    /// Non-digit characters (commas, whitespace, brackets) terminate numbers.
    static void parse_numbers_from_string(const std::string& s, std::vector<uint32_t>& out) {
        uint32_t num = 0;
        bool in_num = false;
        for (size_t i = 0; i < s.size(); i++) {
            char c = s[i];
            if (c >= '0' && c <= '9') {
                const uint32_t digit = static_cast<uint32_t>(c - '0');
                // 溢出检查：num * 10 + digit 超出 uint32 范围即视为损坏/恶意输入。
                if (num > (UINT32_MAX - digit) / 10) {
                    throw std::runtime_error(
                        "heapsnapshot: numeric array contains an integer exceeding uint32");
                }
                num = num * 10 + digit;
                in_num = true;
            } else if (in_num) {
                out.push_back(num);
                num = 0;
                in_num = false;
            }
        }
    }
};

// ============================================================================
//  File reading helpers
// ============================================================================

struct HeaderResult {
    std::string buf;            // accumulated header buffer
    size_t nodes_marker_pos;    // byte offset of "\"nodes\":" in buf
};

/// Read file header in META_CHUNK-sized chunks until both "\"snapshot\":"
/// and "\"nodes\":" markers are found.  Returns the accumulated buffer
/// and the position of the nodes marker.
/// Throws if markers are not found within META_LIMIT bytes.
static HeaderResult read_header(std::ifstream& file, size_t file_size) {
    size_t limit = std::min<size_t>(file_size, META_LIMIT);
    std::string acc;
    size_t pos = 0;

    while (pos < limit) {
        size_t chunk_size = std::min<size_t>(META_CHUNK, limit - pos);
        std::string chunk(chunk_size, '\0');
        file.read(&chunk[0], chunk_size);
        size_t bytes_read = file.gcount();
        if (bytes_read == 0) break;

        acc.append(chunk.data(), bytes_read);
        pos += bytes_read;

        size_t nodes_idx = acc.find("\"nodes\":");
        if (nodes_idx != std::string::npos && acc.find("\"snapshot\":") != std::string::npos) {
            return {acc, nodes_idx};
        }
    }
    throw std::runtime_error("cannot find \"snapshot\" and \"nodes\" fields in file header");
}

/// Stream-parse a flat numeric JSON array from file starting at byte_offset.
/// Uses ChunkedUintParser with CHUNK_SIZE (16 MB) chunks.
static std::vector<uint32_t> parse_array_chunked(std::ifstream& file, size_t byte_offset, size_t file_size) {
    ChunkedUintParser parser;
    std::string read_buf(CHUNK_SIZE, '\0');
    size_t pos = byte_offset;

    while (pos < file_size) {
        size_t to_read = std::min<size_t>(CHUNK_SIZE, file_size - pos);
        file.seekg(pos);
        file.read(&read_buf[0], to_read);
        size_t bytes_read = file.gcount();
        if (bytes_read == 0) break;

        parser.feed(std::string(read_buf.data(), bytes_read));
        pos += bytes_read;
        if (parser.is_done()) break;
    }

    parser.done();
    return std::move(parser.result);
}

/// Search for a marker string in the file, scanning in META_CHUNK-sized windows.
/// To avoid missing markers that straddle chunk boundaries, each window
/// overlaps the previous one by (marker.length() - 1) bytes.
/// Returns the absolute byte offset of the marker, or std::string::npos.
static size_t find_marker_in_file(std::ifstream& file, const std::string& marker,
                                   size_t start_pos, size_t file_size) {
    std::string read_buf(META_CHUNK, '\0');
    file.clear();  // ensure stream is in good state before scanning
    size_t pos = start_pos;

    while (pos < file_size) {
        size_t to_read = std::min<size_t>(META_CHUNK, file_size - pos);
        file.seekg(pos);
        file.read(&read_buf[0], to_read);
        size_t bytes_read = file.gcount();
        if (bytes_read == 0) break;

        std::string view(read_buf.data(), bytes_read);
        size_t idx = view.find(marker);
        if (idx != std::string::npos) return pos + idx;

        // Overlap windows to catch markers that straddle boundaries
        if (pos + bytes_read < file_size) {
            pos += bytes_read;
            if (pos >= marker.length()) pos -= (marker.length() - 1);
        } else {
            break;
        }
    }
    return std::string::npos;
}

/// Find the position of '[' that starts a JSON array after a given marker.
/// e.g., find_array_bracket_after(file, "\"nodes\":", start_pos, file_size)
/// returns the byte offset of the '[' after the marker.
static size_t find_array_bracket_after(std::ifstream& file, const std::string& marker,
                                        size_t start_pos, size_t file_size) {
    size_t mark_pos = find_marker_in_file(file, marker, start_pos, file_size);
    if (mark_pos == std::string::npos) return std::string::npos;

    std::string buf(256, '\0');
    size_t search_start = mark_pos + marker.size();
    file.seekg(search_start);
    file.read(&buf[0], 256);
    size_t bytes = file.gcount();
    std::string view(buf.data(), bytes);
    size_t bracket = view.find('[');
    if (bracket == std::string::npos) return std::string::npos;
    return search_start + bracket;
}

// ============================================================================
//  Strings array parsing
//
//  The strings table is a JSON array of strings: ["str1","str2",...].
//  Unlike the nodes/edges arrays, it's parsed via JSON (nlohmann::json)
//  because the values are strings, not numbers.
//
//  Some heapsnapshot files contain unescaped control characters (U+0000-
//  U+001F) or orphan surrogate escapes (\uD8xx-\uDFxx) in string values,
//  which nlohmann::json rejects by spec.  We handle this with a fallback
//  sanitization pass that escapes control characters and replaces orphan
//  surrogates with U+FFFD.
// ============================================================================

/// Find the matching closing ']' for an opening '[' at position start.
/// Correctly handles string literals so ']' inside strings is not mistaken
/// for the array terminator.  Works for flat JSON arrays of strings
/// (no nested objects/arrays).
static size_t find_matching_close_bracket(const std::string& text, size_t start) {
    if (start >= text.size() || text[start] != '[') return std::string::npos;
    size_t pos = start + 1;
    while (pos < text.size()) {
        char c = text[pos];
        if (c == '"') {
            // Skip string literal — handle escape sequences
            pos++;
            while (pos < text.size()) {
                if (text[pos] == '\\') { pos += 2; continue; }  // skip escaped char
                if (text[pos] == '"') break;                     // end of string
                pos++;
            }
            if (pos >= text.size()) return std::string::npos;   // unterminated string
            pos++;
        } else if (c == ']') {
            return pos;  // found matching bracket
        } else {
            pos++;
        }
    }
    return std::string::npos;
}

/// Sanitize a JSON string array buffer for nlohmann::json compatibility.
/// Two transformations are applied:
///   1. Control characters (U+0000-U+001F, except \t \n \r) are escaped as \u00XX
///   2. Orphan surrogate pairs (\uD800-\uDFFF) are replaced with \uFFFD
/// The sanitization is done in-place on a copy; the original buffer is unchanged.
static std::string sanitize_json_string_array(const std::string& str_buf) {
    std::string sanitized;
    sanitized.reserve(str_buf.size());
    size_t i = 0;
    while (i < str_buf.size()) {
        char c = str_buf[i];
        if (c == '"') {
            // Inside a string literal — copy until closing quote
            sanitized += c;
            i++;
            while (i < str_buf.size()) {
                char sc = str_buf[i];
                if (sc == '\\') {
                    // Check for orphan surrogate: \uD[89AB]xx or \uD[CDEF]xx
                    if (i + 5 < str_buf.size() && str_buf[i+1] == 'u') {
                        char h1 = str_buf[i+2] | 0x20;  // lowercase
                        char h2 = str_buf[i+3];
                        if (h1 == 'd' && ((h2 >= '8' && h2 <= 'f') || (h2 >= 'A' && h2 <= 'F'))) {
                            sanitized += "\\uFFFD";  // replace with replacement character
                            i += 6;
                            continue;
                        }
                    }
                    sanitized += sc;
                    i++;
                    if (i < str_buf.size()) { sanitized += str_buf[i]; i++; }
                    continue;
                }
                if (sc == '"') {
                    sanitized += sc;
                    i++;
                    break;  // end of string literal
                }
                // Escape control characters (except \t, \n, \r)
                unsigned char uc = static_cast<unsigned char>(sc);
                if (uc < 0x20 && uc != '\t' && uc != '\n' && uc != '\r') {
                    const char* hex = "0123456789abcdef";
                    sanitized += "\\u00";
                    sanitized += hex[(uc >> 4) & 0xF];
                    sanitized += hex[uc & 0xF];
                    i++;
                } else {
                    sanitized += sc;
                    i++;
                }
            }
        } else {
            sanitized += c;
            i++;
        }
    }
    return sanitized;
}

/// Read and parse the strings table from file.
/// The strings array is located at bracket_pos and parsed via JSON.
/// If the initial parse fails, the buffer is sanitized and retried.
/// If that also fails, an empty string table is returned as fallback.
static std::vector<std::string> read_strings_array(std::ifstream& file, size_t bracket_pos, size_t file_size) {
    // Read up to 1 MB (META_CHUNK * 16) for the strings array
    size_t remaining = file_size - bracket_pos;
    size_t read_size = std::min<size_t>(remaining, META_CHUNK * 16);
    std::string str_buf(read_size, '\0');
    file.clear();
    file.seekg(bracket_pos);
    file.read(&str_buf[0], read_size);
    size_t bytes_read = file.gcount();
    str_buf.resize(bytes_read);

    // Find the closing bracket to trim the buffer to exact array content
    size_t close_bracket = find_matching_close_bracket(str_buf, 0);
    if (close_bracket != std::string::npos && close_bracket + 1 < str_buf.size()) {
        str_buf.resize(close_bracket + 1);
    } else if (close_bracket == std::string::npos && bytes_read == read_size && read_size < remaining) {
        // Strings array is larger than 1 MB — read the entire rest of the file
        std::string rest_buf(remaining, '\0');
        file.clear();
        file.seekg(bracket_pos);
        file.read(&rest_buf[0], remaining);
        size_t rest_bytes = file.gcount();
        rest_buf.resize(rest_bytes);
        size_t full_close = find_matching_close_bracket(rest_buf, 0);
        if (full_close != std::string::npos) rest_buf.resize(full_close + 1);
        str_buf = std::move(rest_buf);
    }

    // First attempt: direct JSON parse
    try {
        return json::parse(str_buf).get<std::vector<std::string>>();
    } catch (const std::exception&) {
        // Second attempt: sanitize and retry
        std::string sanitized = sanitize_json_string_array(str_buf);
        try {
            return json::parse(sanitized).get<std::vector<std::string>>();
        } catch (const std::exception&) {
            // 不再静默回退成空表：空 strings 会让所有节点名/分类/引用链/总量
            // 静默失效，报告"看起来成功"却毫无分析价值。宁可显式失败。
            throw std::runtime_error("failed to parse snapshot strings table");
        }
    }
}

// ============================================================================
//  Phase 1: parse snapshot metadata
//
//  Extracts the "snapshot" JSON object from the file header and discovers:
//    - node_fields / edge_fields arrays → field offsets
//    - edge_types / node_types arrays → numeric type values
//  Also returns the byte position of the nodes array bracket for Phase 2.
// ============================================================================
static void parse_snapshot_meta(HeapSnapshot& snapshot,
                                 const std::string& head_buf,
                                 size_t nodes_marker_pos,
                                 size_t& nodes_array_bracket) {
    // Locate the snapshot metadata JSON between "snapshot": and "nodes":
    size_t snap_idx = head_buf.find("\"snapshot\":");
    nodes_array_bracket = head_buf.find('[', nodes_marker_pos + 8);
    size_t meta_end = head_buf.rfind('}', nodes_array_bracket);
    if (meta_end == std::string::npos) {
        throw std::runtime_error("cannot find snapshot metadata end");
    }
    meta_end += 1;

    // Extract and trim the JSON string
    std::string snapshot_json_str = head_buf.substr(snap_idx + 11, meta_end - (snap_idx + 11));
    const size_t first_nonspace = snapshot_json_str.find_first_not_of(" \t\r\n");
    if (first_nonspace != std::string::npos) {
        snapshot_json_str.erase(0, first_nonspace);
    } else {
        snapshot_json_str.clear();
    }
    const size_t last_nonspace = snapshot_json_str.find_last_not_of(" \t\r\n");
    if (last_nonspace != std::string::npos) {
        snapshot_json_str.erase(last_nonspace + 1);
    } else {
        snapshot_json_str.clear();
    }

    json snapshot_meta;
    try {
        snapshot_meta = json::parse(snapshot_json_str);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("failed to parse snapshot metadata: ") + e.what());
    }

    auto meta = snapshot_meta["meta"];
    auto node_fields = meta["node_fields"].get<std::vector<std::string>>();
    auto edge_fields = meta["edge_fields"].get<std::vector<std::string>>();

    snapshot.node_fields_count = static_cast<int>(node_fields.size());
    snapshot.edge_fields_count = static_cast<int>(edge_fields.size());
    if (snapshot.node_fields_count <= 0) {
        throw std::runtime_error("invalid snapshot metadata: empty node_fields");
    }

    // Discover node field offsets by name
    // Typical V8 node_fields: ["type","name","id","self_size","edge_count","trace_node_id","detachedness"]
    for (int i = 0; i < static_cast<int>(node_fields.size()); i++) {
        if (node_fields[i] == "type") snapshot.node_type_offset = i;
        else if (node_fields[i] == "name") snapshot.node_name_offset = i;
        else if (node_fields[i] == "id") snapshot.node_id_offset = i;
        else if (node_fields[i] == "self_size") snapshot.node_self_size_offset = i;
        else if (node_fields[i] == "native_size") snapshot.node_native_size_offset = i;
        else if (node_fields[i] == "edge_count") snapshot.node_edge_count_offset = i;
    }

    // Discover edge field offsets by name
    // Typical V8 edge_fields: ["type","name_or_index","to_node"]
    for (int i = 0; i < static_cast<int>(edge_fields.size()); i++) {
        if (edge_fields[i] == "type") snapshot.edge_type_offset = i;
        else if (edge_fields[i] == "name_or_index") snapshot.edge_name_or_index_offset = i;
        else if (edge_fields[i] == "to_node") snapshot.edge_to_node_offset = i;
    }

    // 元数据完整性校验：缺失必填字段会让下游用 -1（转成 SIZE_MAX）当数组下标越界读，
    // edge_fields 为空则会在边解析处除以 0。畸形/异构生成器产出的快照挡在这里。
    if (snapshot.edge_fields_count <= 0) {
        throw std::runtime_error("invalid snapshot metadata: empty edge_fields");
    }
    if (snapshot.node_type_offset < 0 || snapshot.node_name_offset < 0 ||
        snapshot.node_id_offset < 0 || snapshot.node_self_size_offset < 0 ||
        snapshot.node_edge_count_offset < 0) {
        throw std::runtime_error(
            "invalid snapshot metadata: missing required node field "
            "(type/name/id/self_size/edge_count)");
    }
    if (snapshot.edge_type_offset < 0 || snapshot.edge_name_or_index_offset < 0 ||
        snapshot.edge_to_node_offset < 0) {
        throw std::runtime_error(
            "invalid snapshot metadata: missing required edge field (type/name_or_index/to_node)");
    }

    // Resolve edge type numeric values from meta.edge_types[type_offset]
    // Typical V8 edge types: ["context","element","property","internal","hidden","shortcut","weak"]
    snapshot.edge_weak_type = -1;
    snapshot.edge_shortcut_type = -1;
    snapshot.edge_hidden_type = -1;
    snapshot.edge_element_type = -1;
    snapshot.edge_internal_type = -1;
    if (meta.contains("edge_types") && snapshot.edge_type_offset >= 0) {
        auto edge_types = meta["edge_types"][snapshot.edge_type_offset];
        if (edge_types.is_array()) {
            auto types = edge_types.get<std::vector<std::string>>();
            for (int i = 0; i < static_cast<int>(types.size()); i++) {
                if (types[i] == "weak") snapshot.edge_weak_type = i;
                else if (types[i] == "shortcut") snapshot.edge_shortcut_type = i;
                else if (types[i] == "hidden") snapshot.edge_hidden_type = i;
                else if (types[i] == "element") snapshot.edge_element_type = i;
                else if (types[i] == "internal") snapshot.edge_internal_type = i;
            }
        }
    }

    // Parse node type names from meta.node_types[type_offset]
    // Typical V8 node types: ["hidden","array","string","object","code","closure",...]
    if (meta.contains("node_types") && snapshot.node_type_offset >= 0) {
        auto node_types = meta["node_types"][snapshot.node_type_offset];
        if (node_types.is_array()) {
            snapshot.node_type_names = node_types.get<std::vector<std::string>>();
        }
    }
}

// ============================================================================
//  Phase 2: parse nodes array
//
//  The nodes array is a flat list of uint32 values, grouped by node_fields_count.
//  Each node occupies node_fields_count consecutive integers.
//  Parsed via ChunkedUintParser to handle large arrays without loading the
//  entire JSON string into memory.
// ============================================================================
static void parse_nodes_array(HeapSnapshot& snapshot, std::ifstream& file,
                               size_t nodes_array_bracket, size_t file_size) {
    snapshot.nodes = parse_array_chunked(file, nodes_array_bracket + 1, file_size);
    size_t nc = snapshot.nodes.size() / snapshot.node_fields_count;
    if (nc > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error("malformed heapsnapshot: node count exceeds INT_MAX");
    snapshot.node_count = static_cast<int>(nc);
}

// ============================================================================
//  Phase 3: parse edges array
//
//  The edges array is a flat list of uint32 values, grouped by edge_fields_count.
//  The edges marker may be in the header buffer or further in the file
//  (after the nodes array).  We try the header first, then scan forward.
//  Returns the bracket position for use in the strings phase.
// ============================================================================
static size_t parse_edges_array(HeapSnapshot& snapshot, std::ifstream& file,
                                 const std::string& head_buf,
                                 size_t nodes_array_bracket, size_t file_size) {
    size_t edges_bracket_pos = std::string::npos;

    // First try: header buffer (edges may be close to the beginning)
    size_t edges_marker_pos = head_buf.find("\"edges\":");
    if (edges_marker_pos != std::string::npos) {
        size_t bracket = head_buf.find('[', edges_marker_pos + 8);
        if (bracket != std::string::npos) edges_bracket_pos = bracket;
    }

    // Second try: scan forward from after the nodes array
    if (edges_bracket_pos == std::string::npos) {
        // Estimate: nodes array occupies roughly nodes.size() * 2 bytes (JSON text)
        size_t search_start = nodes_array_bracket + snapshot.nodes.size() * 2;
        edges_bracket_pos = find_array_bracket_after(file, "\"edges\":", search_start, file_size);
        if (edges_bracket_pos == std::string::npos) {
            throw std::runtime_error("cannot find \"edges\" field");
        }
    }

    snapshot.edges = parse_array_chunked(file, edges_bracket_pos + 1, file_size);
    size_t ec = snapshot.edges.size() / snapshot.edge_fields_count;
    if (ec > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error("malformed heapsnapshot: edge count exceeds INT_MAX");
    snapshot.edge_count = static_cast<int>(ec);
    return edges_bracket_pos;
}

// ============================================================================
//  Phase 4: parse strings table
//
//  The strings table is a JSON array of strings, typically located after
//  the edges array.  Three search passes are used:
//    Pass 1: header buffer
//    Pass 2: tail of file (strings often sit at the very end)
//    Pass 3: forward scan from after edges
// ============================================================================
static void parse_strings_table(HeapSnapshot& snapshot, std::ifstream& file,
                                 const std::string& head_buf,
                                 size_t edges_bracket_pos, size_t file_size) {
    size_t strings_bracket_pos = std::string::npos;

    // Clear stream state (may have failbit from earlier reads)
    file.clear();
    file.seekg(0);

    // Pass 1: header buffer
    size_t strings_marker_pos = head_buf.find("\"strings\":");
    if (strings_marker_pos != std::string::npos) {
        size_t bracket = head_buf.find('[', strings_marker_pos + 10);
        if (bracket != std::string::npos) strings_bracket_pos = bracket;
    }

    // Pass 2: tail of file (strings array is often the last field)
    if (strings_bracket_pos == std::string::npos && file_size > META_LIMIT) {
        size_t tail_read_size = std::min<size_t>(file_size, META_LIMIT);
        std::string tail_buf(tail_read_size, '\0');
        file.clear();
        file.seekg(file_size - tail_read_size);
        file.read(&tail_buf[0], tail_read_size);
        size_t bytes = file.gcount();
        tail_buf.resize(bytes);

        strings_marker_pos = tail_buf.find("\"strings\":");
        if (strings_marker_pos != std::string::npos) {
            size_t bracket = tail_buf.find('[', strings_marker_pos + 10);
            if (bracket != std::string::npos) {
                strings_bracket_pos = (file_size - tail_read_size) + bracket;
            }
        }
    }

    // Pass 3: forward scan from after edges
    if (strings_bracket_pos == std::string::npos) {
        size_t search_start = std::max<size_t>(META_LIMIT, edges_bracket_pos + 1);
        size_t mark_pos = find_marker_in_file(file, "\"strings\":", search_start, file_size);
        if (mark_pos != std::string::npos) {
            std::string buf(256, '\0');
            file.clear();
            file.seekg(mark_pos + 10);
            file.read(&buf[0], 256);
            size_t bytes = file.gcount();
            std::string view(buf.data(), bytes);
            size_t bracket = view.find('[');
            if (bracket != std::string::npos) strings_bracket_pos = mark_pos + 10 + bracket;
        }
    }

    if (strings_bracket_pos != std::string::npos) {
        snapshot.strings = read_strings_array(file, strings_bracket_pos, file_size);
    }
}

// ============================================================================
//  Phase 5: build CSR indices + node_id map
//
//  Builds two indexing structures:
//    - first_edge_pointers: CSR (Compressed Sparse Row) for forward edges.
//      For node i, edges start at first_edge_pointers[i] and end at
//      first_edge_pointers[i+1].  Each edge occupies edge_fields_count uint32s.
//    - node_id_to_idx: unordered_map from node id to node index.
//      Used for O(1) lookup of nodes by their numeric id.
// ============================================================================
static void build_csr_and_id_map(HeapSnapshot& snapshot) {
    // ---- Build forward CSR ----
    // The total number of edge fields is the sum of edge_count * edge_fields_count
    // for each node.  We iterate nodes and accumulate the edge pointer.
    snapshot.first_edge_pointers.resize(snapshot.node_count + 1, 0);
    // 用 64 位累加，避免 4G 个字段以上的无符号回绕；累加结果必须与解析到的
    // edges 数组完全一致 —— 截断的文件或 edge_count 与实际数组不符的生成器，
    // 否则会让所有消费方越界读 edges，把垃圾当成图结构、静默产出错误报告。
    uint64_t total_edge_fields = 0;
    for (int i = 0; i < snapshot.node_count; i++) {
        snapshot.first_edge_pointers[i] = static_cast<uint32_t>(total_edge_fields);
        total_edge_fields +=
            static_cast<uint64_t>(
                snapshot.nodes[i * snapshot.node_fields_count + snapshot.node_edge_count_offset]) *
            static_cast<uint64_t>(snapshot.edge_fields_count);
    }
    snapshot.first_edge_pointers[snapshot.node_count] = static_cast<uint32_t>(total_edge_fields);
    if (total_edge_fields != static_cast<uint64_t>(snapshot.edges.size())) {
        throw std::runtime_error(
            "invalid snapshot: edges array size (" + std::to_string(snapshot.edges.size()) +
            ") does not match the sum of per-node edge counts (" +
            std::to_string(total_edge_fields) + ")");
    }
    // 一次遍历校验所有 to_node 在范围内；不在每个 for_each_edge 里加分支
    // （边数千万级，逐边检查会拖慢分析）。
    for (size_t ptr = 0; ptr < snapshot.edges.size();
         ptr += static_cast<size_t>(snapshot.edge_fields_count)) {
        const uint32_t to_node =
            snapshot.edges[ptr + static_cast<size_t>(snapshot.edge_to_node_offset)] /
            static_cast<uint32_t>(snapshot.node_fields_count);
        if (to_node >= static_cast<uint32_t>(snapshot.node_count)) {
            throw std::runtime_error("invalid snapshot: edge target " + std::to_string(to_node) +
                                     " exceeds node count " + std::to_string(snapshot.node_count));
        }
    }

    // ---- Build node_id_to_idx map ----
    for (int i = 0; i < snapshot.node_count; i++) {
        if (snapshot.node_id_offset >= 0) {
            uint32_t id = snapshot.nodes[i * snapshot.node_fields_count + snapshot.node_id_offset];
            snapshot.node_id_to_idx[id] = static_cast<uint32_t>(i);
        }
    }
}

// ============================================================================
//  Phase 6: compute shortest paths + retained sizes
//
//  Runs the two main analysis passes:
//    - compute_shortest_paths: BFS from GC roots to find the shortest
//      reference chain to every reachable node
//    - compute_node_sizes: LT dominator algorithm to compute retained sizes
// ============================================================================
static void compute_paths_and_retained(HeapSnapshot& snapshot) {
    // Shortest paths BFS: exclude weak, shortcut, and hidden edges
    {
        auto paths = compute_shortest_paths(snapshot, true, true, true);
        snapshot.path_parent_index.resize(snapshot.node_count);
        snapshot.path_edge_start.resize(snapshot.node_count);
        constexpr uint32_t INVALID = 0xFFFFFFFF;
        for (uint32_t i = 0; i < static_cast<uint32_t>(snapshot.node_count); i++) {
            snapshot.path_parent_index[i] = paths.path_edges[i].parent_node_index;
            snapshot.path_edge_start[i] = paths.path_edges[i].edge_field_start;
        }
    }

    // Dominator-based retained sizes: exclude weak and shortcut edges
    {
        auto ret = compute_node_sizes(snapshot, true, true);
        snapshot.retained_sizes = std::move(ret.retained_sizes);
        snapshot.dominators = std::move(ret.dominators);
        snapshot.reachable = std::move(ret.reachable);
    }
}

// ============================================================================
//  HeapSnapshot::from_file — main entry point
//
//  Orchestrates the six-phase pipeline:
//    1. Parse snapshot metadata (field offsets, type values)
//    2. Stream-parse nodes array
//    3. Stream-parse edges array
//    4. Parse strings table (JSON with fallback sanitization)
//    5. Build CSR indices and node_id_to_idx map
//    6. Compute shortest paths and retained sizes
// ============================================================================

HeapSnapshot HeapSnapshot::from_file(const std::string& path) {
    // Open file in binary mode, seek to end to get file size
    auto file = open_ifstream(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open file: " + path);
    }
    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0);

    StageTimer timer("HeapSnapshot");
    HeapSnapshot snapshot;

    // ---- Phase 1: metadata ----
    timer.stage("Reading header");
    auto header = read_header(file, file_size);
    size_t nodes_array_bracket = 0;
    parse_snapshot_meta(snapshot, header.buf, header.nodes_marker_pos, nodes_array_bracket);
    timer.tick("meta_parsed");

    // ---- Phase 2: nodes ----
    timer.stage("Parsing nodes array");
    parse_nodes_array(snapshot, file, nodes_array_bracket, file_size);
    timer.tick("nodes", std::to_string(snapshot.node_count) + " nodes");
    // 空 nodes 数组会让下游 compute_shortest_paths 对空 vector 执行 path_edges[0] = ...
    if (snapshot.node_count <= 0) {
        throw std::runtime_error("invalid snapshot: nodes array is empty");
    }

    // ---- Phase 3: edges ----
    timer.stage("Parsing edges array");
    size_t edges_bracket_pos = parse_edges_array(snapshot, file, header.buf,
                                                  nodes_array_bracket, file_size);
    timer.tick("edges", std::to_string(snapshot.edge_count) + " edges");

    // ---- Phase 4: strings ----
    timer.stage("Parsing strings table");
    parse_strings_table(snapshot, file, header.buf, edges_bracket_pos, file_size);
    timer.tick("strings", std::to_string(snapshot.strings.size()) + " strings");

    // ---- Phase 5: CSR + node_id_to_idx ----
    timer.stage("Building CSR indices");
    build_csr_and_id_map(snapshot);
    timer.tick("csr");
    timer.stage("Building node_id_to_idx");
    timer.tick("node_id_map", std::to_string(snapshot.node_id_to_idx.size()) + " entries");

    // ---- Phase 5.5: locate SyntheticRoot once for O(1) get_total_heap_size (#27) ----
    snapshot.synthetic_root_index = UINT32_MAX;
    for (int i = 0; i < snapshot.node_count; i++) {
        uint32_t ui = static_cast<uint32_t>(i);
        if (snapshot.get_node_name(ui) == "SyntheticRoot") {
            snapshot.synthetic_root_index = ui;
            break;
        }
    }

    // ---- Phase 6: shortest paths + retained sizes ----
    timer.stage("Computing shortest paths");
    compute_paths_and_retained(snapshot);
    timer.tick("shortest_paths");
    timer.stage("Computing retained sizes");
    timer.tick("retained_sizes");

    timer.done();
    log::logok("  Heap snapshot loaded: ", snapshot.node_count, " nodes, ",
               snapshot.edge_count, " edges, ", snapshot.strings.size(), " strings.");
    return snapshot;
}

// ============================================================================
//  HeapSnapshot member functions — node/edge accessors
// ============================================================================

HeapSnapshot::NodeView HeapSnapshot::get_node_by_id(uint32_t id) const {
    auto it = node_id_to_idx.find(id);
    if (it == node_id_to_idx.end()) {
        return {0xFFFFFFFF, 0, "", 0};  // sentinel: not found
    }
    uint32_t node_index = it->second;
    uint32_t base = node_index * node_fields_count;
    uint32_t name_idx = nodes[base + node_name_offset];
    std::string name = (name_idx < strings.size()) ? strings[name_idx] : "";
    return {node_index, id, std::move(name), nodes[base + node_self_size_offset]};
}

std::string HeapSnapshot::get_node_name(uint32_t node_index) const {
    if (node_index >= static_cast<uint32_t>(node_count)) return "";
    uint32_t base = node_index * static_cast<uint32_t>(node_fields_count);
    uint32_t name_idx = nodes[base + static_cast<size_t>(node_name_offset)];
    return (name_idx < strings.size()) ? strings[name_idx] : "";
}

std::string HeapSnapshot::get_node_type_name(uint32_t node_index) const {
    if (node_index >= static_cast<uint32_t>(node_count)) return "";
    if (node_type_names.empty()) return "";
    uint32_t base = node_index * static_cast<uint32_t>(node_fields_count);
    uint32_t type_idx = nodes[base + static_cast<size_t>(node_type_offset)];
    return (type_idx < node_type_names.size()) ? node_type_names[type_idx] : "";
}

uint32_t HeapSnapshot::get_node_edge_count(uint32_t node_index) const {
    if (node_index >= static_cast<uint32_t>(node_count)) return 0;
    uint32_t base = node_index * static_cast<uint32_t>(node_fields_count);
    return nodes[base + static_cast<size_t>(node_edge_count_offset)];
}

/// Extract the ArkInternalHash value from a node's edges.
/// ArkTS (HarmonyOS) stores internal hash values in edge names matching
/// "ArkInternalHash", with the target node name containing "Int: <hash>".
uint32_t HeapSnapshot::get_node_internal_hash(uint32_t node_index) const {
    if (node_index >= static_cast<uint32_t>(node_count)) return 0;
    uint32_t hash = 0;
    static const std::regex pattern(R"(Int:\s*(\d+))");
    std::smatch matches;

    for_each_edge(node_index, [&](const EdgeInfo &edge, size_t) {
        if (edge.name_or_index < static_cast<uint32_t>(strings.size())) {
            std::string edge_name = strings[edge.name_or_index];
            if (edge_name == "ArkInternalHash") {
                std::string target_node_name = get_node_name(edge.to_node);
                if (std::regex_search(target_node_name, matches, pattern)) {
                    try {
                        hash = std::stoi(matches[1].str());
                    } catch (const std::exception&) {
                        hash = 0;  // 越界/非法 ArkInternalHash 值按 0 处理
                    }
                }
            }
        }
    });
    return hash;
}

/// Format the shortest reference chain from a GC root to the given node.
/// Walks up via path_parent_index (set by compute_shortest_paths).
/// Root nodes have path_parent_index[root] == root (self-reference).
/// Unreachable nodes have path_parent_index == INVALID.
/// Format: "RootNode -> ChildNode -> TargetNode"
std::string HeapSnapshot::format_ref_chain(uint32_t node_index) const {
    constexpr uint32_t INVALID = 0xFFFFFFFF;
    
    // 先收集节点索引，避免在收集过程中反复访问名称
    std::vector<uint32_t> indices;
    indices.reserve(32); // 预留常见链长度
    
    uint32_t cur = node_index;
    uint32_t guard = 0;
    while (cur < static_cast<uint32_t>(node_count)) {
        if (++guard > static_cast<uint32_t>(node_count)) {
            log::logw("[ref_chain] WARN cycle in path_parent_index at node_index=", node_index);
            return "";
        }
        uint32_t parent = path_parent_index[cur];
        if (parent == INVALID) {
            return ""; // unreachable
        }
        indices.push_back(cur);
        if (parent == cur) break; // 到达根节点
        cur = parent;
    }
    
    if (indices.empty()) return "";
    
    // 预计算最终字符串长度，避免多次 realloc
    size_t total_len = 0;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        // 直接获取节点名称，避免临时拷贝
        const std::string& name = get_extended_node_name(*it);
        total_len += name.size();
        if (it != indices.rbegin()) total_len += 4; // " -> "
    }
    
    std::string result;
    result.reserve(total_len);
    
    bool first = true;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        if (!first) result += " -> ";
        first = false;
        result += get_extended_node_name(*it);
    }
    return result;
}

std::vector<std::string> HeapSnapshot::format_ref_chain_frames(uint32_t node_index) const {
    constexpr uint32_t INVALID = 0xFFFFFFFF;

    std::vector<uint32_t> indices;
    indices.reserve(32);

    uint32_t cur = node_index;
    uint32_t guard = 0;
    while (cur < static_cast<uint32_t>(node_count)) {
        if (++guard > static_cast<uint32_t>(node_count)) {
            log::logw("[ref_chain] WARN cycle in path_parent_index at node_index=", node_index);
            return {};
        }
        uint32_t parent = path_parent_index[cur];
        if (parent == INVALID) {
            return {};  // unreachable
        }
        indices.push_back(cur);
        if (parent == cur) break;  // 到达根节点
        cur = parent;
    }

    if (indices.empty()) return {};

    // 反向遍历，从根到目标，只取扩展名，不做 " -> " 拼接。
    std::vector<std::string> frames;
    frames.reserve(indices.size());
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        frames.push_back(get_extended_node_name(*it));
    }
    return frames;
}

namespace {
// FNV-1a（lo）/ FNV-1（hi）双路 128-bit 链 hash 的种子与继续。
// 方向为「正向」（root → node）：H(child) = fnv_continue(H(parent), name[child])。
constexpr uint64_t FNV1A_OFFSET = 14695981039346656037ULL;
constexpr uint64_t FNV1A_PRIME = 1099511628211ULL;
constexpr uint64_t FNV1_SEED = 0x8CF9DB0E64FD9E9BULL;
constexpr uint64_t FNV1_PRIME = 0x9E3779B97F4A7C15ULL;
constexpr uint8_t FNV_SEP = 0x1Fu;

RefChainKey fnv_seed(const std::string& name) {
    uint64_t h1 = FNV1A_OFFSET;
    uint64_t h2 = FNV1_SEED;
    for (unsigned char c : name) {
        h1 ^= c;
        h1 *= FNV1A_PRIME;
        h2 *= FNV1_PRIME;
        h2 ^= c;
    }
    return RefChainKey{h1, h2};
}

RefChainKey fnv_continue(const RefChainKey& prev, const std::string& name) {
    uint64_t h1 = prev.lo;
    uint64_t h2 = prev.hi;
    // 先喂分隔符，再喂子节点名（避免 "AB"+"C" vs "A"+"BC" 边界碰撞）。
    h1 ^= FNV_SEP;
    h1 *= FNV1A_PRIME;
    h2 *= FNV1_PRIME;
    h2 ^= FNV_SEP;
    for (unsigned char c : name) {
        h1 ^= c;
        h1 *= FNV1A_PRIME;
        h2 *= FNV1_PRIME;
        h2 ^= c;
    }
    return RefChainKey{h1, h2};
}
} // anonymous namespace

RefChainKey HeapSnapshot::ref_chain_hash(uint32_t node_index) const {
    constexpr uint32_t INVALID = 0xFFFFFFFF;
    if (node_index >= static_cast<uint32_t>(node_count)) return RefChainKey{};

    // 一次性预构建「正向链 hash」表（root → node），之后 O(1) 查表。
    std::call_once(*chain_hash_once_, [this]() {
        const auto t0 = std::chrono::steady_clock::now();
        log::logd("[ref_chain] build chain_hash start node_count=", node_count);

        const int N = node_count;
        if (N > 0) get_extended_node_name(0);  // 触发 ext_names_ 预构建

        chain_hash_.resize(static_cast<size_t>(N));
        chain_hash_computed_.assign(static_cast<size_t>(N), 0);

        // 复用 scratch：沿父链反向收集未算节点，再反转回填（天然正向拓扑序）。
        std::vector<uint32_t> path;
        path.reserve(64);
        const size_t cycle_limit = static_cast<size_t>(N) + 1;

        for (int ni = 0; ni < N; ++ni) {
            const uint32_t node = static_cast<uint32_t>(ni);
            if (chain_hash_computed_[node]) continue;

            path.clear();
            uint32_t cur = node;
            while (!chain_hash_computed_[cur] && path.size() < cycle_limit) {
                path.push_back(cur);
                const uint32_t parent = path_parent_index[cur];
                if (parent == INVALID || parent == cur) break;
                cur = parent;
            }
            if (path.size() >= cycle_limit) {
                log::logw("[ref_chain] WARN cycle in path_parent_index at node_index=", node);
            }

            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                const uint32_t u = *it;
                const uint32_t p = path_parent_index[u];
                if (p == INVALID || p >= static_cast<uint32_t>(N)) {
                    chain_hash_[u] = RefChainKey{};  // 不可达 => {0,0}
                } else if (p == u) {
                    chain_hash_[u] = fnv_seed(ext_names_[u]);  // root：种子
                } else if (chain_hash_computed_[p]) {
                    chain_hash_[u] = fnv_continue(chain_hash_[p], ext_names_[u]);
                } else {
                    chain_hash_[u] = RefChainKey{};  // p 未算（环/异常）兜底
                }
                chain_hash_computed_[u] = 1;
            }
        }

        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - t0)
                            .count();
        log::logd("[ref_chain] build chain_hash done (", ms, "ms)");
    });

    return chain_hash_[node_index];
}

} // namespace mem_analyzer