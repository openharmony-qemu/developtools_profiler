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
#include "tshandle/arkts_analyzer.hpp"
#include "tshandle/tshandle_retained_size.hpp"
#include "config.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <unordered_set>
#include <cctype>

using json = nlohmann::ordered_json;

// ============================================================================
//  Anonymous namespace helpers (local to this translation unit)
// ============================================================================

namespace {

// ---------------------------------------------------------------------------
//  normalize_addr — strip 0x prefix, validate hex, lowercase without leading zeros
// ---------------------------------------------------------------------------

static std::string NormalizeAddr(const std::string& raw)
{
    std::string s = raw;
    // trim whitespace
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    s = s.substr(start);
    // remove 0x or 0X prefix
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s = s.substr(2);
    }
    if (s.empty()) {
        return "";
    }
    // check valid hex
    for (char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return "";
        }
    }
    // parse with stoull and convert to lowercase hex (no leading zeros)
    try {
        unsigned long long val = std::stoull("0x" + s, nullptr, 16);
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    } catch (...)
    {
        return "";
    }
}

// ---------------------------------------------------------------------------
//  BuildNativeRefResult — local struct (avoids conflict with heap_snapshot.hpp)
// ---------------------------------------------------------------------------

struct BuildNativeRefResult {
    std::unordered_map<uint32_t, std::vector<mem_analyzer::NativeRefEntry>> nodeNativeRef;
    int multiCount = 0;
    int noNodeCount = 0;
    int invalidCount = 0;
    // detailed anomaly records for report
    std::vector<std::string> multiKeys;
    std::vector<std::string> multiOriginalAddrs;
    std::vector<std::vector<long long>> multiCallstackIds;
    std::vector<std::string> noNodeAddrs;
    std::vector<long long> noNodeCallstackIds;
    std::vector<std::string> invalidDetailAddrs;
    std::vector<long long> invalidDetailCallstackIds;
};

/**
 * Build nativeRef from callStacks.existingAddrs.
 * Reference: TS buildNativeRefFromCallStacks
 */
static BuildNativeRefResult BuildNativeRef(
    const std::vector<mem_analyzer::ResolvedCallStack>& callStacks,
    const std::unordered_map<std::string, uint32_t>& addrToNodeId)
{
    BuildNativeRefResult result;
    // nodeId -> (callStackId -> NativeRefEntry)
    std::unordered_map<uint32_t, std::unordered_map<long long, mem_analyzer::NativeRefEntry>> nodeByCs;
    // normalized addr -> first callStackId that claimed it
    std::unordered_map<std::string, long long> addrFirstCs;

    for (const auto& cs : callStacks) {
        if (!cs.existingAddrs) {
            continue;
        }
        for (const auto& raw : *cs.existingAddrs) {
            std::string key = NormalizeAddr(raw);
            if (key.empty()) {
                result.invalidCount++;
                result.invalidDetailAddrs.push_back(raw);
                result.invalidDetailCallstackIds.push_back(cs.id);
                continue;
            }

            // multi detection: same addr in multiple callStacks
            auto prevIt = addrFirstCs.find(key);
            if (prevIt != addrFirstCs.end()) {
                result.multiCount++;
                result.multiKeys.push_back(key);
                result.multiOriginalAddrs.push_back(raw);
                result.multiCallstackIds.push_back({prevIt->second, cs.id});
                continue;
            }

            // lookup nodeId from addr mapping
            auto nodeIt = addrToNodeId.find(key);
            if (nodeIt == addrToNodeId.end()) {
                result.noNodeCount++;
                result.noNodeAddrs.push_back(raw);
                result.noNodeCallstackIds.push_back(cs.id);
                continue;
            }
            uint32_t nodeId = nodeIt->second;

            // only claim the addr after successful mapping
            addrFirstCs[key] = cs.id;

            auto& byCs = nodeByCs[nodeId];
            auto entryIt = byCs.find(cs.id);
            if (entryIt == byCs.end()) {
                mem_analyzer::NativeRefEntry entry;
                entry.handleType = cs.type;
                entry.callStackId = cs.id;
                entry.addrs.push_back(raw);
                byCs[cs.id] = std::move(entry);
            } else {
                entryIt->second.addrs.push_back(raw);
            }
        }
    }

    // flatten node_by_cs into result.node_native_ref
    for (auto& [nodeId, byCs] : nodeByCs) {
        std::vector<mem_analyzer::NativeRefEntry> entries;
        entries.reserve(byCs.size());
        for (auto& [csId, entry] : byCs) {
            entries.push_back(std::move(entry));
        }
        result.nodeNativeRef[nodeId] = std::move(entries);
    }

    return result;
}

} // anonymous namespace

// ============================================================================
//  mem_analyzer namespace — public API
// ============================================================================

