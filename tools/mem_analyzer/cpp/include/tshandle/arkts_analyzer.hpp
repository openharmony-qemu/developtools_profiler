#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

#include "tshandle/tshandle_processor.hpp"
#include "shared/heapsnapshot/heap_snapshot.hpp"
#include "tshandle/map_parser.hpp"

namespace mem_analyzer {

// NativeRefEntry, ArkTsInstance, ArkTsClass, AnomalyCounts are now defined in
// tshandle/tshandle_processor.hpp (shared with TshandleParseResult).

struct ArkTsAnalyzeResult {
    std::vector<ArkTsClass> arkts_classes;
    AnomalyCounts anomaly_counts;
    std::vector<std::string> anomaly_lines;  // lines for addr_anomaly.txt
};

/**
 * Full ArkTS class analysis pipeline:
 * 1. Parse addr file → address→nodeId mapping
 * 2. Load heapsnapshot (retained sizes computed inside from_file, LT)
 * 3. Build nativeRef from callStacks.existingAddrs
 * 4. Aggregate by class name
 * 5. Generate anomaly report
 */
ArkTsAnalyzeResult analyze_arkts_classes(
    const std::string& heapsnapshot_path,
    const std::string& addr_file_path,
    std::vector<ResolvedCallStack>& call_stacks
);

/**
 * Write anomaly report to file (if any anomalies).
 * Returns the file path written to, or empty string if no anomalies.
 */
std::string write_anomaly_report(const std::string& output_dir, const ArkTsAnalyzeResult& result);

} // namespace mem_analyzer
