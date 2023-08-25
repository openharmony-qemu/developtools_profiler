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
    void ParseMemoryConfig(HtraceDataSegment& seg);
    void Finish();
    enum smaps_mem_type {
        SMAPS_MEM_TYPE_CODE_SYS = 0,     // 系统代码段
        SMAPS_MEM_TYPE_CODE_APP = 1,     // 应用代码段
        SMAPS_MEM_TYPE_DATA_SYS = 2,     // 系统数据段
        SMAPS_MEM_TYPE_DATA_APP = 3,     // 应用数据段
        SMAPS_MEM_TYPE_UNKNOWN_ANON = 4, // 系统未知匿名内存
        SMAPS_MEM_TYPE_STACK = 5,        // 栈
        SMAPS_MEM_TYPE_JS_HEAP = 6,      // js堆
        SMAPS_MEM_TYPE_JAVA_VM = 7,      // java虚拟机
        SMAPS_MEM_TYPE_NATIVE_HEAP = 8,  // native堆
        SMAPS_MEM_TYPE_ASHMEM = 9,       // Ashmem
        SMAPS_MEM_TYPE_OTHER_SYS = 10,   // 系统其他杂类资源
        SMAPS_MEM_TYPE_OTHER_APP = 11,   // 应用其他杂类资源
    };
    enum mem_process_type {
        PID_TYPE_COMPOSER = 0,
        PID_TYPE_RENDER_SERVICES,
        PID_TYPE_APP,
    };
    enum mem_deduplicate_flag {
        MEM_DEDUPLICATE_FLAG_NOMAL = 0,
        MEM_DEDUPLICATE_FLAG_DUP_SAME_PROCESS,
        MEM_DEDUPLICATE_FLAG_DUP_DIFF_PROCESS,
    };

private:
    void ParseProcessInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseMemInfoEasy(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseVMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseVMemInfoEasy(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseSmapsInfoEasy(const ProtoReader::ProcessMemoryInfo_Reader* memInfo,
                            uint64_t timeStamp,
                            uint64_t ipid) const;
    uint32_t ParseSmapsBlockType(ProtoReader::SmapsInfo_Reader& smapsInfo) const;
    void ParseAshmemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseDmaMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseGpuProcessMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void ParseGpuWindowMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const;
    void AshMemDeduplicate() const;
    void DmaMemDeduplicate() const;
    mem_process_type GetMemProcessType(uint64_t ipid) const;

    std::map<MemInfoType, DataIndex> memNameDictMap_ = {};
    std::map<uint32_t, DataIndex> sysMemNameDictMap_ = {};
    std::map<uint32_t, DataIndex> sysVMemNameDictMap_ = {};
    uint64_t zram_ = 0;
    uint64_t gpuLimit_ = 0;
    uint64_t gpuUsed_ = 0;
    const DataIndex zramIndex_ = traceDataCache_->GetDataIndex("sys.mem.zram");
    const DataIndex gpuLimitSizeIndex_ = traceDataCache_->GetDataIndex("sys.mem.gpu.limit");
    const DataIndex gpuUsedSizeIndex_ = traceDataCache_->GetDataIndex("sys.mem.gpu.used");

    TraceStreamerConfig config_{};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_MEM_PARSER_H
