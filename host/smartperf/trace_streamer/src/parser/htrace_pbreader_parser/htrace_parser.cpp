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

#include "htrace_parser.h"
#include <unistd.h>
#include "app_start_filter.h"
#include "binder_filter.h"
#include "common_types.pbreader.h"
#include "cpu_filter.h"
#include "data_area.h"
#include "ftrace_event.pbreader.h"
#include "log.h"
#include "memory_plugin_result.pbreader.h"
#include "stat_filter.h"
#include "trace_plugin_result.pbreader.h"
#if IS_WASM
#include "../rpc/wasm_func.h"
#endif
namespace SysTuning {
namespace TraceStreamer {
HtraceParser::HtraceParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters)
    : ParserBase(filters),
      traceDataCache_(dataCache),
      htraceCpuDetailParser_(std::make_unique<HtraceCpuDetailParser>(dataCache, filters)),
      htraceSymbolsDetailParser_(std::make_unique<HtraceSymbolsDetailParser>(dataCache, filters)),
      htraceMemParser_(std::make_unique<HtraceMemParser>(dataCache, filters)),
      htraceClockDetailParser_(std::make_unique<HtraceClockDetailParser>(dataCache, filters)),
      htraceHiLogParser_(std::make_unique<HtraceHiLogParser>(dataCache, filters)),
      htraceNativeHookParser_(std::make_unique<HtraceNativeHookParser>(dataCache, filters)),
      htraceHidumpParser_(std::make_unique<HtraceHidumpParser>(dataCache, filters)),
      cpuUsageParser_(std::make_unique<HtraceCpuDataParser>(dataCache, filters)),
      networkParser_(std::make_unique<HtraceNetworkParser>(dataCache, filters)),
      diskIOParser_(std::make_unique<HtraceDiskIOParser>(dataCache, filters)),
      processParser_(std::make_unique<HtraceProcessParser>(dataCache, filters)),
      hisyseventParser_(std::make_unique<HtraceHisyseventParser>(dataCache, filters)),
      jsMemoryParser_(std::make_unique<HtraceJSMemoryParser>(dataCache, filters)),
      perfDataParser_(std::make_unique<PerfDataParser>(dataCache, filters)),
#ifdef SUPPORTTHREAD
      supportThread_(true),
      dataSegArray_(std::make_unique<HtraceDataSegment[]>(MAX_SEG_ARRAY_SIZE))
#else
      ebpfDataParser_(std::make_unique<EbpfDataParser>(dataCache, filters)),
      dataSegArray_(std::make_unique<HtraceDataSegment[]>(1))
#endif
{
}
void HtraceParser::GetSymbols(std::unique_ptr<ElfFile> elfPtr,
                              std::shared_ptr<ElfSymbolTable> symbols,
                              const std::string& filename)
{
    symbols->filePathIndex = traceDataCache_->dataDict_.GetStringIndex(filename.c_str());
    symbols->textVaddr = (std::numeric_limits<uint64_t>::max)();
    for (auto& item : elfPtr->phdrs_) {
        if ((item->type_ == PT_LOAD) && (item->flags_ & PF_X)) {
            // find the min addr
            if (symbols->textVaddr != (std::min)(symbols->textVaddr, item->vaddr_)) {
                symbols->textVaddr = (std::min)(symbols->textVaddr, item->vaddr_);
                symbols->textOffset = item->offset_;
            }
        }
    }
    if (symbols->textVaddr == (std::numeric_limits<uint64_t>::max)()) {
        TS_LOGE("GetSymbols get textVaddr failed");
        return;
    }

    std::string symSecName;
    std::string strSecName;
    if (elfPtr->shdrs_.find(".symtab") != elfPtr->shdrs_.end()) {
        symSecName = ".symtab";
        strSecName = ".strtab";
    } else if (elfPtr->shdrs_.find(".dynsym") != elfPtr->shdrs_.end()) {
        symSecName = ".dynsym";
        strSecName = ".dynstr";
    } else {
        return;
    }
    const auto& sym = elfPtr->shdrs_[static_cast<const std::string>(symSecName)];
    const uint8_t* symData = elfPtr->GetSectionData(sym->secIndex_);
    const auto& str = elfPtr->shdrs_[static_cast<const std::string>(strSecName)];
    const uint8_t* strData = elfPtr->GetSectionData(str->secIndex_);

    if (!sym->secSize_ || !str->secSize_) {
        TS_LOGE(
            "GetSymbols get section size failed, \
            sym size: %" PRIu64 ", str size: %" PRIu64 "",
            sym->secSize_, str->secSize_);
        return;
    }
    symbols->symEntSize = sym->secEntrySize_;
    std::string symTable(symData, symData + sym->secSize_);
    symbols->symTable = std::move(symTable);
    std::string strTable(strData, strData + str->secSize_);
    symbols->strTable = std::move(strTable);
}

bool HtraceParser::ParserFileSO(std::string& directory, std::vector<std::string>& relativeFilePaths)
{
    elfSymbolTables_ = std::make_shared<std::vector<std::shared_ptr<ElfSymbolTable>>>();
    std::cout << "start Parser File so" << std::endl;
    for (auto relativeFilePath : relativeFilePaths) {
        if (relativeFilePath.compare(0, directory.length(), directory)) {
            TS_LOGI("%s not in directory %s", relativeFilePath.c_str(), directory.c_str());
            continue;
        }
        std::unique_ptr<ElfFile> elfFile = ElfFile::MakeUnique(relativeFilePath);
        if (elfFile == nullptr) {
            TS_LOGI("elf %s load failed", relativeFilePath.c_str());
            continue;
        } else {
            TS_LOGI("loaded elf %s", relativeFilePath.c_str());
        }
        auto symbolInfo = std::make_shared<ElfSymbolTable>();
        auto absoluteFilePath = relativeFilePath.substr(directory.length());
        GetSymbols(std::move(elfFile), symbolInfo, absoluteFilePath);
        elfSymbolTables_->emplace_back(symbolInfo);
    }
    if (!elfSymbolTables_->size()) {
        return false;
    }
    return true;
}

HtraceParser::~HtraceParser()
{
    TS_LOGI("clockid 2 is for RealTime and 1 is for BootTime");
}

bool HtraceParser::ReparseSymbolFilesAndResymbolization(std::string& symbolsPath,
                                                        std::vector<std::string>& symbolsPaths)
{
    auto parsePerfStatus = false;
    std::vector<std::string> dir;
    dir.emplace_back(symbolsPath);
    parsePerfStatus = perfDataParser_->PerfReloadSymbolFiles(dir);
    auto parseFileSOStatus = ParserFileSO(symbolsPath, symbolsPaths);
    if (!parseFileSOStatus) {
        elfSymbolTables_.reset();
        return parsePerfStatus;
    }
    if (htraceNativeHookParser_->SupportImportSymbolTable()) {
        htraceNativeHookParser_->NativeHookReloadElfSymbolTable(elfSymbolTables_);
    }
    if (ebpfDataParser_->SupportImportSymbolTable()) {
        ebpfDataParser_->EBPFReloadElfSymbolTable(elfSymbolTables_);
    }
    elfSymbolTables_.reset();
    return true;
}
void HtraceParser::WaitForParserEnd()
{
    if (parseThreadStarted_ || filterThreadStarted_) {
        toExit_ = true;
        while (!exited_) {
            usleep(sleepDur_ * sleepDur_);
        }
    }
    hasGotHeader_ = false;
    htraceCpuDetailParser_->FilterAllEvents();
    htraceNativeHookParser_->FinishParseNativeHookData();
    htraceHiLogParser_->Finish();
    htraceHidumpParser_->Finish();
    cpuUsageParser_->Finish();
    networkParser_->Finish();
    processParser_->Finish();
    diskIOParser_->Finish();
    hisyseventParser_->Finish();
    jsMemoryParser_->Finish();
    // keep final upate perf and ebpf data time range
    ebpfDataParser_->Finish();
    perfDataParser_->Finish();
    htraceNativeHookParser_->Finish();
    htraceMemParser_->Finish();
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_TRACE,
                                                                      dataSourceTypeTraceClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_MEM, dataSourceTypeMemClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_HILOG,
                                                                      dataSourceTypeHilogClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_NATIVEHOOK,
                                                                      dataSourceTypeNativeHookClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_FPS, dataSourceTypeFpsClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_NETWORK,
                                                                      dataSourceTypeNetworkClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_DISKIO,
                                                                      dataSourceTypeDiskioClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_CPU, dataSourceTypeCpuClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_PROCESS,
                                                                      dataSourceTypeProcessClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_HISYSEVENT,
                                                                      dataSourceTypeHisyseventClockid_);
    traceDataCache_->GetDataSourceClockIdData()->SetDataSourceClockId(DATA_SOURCE_TYPE_JSMEMORY,
                                                                      dataSourceTypeJSMemoryClockid_);
    traceDataCache_->GetDataSourceClockIdData()->Finish();
    dataSegArray_.reset();
    processedDataLen_ = 0;
}

