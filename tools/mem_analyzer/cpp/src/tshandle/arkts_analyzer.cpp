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

static std::string normalize_addr(const std::string& raw) {
    std::string s = raw;
    // trim whitespace
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    s = s.substr(start);
    // remove 0x or 0X prefix
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s = s.substr(2);
    }
    if (s.empty()) return "";
    // check valid hex
    for (char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return "";
    }
    // parse with stoull and convert to lowercase hex (no leading zeros)
    try {
        unsigned long long val = std::stoull("0x" + s, nullptr, 16);
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    } catch (...) {
        return "";
    }
}

// ---------------------------------------------------------------------------
//  BuildNativeRefResult — local struct (avoids conflict with heap_snapshot.hpp)
// ---------------------------------------------------------------------------

struct BuildNativeRefResult {
    std::unordered_map<uint32_t, std::vector<mem_analyzer::NativeRefEntry>> node_native_ref;
    int multi_count = 0;
    int no_node_count = 0;
    int invalid_count = 0;
    // detailed anomaly records for report
    std::vector<std::string> multi_keys;
    std::vector<std::string> multi_original_addrs;
    std::vector<std::vector<long long>> multi_callstack_ids;
    std::vector<std::string> no_node_addrs;
    std::vector<long long> no_node_callstack_ids;
    std::vector<std::string> invalid_detail_addrs;
    std::vector<long long> invalid_detail_callstack_ids;
};

/**
 * Build nativeRef from callStacks.existingAddrs.
 * Reference: TS buildNativeRefFromCallStacks
 */
static BuildNativeRefResult build_native_ref(
    const std::vector<mem_analyzer::ResolvedCallStack>& call_stacks,
    const std::unordered_map<std::string, uint32_t>& addr_to_node_id)
{
    BuildNativeRefResult result;
    // nodeId -> (callStackId -> NativeRefEntry)
    std::unordered_map<uint32_t, std::unordered_map<long long, mem_analyzer::NativeRefEntry>> node_by_cs;
    // normalized addr -> first callStackId that claimed it
    std::unordered_map<std::string, long long> addr_first_cs;

    for (const auto& cs : call_stacks) {
        if (!cs.existing_addrs) continue;
        for (const auto& raw : *cs.existing_addrs) {
            std::string key = normalize_addr(raw);
            if (key.empty()) {
                result.invalid_count++;
                result.invalid_detail_addrs.push_back(raw);
                result.invalid_detail_callstack_ids.push_back(cs.id);
                continue;
            }

            // multi detection: same addr in multiple callStacks
            auto prev_it = addr_first_cs.find(key);
            if (prev_it != addr_first_cs.end()) {
                result.multi_count++;
                result.multi_keys.push_back(key);
                result.multi_original_addrs.push_back(raw);
                result.multi_callstack_ids.push_back({prev_it->second, cs.id});
                continue;
            }

            // lookup nodeId from addr mapping
            auto node_it = addr_to_node_id.find(key);
            if (node_it == addr_to_node_id.end()) {
                result.no_node_count++;
                result.no_node_addrs.push_back(raw);
                result.no_node_callstack_ids.push_back(cs.id);
                continue;
            }
            uint32_t node_id = node_it->second;

            // only claim the addr after successful mapping
            addr_first_cs[key] = cs.id;

            auto& by_cs = node_by_cs[node_id];
            auto entry_it = by_cs.find(cs.id);
            if (entry_it == by_cs.end()) {
                mem_analyzer::NativeRefEntry entry;
                entry.handle_type = cs.type;
                entry.call_stack_id = cs.id;
                entry.addrs.push_back(raw);
                by_cs[cs.id] = std::move(entry);
            } else {
                entry_it->second.addrs.push_back(raw);
            }
        }
    }

    // flatten node_by_cs into result.node_native_ref
    for (auto& [node_id, by_cs] : node_by_cs) {
        std::vector<mem_analyzer::NativeRefEntry> entries;
        entries.reserve(by_cs.size());
        for (auto& [cs_id, entry] : by_cs) {
            entries.push_back(std::move(entry));
        }
        result.node_native_ref[node_id] = std::move(entries);
    }

    return result;
}

} // anonymous namespace

// ============================================================================
//  mem_analyzer namespace — public API
// ============================================================================