namespace mem_analyzer {

ArkTsAnalyzeResult AnalyzeArktsClasses(
    const std::string& heapsnapshotPath,
    const std::string& addrFilePath,
    std::vector<ResolvedCallStack>& callStacks)
{
    ArkTsAnalyzeResult result;

    StageTimer timer("ArkTS");

    // ---- 1. Read addr file (auto-detect text / protobuf format) ----
    auto parseResult = ParseAddrFile(addrFilePath);
    timer.tick("parse_addr");

    // ---- 3. Load heapsnapshot (retained sizes computed inside from_file) ----
    timer.stage("Loading heap snapshot");
    auto heap = HeapSnapshot::FromFile(heapsnapshotPath);
    timer.tick("load_heap", std::to_string(heap.nodeCount) + " nodes, " +
              std::to_string(heap.edgeCount) + " edges");

    // ---- 5. Build nativeRef from callStacks ----
    timer.stage("Building nativeRef");
    auto nativeResult = BuildNativeRef(callStacks, parseResult.addrToNodeId);
    timer.tick("build_native_ref");

    // ---- 6. Aggregate by class name ----
    timer.stage("Aggregating by class name");
    std::map<std::string, ArkTsClass> classMap;
    int instanceCount = 0;
    int missingNodeCount = 0;
    std::vector<uint32_t> missingNodeIds;
    std::unordered_map<long long, double> localRetainedSizeByCallStack;

    for (const auto& [nodeId, nativeRef] : nativeResult.nodeNativeRef) {
        auto node = heap.GetNodeById(nodeId);
        if (node.nodeIndex == 0xFFFFFFFF) {
            missingNodeCount++;
            missingNodeIds.push_back(nodeId);
            continue;
        }

        std::string name;
        std::string typeName = heap.GetNodeTypeName(node.nodeIndex);
        if (typeName == "string" || node.name == "JSObject" || node.name == "JSSharedObject") {
            // 字符串命名与快照分析保持一致：string(size: <区间>, content=xx)，
            // 最终由 formatter 按 size 区间聚合，避免条目过于分散
            name = heap.GetExtendedNodeName(node.nodeIndex);
        } else {
            name = node.name.empty() ? "(unknown)" : node.name;
        }
        long long shallowSize = node.selfSize;
        long long retainedSize = static_cast<long long>(heap.GetRetainedSize(node.nodeIndex));

        std::vector<TshandleRetainedShare> shares;
        shares.reserve(nativeRef.size());
        for (const auto& ref : nativeRef) {
            shares.push_back({ref.callStackId, ref.addrs.size()});
        }
        DistributeTshandleRetainedSize(
            static_cast<double>(retainedSize), shares, localRetainedSizeByCallStack);

        instanceCount++;
        ArkTsInstance inst;
        inst.id = nodeId;
        inst.shallowSize = shallowSize;
        inst.retainedSize = retainedSize;
        inst.nativeRef = nativeRef;

        auto clsIt = classMap.find(name);
        if (clsIt == classMap.end()) {
            ArkTsClass cls;
            cls.name = name;
            cls.shallowSize = shallowSize;
            cls.retainedSize = retainedSize;
            cls.instances.push_back(std::move(inst));
            classMap[name] = std::move(cls);
        } else {
            clsIt->second.instances.push_back(std::move(inst));
            clsIt->second.shallowSize += shallowSize;
            clsIt->second.retainedSize += retainedSize;
        }
    }
    timer.tick("aggregate", std::to_string(classMap.size()) + " classes, " +
              std::to_string(instanceCount) + " instances");

    ApplyTshandleRetainedSizes(callStacks, localRetainedSizeByCallStack);

    // ---- 7. Build anomaly report data ----

    // Populate counts
    result.anomalyCounts.multi = nativeResult.multiCount;
    result.anomalyCounts.noNode = nativeResult.noNodeCount;
    result.anomalyCounts.missingNode = missingNodeCount;
    result.anomalyCounts.invalidAddr = nativeResult.invalidCount;
    result.anomalyCounts.duplicate = parseResult.duplicateCount;
    result.anomalyCounts.addrFileInvalid = parseResult.addrFileInvalidCount;

    // Build anomaly lines
    auto& lines = result.anomalyLines;
    lines.push_back("# addr anomaly report");

    {
        std::ostringstream ss;
        ss << "# summary: multi=" << result.anomalyCounts.multi <<
           " noNode=" << result.anomalyCounts.noNode <<
           " missingNode=" << result.anomalyCounts.missingNode <<
           " invalid=" << result.anomalyCounts.invalidAddr <<
           " duplicate=" << result.anomalyCounts.duplicate <<
           " addrFileInvalid=" << result.anomalyCounts.addrFileInvalid;
        lines.push_back(ss.str());
    }

    // multi anomalies
    if (!nativeResult.multiKeys.empty()) {
        lines.push_back("# [multi] existing addr in multiple callStacks (kept first)");
        for (size_t i = 0; i < nativeResult.multiKeys.size(); i++) {
            std::ostringstream ss;
            ss << nativeResult.multiKeys[i] << "\t" <<
               nativeResult.multiOriginalAddrs[i] << "\tmulti\t";
            for (size_t j = 0; j < nativeResult.multiCallstackIds[i].size(); j++) {
                if (j > 0) {
                    ss << ",";
                }
                ss << nativeResult.multiCallstackIds[i][j];
            }
            lines.push_back(ss.str());
        }
    }

    // noNode anomalies
    if (!nativeResult.noNodeAddrs.empty()) {
        lines.push_back("# [noNode] existing addr has no nodeid mapping in addr file");
        for (size_t i = 0; i < nativeResult.noNodeAddrs.size(); i++) {
            std::ostringstream ss;
            ss << nativeResult.noNodeAddrs[i] << "\tnoNode\t" <<
               nativeResult.noNodeCallstackIds[i];
            lines.push_back(ss.str());
        }
    }

    // missingNode anomalies
    if (!missingNodeIds.empty()) {
        lines.push_back("# [missingNode] nodeid not found in heapsnapshot");
        for (auto id : missingNodeIds) {
            std::ostringstream ss;
            ss << id << "\tmissingNode";
            lines.push_back(ss.str());
        }
    }

    // invalid addr (existing addr not valid hex)
    if (!nativeResult.invalidDetailAddrs.empty()) {
        lines.push_back("# [invalid] existing addr not valid hex");
        for (size_t i = 0; i < nativeResult.invalidDetailAddrs.size(); i++) {
            std::ostringstream ss;
            ss << nativeResult.invalidDetailAddrs[i] << "\tinvalid\t" <<
               nativeResult.invalidDetailCallstackIds[i];
            lines.push_back(ss.str());
        }
    }

    // duplicate anomalies
    if (!parseResult.dupKeys.empty()) {
        lines.push_back("# [duplicate] addr file: same address different nodeid (kept first)");
        for (size_t i = 0; i < parseResult.dupKeys.size(); i++) {
            std::ostringstream ss;
            ss << parseResult.dupKeys[i] << "\t" <<
               parseResult.dupOriginalAddrs[i] << "\tduplicate\t";
            for (size_t j = 0; j < parseResult.dupNodeIds[i].size(); j++) {
                if (j > 0) {
                    ss << ",";
                }
                ss << parseResult.dupNodeIds[i][j];
            }
            lines.push_back(ss.str());
        }
    }

    // addrFileInvalid anomalies
    if (!parseResult.invalidOriginalAddrs.empty()) {
        lines.push_back("# [addrFileInvalid] addr file: not valid hex");
        for (const auto& addr : parseResult.invalidOriginalAddrs) {
            std::ostringstream ss;
            ss << addr << "\taddrFileInvalid";
            lines.push_back(ss.str());
        }
    }

    // ---- 8. Build result ----
    result.arktsClasses.reserve(classMap.size());
    for (auto& [name, cls] : classMap) {
        result.arktsClasses.push_back(std::move(cls));
    }

    // Print statistics
    {
        std::ostringstream ss;
        ss << classMap.size() << " classes, " << instanceCount << " instances, " <<
           missingNodeCount << " missNode, " << result.anomalyCounts.noNode <<
           " noNode, " << result.anomalyCounts.multi << " multi, " <<
           result.anomalyCounts.invalidAddr << " invalid, " <<
           result.anomalyCounts.duplicate << " dup, " <<
           result.anomalyCounts.addrFileInvalid << " addrInvalid";
        timer.tick("anomaly_report", ss.str());
    }
    timer.done();

    return result;
}

// ---------------------------------------------------------------------------
//  write_anomaly_report
// ---------------------------------------------------------------------------

std::string WriteAnomalyReport(const std::string& outputDir, const ArkTsAnalyzeResult& result)
{
    const auto& c = result.anomalyCounts;
    bool empty = (c.multi == 0 && c.noNode == 0 && c.missingNode == 0 &&
                  c.invalidAddr == 0 && c.duplicate == 0 && c.addrFileInvalid == 0);
    if (empty) {
        return "";
    }

    std::string outPath = outputDir + "/addr_anomaly.txt";

    auto outFile = OpenOfstream(outPath);
    if (!outFile.is_open()) {
        log::Logw("  warning: cannot write anomaly report to ", outPath);
        return "";
    }

    for (const auto& line : result.anomalyLines) {
        outFile << line << "\n";
    }

    outFile.close();

    log::Logi("  addr anomaly report: ", outPath,
        " (multi=", c.multi, " noNode=", c.noNode,
        " missingNode=", c.missingNode,
        " invalid=", c.invalidAddr,
        " duplicate=", c.duplicate,
        " addrFileInvalid=", c.addrFileInvalid, ")");

    return outPath;
}

} // namespace mem_analyzer