void HtraceParser::ParseTraceDataItem(const std::string& buffer)
{
    int32_t head = rawDataHead_;
    if (!supportThread_) {
        dataSegArray_[head].seg = std::make_shared<std::string>(std::move(buffer));
        dataSegArray_[head].status = TS_PARSE_STATUS_SEPRATED;
        ParserData(dataSegArray_[head], traceDataCache_->isSplitFile_);
        return;
    }
    while (!toExit_) {
        if (dataSegArray_[head].status.load() != TS_PARSE_STATUS_INIT) {
            usleep(sleepDur_);
            continue;
        }
        dataSegArray_[head].seg = std::make_shared<std::string>(std::move(buffer));
        dataSegArray_[head].status = TS_PARSE_STATUS_SEPRATED;
        rawDataHead_ = (rawDataHead_ + 1) % MAX_SEG_ARRAY_SIZE;
        break;
    }
    if (!parseThreadStarted_) {
        parseThreadStarted_ = true;
        int32_t tmp = maxThread_;
        while (tmp--) {
            parserThreadCount_++;
            std::thread ParseTypeThread(&HtraceParser::ParseThread, this);
            ParseTypeThread.detach();
            TS_LOGD("parser Thread:%d/%d start working ...\n", maxThread_ - tmp, maxThread_);
        }
    }
}

