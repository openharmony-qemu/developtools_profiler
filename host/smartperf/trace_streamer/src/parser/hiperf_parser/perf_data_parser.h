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
#ifndef PERF_DATA_PARSER_H
#define PERF_DATA_PARSER_H
#include <linux/perf_event.h>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <set>
#include "event_parser_base.h"
#include "htrace_plugin_time_parser.h"
#include "log.h"
#if is_mingw
#define unw_word_t uint64_t
#endif
#include "perf_events.h"
#include "perf_file_format.h"
#include "perf_file_reader.h"
#include "quatra_map.h"
#include "report.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace OHOS::Developtools::HiPerf;
class PerfDataParser : public EventParserBase, public HtracePluginTimeParser {
public:
    PerfDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~PerfDataParser();
    void InitPerfDataAndLoad(const std::deque<uint8_t> dequeBuffer, uint64_t size);
    void Finish();
    bool PerfReloadSymbolFiles(std::vector<std::string>& symbolsPaths);

private:
    bool Reload();
    bool LoadPerfData();
    void UpdateEventConfigInfo();
    void UpdateCmdlineInfo() const;
    void LoadEventDesc();
    void UpdateReportWorkloadInfo() const;
    void UpdateSymbolAndFilesData();
    void UpdateClockType();
    bool RecordCallBack(std::unique_ptr<PerfEventRecord> record);
    void UpdatePerfSampleData(uint32_t callChainId, std::unique_ptr<PerfRecordSample>& sample);
    uint32_t UpdatePerfCallChainData(const std::unique_ptr<PerfRecordSample>& sample);

    class CallStackTemp {
    public:
        CallStackTemp() {}
        CallStackTemp(uint32_t depth, uint64_t vaddr, uint64_t fileId, uint32_t symbolId)
            : depth_(depth), vaddrInFile_(vaddr), fileId_(fileId), symbolId_(symbolId)
        {
        }
        ~CallStackTemp() {}
        uint32_t depth_ = 0;
        uint64_t vaddrInFile_ = 0;
        uint64_t fileId_ = 0;
        uint32_t symbolId_ = 0;
    };
    uint32_t callChainId_ = 0;
    std::unique_ptr<PerfFileReader> recordDataReader_ = nullptr;
    const std::string cpuOffEventName_ = "sched:sched_switch";
    const std::string wakingEventName_ = "sched:sched_waking";
    std::unique_ptr<uint8_t[]> buffer_ = {};
    size_t bufferSize_ = 0;
    bool cpuOffMode_ = false;
    std::unique_ptr<Report> report_ = nullptr;
    uint32_t useClockId_ = 0;
    uint32_t clockId_ = 0;
    enum PerfClockType {
        PERF_CLOCK_REALTIME = 0,
        PERF_CLOCK_MONOTONIC,
        PERF_CLOCK_MONOTONIC_RAW = 4,
        PERF_CLOCK_BOOTTIME = 7,
    };
    DataIndex configNameIndex_ = 0;
    DataIndex workloaderIndex_ = 0;
    DataIndex cmdlineIndex_ = 0;
    DataIndex runingStateIndex_ = 0;
    DataIndex suspendStatIndex_ = 0;
    DataIndex unkonwnStateIndex_ = 0;
    std::unordered_multimap<uint64_t, uint64_t> tidToPid_ = {};
    const std::map<uint32_t, uint32_t> perfToTSClockType_ = {{PERF_CLOCK_REALTIME, TS_CLOCK_REALTIME},
                                                             {PERF_CLOCK_MONOTONIC, TS_MONOTONIC},
                                                             {PERF_CLOCK_MONOTONIC_RAW, TS_MONOTONIC_RAW},
                                                             {PERF_CLOCK_BOOTTIME, TS_CLOCK_BOOTTIME}};
    std::map<uint64_t, uint64_t> fileDataDictIdToFileId_ = {};
    QuatraMap<uint64_t, uint64_t, uint64_t, uint64_t, uint32_t> frameToCallChainId_;
    const std::string tmpPerfData_ = "ts_tmp.perf.data";
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // PERF_DATA_PARSER_H
