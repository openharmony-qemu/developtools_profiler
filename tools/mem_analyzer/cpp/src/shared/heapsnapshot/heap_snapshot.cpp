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

constexpr int NAME_SET_INITIAL_CAPACITY = 16;
constexpr int MAX_EXPORT_NAMES = 4;
constexpr int BRACKET_SEARCH_BUF_SIZE = 256;
constexpr int STRINGS_SEARCH_BUF_SIZE = 256;
constexpr int REF_CHAIN_PATH_RESERVE = 64;
constexpr int REF_CHAIN_INDICES_RESERVE = 32;
constexpr int UNICODE_ESCAPE_LEN = 6;
constexpr int NODE_NAME_DISPLAY_MAX_LEN = 20;
constexpr int NODES_MARKER_LEN = 8;
constexpr int STRINGS_MARKER_LEN = 10;
constexpr int UNICODE_ESCAPE_TAIL_LEN = 5;
constexpr int MAX_PROP_NAMES = 10;
constexpr int VISITED_SET_INITIAL_CAPACITY = 16;
constexpr int ARROW_SEPARATOR_LEN = 4;
constexpr int ESCAPE_CHAR_SKIP_LEN = 2;
constexpr int HEX_UPPER_NIBBLE_SHIFT = 4;

// 快速判断字符串是否为纯数字（内联）
inline bool IsNumericString(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

} // anonymous namespace

// 获取 JSObject 扩展名
std::string HeapSnapshot::GetJsObjectExtName(uint32_t nodeIndex) const
{
    uint32_t ctorNode = 0xFFFFFFFF;
    std::string ctorName;
    std::vector<std::string> propNames;
    propNames.reserve(MAX_PROP_NAMES);
    std::unordered_set<std::string> added;
    added.reserve(NAME_SET_INITIAL_CAPACITY);

    size_t begin = firstEdgePointers[nodeIndex];
    size_t end = firstEdgePointers[nodeIndex + 1];
    for (size_t ptr = begin; ptr < end; ptr += edgeFieldsCount) {
        uint32_t nameIdx = edges[ptr + edgeNameOrIndexOffset];
        if (nameIdx >= strings.size()) {
            continue;
        }
        const std::string& en = strings[nameIdx];

        // 查找 constructor
        if (en == "constructor") {
            uint32_t child = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
            if (child < static_cast<uint32_t>(nodeCount)) {
                ctorNode = child;
                ctorName = GetNodeName(child);
            }
            continue;
        }

        // 跳过系统属性和数字索引
        if (en == "(object elements)" || en == "(object properties)" ||
            en == "ArkInternalHash" || en == "hclass" || en == "__proto__" ||
            en == "Properties" || en == "Elements" || IsNumericString(en) ||
            en == "HashField") {
            continue;
        }
        if (added.find(en) != added.end()) {
            continue;
        }
        if (propNames.size() >= MAX_PROP_NAMES) {
            break;
        }
        propNames.push_back(en);
        added.insert(en);
    }

    std::ostringstream oss;
    if (!ctorName.empty()) {
        oss << "JSObject(ctor=" << ctorName << ", ";
    } else {
        oss << "JSObject(";
    }
    uint32_t edgeCount = GetNodeEdgeCount(nodeIndex);
    oss << "len=" << edgeCount;
    if (!propNames.empty()) {
        oss << ", prop=";
        for (size_t i = 0; i < propNames.size(); ++i) {
            if (i > 0) {
                oss << ",";
            }
            oss << propNames[i];
        }
    }
    oss << ")";
    return oss.str();
}

// 获取 String 扩展名
std::string HeapSnapshot::GetStringExtName(uint32_t nodeIndex) const
{
    static constexpr uint32_t STRING_SIZE_10KB = 10 * 1024;
    static constexpr uint32_t STRING_SIZE_1MB = 1024 * 1024;

    uint32_t base = nodeIndex * nodeFieldsCount;
    uint32_t size = nodes[base + nodeSelfSizeOffset];
    std::string content = GetNodeName(nodeIndex);
    if (content.length() > NODE_NAME_DISPLAY_MAX_LEN) {
        content = content.substr(0, NODE_NAME_DISPLAY_MAX_LEN);
    }

    // 按大小区间归类展示，避免精确 size 导致字符串对象过于分散
    const char* sizeRange = "<10KB";
    if (size >= STRING_SIZE_1MB) {
        sizeRange = ">=1MB";
    } else if (size >= STRING_SIZE_10KB) {
        sizeRange = ">=10KB && <1MB";
    }

    std::ostringstream oss;
    oss << "string(size: " << sizeRange << ", content=" << content << ")";
    return oss.str();
}

