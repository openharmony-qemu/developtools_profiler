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
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_file_header.h"
#include "htrace_plugin_time_parser.h"
#include "log.h"
#if is_mingw
#define unw_word_t uint64_t
#endif
#include "numerical_to_string.h"
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
enum class SplitPerfState {
    STARTING,
    PARSING_HEAD,
    WAIT_FOR_ATTR,
    PARSING_ATTR,
    WAIT_FOR_DATA,
    PARSING_DATA,
    PARSING_FEATURE_SECTION,
    WAIT_FOR_FINISH
};
class PerfDataParser : public EventParserBase, public HtracePluginTimeParser {
public:
    PerfDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~PerfDataParser();
    uint64_t InitPerfDataAndLoad(const std::deque<uint8_t> dequeBuffer,
                                 uint64_t size,
                                 uint64_t offset,
                                 bool isSplitFile,
                                 bool isFinish);
    void Finish();
    bool PerfReloadSymbolFiles(std::vector<std::string>& symbolsPaths);
    const auto& GetPerfSplitResult()
    {
        return splitResult_;
    }
    void ClearPerfSplitResult()
    {
        SplitDataWithdraw();
        perfSplitError_ = false;
        if (traceDataCache_->traceStartTime_ == INVALID_UINT64 || traceDataCache_->traceEndTime_ == 0) {
            traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
        }
        return;
    }
    void SplitDataWithdraw()
    {
        processedLength_ = 0;
        splitDataEnd_ = false;
        perfDataOffset_ = 0;
        featureSection_.reset();
        featureSectioSize_ = 0;
        splitState_ = SplitPerfState::STARTING;
        splitResult_.clear();
        splitDataSize_ = 0;
        return;
    }
    void RecordPerfProfilerHeader(uint8_t* buffer, uint32_t len)
    {
        (void)memcpy_s(&profilerHeader_, sizeof(profilerHeader_), buffer, len);
        hasProfilerHead_ = true;
    }

private:
    bool Reload();
    bool LoadPerfData();
    void UpdateEventConfigInfo();
    void UpdateCmdlineInfo() const;
    void LoadEventDesc();
    void ProcessUniStackTableData();
    void UpdateReportWorkloadInfo() const;
    void UpdateSymbolAndFilesData();
    void UpdateClockType();
    bool RecordCallBack(std::unique_ptr<PerfEventRecord> record);
    void UpdatePerfSampleData(uint32_t callChainId, std::unique_ptr<PerfRecordSample>& sample);
    uint32_t UpdateCallChainUnCompressed(const std::unique_ptr<PerfRecordSample>& sample);
    uint32_t UpdateCallChainCompressed(const std::unique_ptr<PerfRecordSample>& sample);
    bool PerfSplitCallBack(std::unique_ptr<PerfEventRecord> record);
    uint64_t SplitPerfData(const std::deque<uint8_t>& dequeBuffer, uint64_t size, uint64_t offset, bool isFinish);
    bool SplitPerfStarting(const std::deque<uint8_t>& dequeBuffer,
                           uint64_t size,
                           uint64_t& processedLen,
                           bool& invalid);
    bool SplitPerfParsingHead(const std::deque<uint8_t>& dequeBuffer,
                              uint64_t size,
                              uint64_t& processedLen,
                              bool& invalid);
    bool SplitPerfWaitForAttr(const std::deque<uint8_t>& dequeBuffer,
                              uint64_t size,
                              uint64_t& processedLen,
                              bool& invalid);
    bool SplitPerfParsingAttr(const std::deque<uint8_t>& dequeBuffer,
                              uint64_t size,
                              uint64_t& processedLen,
                              bool& invalid);
    bool SplitPerfWaitForData(const std::deque<uint8_t>& dequeBuffer,
                              uint64_t size,
                              uint64_t& processedLen,
                              bool& invalid);
    bool SplitPerfParsingData(const std::deque<uint8_t>& dequeBuffer,
                              uint64_t size,
                              uint64_t& processedLen,
                              bool& invalid);
    bool SplitPerfParsingFeatureSection(const std::deque<uint8_t>& dequeBuffer,
                                        uint64_t size,
                                        uint64_t& processedLen,
                                        bool& invalid);
    bool SplitPerfWaitForFinish(const std::deque<uint8_t>& dequeBuffer,
                                uint64_t size,
                                uint64_t& processedLen,
                                bool& invalid);

    uint32_t callChainId_ = 0;
    std::unique_ptr<PerfFileReader> recordDataReader_ = nullptr;
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
    std::hash<std::string_view> hashFun_;
    DoubleMap<uint32_t, uint64_t, uint32_t> pidAndStackHashToCallChainId_;
    const std::string tmpPerfData_ = "ts_tmp.perf.data";
    const std::string cpuOffEventName_ = "sched:sched_switch";
    const std::string wakingEventName_ = "sched:sched_waking";

    ProfilerTraceFileHeader profilerHeader_;
    bool hasProfilerHead_ = false;
    perf_file_header perfHeader_;
    uint32_t featureCount_ = 0;
    uint64_t sampleType_ = 0;
    uint32_t sampleTimeOffset_ = 0;
    uint64_t processedLength_ = 0;
    bool splitDataEnd_ = false;
    bool perfSplitError_ = false;
    int32_t perfDataOffset_ = 0;
    std::unique_ptr<uint8_t[]> featureSection_ = nullptr;
    size_t featureSectioSize_ = 0;
    SplitPerfState splitState_ = SplitPerfState::STARTING;
    uint64_t splitDataSize_ = 0;
    std::vector<HtraceSplitResult> splitResult_;
    bool stackCompressedMode_ = false;
    std::set<uint32_t> savedCompressedCallChainId_ = {};
    uint32_t compressFailedCallChainId_ = INVALID_UINT32;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // PERF_DATA_PARSER_H
