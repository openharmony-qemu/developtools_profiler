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
#ifndef HTRACE_MEM_PARSER_H
#define HTRACE_MEM_PARSER_H

#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_plugin_time_parser.h"
#include "memory_plugin_result.pbreader.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"
namespace SysTuning {
namespace TraceStreamer {
class HtraceMemParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceMemParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceMemParser();
    void Parse(HtraceDataSegment& seg, uint64_t, BuiltinClocks clock);
    void Finish();

private:
    void ParseProcessInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseMemInfoEasy(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseVMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseVMemInfoEasy(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseSmapsInfoEasy(const ProtoReader::ProcessMemoryInfo_Reader* memInfo, uint64_t timeStamp) const;
    std::map<MemInfoType, DataIndex> memNameDictMap_ = {};
    std::map<SysMeminfoType, DataIndex> sysMemNameDictMap_ = {};
    std::map<SysVMeminfoType, DataIndex> sysVMemNameDictMap_ = {};
    uint64_t zram_ = 0;
    const DataIndex zramIndex_ = traceDataCache_->GetDataIndex("sys.mem.zram");
    TraceStreamerConfig config_{};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_MEM_PARSER_H