void HtraceParser::EnableFileSeparate(bool enabled)
{
    jsMemoryParser_->EnableSaveFile(enabled);
}
void HtraceParser::FilterData(HtraceDataSegment& seg, bool isSplitFile)
{
    bool haveSplitSeg = false;
    if (seg.dataType == DATA_SOURCE_TYPE_NATIVEHOOK) {
        htraceNativeHookParser_->Parse(seg, haveSplitSeg);
    } else if (seg.dataType == DATA_SOURCE_TYPE_NATIVEHOOK_CONFIG) {
        htraceNativeHookParser_->ParseConfigInfo(seg);
    } else if (seg.dataType == DATA_SOURCE_TYPE_TRACE) {
        ProtoReader::TracePluginResult_Reader tracePluginResult(seg.protoData);
        if (tracePluginResult.has_ftrace_cpu_detail()) {
            htraceCpuDetailParser_->Parse(seg, seg.clockId, haveSplitSeg);
        }
        if (tracePluginResult.has_symbols_detail()) {
            htraceSymbolsDetailParser_->Parse(seg.protoData); // has Event
            haveSplitSeg = true;
        }
        if (tracePluginResult.has_clocks_detail()) {
            htraceClockDetailParser_->Parse(seg.protoData); // has Event
            haveSplitSeg = true;
        }
    } else if (seg.dataType == DATA_SOURCE_TYPE_MEM) {
        htraceMemParser_->Parse(seg, seg.timeStamp, seg.clockId);
    } else if (seg.dataType == DATA_SOURCE_TYPE_HILOG) {
        htraceHiLogParser_->Parse(seg.protoData, haveSplitSeg);
    } else if (seg.dataType == DATA_SOURCE_TYPE_CPU) {
        cpuUsageParser_->Parse(seg.protoData, seg.timeStamp);
    } else if (seg.dataType == DATA_SOURCE_TYPE_FPS) {
        htraceHidumpParser_->Parse(seg.protoData);
        dataSourceTypeFpsClockid_ = htraceHidumpParser_->ClockId();
    } else if (seg.dataType == DATA_SOURCE_TYPE_NETWORK) {
        networkParser_->Parse(seg.protoData, seg.timeStamp);
    } else if (seg.dataType == DATA_SOURCE_TYPE_PROCESS) {
        processParser_->Parse(seg.protoData, seg.timeStamp);
    } else if (seg.dataType == DATA_SOURCE_TYPE_DISKIO) {
        diskIOParser_->Parse(seg.protoData, seg.timeStamp);
    } else if (seg.dataType == DATA_SOURCE_TYPE_JSMEMORY) {
        jsMemoryParser_->Parse(seg.protoData, seg.timeStamp, traceDataCache_->SplitFileMinTime(),
                               traceDataCache_->SplitFileMaxTime(), profilerPluginData_);
    } else if (seg.dataType == DATA_SOURCE_TYPE_JSMEMORY_CONFIG) {
        jsMemoryParser_->ParseJSMemoryConfig(seg.protoData);
    } else if (seg.dataType == DATA_SOURCE_TYPE_HISYSEVENT) {
        ProtoReader::HisyseventInfo_Reader hisyseventInfo(seg.protoData.data_, seg.protoData.size_);
        hisyseventParser_->Parse(&hisyseventInfo, seg.timeStamp);
    } else if (seg.dataType == DATA_SOURCE_TYPE_HISYSEVENT_CONFIG) {
        ProtoReader::HisyseventConfig_Reader hisyseventConfig(seg.protoData.data_, seg.protoData.size_);
        hisyseventParser_->Parse(&hisyseventConfig, seg.timeStamp);
    } else if (seg.dataType == DATA_SOURCE_TYPE_MEM_CONFIG) {
        htraceMemParser_->ParseMemoryConfig(seg);
    }
    if (traceDataCache_->isSplitFile_ && haveSplitSeg) {
        mTraceDataHtrace_.emplace(splitFileOffset_, nextLength_ + PACKET_SEG_LENGTH);
    }
    if (supportThread_) {
        filterHead_ = (filterHead_ + 1) % MAX_SEG_ARRAY_SIZE;
    }
    seg.status = TS_PARSE_STATUS_INIT;
}
void HtraceParser::FilterThread()
{
    TS_LOGI("filter thread start work!");
    while (true) {
        HtraceDataSegment& seg = dataSegArray_[filterHead_];
        if (seg.status.load() == TS_PARSE_STATUS_INVALID) {
            seg.status = TS_PARSE_STATUS_INIT;
            filterHead_ = (filterHead_ + 1) % MAX_SEG_ARRAY_SIZE;
            streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_INVALID);
            TS_LOGD("seprateHead_d:\t%d, parseHead_:\t%d, filterHead_:\t%d\n", rawDataHead_, parseHead_, filterHead_);
            continue;
        }
        if (seg.status.load() != TS_PARSE_STATUS_PARSED) {
            if (toExit_ && !parserThreadCount_) {
                TS_LOGI("exiting Filter Thread");
                exited_ = true;
                filterThreadStarted_ = false;
                TS_LOGI("seprateHead:\t%d, parseHead_:\t%d, filterHead_:\t%d, status:%d\n", rawDataHead_, parseHead_,
                        filterHead_, seg.status.load());
                return;
            }
            TS_LOGD("seprateHead:\t%d, parseHead_:\t%d, filterHead_:\t%d, status:%d\n", rawDataHead_, parseHead_,
                    filterHead_, seg.status.load());
            usleep(sleepDur_);
            continue;
        }
        FilterData(seg, false);
    }
}
void HtraceParser::ParserData(HtraceDataSegment& dataSeg, bool isSplitFile)
{
    ProtoReader::ProfilerPluginData_Reader pluginDataZero(reinterpret_cast<const uint8_t*>(dataSeg.seg->c_str()),
                                                          dataSeg.seg->length());
    std::string pluginName;
    if (pluginDataZero.has_name()) {
        pluginName = pluginDataZero.name().ToStdString();
        if (isSplitFile && EndWith(pluginName, "arkts-plugin_config")) {
            std::string dataString(dataSeg.seg->c_str(), dataSeg.seg->length());
            arkTsConfigData_ = lenBuffer_ + dataString;
            return;
        } else if (isSplitFile && EndWith(pluginName, "config")) {
            mTraceDataHtrace_.emplace(splitFileOffset_, nextLength_ + PACKET_SEG_LENGTH);
            return;
        }
    }
    if (pluginDataZero.has_tv_sec() && pluginDataZero.has_tv_nsec()) {
        dataSeg.timeStamp = pluginDataZero.tv_sec() * SEC_TO_NS + pluginDataZero.tv_nsec();
    }
    bool isHookData = (pluginName == "nativehook" || pluginName == "hookdaemon");
    bool isFtrace = (pluginDataZero.name().ToStdString() == "ftrace-plugin" ||
                     pluginDataZero.name().ToStdString() == "/data/local/tmp/libftrace_plugin.z.so");
    bool isHilog = (pluginName == "hilog-plugin" || pluginName == "/data/local/tmp/libhilogplugin.z.so");
    if (isSplitFile && !isHookData && !isHilog && !isFtrace) {
        bool needToPrimaryTimePlugin = (pluginName == "hisysevent-plugin" || pluginName == "memory-plugin");
        if (needToPrimaryTimePlugin) {
            dataSeg.timeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, dataSeg.timeStamp);
            UpdatePluginTimeRange(TS_CLOCK_BOOTTIME, dataSeg.timeStamp, dataSeg.timeStamp);
        }
        if (dataSeg.timeStamp >= traceDataCache_->SplitFileMinTime() &&
            dataSeg.timeStamp <= traceDataCache_->SplitFileMaxTime()) {
            mTraceDataHtrace_.emplace(splitFileOffset_, nextLength_ + PACKET_SEG_LENGTH);
        }
        if (!StartWith(pluginName, "arkts")) {
            return;
        }
    }
    if (isHookData) {
        dataSourceTypeNativeHookClockid_ = TS_CLOCK_REALTIME;
        dataSeg.dataType = DATA_SOURCE_TYPE_NATIVEHOOK;
        if (isSplitFile) {
            dataSourceType_ = DATA_SOURCE_TYPE_NATIVEHOOK;
        }
        dataSeg.protoData = pluginDataZero.data();
    } else if (pluginName == "nativehook_config") {
        dataSeg.dataType = DATA_SOURCE_TYPE_NATIVEHOOK_CONFIG;
        dataSeg.protoData = pluginDataZero.data();
    } else if (isFtrace) { // ok
        dataSeg.dataType = DATA_SOURCE_TYPE_TRACE;
        dataSeg.protoData = pluginDataZero.data();
        ParseFtrace(dataSeg);
    } else if (pluginName == "memory-plugin") {
        dataSeg.protoData = pluginDataZero.data();
        dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
        ParseMemory(&pluginDataZero, dataSeg);
    } else if (isHilog) {
        dataSeg.protoData = pluginDataZero.data();
        ParseHilog(dataSeg);
    } else if (pluginName == "hidump-plugin" || pluginName == "/data/local/tmp/libhidumpplugin.z.so") {
        dataSeg.protoData = pluginDataZero.data();
        ParseFPS(dataSeg);
    } else if (pluginName == "cpu-plugin") {
        dataSeg.protoData = pluginDataZero.data();
        ParseCpuUsage(dataSeg);
    } else if (pluginName == "network-plugin") {
        dataSeg.protoData = pluginDataZero.data();
        ParseNetwork(dataSeg);
    } else if (pluginName == "diskio-plugin") {
        dataSeg.protoData = pluginDataZero.data();
        ParseDiskIO(dataSeg);
    } else if (pluginName == "process-plugin") {
        dataSeg.protoData = pluginDataZero.data();
        ParseProcess(dataSeg);
    } else if (pluginName == "hisysevent-plugin") {
        dataSeg.protoData = pluginDataZero.data();
        ParseHisysevent(dataSeg);
    } else if (pluginName == "hisysevent-plugin_config") {
        dataSeg.protoData = pluginDataZero.data();
        ParseHisyseventConfig(dataSeg);
    } else if (pluginName == "arkts-plugin") {
        if (isSplitFile) {
            dataSourceType_ = DATA_SOURCE_TYPE_JSMEMORY;
            memcpy_s(&profilerPluginData_, sizeof(profilerPluginData_), dataSeg.seg->c_str(), dataSeg.seg->length());
        }
        dataSeg.protoData = pluginDataZero.data();
        ParseJSMemory(dataSeg);
    } else if (pluginName == "arkts-plugin_config") {
        dataSeg.protoData = pluginDataZero.data();
        ParseJSMemoryConfig(dataSeg);
    } else if (pluginName == "memory-plugin_config") {
        if (pluginDataZero.has_sample_interval()) {
            uint32_t sampleInterval = pluginDataZero.sample_interval();
            traceDataCache_->GetTraceConfigData()->AppendNewData("memory_config", "sample_interval",
                                                                 std::to_string(sampleInterval));
        }
        dataSeg.dataType = DATA_SOURCE_TYPE_MEM_CONFIG;
        dataSeg.protoData = pluginDataZero.data();
    } else {
#if IS_WASM
        TraceStreamer_Plugin_Out_Filter(reinterpret_cast<const char*>(pluginDataZero.data().data_),
                                        pluginDataZero.data().size_, pluginName);
#endif
        dataSeg.status = TS_PARSE_STATUS_INVALID;
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_INVALID);
        return;
    }
    if (!supportThread_) { // do it only in wasm mode, wasm noThead_ will be true
        if (dataSeg.status == TS_PARSE_STATUS_INVALID) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_INVALID);
            return;
        }
        FilterData(dataSeg, isSplitFile);
    }
}
void HtraceParser::ParseThread()
{
    TS_LOGI("parser thread start work!\n");
    while (true) {
        if (supportThread_ && !filterThreadStarted_) {
            filterThreadStarted_ = true;
            std::thread ParserThread(&HtraceParser::FilterThread, this);
            TS_LOGD("FilterThread start working ...\n");
            ParserThread.detach();
        }
        int32_t head = GetNextSegment();
        if (head < 0) {
            if (head == ERROR_CODE_EXIT) {
                TS_LOGI("parse thread exit\n");
                return;
            } else if (head == ERROR_CODE_NODATA) {
                continue;
            }
        }
        HtraceDataSegment& dataSeg = dataSegArray_[head];
        ParserData(dataSeg, false);
    }
}

