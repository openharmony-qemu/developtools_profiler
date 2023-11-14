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

#ifndef HTRACE_PARSER_H
#define HTRACE_PARSER_H
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include "common_types.h"
#include "common_types.pbreader.h"
#include "ebpf_data_parser.h"
#include "elf_parser.h"
#include "file.h"
#include "htrace_clock_detail_parser.h"
#include "htrace_cpu_detail_parser.h"
#include "htrace_cpu_data_parser.h"
#include "htrace_disk_io_parser.h"
#include "htrace_file_header.h"
#include "htrace_hidump_parser.h"
#include "htrace_hilog_parser.h"
#include "htrace_hisysevent_parser.h"
#include "htrace_js_memory_parser.h"
#include "htrace_mem_parser.h"
#include "htrace_native_hook_parser.h"
#include "htrace_network_parser.h"
#include "htrace_process_parser.h"
#include "htrace_symbols_detail_parser.h"
#include "log.h"
#include "parser_base.h"
#include "perf_data_parser.h"
#include "proto_reader_help.h"
#include "string_help.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace SysTuning::base;
using namespace OHOS::Developtools::HiPerf::ELF;
using namespace OHOS::Developtools::HiPerf;
class HtraceParser : public ParserBase, public HtracePluginTimeParser {
public:
    HtraceParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters);
    ~HtraceParser();
    void ParseTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size) override;
    bool ReparseSymbolFilesAndResymbolization(std::string& symbolsPath, std::vector<std::string>& symbolsPaths);
    void WaitForParserEnd();
    void EnableFileSeparate(bool enabled);

    void GetSymbols(std::unique_ptr<ElfFile> elfPtr,
                    std::shared_ptr<ElfSymbolTable> symbols,
                    const std::string& filename);
    bool ParserFileSO(std::string& directory, std::vector<std::string>& relativeFilePaths);
    void TraceDataSegmentEnd(bool isSplitFile);
    void StoreTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size, int32_t isFinish);
    const auto& GetTraceDataHtrace()
    {
        return mTraceDataHtrace_;
    }
    auto GetProfilerHeader()
    {
        return profilerTraceFileHeader_;
    }
    auto ClearNativehookData()
    {
        htraceNativeHookParser_->FinishSplitNativeHook();
    }
    auto GetDataSourceType()
    {
        return dataSourceType_;
    }
    auto GetJsMemoryData()
    {
        return jsMemoryParser_.get();
    }
    auto GetArkTsConfigData()
    {
        return arkTsConfigData_;
    }
    auto ClearTraceDataHtrace()
    {
        perfDataParser_->ClearPerfSplitResult();
        ebpfDataParser_->ClearEbpfSplitResult();
        processedDataLen_ = 0;
        perfProcessedLen_ = 0;
        splitFileOffset_ = 0;
        hasGotHeader_ = false;
        hasInitEbpfPublicData_ = false;
        parsedEbpfOver_ = false;
        return mTraceDataHtrace_.clear();
    }
    const auto& GetPerfSplitResult()
    {
        return perfDataParser_->GetPerfSplitResult();
    }
    const auto& GetEbpfDataParser()
    {
        return ebpfDataParser_;
    }

private:
    bool ParseDataRecursively(std::deque<uint8_t>::iterator& packagesBegin, size_t& currentLength);
    bool ParseHiperfData(std::deque<uint8_t>::iterator& packagesBegin, size_t& currentLength);
    void ParseTraceDataItem(const std::string& buffer) override;
    void FilterData(HtraceDataSegment& seg, bool isSplitFile);
    void ParserData(HtraceDataSegment& dataSeg, bool isSplitFile);