// 沿 hclass → Proto 交替上溯，提取 stateMgmt 包装对象背后被包装的类名。
// skip_markers：原型名命中其中任一子串即视为「仍是包装层」，跳过并继续上溯。
// 返回被包装的类名；返回空表示链上未找到可用类名。
std::string HeapSnapshot::ResolveStateMgmtWrappedClass(
    uint32_t nodeIndex, const std::vector<std::string>& skipMarkers) const
{
    std::unordered_set<uint32_t> visited;
    visited.reserve(VISITED_SET_INITIAL_CAPACITY);
    uint32_t cur = nodeIndex;
    std::string resultProto;
    bool lookingForHclass = true;

    while (cur < static_cast<uint32_t>(nodeCount) && visited.insert(cur).second) {
        size_t begin = firstEdgePointers[cur];
        size_t end = firstEdgePointers[cur + 1];
        bool found = false;

        if (lookingForHclass) {
            for (size_t ptr = begin; ptr < end; ptr += edgeFieldsCount) {
                uint32_t nameIdx = edges[ptr + edgeNameOrIndexOffset];
                if (nameIdx < strings.size() && strings[nameIdx] == "hclass") {
                    cur = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
                    found = true;
                    lookingForHclass = false;
                    break;
                }
            }
        } else {
            for (size_t ptr = begin; ptr < end; ptr += edgeFieldsCount) {
                uint32_t nameIdx = edges[ptr + edgeNameOrIndexOffset];
                if (nameIdx >= strings.size()) {
                    continue;
                }
                const std::string& en = strings[nameIdx];
                if (en == "Proto" || en == "__proto__") {
                    uint32_t proto = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
                    std::string protoName = GetNodeName(proto);
                    if (protoName.find('#') != std::string::npos && protoName.find(".ts") != std::string::npos) {
                        return protoName;
                    }
                    bool isWrapper = false;
                    for (const std::string& marker : skipMarkers) {
                        if (protoName.find(marker) != std::string::npos) {
                            isWrapper = true;
                            break;
                        }
                    }
                    if (isWrapper) {
                        resultProto.clear();
                    } else if (resultProto.empty()) {
                        resultProto = protoName;
                    }
                    cur = proto;
                    lookingForHclass = true;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            break;
        }
    }
    return resultProto;
}

// 获取 stateMgmt.js#observedClass 全局对象的扩展名（沿原型链提取 ArkTS 类名）
std::string HeapSnapshot::GetObservedClassExtName(uint32_t nodeIndex) const
{
    std::string resultProto = ResolveStateMgmtWrappedClass(nodeIndex, {"stateMgmt.js#observedClass"});
    if (!resultProto.empty()) {
        return "stateMgmt.js#observedClass(" + resultProto + ")";
    }

    std::string nodeName = GetNodeName(nodeIndex);
    if (nodeName.find('#') != std::string::npos && nodeName.find(".js") != std::string::npos) {
        size_t hash = nodeName.find('#');
        if (hash != std::string::npos && hash + 1 < nodeName.size()) {
            return "stateMgmt.js#observedClass(" + nodeName.substr(hash + 1) + ")";
        }
    }
    return nodeName;
}

// 获取 declarative_frontend/stateMgmt.js#JSObject 的扩展名：与 observedClass 同一套路，
// 沿 hclass → Proto → hclass → Proto 交替上溯，用被包装的 ArkTS 类名替换掉 JSObject 本身。
// 链上找不到可用类名时，退回到「去掉 declarative_frontend/ 前缀」的旧行为。
std::string HeapSnapshot::GetStateMgmtJsObjectExtName(uint32_t nodeIndex, const std::string& nodeName) const
{
    const std::string wrapped = ResolveStateMgmtWrappedClass(
        nodeIndex, {"stateMgmt.js#JSObject", "stateMgmt.js#observedClass"});
    if (!wrapped.empty()) {
        return "stateMgmt.js#JSObject(" + wrapped + ")";
    }
    size_t pos = nodeName.find("stateMgmt.js");
    return (pos != std::string::npos) ? nodeName.substr(pos) : nodeName;
}

// declarative_frontend/stateMgmt.js 对象扩展名的统一入口：
// observedClass 沿原型链取 ArkTS 类名，JSObject 展开被包装的类，其余对象只去掉前缀。
std::string HeapSnapshot::GetStateMgmtExtName(uint32_t nodeIndex, const std::string& nodeName) const
{
    if (nodeName.find("stateMgmt.js#observedClass") != std::string::npos) {
        return GetObservedClassExtName(nodeIndex);
    }
    if (nodeName.find("stateMgmt.js#JSObject") != std::string::npos) {
        return GetStateMgmtJsObjectExtName(nodeIndex, nodeName);
    }
    // 其余 stateMgmt 对象：去掉 declarative_frontend/ 前缀，保留 stateMgmt.js#后面的字符串
    size_t pos = nodeName.find("stateMgmt.js");
    return (pos != std::string::npos) ? nodeName.substr(pos) : nodeName;
}

// 获取 SourceTextModule 扩展名
std::string HeapSnapshot::GetSourceTextModuleExtName(uint32_t nodeIndex) const
{
    uint32_t localExportEntriesNode = 0xFFFFFFFF;
    uint32_t nameDictNode = 0xFFFFFFFF;

    size_t begin = firstEdgePointers[nodeIndex];
    size_t end = firstEdgePointers[nodeIndex + 1];
    for (size_t ptr = begin; ptr < end; ptr += edgeFieldsCount) {
        uint32_t nameIdx = edges[ptr + edgeNameOrIndexOffset];
        if (nameIdx >= strings.size()) {
            continue;
        }
        const std::string& en = strings[nameIdx];
        if (en == "LocalExportEntries") {
            localExportEntriesNode = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
        } else if (en == "NameDictionary") {
            nameDictNode = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
        }
        if (localExportEntriesNode != 0xFFFFFFFF && nameDictNode != 0xFFFFFFFF) {
            break;
        }
    }

    std::string moduleName;
    if (nameDictNode != 0xFFFFFFFF && nameDictNode < static_cast<uint32_t>(nodeCount)) {
        size_t dBegin = firstEdgePointers[nameDictNode];
        size_t dEnd = firstEdgePointers[nameDictNode + 1];
        for (size_t ptr = dBegin; ptr < dEnd; ptr += edgeFieldsCount) {
            uint32_t child = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
            if (child < static_cast<uint32_t>(nodeCount)) {
                std::string childName = GetNodeName(child);
                if (childName.find('#') != std::string::npos) {
                    moduleName = childName.substr(0, childName.find('#'));
                    break;
                }
            }
        }
    }

    uint32_t exportCount = 0;
    if (localExportEntriesNode != 0xFFFFFFFF && localExportEntriesNode < static_cast<uint32_t>(nodeCount)) {
        exportCount = GetNodeEdgeCount(localExportEntriesNode);
        if (exportCount > 0) {
            exportCount -= 1;
        }
    }

    std::vector<std::string> exportNames;
    exportNames.reserve(MAX_EXPORT_NAMES);
    if (localExportEntriesNode != 0xFFFFFFFF && localExportEntriesNode < static_cast<uint32_t>(nodeCount)) {
        size_t eBegin = firstEdgePointers[localExportEntriesNode];
        size_t eEnd = firstEdgePointers[localExportEntriesNode + 1];
        for (size_t ptr = eBegin; ptr < eEnd && exportNames.size() < MAX_EXPORT_NAMES; ptr += edgeFieldsCount) {
            uint32_t nameIdx = edges[ptr + edgeNameOrIndexOffset];
            if (nameIdx < strings.size() && IsNumericString(strings[nameIdx])) {
                uint32_t entryNode = edges[ptr + edgeToNodeOffset] / nodeFieldsCount;
                if (entryNode >= static_cast<uint32_t>(nodeCount)) {
                    continue;
                }
                size_t e2Begin = firstEdgePointers[entryNode];
                size_t e2End = firstEdgePointers[entryNode + 1];
                for (size_t ptr2 = e2Begin; ptr2 < e2End; ptr2 += edgeFieldsCount) {
                    uint32_t enameIdx = edges[ptr2 + edgeNameOrIndexOffset];
                    if (enameIdx < strings.size() && strings[enameIdx] == "ExportName") {
                        uint32_t exportNameNode = edges[ptr2 + edgeToNodeOffset] / nodeFieldsCount;
                        if (exportNameNode < static_cast<uint32_t>(nodeCount)) {
                            std::string ename = GetNodeName(exportNameNode);
                            if (ename == "default" || ename.empty()) {
                                if (nameDictNode != 0xFFFFFFFF) {
                                    size_t dBegin2 = firstEdgePointers[nameDictNode];
                                    size_t dEnd2 = firstEdgePointers[nameDictNode + 1];
                                    for (size_t ptr3 = dBegin2; ptr3 < dEnd2; ptr3 += edgeFieldsCount) {
                                        uint32_t keyIdx = edges[ptr3 + edgeNameOrIndexOffset];
                                        if (keyIdx < strings.size() && (strings[keyIdx] == "0" || strings[keyIdx] == "1")) {
                                            uint32_t targetNode = edges[ptr3 + edgeToNodeOffset] / nodeFieldsCount;
                                            if (targetNode < static_cast<uint32_t>(nodeCount)) {
                                                std::string extName = GetJsObjectExtName(targetNode);
                                                // get_js_object_ext_name 总是返回 "JSObject(...)"（:98/:100），
                                                // 裸 "JSObject" 永远不会出现；按前缀判断才能真正跳过无信息对象。
                                                if (extName.rfind("JSObject(", 0) != 0 &&
                                                    extName != "JSSharedObject" &&
                                                    extName.rfind("JSSharedObject(", 0) != 0) {
                                                    exportNames.push_back(extName);
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                exportNames.push_back(ename);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    std::ostringstream oss;
    oss << "SourceTextModule(nums=" << exportCount;
    if (!moduleName.empty()) {
        oss << ", module=" << moduleName;
    } else {
        oss << ", module=None";
    }
    if (!exportNames.empty()) {
        oss << ", names=";
        for (size_t i = 0; i < exportNames.size(); ++i) {
            if (i > 0) {
                oss << ", ";
            }
            oss << exportNames[i];
        }
    }
    oss << ")";
    return oss.str();
}

// 获取节点的扩展名称
// 计算单个节点的扩展名（不查缓存、不递归调用 get_extended_node_name）。
std::string HeapSnapshot::ComputeExtendedName(uint32_t nodeIndex) const
{
    std::string result;
    std::string nodeName = GetNodeName(nodeIndex);
    std::string nodeType = GetNodeTypeName(nodeIndex);
    // 先按类型分流：string 节点的 node_name 就是字符串内容本身，若内容恰好含
    // "declarative_frontend/stateMgmt.js"（例如堆里保存的模块 URL），会被误判成
    // stateMgmt 对象，进而改变 property-tag / compare / 引用链的分组键。
    if (nodeType != "string" && nodeName.find("declarative_frontend/stateMgmt.js") != std::string::npos) {
        result = GetStateMgmtExtName(nodeIndex, nodeName);
    } else if (nodeType == "string") {
        result = GetStringExtName(nodeIndex);
    } else if (nodeType == "object" &&
               (nodeName == "JSObject" || nodeName == "JSSharedObject" || nodeName == "js_shared_object")) {
        result = GetJsObjectExtName(nodeIndex);
    } else if (nodeType == "native" &&
               (nodeName == "source_text_module_record" || nodeName == "SourceTextModule")) {
        result = GetSourceTextModuleExtName(nodeIndex);
    } else {
        result = nodeName;
    }
    return result;
}

// 获取节点的扩展名称
const std::string& HeapSnapshot::GetExtendedNodeName(uint32_t nodeIndex) const
{
    if (nodeIndex >= static_cast<uint32_t>(nodeCount)) {
        static const std::string empty;
        return empty;
    }

    // 一次性并行预构建全量扩展名，之后并行阶段无锁只读。
    std::call_once(*extNamesOnce_, [this]() {
        const auto t0 = std::chrono::steady_clock::now();
        log::Logd("[ref_chain] build ext_names start node_count=", nodeCount);

        extNames_.resize(static_cast<size_t>(nodeCount));
        const int T = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
        const int N = nodeCount;
        std::vector<std::thread> workers;
        workers.reserve(T);
        for (int t = 0; t < T; ++t) {
            const int b = static_cast<int>((static_cast<long long>(N) * t) / T);
            const int e = static_cast<int>((static_cast<long long>(N) * (t + 1)) / T);
            workers.emplace_back([this, b, e]() {
                for (int i = b; i < e; ++i) {
                    extNames_[static_cast<size_t>(i)] = ComputeExtendedName(static_cast<uint32_t>(i));
                }
            });
        }
        for (auto& w : workers) {
            w.join();
        }

        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0)
            .count();
        log::Logd("[ref_chain] build ext_names done (", ms, "ms)");
    });
    return extNames_[nodeIndex];
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
    void Feed(const std::string& chunkText)
    {
        if (done_) {
            return;
        }
        std::string text = overflow_ + chunkText;
        overflow_.clear();

        // Check if this chunk contains the closing bracket
        size_t rBracket = text.find(']');
        if (rBracket != std::string::npos) {
            // Append a trailing comma to flush the last number, then parse
            std::string complete = text.substr(0, rBracket) + ',';
            ParseNumbersFromString(complete, result);
            done_ = true;
            return;
        }

        // No ']' — save everything after the last ',' as overflow.
        // The number before the last comma is safe to parse because the
        // trailing comma terminates it.  Without the comma, the number
        // would be lost, causing a -1 shift at every chunk boundary.
        size_t lastComma = text.rfind(',');
        if (lastComma != std::string::npos) {
            std::string complete = text.substr(0, lastComma + 1);
            overflow_ = text.substr(lastComma + 1);
            ParseNumbersFromString(complete, result);
        } else {
            // Entire chunk may be part of a single number (unlikely but possible)
            overflow_ = text;
        }
    }

    /// Must be called after the last chunk to flush pending overflow.
    void Done()
    {
        if (done_) {
            return;
        }
        if (!overflow_.empty()) {
            ParseNumbersFromString(overflow_, result);
            overflow_.clear();
        }
        done_ = true;
    }

    bool IsDone() const { return done_; }

private:
    /// Parse comma-separated unsigned integers from a string.
    /// Non-digit characters (commas, whitespace, brackets) terminate numbers.
    static void ParseNumbersFromString(const std::string& s, std::vector<uint32_t>& out)
    {
        uint32_t num = 0;
        bool inNum = false;
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
                inNum = true;
            } else if (inNum) {
                out.push_back(num);
                num = 0;
                inNum = false;
            }
        }
    }
};

// ============================================================================
//  File reading helpers
// ============================================================================

struct HeaderResult {
    std::string buf;            // accumulated header buffer
    size_t nodesMarkerPos;    // byte offset of "\"nodes\":" in buf
};

/// Read file header in META_CHUNK-sized chunks until both "\"snapshot\":"
/// and "\"nodes\":" markers are found.  Returns the accumulated buffer
/// and the position of the nodes marker.
/// Throws if markers are not found within META_LIMIT bytes.
static HeaderResult ReadHeader(std::ifstream& file, size_t fileSize)
{
    size_t limit = std::min<size_t>(fileSize, META_LIMIT);
    std::string acc;
    size_t pos = 0;

    while (pos < limit) {
        size_t chunkSize = std::min<size_t>(META_CHUNK, limit - pos);
        std::string chunk(chunkSize, '\0');
        file.read(&chunk[0], chunkSize);
        size_t bytesRead = file.gcount();
        if (bytesRead == 0) {
            break;
        }

        acc.append(chunk.data(), bytesRead);
        pos += bytesRead;

        size_t nodesIdx = acc.find("\"nodes\":");
        if (nodesIdx != std::string::npos && acc.find("\"snapshot\":") != std::string::npos) {
            return {acc, nodesIdx};
        }
    }
    throw std::runtime_error("cannot find \"snapshot\" and \"nodes\" fields in file header");
}

/// Stream-parse a flat numeric JSON array from file starting at byte_offset.
/// Uses ChunkedUintParser with CHUNK_SIZE (16 MB) chunks.
static std::vector<uint32_t> parseArrayChunked(std::ifstream& file, size_t byteOffset, size_t fileSize)
{
    ChunkedUintParser parser;
    std::string readBuf(CHUNK_SIZE, '\0');
    size_t pos = byteOffset;

    while (pos < fileSize) {
        size_t toRead = std::min<size_t>(CHUNK_SIZE, fileSize - pos);
        file.seekg(pos);
        file.read(&readBuf[0], toRead);
        size_t bytesRead = file.gcount();
        if (bytesRead == 0) {
            break;
        }

        parser.Feed(std::string(readBuf.data(), bytesRead));
        pos += bytesRead;
        if (parser.IsDone()) {
            break;
        }
    }

    parser.Done();
    return std::move(parser.result);
}

/// Search for a marker string in the file, scanning in META_CHUNK-sized windows.
/// To avoid missing markers that straddle chunk boundaries, each window
/// overlaps the previous one by (marker.length() - 1) bytes.
/// Returns the absolute byte offset of the marker, or std::string::npos.
static size_t FindMarkerInFile(std::ifstream& file, const std::string& marker, size_t startPos, size_t fileSize)
{
    std::string readBuf(META_CHUNK, '\0');
    file.clear();  // ensure stream is in good state before scanning
    size_t pos = startPos;

    while (pos < fileSize) {
        size_t toRead = std::min<size_t>(META_CHUNK, fileSize - pos);
        file.seekg(pos);
        file.read(&readBuf[0], toRead);
        size_t bytesRead = file.gcount();
        if (bytesRead == 0) {
            break;
        }

        std::string view(readBuf.data(), bytesRead);
        size_t idx = view.find(marker);
        if (idx != std::string::npos) {
            return pos + idx;
        }

        // Overlap windows to catch markers that straddle boundaries
        if (pos + bytesRead < fileSize) {
            pos += bytesRead;
            if (pos >= marker.length()) {
                pos -= (marker.length() - 1);
            }
        } else {
            break;
        }
    }
    return std::string::npos;
}

/// Find the position of '[' that starts a JSON array after a given marker.
/// e.g., find_array_bracket_after(file, "\"nodes\":", start_pos, fileSize)
/// returns the byte offset of the '[' after the marker.
static size_t FindArrayBracketAfter(std::ifstream& file, const std::string& marker, size_t startPos, size_t fileSize)
{
    size_t markPos = FindMarkerInFile(file, marker, startPos, fileSize);
    if (markPos == std::string::npos) {
        return std::string::npos;
    }

    std::string buf(BRACKET_SEARCH_BUF_SIZE, '\0');
    size_t searchStart = markPos + marker.size();
    file.seekg(searchStart);
    file.read(&buf[0], BRACKET_SEARCH_BUF_SIZE);
    size_t bytes = file.gcount();
    std::string view(buf.data(), bytes);
    size_t bracket = view.find('[');
    if (bracket == std::string::npos) {
        return std::string::npos;
    }
    return searchStart + bracket;
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
static size_t FindMatchingCloseBracket(const std::string& text, size_t start)
{
    if (start >= text.size() || text[start] != '[') {
        return std::string::npos;
    }
    size_t pos = start + 1;
    while (pos < text.size()) {
        char c = text[pos];
        if (c == '"') {
            // Skip string literal — handle escape sequences
            pos++;
            while (pos < text.size()) {
                if (text[pos] == '\\') {
                    pos += ESCAPE_CHAR_SKIP_LEN;
                    continue;
                }  // skip escaped char
                if (text[pos] == '"') {
                    break; // end of string
                }
                pos++;
            }
            if (pos >= text.size()) { // unterminated string
                return std::string::npos;
            }
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
static std::string SanitizeJsonStringArray(const std::string& strBuf)
{
    std::string sanitized;
    sanitized.reserve(strBuf.size());
    size_t i = 0;
    while (i < strBuf.size()) {
        char c = strBuf[i];
        if (c == '"') {
            // Inside a string literal — copy until closing quote
            sanitized += c;
            i++;
            while (i < strBuf.size()) {
                char sc = strBuf[i];
                if (sc == '\\') {
                    // Check for orphan surrogate: \uD[89AB]xx or \uD[CDEF]xx
                    if (i + UNICODE_ESCAPE_TAIL_LEN < strBuf.size() && strBuf[i+1] == 'u') {
                        char h1 = strBuf[i+2] | 0x20;  // lowercase
                        char h2 = strBuf[i+3];
                        if (h1 == 'd' && ((h2 >= '8' && h2 <= 'f') || (h2 >= 'A' && h2 <= 'F'))) {
                            sanitized += "\\uFFFD";  // replace with replacement character
                            i += UNICODE_ESCAPE_LEN;
                            continue;
                        }
                    }
                    sanitized += sc;
                    i++;
                    if (i < strBuf.size()) {
                        sanitized += strBuf[i];
                        i++;
                    }
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
                    sanitized += hex[(uc >> HEX_UPPER_NIBBLE_SHIFT) & 0xF];
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
static std::vector<std::string> readStringsArray(std::ifstream& file, size_t bracketPos, size_t fileSize)
{
    // Read up to 1 MB (META_CHUNK * 16) for the strings array
    size_t remaining = fileSize - bracketPos;
    size_t readSize = std::min<size_t>(remaining, META_CHUNK * 16);
    std::string strBuf(readSize, '\0');
    file.clear();
    file.seekg(bracketPos);
    file.read(&strBuf[0], readSize);
    size_t bytesRead = file.gcount();
    strBuf.resize(bytesRead);

    // Find the closing bracket to trim the buffer to exact array content
    size_t closeBracket = FindMatchingCloseBracket(strBuf, 0);
    if (closeBracket != std::string::npos && closeBracket + 1 < strBuf.size()) {
        strBuf.resize(closeBracket + 1);
    } else if (closeBracket == std::string::npos && bytesRead == readSize && readSize < remaining) {
        // Strings array is larger than 1 MB — read the entire rest of the file
        std::string restBuf(remaining, '\0');
        file.clear();
        file.seekg(bracketPos);
        file.read(&restBuf[0], remaining);
        size_t restBytes = file.gcount();
        restBuf.resize(restBytes);
        size_t fullClose = FindMatchingCloseBracket(restBuf, 0);
        if (fullClose != std::string::npos) {
            restBuf.resize(fullClose + 1);
        }
        strBuf = std::move(restBuf);
    }

    // First attempt: direct JSON parse
    try {
        return json::parse(strBuf).get<std::vector<std::string>>();
    } catch (const std::exception&) {
        // Second attempt: sanitize and retry
        std::string sanitized = SanitizeJsonStringArray(strBuf);
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
static void ParseSnapshotMeta(HeapSnapshot& snapshot,
    const std::string& headBuf,
    size_t nodesMarkerPos,
    size_t& nodesArrayBracket)
{
    // Locate the snapshot metadata JSON between "snapshot": and "nodes":
    size_t snapIdx = headBuf.find("\"snapshot\":");
    nodesArrayBracket = headBuf.find('[', nodesMarkerPos + NODES_MARKER_LEN);
    size_t metaEnd = headBuf.rfind('}', nodesArrayBracket);
    if (metaEnd == std::string::npos) {
        throw std::runtime_error("cannot find snapshot metadata end");
    }
    metaEnd += 1;

    // Extract and trim the JSON string
    std::string snapshotJsonStr = headBuf.substr(snapIdx + 11, metaEnd - (snapIdx + 11));
    const size_t firstNonspace = snapshotJsonStr.find_first_not_of(" \t\r\n");
    if (firstNonspace != std::string::npos) {
        snapshotJsonStr.erase(0, firstNonspace);
    } else {
        snapshotJsonStr.clear();
    }
    const size_t lastNonspace = snapshotJsonStr.find_last_not_of(" \t\r\n");
    if (lastNonspace != std::string::npos) {
        snapshotJsonStr.erase(lastNonspace + 1);
    } else {
        snapshotJsonStr.clear();
    }

    json snapshotMeta;
    try {
        snapshotMeta = json::parse(snapshotJsonStr);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("failed to parse snapshot metadata: ") + e.what());
    }

    auto meta = snapshotMeta["meta"];
    auto nodeFields = meta["node_fields"].get<std::vector<std::string>>();
    auto edgeFields = meta["edge_fields"].get<std::vector<std::string>>();

    snapshot.nodeFieldsCount = static_cast<int>(nodeFields.size());
    snapshot.edgeFieldsCount = static_cast<int>(edgeFields.size());
    if (snapshot.nodeFieldsCount <= 0) {
        throw std::runtime_error("invalid snapshot metadata: empty node_fields");
    }

    // Discover node field offsets by name
    // Typical V8 node_fields: ["type","name","id","self_size","edge_count","trace_node_id","detachedness"]
    for (int i = 0; i < static_cast<int>(nodeFields.size()); i++) {
        if (nodeFields[i] == "type") {
            snapshot.nodeTypeOffset = i;
        }
        else if (nodeFields[i] == "name") snapshot.nodeNameOffset = i;
        else if (nodeFields[i] == "id") snapshot.nodeIdOffset = i;
        else if (nodeFields[i] == "self_size") snapshot.nodeSelfSizeOffset = i;
        else if (nodeFields[i] == "native_size") snapshot.nodeNativeSizeOffset = i;
        else if (nodeFields[i] == "edge_count") snapshot.nodeEdgeCountOffset = i;
    }

    // Discover edge field offsets by name
    // Typical V8 edge_fields: ["type","name_or_index","to_node"]
    for (int i = 0; i < static_cast<int>(edgeFields.size()); i++) {
        if (edgeFields[i] == "type") {
            snapshot.edgeTypeOffset = i;
        } else if (edgeFields[i] == "name_or_index") {
            snapshot.edgeNameOrIndexOffset = i;
        } else if (edgeFields[i] == "to_node") {
            snapshot.edgeToNodeOffset = i;
        }
    }

    // 元数据完整性校验：缺失必填字段会让下游用 -1（转成 SIZE_MAX）当数组下标越界读，
    // edge_fields 为空则会在边解析处除以 0。畸形/异构生成器产出的快照挡在这里。
    if (snapshot.edgeFieldsCount <= 0) {
        throw std::runtime_error("invalid snapshot metadata: empty edge_fields");
    }
    if (snapshot.nodeTypeOffset < 0 || snapshot.nodeNameOffset < 0 ||
        snapshot.nodeIdOffset < 0 || snapshot.nodeSelfSizeOffset < 0 ||
        snapshot.nodeEdgeCountOffset < 0) {
        throw std::runtime_error(
            "invalid snapshot metadata: missing required node field "
            "(type/name/id/self_size/edge_count)");
    }
    if (snapshot.edgeTypeOffset < 0 || snapshot.edgeNameOrIndexOffset < 0 ||
        snapshot.edgeToNodeOffset < 0) {
        throw std::runtime_error(
            "invalid snapshot metadata: missing required edge field (type/name_or_index/to_node)");
    }

    // Resolve edge type numeric values from meta.edge_types[type_offset]
    // Typical V8 edge types: ["context","element","property","internal","hidden","shortcut","weak"]
    snapshot.edgeWeakType = -1;
    snapshot.edgeShortcutType = -1;
    snapshot.edgeHiddenType = -1;
    snapshot.edgeElementType = -1;
    snapshot.edgeInternalType = -1;
    if (meta.contains("edge_types") && snapshot.edgeTypeOffset >= 0) {
        auto edgeTypes = meta["edge_types"][snapshot.edgeTypeOffset];
        if (edgeTypes.is_array()) {
            auto types = edgeTypes.get<std::vector<std::string>>();
            for (int i = 0; i < static_cast<int>(types.size()); i++) {
                if (types[i] == "weak") {
                    snapshot.edgeWeakType = i;
                } else if (types[i] == "shortcut") {
                    snapshot.edgeShortcutType = i;
                } else if (types[i] == "hidden") {
                    snapshot.edgeHiddenType = i;
                } else if (types[i] == "element") {
                    snapshot.edgeElementType = i;
                } else if (types[i] == "internal") {
                    snapshot.edgeInternalType = i;
                }
            }
        }
    }

    // Parse node type names from meta.node_types[type_offset]
    // Typical V8 node types: ["hidden","array","string","object","code","closure",...]
    if (meta.contains("node_types") && snapshot.nodeTypeOffset >= 0) {
        auto NodeTypes = meta["node_types"][snapshot.nodeTypeOffset];
        if (NodeTypes.is_array()) {
            snapshot.nodeTypeNames = NodeTypes.get<std::vector<std::string>>();
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
static void ParseNodesArray(HeapSnapshot& snapshot, std::ifstream& file, size_t nodesArrayBracket, size_t fileSize)
{
    snapshot.nodes = parseArrayChunked(file, nodesArrayBracket + 1, fileSize);
    size_t nc = snapshot.nodes.size() / snapshot.nodeFieldsCount;
    if (nc > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error("malformed heapsnapshot: node count exceeds INT_MAX");
    snapshot.nodeCount = static_cast<int>(nc);
}

// ============================================================================
//  Phase 3: parse edges array
//
//  The edges array is a flat list of uint32 values, grouped by edge_fields_count.
//  The edges marker may be in the header buffer or further in the file
//  (after the nodes array).  We try the header first, then scan forward.
//  Returns the bracket position for use in the strings phase.
// ============================================================================
static size_t ParseEdgesArray(HeapSnapshot& snapshot, std::ifstream& file,
    const std::string& headBuf,
    size_t nodesArrayBracket, size_t fileSize)
{
    size_t edgesBracketPos = std::string::npos;

    // First try: header buffer (edges may be close to the beginning)
    size_t edgesMarkerPos = headBuf.find("\"edges\":");
    if (edgesMarkerPos != std::string::npos) {
        size_t bracket = headBuf.find('[', edgesMarkerPos + 8);
        if (bracket != std::string::npos) {
            edgesBracketPos = bracket;
        }
    }

    // Second try: scan forward from after the nodes array
    if (edgesBracketPos == std::string::npos) {
        // Estimate: nodes array occupies roughly nodes.size() * 2 bytes (JSON text)
        size_t searchStart = nodesArrayBracket + snapshot.nodes.size() * 2;
        edgesBracketPos = FindArrayBracketAfter(file, "\"edges\":", searchStart, fileSize);
        if (edgesBracketPos == std::string::npos) {
            throw std::runtime_error("cannot find \"edges\" field");
        }
    }

    snapshot.edges = parseArrayChunked(file, edgesBracketPos + 1, fileSize);
    size_t ec = snapshot.edges.size() / snapshot.edgeFieldsCount;
    if (ec > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error("malformed heapsnapshot: edge count exceeds INT_MAX");
    snapshot.edgeCount = static_cast<int>(ec);
    return edgesBracketPos;
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
static void ParseStringsTable(HeapSnapshot& snapshot, std::ifstream& file,
    const std::string& headBuf,
    size_t edgesBracketPos, size_t fileSize)
{
    size_t stringsBracketPos = std::string::npos;

    // Clear stream state (may have failbit from earlier reads)
    file.clear();
    file.seekg(0);

    // Pass 1: header buffer
    size_t stringsMarkerPos = headBuf.find("\"strings\":");
    if (stringsMarkerPos != std::string::npos) {
        size_t bracket = headBuf.find('[', stringsMarkerPos + 10);
        if (bracket != std::string::npos) {
            stringsBracketPos = bracket;
        }
    }

    // Pass 2: tail of file (strings array is often the last field)
    if (stringsBracketPos == std::string::npos && fileSize > META_LIMIT) {
        size_t tailReadSize = std::min<size_t>(fileSize, META_LIMIT);
        std::string tailBuf(tailReadSize, '\0');
        file.clear();
        file.seekg(fileSize - tailReadSize);
        file.read(&tailBuf[0], tailReadSize);
        size_t bytes = file.gcount();
        tailBuf.resize(bytes);

        stringsMarkerPos = tailBuf.find("\"strings\":");
        if (stringsMarkerPos != std::string::npos) {
            size_t bracket = tailBuf.find('[', stringsMarkerPos + 10);
            if (bracket != std::string::npos) {
                stringsBracketPos = (fileSize - tailReadSize) + bracket;
            }
        }
    }

    // Pass 3: forward scan from after edges
    if (stringsBracketPos == std::string::npos) {
        size_t searchStart = std::max<size_t>(META_LIMIT, edgesBracketPos + 1);
        size_t markPos = FindMarkerInFile(file, "\"strings\":", searchStart, fileSize);
        if (markPos != std::string::npos) {
            std::string buf(STRINGS_SEARCH_BUF_SIZE, '\0');
            file.clear();
            file.seekg(markPos + STRINGS_MARKER_LEN);
            file.read(&buf[0], STRINGS_SEARCH_BUF_SIZE);
            size_t bytes = file.gcount();
            std::string view(buf.data(), bytes);
            size_t bracket = view.find('[');
            if (bracket != std::string::npos) {
                stringsBracketPos = markPos + STRINGS_MARKER_LEN + bracket;
            }
        }
    }

    if (stringsBracketPos != std::string::npos) {
        snapshot.strings = readStringsArray(file, stringsBracketPos, fileSize);
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
static void BuildCsrAndIdMap(HeapSnapshot& snapshot)
{
    // ---- Build forward CSR ----
    // The total number of edge fields is the sum of edge_count * edge_fields_count
    // for each node.  We iterate nodes and accumulate the edge pointer.
    snapshot.firstEdgePointers.resize(snapshot.nodeCount + 1, 0);
    // 用 64 位累加，避免 4G 个字段以上的无符号回绕；累加结果必须与解析到的
    // edges 数组完全一致 —— 截断的文件或 edge_count 与实际数组不符的生成器，
    // 否则会让所有消费方越界读 edges，把垃圾当成图结构、静默产出错误报告。
    uint64_t totalEdgeFields = 0;
    for (int i = 0; i < snapshot.nodeCount; i++) {
        snapshot.firstEdgePointers[i] = static_cast<uint32_t>(totalEdgeFields);
        totalEdgeFields +=
            static_cast<uint64_t>(
                snapshot.nodes[i * snapshot.nodeFieldsCount + snapshot.nodeEdgeCountOffset]) *
            static_cast<uint64_t>(snapshot.edgeFieldsCount);
    }
    snapshot.firstEdgePointers[snapshot.nodeCount] = static_cast<uint32_t>(totalEdgeFields);
    if (totalEdgeFields != static_cast<uint64_t>(snapshot.edges.size())) {
        throw std::runtime_error(
            "invalid snapshot: edges array size (" + std::to_string(snapshot.edges.size()) +
            ") does not match the sum of per-node edge counts (" +
            std::to_string(totalEdgeFields) + ")");
    }
    // 一次遍历校验所有 to_node 在范围内；不在每个 for_each_edge 里加分支
    // （边数千万级，逐边检查会拖慢分析）。
    for (size_t ptr = 0; ptr < snapshot.edges.size();
         ptr += static_cast<size_t>(snapshot.edgeFieldsCount)) {
        const uint32_t toNode =
            snapshot.edges[ptr + static_cast<size_t>(snapshot.edgeToNodeOffset)] /
            static_cast<uint32_t>(snapshot.nodeFieldsCount);
        if (toNode >= static_cast<uint32_t>(snapshot.nodeCount)) {
            throw std::runtime_error("invalid snapshot: edge target " + std::to_string(toNode) +
                                     " exceeds node count " + std::to_string(snapshot.nodeCount));
        }
    }

    // ---- Build node_id_to_idx map ----
    for (int i = 0; i < snapshot.nodeCount; i++) {
        if (snapshot.nodeIdOffset >= 0) {
            uint32_t id = snapshot.nodes[i * snapshot.nodeFieldsCount + snapshot.nodeIdOffset];
            snapshot.nodeIdToIdx[id] = static_cast<uint32_t>(i);
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
static void ComputePathsAndRetained(HeapSnapshot& snapshot)
{
    // Shortest paths BFS: exclude weak, shortcut, and hidden edges
    {
        auto paths = ComputeShortestPaths(snapshot, true, true, true);
        snapshot.pathParentIndex.resize(snapshot.nodeCount);
        snapshot.pathEdgeStart.resize(snapshot.nodeCount);
        constexpr uint32_t invalid = 0xFFFFFFFF;
        for (uint32_t i = 0; i < static_cast<uint32_t>(snapshot.nodeCount); i++) {
            snapshot.pathParentIndex[i] = paths.pathEdges[i].parentNodeIndex;
            snapshot.pathEdgeStart[i] = paths.pathEdges[i].edgeFieldStart;
        }
    }

    // Dominator-based retained sizes: exclude weak and shortcut edges
    {
        auto ret = ComputeNodeSizes(snapshot, true, true);
        snapshot.retainedSizes = std::move(ret.retainedSizes);
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

HeapSnapshot HeapSnapshot::FromFile(const std::string& path)
{
    // Open file in binary mode, seek to end to get file size
    auto file = OpenIfstream(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open file: " + path);
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);

    StageTimer timer("HeapSnapshot");
    HeapSnapshot snapshot;

    // ---- Phase 1: metadata ----
    timer.Stage("Reading header");
    auto header = ReadHeader(file, fileSize);
    size_t nodesArrayBracket = 0;
    ParseSnapshotMeta(snapshot, header.buf, header.nodesMarkerPos, nodesArrayBracket);
    timer.Tick("meta_parsed");

    // ---- Phase 2: nodes ----
    timer.Stage("Parsing nodes array");
    ParseNodesArray(snapshot, file, nodesArrayBracket, fileSize);
    timer.Tick("nodes", std::to_string(snapshot.nodeCount) + " nodes");
    // 空 nodes 数组会让下游 compute_shortest_paths 对空 vector 执行 path_edges[0] = ...
    if (snapshot.nodeCount <= 0) {
        throw std::runtime_error("invalid snapshot: nodes array is empty");
    }

    // ---- Phase 3: edges ----
    timer.Stage("Parsing edges array");
    size_t edgesBracketPos = ParseEdgesArray(snapshot, file, header.buf,
        nodesArrayBracket, fileSize);
    timer.Tick("edges", std::to_string(snapshot.edgeCount) + " edges");

    // ---- Phase 4: strings ----
    timer.Stage("Parsing strings table");
    ParseStringsTable(snapshot, file, header.buf, edgesBracketPos, fileSize);
    timer.Tick("strings", std::to_string(snapshot.strings.size()) + " strings");

    // ---- Phase 5: CSR + node_id_to_idx ----
    timer.Stage("Building CSR indices");
    BuildCsrAndIdMap(snapshot);
    timer.Tick("csr");
    timer.Stage("Building node_id_to_idx");
    timer.Tick("node_id_map", std::to_string(snapshot.nodeIdToIdx.size()) + " entries");

    // ---- Phase 5.5: locate SyntheticRoot once for O(1) get_total_heap_size (#27) ----
    snapshot.syntheticRootIndex = UINT32_MAX;
    for (int i = 0; i < snapshot.nodeCount; i++) {
        uint32_t ui = static_cast<uint32_t>(i);
        if (snapshot.GetNodeName(ui) == "SyntheticRoot") {
            snapshot.syntheticRootIndex = ui;
            break;
        }
    }

    // ---- Phase 6: shortest paths + retained sizes ----
    timer.Stage("Computing shortest paths");
    ComputePathsAndRetained(snapshot);
    timer.Tick("shortest_paths");
    timer.Stage("Computing retained sizes");
    timer.Tick("retained_sizes");

    timer.Done();
    log::Logok("  Heap snapshot loaded: ", snapshot.nodeCount, " nodes, ",
        snapshot.edgeCount, " edges, ", snapshot.strings.size(), " strings.");
    return snapshot;
}

// ============================================================================
//  HeapSnapshot member functions — node/edge accessors
// ============================================================================

HeapSnapshot::NodeView HeapSnapshot::GetNodeById(uint32_t id) const
{
    auto it = nodeIdToIdx.find(id);
    if (it == nodeIdToIdx.end()) {
        return {0xFFFFFFFF, 0, "", 0};  // sentinel: not found
    }
    uint32_t nodeIndex = it->second;
    uint32_t base = nodeIndex * nodeFieldsCount;
    uint32_t nameIdx = nodes[base + nodeNameOffset];
    std::string name = (nameIdx < strings.size()) ? strings[nameIdx] : "";
    return {nodeIndex, id, std::move(name), nodes[base + nodeSelfSizeOffset]};
}

std::string HeapSnapshot::GetNodeName(uint32_t nodeIndex) const
{
    if (nodeIndex >= static_cast<uint32_t>(nodeCount)) {
        return "";
    }
    uint32_t base = nodeIndex * static_cast<uint32_t>(nodeFieldsCount);
    uint32_t nameIdx = nodes[base + static_cast<size_t>(nodeNameOffset)];
    return (nameIdx < strings.size()) ? strings[nameIdx] : "";
}

std::string HeapSnapshot::GetNodeTypeName(uint32_t nodeIndex) const
{
    if (nodeIndex >= static_cast<uint32_t>(nodeCount)) {
        return "";
    }
    if (nodeTypeNames.empty()) {
        return "";
    }
    uint32_t base = nodeIndex * static_cast<uint32_t>(nodeFieldsCount);
    uint32_t typeIdx = nodes[base + static_cast<size_t>(nodeTypeOffset)];
    return (typeIdx < nodeTypeNames.size()) ? nodeTypeNames[typeIdx] : "";
}

uint32_t HeapSnapshot::GetNodeEdgeCount(uint32_t nodeIndex) const
{
    if (nodeIndex >= static_cast<uint32_t>(nodeCount)) {
        return 0;
    }
    uint32_t base = nodeIndex * static_cast<uint32_t>(nodeFieldsCount);
    return nodes[base + static_cast<size_t>(nodeEdgeCountOffset)];
}

/// Extract the ArkInternalHash value from a node's edges.
/// ArkTS (HarmonyOS) stores internal hash values in edge names matching
/// "ArkInternalHash", with the target node name containing "Int: <hash>".
uint32_t HeapSnapshot::GetNodeInternalHash(uint32_t nodeIndex) const
{
    if (nodeIndex >= static_cast<uint32_t>(nodeCount)) {
        return 0;
    }
    uint32_t hash = 0;
    static const std::regex pattern(R"(Int:\s*(\d+))");
    std::smatch matches;

    ForEachEdge(nodeIndex, [&](const EdgeInfo &edge, size_t) {
        if (edge.nameOrIndex < static_cast<uint32_t>(strings.size())) {
            std::string edgeName = strings[edge.nameOrIndex];
            if (edgeName == "ArkInternalHash") {
                std::string targetNodeName = GetNodeName(edge.toNode);
                if (std::regex_search(targetNodeName, matches, pattern)) {
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
/// Unreachable nodes have path_parent_index == invalid.
/// Format: "RootNode -> ChildNode -> TargetNode"
std::string HeapSnapshot::FormatRefChain(uint32_t nodeIndex) const
{
    constexpr uint32_t invalid = 0xFFFFFFFF;
    
    // 先收集节点索引，避免在收集过程中反复访问名称
    std::vector<uint32_t> indices;
    indices.reserve(REF_CHAIN_INDICES_RESERVE); // 预留常见链长度
    
    uint32_t cur = nodeIndex;
    uint32_t guard = 0;
    while (cur < static_cast<uint32_t>(nodeCount)) {
        if (++guard > static_cast<uint32_t>(nodeCount)) {
            log::Logw("[ref_chain] WARN cycle in path_parent_index at node_index=", nodeIndex);
            return "";
        }
        uint32_t parent = pathParentIndex[cur];
        if (parent == invalid) {
            return ""; // unreachable
        }
        indices.push_back(cur);
        if (parent == cur) { // 到达根节点
            break;
        }
        cur = parent;
    }
    
    if (indices.empty()) {
        return "";
    }
    
    // 预计算最终字符串长度，避免多次 realloc
    size_t totalLen = 0;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        // 直接获取节点名称，避免临时拷贝
        const std::string& name = GetExtendedNodeName(*it);
        totalLen += name.size();
        if (it != indices.rbegin()) { // " -> "
            totalLen += ARROW_SEPARATOR_LEN;
        }
    }
    
    std::string result;
    result.reserve(totalLen);
    
    bool first = true;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        if (!first) {
            result += " -> ";
        }
        first = false;
        result += GetExtendedNodeName(*it);
    }
    return result;
}

std::vector<std::string> HeapSnapshot::FormatRefChainFrames(uint32_t nodeIndex) const
{
    constexpr uint32_t invalid = 0xFFFFFFFF;

    std::vector<uint32_t> indices;
    indices.reserve(REF_CHAIN_INDICES_RESERVE);

    uint32_t cur = nodeIndex;
    uint32_t guard = 0;
    while (cur < static_cast<uint32_t>(nodeCount)) {
        if (++guard > static_cast<uint32_t>(nodeCount)) {
            log::Logw("[ref_chain] WARN cycle in path_parent_index at node_index=", nodeIndex);
            return {};
        }
        uint32_t parent = pathParentIndex[cur];
        if (parent == invalid) {
            return {};  // unreachable
        }
        indices.push_back(cur);
        if (parent == cur) { // 到达根节点
            break;
        }
        cur = parent;
    }

    if (indices.empty()) {
        return {};
    }

    // 反向遍历，从根到目标，只取扩展名，不做 " -> " 拼接。
    std::vector<std::string> frames;
    frames.reserve(indices.size());
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        frames.push_back(GetExtendedNodeName(*it));
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

RefChainKey FnvSeed(const std::string& name)
{
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

RefChainKey FnvContinue(const RefChainKey& prev, const std::string& name)
{
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

RefChainKey HeapSnapshot::RefChainHash(uint32_t nodeIndex) const
{
    constexpr uint32_t invalid = 0xFFFFFFFF;
    if (nodeIndex >= static_cast<uint32_t>(nodeCount)) {
        return RefChainKey{};
    }

    // 一次性预构建「正向链 hash」表（root → node），之后 O(1) 查表。
    std::call_once(*chainHashOnce_, [this]() {
        const auto t0 = std::chrono::steady_clock::now();
        log::Logd("[ref_chain] build chain_hash start node_count=", nodeCount);

        const int N = nodeCount;
        if (N > 0) { // 触发 ext_names_ 预构建
            GetExtendedNodeName(0);
        }

        chainHash_.resize(static_cast<size_t>(N));
        chainHashComputed_.assign(static_cast<size_t>(N), 0);

        // 复用 scratch：沿父链反向收集未算节点，再反转回填（天然正向拓扑序）。
        std::vector<uint32_t> path;
        path.reserve(REF_CHAIN_PATH_RESERVE);
        const size_t cycleLimit = static_cast<size_t>(N) + 1;

        for (int ni = 0; ni < N; ++ni) {
            const uint32_t node = static_cast<uint32_t>(ni);
            if (chainHashComputed_[node]) {
                continue;
            }

            path.clear();
            uint32_t cur = node;
            while (!chainHashComputed_[cur] && path.size() < cycleLimit) {
                path.push_back(cur);
                const uint32_t parent = pathParentIndex[cur];
                if (parent == invalid || parent == cur) {
                    break;
                }
                cur = parent;
            }
            if (path.size() >= cycleLimit) {
                log::Logw("[ref_chain] WARN cycle in path_parent_index at node_index=", node);
            }

            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                const uint32_t u = *it;
                const uint32_t p = pathParentIndex[u];
                if (p == invalid || p >= static_cast<uint32_t>(N)) {
                    chainHash_[u] = RefChainKey{};  // 不可达 => {0,0}
                } else if (p == u) {
                    chainHash_[u] = FnvSeed(extNames_[u]);  // root：种子
                } else if (chainHashComputed_[p]) {
                    chainHash_[u] = FnvContinue(chainHash_[p], extNames_[u]);
                } else {
                    chainHash_[u] = RefChainKey{};  // p 未算（环/异常）兜底
                }
                chainHashComputed_[u] = 1;
            }
        }

        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0)
            .count();
        log::Logd("[ref_chain] build chain_hash done (", ms, "ms)");
    });

    return chainHash_[nodeIndex];
}

} // namespace mem_analyzer