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
#ifndef HTRACE_JS_MEMORY_PARSER_H
#define HTRACE_JS_MEMORY_PARSER_H
#include <cstdint>
#include <string>
#include <vector>
#include "common_types.h"
#include "common_types.pb.h"
#include "event_parser_base.h"
#include "htrace_file_header.h"
#include "htrace_js_cpu_profiler_parser.h"
#include "htrace_plugin_time_parser.h"
#include "js_heap_result.pb.h"
#include "json.hpp"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"
using json = nlohmann::json;

namespace SysTuning {
namespace TraceStreamer {
struct SnapShotData {
    uint64_t startTime;
    uint64_t endTime;
    std::string snapshotData;
};
class HtraceJSMemoryParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceJSMemoryParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceJSMemoryParser();
    void ParseJSMemoryConfig(ProtoReader::BytesView tracePacket);
    void Parse(ProtoReader::BytesView tracePacket,
               uint64_t ts,
               uint64_t startTime,
               uint64_t endTime,
               ProfilerPluginDataHeader profilerPluginData);
    void EnableSaveFile(bool enable);
    void Finish();
    auto GetArkTsSplitFileData()
    {
        return profilerArktsData_;
    }
    auto GetArkTsSize()
    {
        return profilerArktsData_.size();
    }
    void ClearArkTsSplitFileData()
    {
        jsMemorySplitFileData_ = "";
        cpuProfilerSplitFileData_ = "";
        arkTsSplitFileDataResult_ = "";
        profilerArktsData_ = "";
    }

private:
    void ParseTimeLine(ProfilerPluginDataHeader& profilerPluginData, const std::string& jsonString);
    void ParseSnapshot(ProtoReader::BytesView& tracePacket,
                       ProfilerPluginDataHeader& profilerPluginData,
                       const std::string& jsonString,
                       uint64_t& ts);
    void ParseSnapshotOrTimeLineEnd(const std::string& result,
                                    ProtoReader::BytesView& tracePacket,
                                    ProfilerPluginDataHeader& profilerPluginData,
                                    uint64_t ts);
    void ParseJsCpuProfiler(const std::string& result, ProfilerPluginDataHeader& profilerPluginData, uint64_t ts);
    void ParserJSSnapInfo(int32_t fileId, const json& jMessage);
    void ParseNodes(int32_t fileId, const json& jMessage, uint64_t endTime, bool isSplitFile);
    void ParseEdges(int32_t fileId, const json& jMessage);
    void ParseLocation(int32_t fileId, const json& jMessage);
    void ParseSample(int32_t fileId, const json& jMessage, uint64_t startTime, uint64_t endTime, bool isSplitFile);
    void ParseString(int32_t fileId, const json& jMessage);
    void ParseTraceFuncInfo(int32_t fileId, const json& jMessage);
    void ParseTraceNode(int32_t fileId, const json& jMessage);
    void ParserSnapInfo(int32_t fileId, const std::string& key, const std::vector<std::vector<std::string>>& types);
    void SerializeToString(const ProfilerPluginDataHeader& profilerPluginData, uint64_t startTime, uint64_t endTime);
    void SerializeSnapshotData(ProfilerPluginData& profilerPluginDataResult, ArkTSResult& jsHeapResult);
    void SerializeTimelineData(uint64_t startTime,
                               uint64_t endTime,
                               ProfilerPluginData& profilerPluginDataResult,
                               ArkTSResult& jsHeapResult);
    void SerializeCpuProfilerData(uint64_t startTime,
                                  uint64_t endTime,
                                  ProfilerPluginData& profilerPluginDataResult,
                                  ArkTSResult& jsHeapResult);
    struct timespec TimeToTimespec(uint64_t timeMs);
    int32_t type_ = 0;
    const std::string snapshotEnd_ = "{\"id\":1,\"result\":{}}";
    const std::string timeLineEnd_ = "{\"id\":2,\"result\":{}}";
    const std::string jsCpuProfilerStart_ = "{\"id\":3,\"result\":{}}";
    uint64_t startTime_ = std::numeric_limits<uint64_t>::max();
    bool isFirst_ = true;
    bool cpuTimeFirst_ = true;
    std::string jsMemoryString_ = "";
    int32_t fileId_ = 0;
    int32_t jsFileId_ = 0;
    std::list<int32_t> fileIds_ = {};
    uint64_t selfSizeCount_ = 0;
    bool enableFileSave_ = false;
    const std::string tmpJsMemoryTimelineData_ = "ts_tmp.jsmemory_timeline.heapsnapshot";
    const std::string tmpJsMemorySnapshotData_ = "ts_tmp.jsmemory_snapshot";
    const std::string jsSnapshotFileTail = ".heapsnapshot";
    const std::string tmpJsCpuProfilerData_ = "Profile";
    const std::string jsCpuProFiler = ".cpuprofile";
    std::unique_ptr<HtraceJsCpuProfilerParser> jsCpuProfilerParser_;
    std::string jsMemorySplitFileData_ = "";
    std::string cpuProfilerSplitFileData_ = "";
    std::string arkTsSplitFileDataResult_ = "";
    std::string profilerArktsData_ = "";
    json updatedJson_;
    uint32_t nodeFileId_ = INVALID_UINT32;
    uint32_t nodeCount_ = 0;
    bool hasCpuProfiler_ = false;
    SnapShotData snapShotData_;
    bool curTypeIsCpuProfile_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_JS_MEMORY_PARSER_H