private:
    void ParseMemory(ProtoReader::ProfilerPluginData_Reader* pluginDataZero, HtraceDataSegment& dataSeg);
    void ParseHilog(HtraceDataSegment& dataSeg);
    void ParseFtrace(HtraceDataSegment& dataSeg);
    void ParseFPS(HtraceDataSegment& dataSeg);
    void ParseCpuUsage(HtraceDataSegment& dataSeg);
    void ParseNetwork(HtraceDataSegment& dataSeg);
    void ParseDiskIO(HtraceDataSegment& dataSeg);
    void ParseProcess(HtraceDataSegment& dataSeg);
    void ParseHisysevent(HtraceDataSegment& dataSeg);
    void ParseHisyseventConfig(HtraceDataSegment& dataSeg);
    void ParseJSMemory(HtraceDataSegment& dataSeg);
    void ParseJSMemoryConfig(HtraceDataSegment& dataSeg);
    void ParseThread();
    int32_t GetNextSegment();
    void FilterThread();
    bool CalcEbpfCutOffset(std::deque<uint8_t>::iterator& packagesBegin, size_t& currentLength);

    bool InitProfilerTraceFileHeader();
    ProfilerTraceFileHeader profilerTraceFileHeader_;
    uint32_t profilerDataType_ = ProfilerTraceFileHeader::UNKNOW_TYPE;
    uint64_t profilerDataLength_ = 0;
    ProfilerPluginDataHeader profilerPluginData_;
    uint64_t htraceCurentLength_ = 0;
    char standalonePluginName_[ProfilerTraceFileHeader::PLUGIN_MODULE_NAME_MAX + 1] = "";
    bool hasGotSegLength_ = false;
    bool hasGotHeader_ = false;
    uint32_t nextLength_ = 0;
    const size_t PACKET_SEG_LENGTH = 4;
    const size_t PACKET_HEADER_LENGTH = 1024;
    TraceDataCache* traceDataCache_;
    std::unique_ptr<HtraceCpuDetailParser> htraceCpuDetailParser_;
    std::unique_ptr<HtraceSymbolsDetailParser> htraceSymbolsDetailParser_;
    std::unique_ptr<HtraceMemParser> htraceMemParser_;
    std::unique_ptr<HtraceClockDetailParser> htraceClockDetailParser_;
    std::unique_ptr<HtraceHiLogParser> htraceHiLogParser_;
    std::unique_ptr<HtraceNativeHookParser> htraceNativeHookParser_;
    std::unique_ptr<HtraceHidumpParser> htraceHidumpParser_;
    std::unique_ptr<HtraceCpuDataParser> cpuUsageParser_;
    std::unique_ptr<HtraceNetworkParser> networkParser_;
    std::unique_ptr<HtraceDiskIOParser> diskIOParser_;
    std::unique_ptr<HtraceProcessParser> processParser_;
    std::unique_ptr<HtraceHisyseventParser> hisyseventParser_;
    std::unique_ptr<HtraceJSMemoryParser> jsMemoryParser_;
    std::unique_ptr<PerfDataParser> perfDataParser_;
    std::unique_ptr<EbpfDataParser> ebpfDataParser_;
    std::atomic<bool> filterThreadStarted_{false};
    const int32_t MAX_SEG_ARRAY_SIZE = 10000;
    std::unique_ptr<HtraceDataSegment[]> dataSegArray_;
    int32_t rawDataHead_ = 0;
    bool toExit_ = false;
    bool exited_ = false;
    int32_t filterHead_ = 0;
    int32_t parseHead_ = 0;
    size_t htraceLength_ = 1024;
    const int32_t sleepDur_ = 100;
    bool parseThreadStarted_ = false;
    const int32_t maxThread_ = 4; // 4 is the best on ubuntu 113MB/s, max 138MB/s, 6 is best on mac m1 21MB/s,
    int32_t parserThreadCount_ = 0;
    std::mutex htraceDataSegMux_ = {};
    bool supportThread_ = false;
    ClockId dataSourceTypeTraceClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeMemClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeHilogClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeNativeHookClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeFpsClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeNetworkClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeDiskioClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeCpuClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeProcessClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeHisyseventClockid_ = TS_CLOCK_UNKNOW;
    ClockId dataSourceTypeJSMemoryClockid_ = TS_CLOCK_UNKNOW;
    std::shared_ptr<std::vector<std::shared_ptr<ElfSymbolTable>>> elfSymbolTables_;
    std::map<int32_t, int32_t> mTraceDataHtrace_ = {};
    std::string traceDataHtrace_ = "";
    uint64_t splitFileOffset_ = 0;
    uint64_t processedDataLen_ = 0;
    uint64_t perfProcessedLen_ = 0;
    uint64_t parsedFileOffset_ = 0;
    bool hasInitEbpfPublicData_ = false;
    bool parsedEbpfOver_ = false;
    uint32_t dataSourceType_ = INVALID_UINT32;
    std::string arkTsConfigData_ = "";
    std::string lenBuffer_ = "";
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_PARSER_H_
