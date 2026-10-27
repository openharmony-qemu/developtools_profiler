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
