/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef METRICS_H
#define METRICS_H
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "json.hpp"
#include "log.h"
#include "memAggStrategy.h"
#include "memStrategy.h"
#include "metaDataStrategy.h"
#include "sysCallStrategy.h"
#include "traceStateStrategy.h"
#include "traceTaskStrategy.h"

enum METRICS_INDEX {
    METRICS_TRACE_MEM,
    METRICS_TRACE_MEM_TOP_TEN,
    METRICS_TRACE_MEM_UNAGG,
    METRICS_TRACE_TASK_NAMES,
    METRICS_TRACE_STATS,
    METRICS_TRACE_METADATA,
    METRICS_SYS_CALLS,
};

const std::string TRACE_MEM = "trace_mem";
const std::string TRACE_MEM_TOP_TEN = "trace_mem_top10";
const std::string TRACE_MEM_UNAGG = "trace_mem_unagg";
const std::string TRACE_TASK_NAMES = "trace_task_names";
const std::string TRACE_STATS = "trace_stats";
const std::string TRACE_METADATA = "trace_metadata";
const std::string SYS_CALLS = "sys_calls";
const std::string PROCESS_METRICES = "process_metrics:{";
const std::string PROCESS_NAME = "process_name:";
const std::string OVERALL_COUNTERS = "overall_counters:{";
const std::string ANON_RSS = "anon_rss:{";
const std::string MIN = "min:";
const std::string MAX = "max:";
const std::string AVG = "avg:";
const std::string PROCESS_VALUES = "process_value:{";
const std::string TS = "ts:";
const std::string OOM_SCORE = "oom_score:";
const std::string VALUE = "value:";
const std::string FILE_RSS = "file_rss:{";
const std::string SWAP = "swap:{";
const std::string ANON_AND_SWAP = "anon_and_swap:{";
const std::string PROCESS = "process:{";
const std::string PID = "pid:";
const std::string THREAD_NAME = "thread_name:";
const std::string STAT = "stat:{";
const std::string NAME = "name:";
const std::string COUNT = "count:";
const std::string SOURCE = "source:";
const std::string SEVERITY = "severity:";
const std::string FUNCTION = "function:{";
const std::string FUNCTION_NAME = "function_name:";
const std::string DUR_MAX = "dur_max:";
const std::string DUR_MIN = "dur_min:";
const std::string DUR_AVG = "dur_avg:";

namespace SysTuning {
namespace TraceStreamer {
using json = nlohmann::json;
class Metrics {
public:
    Metrics();
    ~Metrics() {}
    using ResultCallBack = std::function<void(const std::string /* json result */, int32_t)>;
    void ParserJson(const std::string& metrics, std::string& result);
    void PrintMetricsResult(uint32_t metricsIndex, ResultCallBack callback);
    auto GetMetricsMap()
    {
        return initMetricsMap_;
    }

private:
    using FuncCall = std::function<void(const std::string& result)>;
    std::map<std::string, FuncCall> metricsFunction_ = {};
    void InitMemoryStrategy(const std::string& result);
    void InitMemoryUnAggStrategy(const std::string& result);
    void InitMemoryTaskNameStrategy(const std::string& result);
    void InitTraceStatsStrategy(const std::string& result);
    void InitTraceMetaDataStrategy(const std::string& result);
    void InitSysCallStrategy(const std::string& result);
    std::string JsonFormat(std::string json);
    std::string GetLevelSpace(int level);
    std::vector<ProcessMetricsItems> memStrategy_ = {};
    std::vector<ProcessValuesItem> memAggStrategy_ = {};
    std::vector<TaskProcessItem> taskNameStrategy_ = {};
    std::vector<StatItem> statStrategy_ = {};
    std::vector<TraceMetadataItem> metaDataStrategy_ = {};
    std::vector<FunctionItem> sysCallStrategy_ = {};
    std::map<int, std::string> initMetricsMap_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // METRICS_H