void HtraceParser::ParseMemory(ProtoReader::ProfilerPluginData_Reader* pluginDataZero, HtraceDataSegment& dataSeg)
{
    BuiltinClocks clockId = TS_CLOCK_REALTIME;
    auto clockIdTemp = pluginDataZero->clock_id();
    if (clockIdTemp == ProtoReader::ProfilerPluginData_ClockId_CLOCKID_REALTIME) {
        clockId = TS_CLOCK_REALTIME;
    }
    dataSourceTypeMemClockid_ = clockId;
    dataSeg.dataType = DATA_SOURCE_TYPE_MEM;
    dataSeg.clockId = clockId;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}
void HtraceParser::ParseHilog(HtraceDataSegment& dataSeg)
{
    dataSeg.dataType = DATA_SOURCE_TYPE_HILOG;
    dataSourceTypeHilogClockid_ = TS_CLOCK_REALTIME;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

void HtraceParser::ParseFtrace(HtraceDataSegment& dataSeg)
{
    ProtoReader::TracePluginResult_Reader tracePluginResult(dataSeg.protoData);
    if (tracePluginResult.has_ftrace_cpu_stats()) {
        auto cpuStats = *tracePluginResult.ftrace_cpu_stats();
        ProtoReader::FtraceCpuStatsMsg_Reader ftraceCpuStatsMsg(cpuStats.data_, cpuStats.size_);
        auto s = *ftraceCpuStatsMsg.per_cpu_stats();
        ProtoReader::PerCpuStatsMsg_Reader perCpuStatsMsg(s.data_, s.size_);
        TS_LOGD("s.overrun():%lu", perCpuStatsMsg.overrun());
        TS_LOGD("s.dropped_events():%lu", perCpuStatsMsg.dropped_events());
        auto clock = ftraceCpuStatsMsg.trace_clock().ToStdString();
        if (clock == "boot") {
            clock_ = TS_CLOCK_BOOTTIME;
        } else if (clock == "mono") {
            clock_ = TS_MONOTONIC;
        } else {
            TS_LOGI("invalid clock:%s", clock.c_str());
            dataSeg.status = TS_PARSE_STATUS_INVALID;
            return;
        }
        dataSeg.clockId = clock_;
        dataSeg.status = TS_PARSE_STATUS_PARSED;
        return;
    }
    dataSeg.clockId = clock_;
    dataSourceTypeTraceClockid_ = clock_;
    if (tracePluginResult.has_clocks_detail() || tracePluginResult.has_ftrace_cpu_detail() ||
        tracePluginResult.has_symbols_detail()) {
        dataSeg.status = TS_PARSE_STATUS_PARSED;
        return;
    }
    dataSeg.status = TS_PARSE_STATUS_INVALID;
}

void HtraceParser::ParseFPS(HtraceDataSegment& dataSeg)
{
    dataSeg.dataType = DATA_SOURCE_TYPE_FPS;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

void HtraceParser::ParseCpuUsage(HtraceDataSegment& dataSeg)
{
    dataSourceTypeProcessClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_CPU;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}
void HtraceParser::ParseNetwork(HtraceDataSegment& dataSeg)
{
    dataSourceTypeProcessClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_NETWORK;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}
void HtraceParser::ParseDiskIO(HtraceDataSegment& dataSeg)
{
    dataSourceTypeProcessClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_DISKIO;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

void HtraceParser::ParseProcess(HtraceDataSegment& dataSeg)
{
    dataSourceTypeProcessClockid_ = TS_CLOCK_BOOTTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_PROCESS;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

void HtraceParser::ParseHisysevent(HtraceDataSegment& dataSeg)
{
    dataSourceTypeHisyseventClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_HISYSEVENT;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}
void HtraceParser::ParseHisyseventConfig(HtraceDataSegment& dataSeg)
{
    dataSourceTypeHisyseventClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_HISYSEVENT_CONFIG;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

void HtraceParser::ParseJSMemory(HtraceDataSegment& dataSeg)
{
    dataSourceTypeJSMemoryClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_JSMEMORY;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

void HtraceParser::ParseJSMemoryConfig(HtraceDataSegment& dataSeg)
{
    dataSourceTypeJSMemoryClockid_ = TS_CLOCK_REALTIME;
    dataSeg.dataType = DATA_SOURCE_TYPE_JSMEMORY_CONFIG;
    dataSeg.status = TS_PARSE_STATUS_PARSED;
}

int32_t HtraceParser::GetNextSegment()
{
    int32_t head;
    htraceDataSegMux_.lock();
    head = parseHead_;
    HtraceDataSegment& htraceDataSegmentSeg = dataSegArray_[head];
    if (htraceDataSegmentSeg.status.load() != TS_PARSE_STATUS_SEPRATED) {
        if (toExit_) {
            parserThreadCount_--;
            TS_LOGI("exiting parser, parserThread Count:%d\n", parserThreadCount_);
            TS_LOGD("seprateHead_x:\t%d, parseHead_:\t%d, filterHead_:\t%d status:%d\n", rawDataHead_, parseHead_,
                    filterHead_, htraceDataSegmentSeg.status.load());
            htraceDataSegMux_.unlock();
            if (!parserThreadCount_ && !filterThreadStarted_) {
                exited_ = true;
            }
            return ERROR_CODE_EXIT;
        }
        if (htraceDataSegmentSeg.status.load() == TS_PARSE_STATUS_PARSING) {
            htraceDataSegMux_.unlock();
            usleep(sleepDur_);
            return ERROR_CODE_NODATA;
        }
        htraceDataSegMux_.unlock();
        usleep(sleepDur_);
        return ERROR_CODE_NODATA;
    }
    parseHead_ = (parseHead_ + 1) % MAX_SEG_ARRAY_SIZE;
    htraceDataSegmentSeg.status = TS_PARSE_STATUS_PARSING;
    htraceDataSegMux_.unlock();
    return head;
}
bool HtraceParser::CalcEbpfCutOffset(std::deque<uint8_t>::iterator& packagesBegin, size_t& currentLength)
{
    auto standaloneDataLength = profilerDataLength_ - PACKET_HEADER_LENGTH;
    if (EBPF_PLUGIN_NAME.compare(standalonePluginName_) == 0) {
        if (traceDataCache_->isSplitFile_ && !parsedEbpfOver_) {
            if (!hasInitEbpfPublicData_) {
                // Record the offset of Hiperf's 1024-byte header relative to the entire file.
                ebpfDataParser_->SetEbpfDataOffset(processedDataLen_);
                ebpfDataParser_->SetSpliteTimeRange(traceDataCache_->SplitFileMinTime(),
                                                    traceDataCache_->SplitFileMaxTime());
                parsedFileOffset_ += profilerDataLength_ - PACKET_HEADER_LENGTH;
                hasInitEbpfPublicData_ = true;
            }
            parsedEbpfOver_ = ebpfDataParser_->AddAndSplitEbpfData(packagesBuffer_);
            packagesBuffer_.clear();
            return true;
        }
        if (packagesBuffer_.size() >= standaloneDataLength) {
            ebpfDataParser_->InitAndParseEbpfData(packagesBuffer_, standaloneDataLength);
            currentLength -= standaloneDataLength;
            packagesBegin += standaloneDataLength;
            profilerDataType_ = ProfilerTraceFileHeader::UNKNOW_TYPE;
            hasGotHeader_ = false;
            return true;
        }
        return false;
    }
}

bool HtraceParser::ParseDataRecursively(std::deque<uint8_t>::iterator& packagesBegin, size_t& currentLength)
{
    if (!hasGotHeader_) {
        if (InitProfilerTraceFileHeader()) {
            packagesBuffer_.erase(packagesBuffer_.begin(), packagesBuffer_.begin() + PACKET_HEADER_LENGTH);
            processedDataLen_ += PACKET_HEADER_LENGTH;
            currentLength -= PACKET_HEADER_LENGTH;
            packagesBegin += PACKET_HEADER_LENGTH;
            parsedFileOffset_ += PACKET_HEADER_LENGTH;
            htraceCurentLength_ = profilerDataLength_;
            htraceCurentLength_ -= PACKET_HEADER_LENGTH;
            hasGotHeader_ = true;
            if (!currentLength) {
                return false;
            }
        } else {
            TS_LOGE("get profiler trace file header failed");
            return false;
        }
    }
    if (profilerDataType_ == ProfilerTraceFileHeader::HIPERF_DATA) {
        return ParseHiperfData(packagesBegin, currentLength);
    }
    if (profilerDataType_ == ProfilerTraceFileHeader::STANDALONE_DATA) {
        CalcEbpfCutOffset(packagesBegin, currentLength);
#if IS_WASM
        if (packagesBuffer_.size() >= profilerDataLength_ - PACKET_HEADER_LENGTH) {
            auto thirdPartySize = profilerDataLength_ - PACKET_HEADER_LENGTH;
            auto buffer = std::make_unique<uint8_t[]>(thirdPartySize).get();
            std::copy(packagesBuffer_.begin(), packagesBuffer_.begin() + thirdPartySize, buffer);
            TraceStreamer_Plugin_Out_Filter(reinterpret_cast<const char*>(buffer), thirdPartySize,
                                            standalonePluginName_);
            return true;
        }
#endif
        return false;
    }
    while (true) {
        if (!hasGotSegLength_) {
            if (currentLength < PACKET_SEG_LENGTH) {
                break;
            }
            std::string bufferLine(packagesBegin, packagesBegin + PACKET_SEG_LENGTH);
            const uint32_t* len = reinterpret_cast<const uint32_t*>(bufferLine.data());
            nextLength_ = *len;
            lenBuffer_ = bufferLine;
            htraceLength_ += nextLength_ + PACKET_SEG_LENGTH;
            hasGotSegLength_ = true;
            currentLength -= PACKET_SEG_LENGTH;
            packagesBegin += PACKET_SEG_LENGTH;
            parsedFileOffset_ += PACKET_SEG_LENGTH;
            splitFileOffset_ = profilerDataLength_ - htraceCurentLength_;
            htraceCurentLength_ -= PACKET_SEG_LENGTH;
        }
        if (currentLength < nextLength_) {
            break;
        }
        std::string bufferLine(packagesBegin, packagesBegin + nextLength_);
        ParseTraceDataItem(bufferLine);
        hasGotSegLength_ = false;
        packagesBegin += nextLength_;
        currentLength -= nextLength_;
        parsedFileOffset_ += nextLength_;
        if (nextLength_ > htraceCurentLength_) {
            TS_LOGE("fatal error, data length not match nextLength_:%u, htraceCurentLength_:%" PRIu64 "", nextLength_,
                    htraceCurentLength_);
        }
        htraceCurentLength_ -= nextLength_;
        if (htraceCurentLength_ == 0) {
            hasGotHeader_ = false;
            processedDataLen_ += packagesBegin - packagesBuffer_.begin();
            packagesBuffer_.erase(packagesBuffer_.begin(), packagesBegin);
            profilerDataType_ = ProfilerTraceFileHeader::UNKNOW_TYPE;
            TS_LOGD("read proto finished!");
            return ParseDataRecursively(packagesBegin, currentLength);
        }
    }
    return true;
}

void HtraceParser::ParseTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size)
{
    packagesBuffer_.insert(packagesBuffer_.end(), &bufferStr[0], &bufferStr[size]);
    auto packagesBegin = packagesBuffer_.begin();
    auto currentLength = packagesBuffer_.size();
    if (ParseDataRecursively(packagesBegin, currentLength)) {
        processedDataLen_ += packagesBegin - packagesBuffer_.begin();
        packagesBuffer_.erase(packagesBuffer_.begin(), packagesBegin);
    }
    return;
}
bool HtraceParser::ParseHiperfData(std::deque<uint8_t>::iterator& packagesBegin, size_t& currentLength)
{
    if (!traceDataCache_->isSplitFile_) {
        if (packagesBuffer_.size() >= profilerDataLength_ - PACKET_HEADER_LENGTH) {
            auto size = profilerDataLength_ - PACKET_HEADER_LENGTH;
            (void)perfDataParser_->InitPerfDataAndLoad(packagesBuffer_, size, processedDataLen_, false, true);
            currentLength -= size;
            packagesBegin += size;
            profilerDataType_ = ProfilerTraceFileHeader::UNKNOW_TYPE;
            hasGotHeader_ = false;
            return true;
        }
        return false;
    }

    bool isFinish = perfProcessedLen_ + packagesBuffer_.size() >= profilerDataLength_ - PACKET_HEADER_LENGTH;
    auto size = packagesBuffer_.size();
    if (isFinish) {
        size = profilerDataLength_ - PACKET_HEADER_LENGTH - perfProcessedLen_;
    }
    auto ret = perfDataParser_->InitPerfDataAndLoad(packagesBuffer_, size, processedDataLen_, true, isFinish);
    perfProcessedLen_ += ret;
    currentLength -= ret;
    packagesBegin += ret;
    if (isFinish) {
        profilerDataType_ = ProfilerTraceFileHeader::UNKNOW_TYPE;
        hasGotHeader_ = false;
    }
    return true;
}
void HtraceParser::StoreTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size, int32_t isFinish)
{
    packagesBuffer_.insert(packagesBuffer_.end(), &bufferStr[0], &bufferStr[size]);
    if (!traceDataCache_->isSplitFile_) {
        return;
    }

    uint64_t length = packagesBuffer_.size();
    auto ret = perfDataParser_->InitPerfDataAndLoad(packagesBuffer_, length, 0, true, isFinish);
    perfProcessedLen_ += ret;
    processedDataLen_ += ret;
    packagesBuffer_.erase(packagesBuffer_.begin(), packagesBuffer_.begin() + ret);
    return;
}
void HtraceParser::TraceDataSegmentEnd(bool isSplitFile)
{
    perfDataParser_->InitPerfDataAndLoad(packagesBuffer_, packagesBuffer_.size(), 0, isSplitFile, true);
    packagesBuffer_.clear();
    return;
}

bool HtraceParser::InitProfilerTraceFileHeader()
{
    if (packagesBuffer_.size() < PACKET_HEADER_LENGTH) {
        TS_LOGE("buffer size less than profiler trace file header");
        return false;
    }
    uint8_t buffer[PACKET_HEADER_LENGTH];
    (void)memset_s(buffer, PACKET_HEADER_LENGTH, 0, PACKET_HEADER_LENGTH);
    int32_t i = 0;
    for (auto it = packagesBuffer_.begin(); it != packagesBuffer_.begin() + PACKET_HEADER_LENGTH; ++it, ++i) {
        buffer[i] = *it;
    }
    ProfilerTraceFileHeader* pHeader = reinterpret_cast<ProfilerTraceFileHeader*>(buffer);
    if (pHeader->data.length <= PACKET_HEADER_LENGTH || pHeader->data.magic != ProfilerTraceFileHeader::HEADER_MAGIC) {
        TS_LOGE("Profiler Trace data is truncated or invalid magic! len = %" PRIu64 ", maigc = %" PRIx64 "",
                pHeader->data.length, pHeader->data.magic);
        return false;
    }
    if (pHeader->data.dataType == ProfilerTraceFileHeader::HIPERF_DATA) {
        perfDataParser_->RecordPerfProfilerHeader(buffer, PACKET_HEADER_LENGTH);
    } else if (pHeader->data.dataType == ProfilerTraceFileHeader::STANDALONE_DATA) {
        ebpfDataParser_->RecordEbpfProfilerHeader(buffer, PACKET_HEADER_LENGTH);
    } else {
        auto ret = memcpy_s(&profilerTraceFileHeader_, sizeof(profilerTraceFileHeader_), buffer, PACKET_HEADER_LENGTH);
        if (ret == -1 || profilerTraceFileHeader_.data.magic != ProfilerTraceFileHeader::HEADER_MAGIC) {
            TS_LOGE("Get profiler trace file header failed! ret = %d, magic = %" PRIx64 "", ret,
                    profilerTraceFileHeader_.data.magic);
            return false;
        }
    }
    profilerDataLength_ = pHeader->data.length;
    profilerDataType_ = pHeader->data.dataType;
    memcpy_s(standalonePluginName_, sizeof(standalonePluginName_), pHeader->data.standalonePluginName,
             sizeof(standalonePluginName_));

    TS_LOGI("magic = %" PRIx64 ", length = %" PRIu64 ", dataType = %x, boottime = %" PRIu64 "", pHeader->data.magic,
            pHeader->data.length, pHeader->data.dataType, pHeader->data.boottime);
#if IS_WASM
    const int32_t DATA_TYPE_CLOCK = 100;
    TraceStreamer_Plugin_Out_SendData(reinterpret_cast<char*>(buffer), PACKET_HEADER_LENGTH, DATA_TYPE_CLOCK);
#endif
    htraceClockDetailParser_->Parse(pHeader);
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