namespace mem_analyzer {

ArkTsAnalyzeResult analyze_arkts_classes(
    const std::string& heapsnapshot_path,
    const std::string& addr_file_path,
    std::vector<ResolvedCallStack>& call_stacks)
{
    ArkTsAnalyzeResult result;

    StageTimer timer("ArkTS");

    // ---- 1. Read addr file (auto-detect text / protobuf format) ----
    auto parse_result = parse_addr_file(addr_file_path);
    timer.tick("parse_addr");

    // ---- 3. Load heapsnapshot (retained sizes computed inside from_file) ----
    timer.stage("Loading heap snapshot");
    auto heap = HeapSnapshot::from_file(heapsnapshot_path);
    timer.tick("load_heap", std::to_string(heap.node_count) + " nodes, "
              + std::to_string(heap.edge_count) + " edges");

    // ---- 5. Build nativeRef from callStacks ----
    timer.stage("Building nativeRef");
    auto native_result = build_native_ref(call_stacks, parse_result.addr_to_node_id);
    timer.tick("build_native_ref");

    // ---- 6. Aggregate by class name ----
    timer.stage("Aggregating by class name");
    std::map<std::string, ArkTsClass> class_map;
    int instance_count = 0;
    int missing_node_count = 0;
    std::vector<uint32_t> missing_node_ids;
    std::unordered_map<long long, double> local_retained_size_by_call_stack;

    for (const auto& [node_id, native_ref] : native_result.node_native_ref) {
        auto node = heap.get_node_by_id(node_id);
        if (node.node_index == 0xFFFFFFFF) {
            missing_node_count++;
            missing_node_ids.push_back(node_id);
            continue;
        }

        std::string name;
        std::string type_name = heap.get_node_type_name(node.node_index);
        if (type_name == "string" || node.name == "JSObject" || node.name == "JSSharedObject") {
            // 字符串命名与快照分析保持一致：string(size: <区间>, content=xx)，
            // 最终由 formatter 按 size 区间聚合，避免条目过于分散
            name = heap.get_extended_node_name(node.node_index);
        } else {
            name = node.name.empty() ? "(unknown)" : node.name;
        }
        long long shallow_size = node.self_size;
        long long retained_size = static_cast<long long>(heap.get_retained_size(node.node_index));

        std::vector<TshandleRetainedShare> shares;
        shares.reserve(native_ref.size());
        for (const auto& ref : native_ref) {
            shares.push_back({ref.call_stack_id, ref.addrs.size()});
        }
        distribute_tshandle_retained_size(
            static_cast<double>(retained_size), shares, local_retained_size_by_call_stack);

        instance_count++;
        ArkTsInstance inst;
        inst.id = node_id;
        inst.shallow_size = shallow_size;
        inst.retained_size = retained_size;
        inst.native_ref = native_ref;

        auto cls_it = class_map.find(name);
        if (cls_it == class_map.end()) {
            ArkTsClass cls;
            cls.name = name;
            cls.shallow_size = shallow_size;
            cls.retained_size = retained_size;
            cls.instances.push_back(std::move(inst));
            class_map[name] = std::move(cls);
        } else {
            cls_it->second.instances.push_back(std::move(inst));
            cls_it->second.shallow_size += shallow_size;
            cls_it->second.retained_size += retained_size;
        }
    }
    timer.tick("aggregate", std::to_string(class_map.size()) + " classes, "
              + std::to_string(instance_count) + " instances");

    apply_tshandle_retained_sizes(call_stacks, local_retained_size_by_call_stack);

    // ---- 7. Build anomaly report data ----

    // Populate counts
    result.anomaly_counts.multi = native_result.multi_count;
    result.anomaly_counts.no_node = native_result.no_node_count;
    result.anomaly_counts.missing_node = missing_node_count;
    result.anomaly_counts.invalid_addr = native_result.invalid_count;
    result.anomaly_counts.duplicate = parse_result.duplicate_count;
    result.anomaly_counts.addr_file_invalid = parse_result.addr_file_invalid_count;

    // Build anomaly lines
    auto& lines = result.anomaly_lines;
    lines.push_back("# addr anomaly report");

    {
        std::ostringstream ss;
        ss << "# summary: multi=" << result.anomaly_counts.multi
           << " noNode=" << result.anomaly_counts.no_node
           << " missingNode=" << result.anomaly_counts.missing_node
           << " invalid=" << result.anomaly_counts.invalid_addr
           << " duplicate=" << result.anomaly_counts.duplicate
           << " addrFileInvalid=" << result.anomaly_counts.addr_file_invalid;
        lines.push_back(ss.str());
    }

    // multi anomalies
    if (!native_result.multi_keys.empty()) {
        lines.push_back("# [multi] existing addr in multiple callStacks (kept first)");
        for (size_t i = 0; i < native_result.multi_keys.size(); i++) {
            std::ostringstream ss;
            ss << native_result.multi_keys[i] << "\t"
               << native_result.multi_original_addrs[i] << "\tmulti\t";
            for (size_t j = 0; j < native_result.multi_callstack_ids[i].size(); j++) {
                if (j > 0) ss << ",";
                ss << native_result.multi_callstack_ids[i][j];
            }
            lines.push_back(ss.str());
        }
    }

    // noNode anomalies
    if (!native_result.no_node_addrs.empty()) {
        lines.push_back("# [noNode] existing addr has no nodeid mapping in addr file");
        for (size_t i = 0; i < native_result.no_node_addrs.size(); i++) {
            std::ostringstream ss;
            ss << native_result.no_node_addrs[i] << "\tnoNode\t"
               << native_result.no_node_callstack_ids[i];
            lines.push_back(ss.str());
        }
    }

    // missingNode anomalies
    if (!missing_node_ids.empty()) {
        lines.push_back("# [missingNode] nodeid not found in heapsnapshot");
        for (auto id : missing_node_ids) {
            std::ostringstream ss;
            ss << id << "\tmissingNode";
            lines.push_back(ss.str());
        }
    }

    // invalid addr (existing addr not valid hex)
    if (!native_result.invalid_detail_addrs.empty()) {
        lines.push_back("# [invalid] existing addr not valid hex");
        for (size_t i = 0; i < native_result.invalid_detail_addrs.size(); i++) {
            std::ostringstream ss;
            ss << native_result.invalid_detail_addrs[i] << "\tinvalid\t"
               << native_result.invalid_detail_callstack_ids[i];
            lines.push_back(ss.str());
        }
    }

    // duplicate anomalies
    if (!parse_result.dup_keys.empty()) {
        lines.push_back("# [duplicate] addr file: same address different nodeid (kept first)");
        for (size_t i = 0; i < parse_result.dup_keys.size(); i++) {
            std::ostringstream ss;
            ss << parse_result.dup_keys[i] << "\t"
               << parse_result.dup_original_addrs[i] << "\tduplicate\t";
            for (size_t j = 0; j < parse_result.dup_node_ids[i].size(); j++) {
                if (j > 0) ss << ",";
                ss << parse_result.dup_node_ids[i][j];
            }
            lines.push_back(ss.str());
        }
    }

    // addrFileInvalid anomalies
    if (!parse_result.invalid_original_addrs.empty()) {
        lines.push_back("# [addrFileInvalid] addr file: not valid hex");
        for (const auto& addr : parse_result.invalid_original_addrs) {
            std::ostringstream ss;
            ss << addr << "\taddrFileInvalid";
            lines.push_back(ss.str());
        }
    }

    // ---- 8. Build result ----
    result.arkts_classes.reserve(class_map.size());
    for (auto& [name, cls] : class_map) {
        result.arkts_classes.push_back(std::move(cls));
    }

    // Print statistics
    {
        std::ostringstream ss;
        ss << class_map.size() << " classes, " << instance_count << " instances, "
           << missing_node_count << " missNode, " << result.anomaly_counts.no_node
           << " noNode, " << result.anomaly_counts.multi << " multi, "
           << result.anomaly_counts.invalid_addr << " invalid, "
           << result.anomaly_counts.duplicate << " dup, "
           << result.anomaly_counts.addr_file_invalid << " addrInvalid";
        timer.tick("anomaly_report", ss.str());
    }
    timer.done();

    return result;
}

// ---------------------------------------------------------------------------
//  write_anomaly_report
// ---------------------------------------------------------------------------

std::string write_anomaly_report(const std::string& output_dir, const ArkTsAnalyzeResult& result) {
    const auto& c = result.anomaly_counts;
    bool empty = (c.multi == 0 && c.no_node == 0 && c.missing_node == 0 &&
                  c.invalid_addr == 0 && c.duplicate == 0 && c.addr_file_invalid == 0);
    if (empty) return "";

    std::string out_path = output_dir + "/addr_anomaly.txt";

    auto out_file = open_ofstream(out_path);
    if (!out_file.is_open()) {
        log::logw("  warning: cannot write anomaly report to ", out_path);
        return "";
    }

    for (const auto& line : result.anomaly_lines) {
        out_file << line << "\n";
    }

    out_file.close();

    log::logi("  addr anomaly report: ", out_path,
              " (multi=", c.multi, " noNode=", c.no_node,
              " missingNode=", c.missing_node,
              " invalid=", c.invalid_addr,
              " duplicate=", c.duplicate,
              " addrFileInvalid=", c.addr_file_invalid, ")");

    return out_path;
}

} // namespace mem_analyzer
