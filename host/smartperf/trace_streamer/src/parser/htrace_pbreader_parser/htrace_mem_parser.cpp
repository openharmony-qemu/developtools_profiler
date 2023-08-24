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
#include "htrace_mem_parser.h"
#include "clock_filter_ex.h"
#include "htrace_event_parser.h"
#include "measure_filter.h"
#include "memory_plugin_common.pbreader.h"
#include "memory_plugin_config.pbreader.h"
#include "memory_plugin_result.pbreader.h"
#include "process_filter.h"
#include "stat_filter.h"
#include "symbols_filter.h"
#include "system_event_measure_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceMemParser::HtraceMemParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx)
{
    for (auto i = 0; i < MEM_MAX; i++) {
        memNameDictMap_.insert(
            std::make_pair(static_cast<MemInfoType>(i),
                           traceDataCache_->GetDataIndex(config_.memNameMap_.at(static_cast<MemInfoType>(i)))));
    }
    for (auto i = 0; i < ProtoReader::PMEM_PINED_PURG + 1; i++) {
        sysMemNameDictMap_.insert(std::make_pair(i, traceDataCache_->GetDataIndex(config_.sysMemNameMap_.at(i))));
    }
    for (auto i = 0; i < ProtoReader::VMEMINFO_WORKINGSET_RESTORE + 1; i++) {
        sysVMemNameDictMap_.insert(
            std::make_pair(i, traceDataCache_->GetDataIndex(config_.sysVirtualMemNameMap_.at(i))));
    }
}

HtraceMemParser::~HtraceMemParser()
{
    TS_LOGI("mem ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
}
void HtraceMemParser::Parse(HtraceDataSegment& seg, uint64_t timeStamp, BuiltinClocks clock)
{
    ProtoReader::MemoryData_Reader memData(seg.protoData.data_, seg.protoData.size_);
    auto newTimeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(clock, timeStamp);
    UpdatePluginTimeRange(clock, timeStamp, newTimeStamp);
    zram_ = memData.zram();
    gpuLimit_ = memData.has_gpu_limit_size() ? memData.gpu_limit_size() : 0;
    gpuUsed_ = memData.has_gpu_used_size() ? memData.gpu_used_size() : 0;
    if (memData.has_processesinfo()) {
        ParseProcessInfo(&memData, newTimeStamp);
    }
    if (memData.has_meminfo()) {
        ParseMemInfo(&memData, newTimeStamp);
    }
    if (memData.has_vmeminfo()) {
        ParseVMemInfo(&memData, newTimeStamp);
    }
    if (memData.has_ashmeminfo()) {
        ParseAshmemInfo(&memData, newTimeStamp);
    }
    if (memData.has_dmainfo()) {
        ParseDmaMemInfo(&memData, newTimeStamp);
    }
    if (memData.has_gpumemoryinfo()) {
        ParseGpuProcessMemInfo(&memData, newTimeStamp);
    }
    if (memData.has_gpudumpinfo()) {
        ParseGpuWindowMemInfo(&memData, newTimeStamp);
    }
}
void HtraceMemParser::ParseProcessInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    if (tracePacket->has_processesinfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_MEMORY, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->processesinfo(); i; ++i) {
        ProtoReader::ProcessMemoryInfo_Reader processMemoryInfo(i->ToBytes().data_, i->ToBytes().size_);
        auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(
            processMemoryInfo.pid(), processMemoryInfo.name().ToStdString());
        uint32_t hasValue = 0;
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_VM_SIZE), timeStamp, processMemoryInfo.vm_size_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_VM_RSS), timeStamp, processMemoryInfo.vm_rss_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_VM_ANON), timeStamp, processMemoryInfo.rss_anon_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_RSS_FILE), timeStamp, processMemoryInfo.rss_file_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_RSS_SHMEM), timeStamp, processMemoryInfo.rss_shmem_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_VM_SWAP), timeStamp, processMemoryInfo.vm_swap_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_VM_LOCKED), timeStamp, processMemoryInfo.vm_locked_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_VM_HWM), timeStamp, processMemoryInfo.vm_hwm_kb());
        hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
            ipid, memNameDictMap_.at(MEM_OOM_SCORE_ADJ), timeStamp, processMemoryInfo.oom_score_adj());
        if (processMemoryInfo.has_purg_sum_kb()) {
            hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
                ipid, memNameDictMap_.at(MEM_PURG_SUM), timeStamp, processMemoryInfo.purg_sum_kb());
        }
        if (processMemoryInfo.has_purg_pin_kb()) {
            hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
                ipid, memNameDictMap_.at(MEM_PURG_PIN), timeStamp, processMemoryInfo.purg_pin_kb());
        }
        if (processMemoryInfo.has_gl_pss_kb()) {
            hasValue += streamFilters_->processMeasureFilter_->AppendNewMeasureData(
                ipid, memNameDictMap_.at(MEM_GL_PSS), timeStamp, processMemoryInfo.gl_pss_kb());
        }
        if (hasValue) {
            streamFilters_->processFilter_->AddProcessMemory(ipid);
        }
        if (processMemoryInfo.has_smapinfo()) {
            ParseSmapsInfoEasy(&processMemoryInfo, timeStamp, ipid);
        }
    }
}

uint32_t HtraceMemParser::ParseSmapsBlockType(ProtoReader::SmapsInfo_Reader& smapsInfo) const
{
    std::string path(smapsInfo.path().ToStdString());
    path.erase(0, path.find_first_not_of(" "));
    path.erase(path.find_last_not_of(" ") + 1);
    if (path.empty()) {
        path = "[anon]";
    }
    std::map<std::string, uint32_t> check_start = {
        {"[stack", SMAPS_MEM_TYPE_STACK},
        {"[anon:stack_and_tls:", SMAPS_MEM_TYPE_STACK},
        {"[anon:stack:", SMAPS_MEM_TYPE_STACK},
        {"[anon:signal_stack:", SMAPS_MEM_TYPE_STACK},
        {"[anon:maple_alloc_ros]", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-allocspace", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-main space", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-large object", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-free list large", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-non moving", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-zygote space", SMAPS_MEM_TYPE_JS_HEAP},
        {"[anon:dalvik-", SMAPS_MEM_TYPE_JAVA_VM},
        {"/dev/ashmem/jit-zygote-cache", SMAPS_MEM_TYPE_JAVA_VM},
        {"/memfd:jit-cache", SMAPS_MEM_TYPE_JAVA_VM},
        {"/memfd:jit-zygote-cache", SMAPS_MEM_TYPE_JAVA_VM},
        {"[heap]", SMAPS_MEM_TYPE_NATIVE_HEAP},
        {"[anon:libc_malloc", SMAPS_MEM_TYPE_NATIVE_HEAP},
        {"[anon:scudo", SMAPS_MEM_TYPE_NATIVE_HEAP},
        {"[anon:GWP-Asan", SMAPS_MEM_TYPE_NATIVE_HEAP},
    };
    std::map<std::string, uint32_t> check_end = {
        {".art", SMAPS_MEM_TYPE_JS_HEAP},
        {".art]", SMAPS_MEM_TYPE_JS_HEAP},
    };
    std::map<std::string, uint32_t> check_contain = {
        {"[anon:ArkJS Heap]", SMAPS_MEM_TYPE_JS_HEAP}, {"[anon:native_heap:jemalloc]", SMAPS_MEM_TYPE_NATIVE_HEAP},
        {"[heap]", SMAPS_MEM_TYPE_NATIVE_HEAP},        {"[anon:native_heap:musl]", SMAPS_MEM_TYPE_NATIVE_HEAP},
        {"/dev/ashmem/", SMAPS_MEM_TYPE_ASHMEM},
    };

    for (const auto& iter : check_start) {
        if (StartWith(path, iter.first)) {
            return iter.second;
        }
    }

    for (const auto& iter : check_end) {
        if (EndWith(path, iter.first)) {
            return iter.second;
        }
    }

    for (const auto& iter : check_contain) {
        if (path.find(iter.first) != std::string::npos) {
            return iter.second;
        }
    }

    bool has_x = smapsInfo.permission().ToStdString().find("x") != std::string::npos;
    bool has_appNmae = path.find("com.huawei.wx") != std::string::npos;
    if (EndWith(path, ".so")) {
        if (has_x) {
            if (StartWith(path, "/data/app/") || has_appNmae) {
                return SMAPS_MEM_TYPE_CODE_APP;
            } else {
                return SMAPS_MEM_TYPE_CODE_SYS;
            }
        } else {
            if (StartWith(path, "[anon:.bss]/data/app/") || StartWith(path, "/data/app/") || has_appNmae) {
                return SMAPS_MEM_TYPE_DATA_APP;
            } else {
                return SMAPS_MEM_TYPE_DATA_SYS;
            }
        }
    }
    if ((EndWith(path, ".jar")) || (EndWith(path, ".apk")) || (EndWith(path, ".vdex")) || (EndWith(path, ".odex")) ||
        (EndWith(path, ".oat")) || (path.find("dex") != std::string::npos)) {
        return has_x ? (has_appNmae ? SMAPS_MEM_TYPE_CODE_APP : SMAPS_MEM_TYPE_CODE_SYS)
                     : (has_appNmae ? SMAPS_MEM_TYPE_DATA_APP : SMAPS_MEM_TYPE_DATA_SYS);
    }
    if (has_x && path.find("/bin/") != std::string::npos) {
        return SMAPS_MEM_TYPE_CODE_SYS;
    }
    if ((!has_x) && (path.find("/bin/") != std::string::npos || path.find("[anon:.bss]") != std::string::npos)) {
        return SMAPS_MEM_TYPE_DATA_SYS;
    }
    if (path.find("[bss]") != std::string::npos) {
        return has_appNmae ? SMAPS_MEM_TYPE_DATA_APP : SMAPS_MEM_TYPE_DATA_SYS;
    }

    if ((path.find("[anon]") != std::string::npos) || (path.find("[anon:") != std::string::npos)) {
        std::vector<std::string> unknown_anon_mem_info = {
            "[anon]",
            "[anon:libwebview reservation]",
            "[anon:atexit handlers]",
            "[anon:cfi shadow]",
            "[anon:thread signal stack]",
            "[anon:bionic_alloc_small_objects]",
            "[anon:bionic_alloc_lob]",
            "[anon:linker_alloc]",
            "[anon:System property context nodes]",
            "[anon:arc4random data]",
        };
        for (auto iter : unknown_anon_mem_info) {
            if (path == iter) {
                return SMAPS_MEM_TYPE_UNKNOWN_ANON;
            }
        }
        return SMAPS_MEM_TYPE_NATIVE_HEAP;
    }

    return has_appNmae ? SMAPS_MEM_TYPE_OTHER_APP : SMAPS_MEM_TYPE_OTHER_SYS;
}

void HtraceMemParser::ParseSmapsInfoEasy(const ProtoReader::ProcessMemoryInfo_Reader* memInfo,
                                         uint64_t timeStamp,
                                         uint64_t ipid) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_SMAPS, STAT_EVENT_RECEIVED);
    for (auto i = memInfo->smapinfo(); i; ++i) {
        ProtoReader::SmapsInfo_Reader smapsInfo(i->ToBytes().data_, i->ToBytes().size_);
        auto startAddr = "0x" + smapsInfo.start_addr().ToStdString();
        auto endAddr = "0x" + smapsInfo.end_addr().ToStdString();
        uint64_t dirty = smapsInfo.dirty();
        uint64_t swapper = smapsInfo.swapper();
        uint64_t rss = smapsInfo.rss();
        uint64_t pss = smapsInfo.pss();
        uint64_t size = smapsInfo.size();
        double reside = smapsInfo.reside();
        DataIndex protection = traceDataCache_->GetDataIndex(smapsInfo.permission().ToStdString());
        DataIndex path = traceDataCache_->GetDataIndex(smapsInfo.path().ToStdString());
        uint64_t private_clean = smapsInfo.has_private_clean() ? smapsInfo.private_clean() : 0;
        uint64_t private_dirty = smapsInfo.has_private_dirty() ? smapsInfo.private_dirty() : 0;
        uint64_t shared_clean = smapsInfo.has_shared_clean() ? smapsInfo.shared_clean() : 0;
        uint64_t shared_dirty = smapsInfo.has_shared_dirty() ? smapsInfo.shared_dirty() : 0;
        uint64_t swap = smapsInfo.has_swap() ? smapsInfo.swap() : 0;
        uint64_t swap_pss = smapsInfo.has_swap_pss() ? smapsInfo.swap_pss() : 0;
        uint32 type = ParseSmapsBlockType(smapsInfo);
        traceDataCache_->GetSmapsData()->AppendNewData(timeStamp, ipid, startAddr, endAddr, dirty, swapper, rss, pss,
                                                       size, reside, protection, path, shared_clean, shared_dirty,
                                                       private_clean, private_dirty, swap, swap_pss, type);
    }
}

void HtraceMemParser::ParseMemInfoEasy(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    if (tracePacket->has_meminfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->meminfo(); i; ++i) {
        ProtoReader::SysMeminfo_Reader sysMeminfo(i->ToBytes());
        if (config_.sysMemNameMap_.find(sysMeminfo.key()) != config_.sysMemNameMap_.end()) {
            streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(sysMemNameDictMap_.at(sysMeminfo.key()),
                                                                             timeStamp, sysMeminfo.value());
        } else {
            streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_MEMORY, STAT_EVENT_DATA_INVALID);
        }
    }
}

void HtraceMemParser::ParseVMemInfoEasy(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    traceDataCache_->UpdateTraceTime(timeStamp);
    if (tracePacket->has_vmeminfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->vmeminfo(); i; ++i) {
        ProtoReader::SysVMeminfo_Reader sysVMeminfo(i->ToBytes());
        if (config_.sysVirtualMemNameMap_.find(sysVMeminfo.key()) != config_.sysVirtualMemNameMap_.end()) {
            streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(sysVMemNameDictMap_.at(sysVMeminfo.key()),
                                                                             timeStamp, sysVMeminfo.value());
        } else {
            streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_DATA_INVALID);
        }
    }
}

void HtraceMemParser::ParseMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_MEMORY, STAT_EVENT_RECEIVED);
    for (auto i = tracePacket->meminfo(); i; ++i) {
        ProtoReader::SysMeminfo_Reader sysMeminfo(i->ToBytes());
        switch (sysMeminfo.key()) {
            case ProtoReader::PMEM_UNSPECIFIED:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_UNSPECIFIED), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_MEM_TOTAL:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_MEM_TOTAL), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_MEM_FREE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_MEM_FREE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_MEM_AVAILABLE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_MEM_AVAILABLE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_BUFFERS:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_BUFFERS), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_CACHED:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_CACHED), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SWAP_CACHED:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SWAP_CACHED), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_ACTIVE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_ACTIVE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_INACTIVE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_INACTIVE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_ACTIVE_ANON:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_ACTIVE_ANON), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_INACTIVE_ANON:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_INACTIVE_ANON), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_ACTIVE_FILE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_ACTIVE_FILE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_INACTIVE_FILE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_INACTIVE_FILE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_UNEVICTABLE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_UNEVICTABLE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_MLOCKED:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_MLOCKED), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SWAP_TOTAL:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SWAP_TOTAL), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SWAP_FREE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SWAP_FREE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_DIRTY:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_DIRTY), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_WRITEBACK:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_WRITEBACK), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_ANON_PAGES:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_ANON_PAGES), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_MAPPED:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_MAPPED), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SHMEM:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SHMEM), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SLAB:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SLAB), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SLAB_RECLAIMABLE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SLAB_RECLAIMABLE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_SLAB_UNRECLAIMABLE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_SLAB_UNRECLAIMABLE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_KERNEL_STACK:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_KERNEL_STACK), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_PAGE_TABLES:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_PAGE_TABLES), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_COMMIT_LIMIT:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_COMMIT_LIMIT), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_COMMITED_AS:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_COMMITED_AS), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_VMALLOC_TOTAL:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_VMALLOC_TOTAL), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_VMALLOC_USED:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_VMALLOC_USED), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_VMALLOC_CHUNK:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_VMALLOC_CHUNK), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_CMA_TOTAL:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_CMA_TOTAL), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_CMA_FREE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_CMA_FREE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_KERNEL_RECLAIMABLE:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_KERNEL_RECLAIMABLE), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_ACTIVE_PURG:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_ACTIVE_PURG), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_INACTIVE_PURG:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_INACTIVE_PURG), timeStamp, sysMeminfo.value());
                break;
            case ProtoReader::PMEM_PINED_PURG:
                streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(
                    sysMemNameDictMap_.at(ProtoReader::PMEM_PINED_PURG), timeStamp, sysMeminfo.value());
                break;
            // case SysMeminfoType_INT_MIN_SENTINEL_DO_NOT_USE_:
            // case SysMeminfoType_INT_MAX_SENTINEL_DO_NOT_USE_:
            default:
                streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_MEMORY, STAT_EVENT_DATA_INVALID);
                break;
        }
    }
    streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(zramIndex_, timeStamp, zram_);
    streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(gpuLimitSizeIndex_, timeStamp, gpuLimit_);
    streamFilters_->sysEventMemMeasureFilter_->AppendNewMeasureData(gpuUsedSizeIndex_, timeStamp, gpuUsed_);
}
void HtraceMemParser::ParseVMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_RECEIVED);
    for (auto i = tracePacket->vmeminfo(); i; ++i) {
        ProtoReader::SysVMeminfo_Reader sysVMeminfo(i->ToBytes());
        switch (sysVMeminfo.key()) {
            case ProtoReader::VMEMINFO_UNSPECIFIED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNSPECIFIED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_FREE_PAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_FREE_PAGES), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ALLOC_BATCH:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ALLOC_BATCH), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_INACTIVE_ANON:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_INACTIVE_ANON), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ACTIVE_ANON:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ACTIVE_ANON), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_INACTIVE_FILE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_INACTIVE_FILE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ACTIVE_FILE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ACTIVE_FILE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_UNEVICTABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_UNEVICTABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_MLOCK:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_MLOCK), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ANON_PAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ANON_PAGES), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_MAPPED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_MAPPED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_FILE_PAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_FILE_PAGES), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_DIRTY:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_DIRTY), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_WRITEBACK:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_WRITEBACK), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SLAB_RECLAIMABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SLAB_RECLAIMABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SLAB_UNRECLAIMABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SLAB_UNRECLAIMABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_PAGE_TABLE_PAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_PAGE_TABLE_PAGES), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_KERNEL_STACK:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_KERNEL_STACK), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_OVERHEAD:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_OVERHEAD), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_UNSTABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_UNSTABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_BOUNCE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_BOUNCE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_VMSCAN_WRITE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_VMSCAN_WRITE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_WRITEBACK_TEMP:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_WRITEBACK_TEMP), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ISOLATED_ANON:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ISOLATED_ANON), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ISOLATED_FILE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ISOLATED_FILE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SHMEM:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SHMEM), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_DIRTIED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_DIRTIED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_WRITTEN:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_WRITTEN), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_PAGES_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_PAGES_SCANNED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_WORKINGSET_REFAULT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_WORKINGSET_REFAULT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_WORKINGSET_ACTIVATE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_WORKINGSET_ACTIVATE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_WORKINGSET_NODERECLAIM:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_WORKINGSET_NODERECLAIM), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_FREE_CMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_FREE_CMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SWAPCACHE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SWAPCACHE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_DIRTY_THRESHOLD:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_DIRTY_THRESHOLD), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGPGIN:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGPGIN), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGPGOUT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGPGOUT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGPGOUTCLEAN:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGPGOUTCLEAN), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PSWPIN:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PSWPIN), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PSWPOUT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PSWPOUT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGALLOC_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGALLOC_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGALLOC_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGALLOC_NORMAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGALLOC_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGALLOC_MOVABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGFREE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGFREE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGACTIVATE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGACTIVATE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGDEACTIVATE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGDEACTIVATE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGFAULT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGFAULT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGMAJFAULT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGMAJFAULT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGREFILL_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGREFILL_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGREFILL_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGREFILL_NORMAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGREFILL_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGREFILL_MOVABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_KSWAPD_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_KSWAPD_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_KSWAPD_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_KSWAPD_NORMAL), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_KSWAPD_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_KSWAPD_MOVABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_DIRECT_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_DIRECT_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_DIRECT_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_DIRECT_NORMAL), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_DIRECT_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_DIRECT_MOVABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_KSWAPD_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_KSWAPD_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_KSWAPD_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_KSWAPD_NORMAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_KSWAPD_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_KSWAPD_MOVABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_DIRECT_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_DIRECT_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_DIRECT_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_DIRECT_NORMAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_DIRECT_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_DIRECT_MOVABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_DIRECT_THROTTLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_DIRECT_THROTTLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGINODESTEAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGINODESTEAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_SLABS_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_SLABS_SCANNED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_KSWAPD_INODESTEAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_KSWAPD_INODESTEAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PAGEOUTRUN:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PAGEOUTRUN), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_ALLOCSTALL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_ALLOCSTALL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGROTATED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGROTATED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_DROP_PAGECACHE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_DROP_PAGECACHE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_DROP_SLAB:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_DROP_SLAB), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGMIGRATE_SUCCESS:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGMIGRATE_SUCCESS), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGMIGRATE_FAIL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGMIGRATE_FAIL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_MIGRATE_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_MIGRATE_SCANNED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_FREE_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_FREE_SCANNED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_ISOLATED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_ISOLATED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_STALL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_STALL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_FAIL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_FAIL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_SUCCESS:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_SUCCESS), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_DAEMON_WAKE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_DAEMON_WAKE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_CULLED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_CULLED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_SCANNED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_RESCUED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_RESCUED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_MLOCKED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_MLOCKED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_CLEARED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_CLEARED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_UNEVICTABLE_PGS_STRANDED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_UNEVICTABLE_PGS_STRANDED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZSPAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZSPAGES), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ION_HEAP:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ION_HEAP), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_GPU_HEAP:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_GPU_HEAP), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_ALLOCSTALL_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_ALLOCSTALL_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_ALLOCSTALL_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_ALLOCSTALL_MOVABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_ALLOCSTALL_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_ALLOCSTALL_NORMAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_DAEMON_FREE_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_DAEMON_FREE_SCANNED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_FASTRPC:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_FASTRPC), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_INDIRECTLY_RECLAIMABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_INDIRECTLY_RECLAIMABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ION_HEAP_POOL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ION_HEAP_POOL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SHADOW_CALL_STACK_BYTES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SHADOW_CALL_STACK_BYTES), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SHMEM_HUGEPAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SHMEM_HUGEPAGES), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_SHMEM_PMDMAPPED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_SHMEM_PMDMAPPED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_UNRECLAIMABLE_PAGES:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_UNRECLAIMABLE_PAGES), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZONE_ACTIVE_ANON:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZONE_ACTIVE_ANON), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZONE_ACTIVE_FILE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZONE_ACTIVE_FILE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZONE_INACTIVE_ANON:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZONE_INACTIVE_ANON), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZONE_INACTIVE_FILE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZONE_INACTIVE_FILE), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZONE_UNEVICTABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZONE_UNEVICTABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_NR_ZONE_WRITE_PENDING:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_NR_ZONE_WRITE_PENDING), timeStamp,
                    sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_OOM_KILL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_OOM_KILL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGLAZYFREE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGLAZYFREE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGLAZYFREED:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGLAZYFREED), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGREFILL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGREFILL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_DIRECT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_DIRECT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSCAN_KSWAPD:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSCAN_KSWAPD), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSKIP_DMA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSKIP_DMA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSKIP_MOVABLE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSKIP_MOVABLE), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSKIP_NORMAL:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSKIP_NORMAL), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_DIRECT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_DIRECT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_PGSTEAL_KSWAPD:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_PGSTEAL_KSWAPD), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_SWAP_RA:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_SWAP_RA), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_SWAP_RA_HIT:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_SWAP_RA_HIT), timeStamp, sysVMeminfo.value());
                break;
            case ProtoReader::VMEMINFO_WORKINGSET_RESTORE:
                streamFilters_->sysEventVMemMeasureFilter_->AppendNewMeasureData(
                    sysVMemNameDictMap_.at(ProtoReader::VMEMINFO_WORKINGSET_RESTORE), timeStamp, sysVMeminfo.value());
                break;
            // case SysVMeminfoType_INT_MIN_SENTINEL_DO_NOT_USE_:
            // case SysVMeminfoType_INT_MAX_SENTINEL_DO_NOT_USE_:
            default:
                streamFilters_->statFilter_->IncreaseStat(TRACE_SYS_VIRTUAL_MEMORY, STAT_EVENT_DATA_INVALID);
        }
    }
}

void HtraceMemParser::ParseAshmemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    if (tracePacket->has_ashmeminfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_ASHMEM, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->ashmeminfo(); i; ++i) {
        ProtoReader::AshmemInfo_Reader AshmemInfo(i->ToBytes().data_, i->ToBytes().size_);
        auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(AshmemInfo.pid(),
                                                                                  AshmemInfo.name().ToStdString());
        uint32_t adj = AshmemInfo.adj();
        uint32_t fd = AshmemInfo.fd();
        DataIndex ashmemNameId = traceDataCache_->GetDataIndex(AshmemInfo.ashmem_name().ToStdString());
        uint64_t size = AshmemInfo.size();
        uint32_t ashmemId = AshmemInfo.id();
        uint64_t time = AshmemInfo.time();
        uint64_t refCount = AshmemInfo.ref_count();
        uint64_t purged = AshmemInfo.purged();
        uint32_t flag = 0;
        uint64_t pss = 0;
        auto smapsData = traceDataCache_->GetConstSmapsData();
        auto smapsCount = smapsData.Size();
        for (auto j = 0; j < smapsCount; j++) {
            auto path = traceDataCache_->GetDataFromDict(smapsData.PathIds()[j]);
            if ((smapsData.Ipids()[j] == ipid) && (path.find("/dev/ashmem/") != std::string::npos)) {
                pss += smapsData.Pss()[j] + smapsData.SwapPss()[j];
            }
        }
        traceDataCache_->GetAshMemData()->AppendNewData(ipid, timeStamp, adj, fd, ashmemNameId, size, pss, ashmemId,
                                                        time, refCount, purged, flag);
    }
    AshMemDeduplicate();
}
void HtraceMemParser::AshMemDeduplicate() const
{
    auto ashMemData = traceDataCache_->GetAshMemData();
    auto ashMemCount = ashMemData->Size();
    if (ashMemCount <= 1) {
        return;
    }

    std::vector<std::pair<size_t, size_t>> dataByTs;
    size_t start = 0;
    auto ts = ashMemData->TimeStampData()[0];
    for (auto i = 0; i < ashMemCount; ++i) {
        auto tsTmp = ashMemData->TimeStampData()[i];
        if (tsTmp != ts) {
            dataByTs.emplace_back(std::make_pair(start, i - 1));
            start = i;
            ts = tsTmp;
        }
    }
    dataByTs.emplace_back(std::make_pair(start, ashMemCount - 1));

    for (const auto& iterator : dataByTs) {
        /* L1 map (key = id+time, value = L2 map)
           L2 map (key = ipid, value = index) */
        std::map<std::pair<uint32_t, uint64_t>, std::map<uint64_t, uint64_t>> AshMemMap;
        for (auto i = iterator.first; i <= iterator.second; ++i) {
            auto ashmemId = ashMemData->AshmemIds()[i];
            auto time = ashMemData->Times()[i];
            auto key = std::make_pair(ashmemId, time);
            auto ipid = ashMemData->Ipids()[i];
            auto& pidMap = AshMemMap[key];
            if (pidMap.find(ipid) == pidMap.end()) {
                pidMap.emplace(ipid, i);
            } else {
                ashMemData->SetFlag(i, MEM_DEDUPLICATE_FLAG_DUP_SAME_PROCESS);
            }
        }

        for (const auto& item : AshMemMap) {
            auto& pidMap = item.second;
            auto iter = pidMap.begin();
            if (iter == pidMap.end()) {
                continue;
            }
            for (++iter; iter != pidMap.end(); ++iter) {
                ashMemData->SetFlag(iter->second, MEM_DEDUPLICATE_FLAG_DUP_DIFF_PROCESS);
            }
        }
    }
}
HtraceMemParser::mem_process_type HtraceMemParser::GetMemProcessType(uint64_t ipid) const
{
    const auto& iterProcess = traceDataCache_->GetConstProcessData(ipid);
    if (iterProcess.cmdLine_ == "composer_host") {
        return PID_TYPE_COMPOSER;
    } else if (iterProcess.cmdLine_ == "render_service") {
        return PID_TYPE_RENDER_SERVICES;
    } else {
        return PID_TYPE_APP;
    }
}

void HtraceMemParser::ParseDmaMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    if (tracePacket->has_dmainfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_DMAMEM, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->dmainfo(); i; ++i) {
        ProtoReader::DmaInfo_Reader DmaMemInfo(i->ToBytes().data_, i->ToBytes().size_);
        auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(DmaMemInfo.pid(),
                                                                                  DmaMemInfo.name().ToStdString());
        uint32_t fd = DmaMemInfo.fd();
        uint64_t size = DmaMemInfo.size();
        uint32_t ino = DmaMemInfo.ino();
        uint64_t expPid = DmaMemInfo.exp_pid();
        DataIndex expTaskCommId = traceDataCache_->GetDataIndex(DmaMemInfo.exp_task_comm().ToStdString());
        DataIndex bufNameId = traceDataCache_->GetDataIndex(DmaMemInfo.buf_name().ToStdString());
        DataIndex expNameId = traceDataCache_->GetDataIndex(DmaMemInfo.exp_name().ToStdString());
        uint32_t flag = 0;
        traceDataCache_->GetDmaMemData()->AppendNewData(ipid, timeStamp, fd, size, ino, expPid, expTaskCommId,
                                                        bufNameId, expNameId, flag);
    }
    DmaMemDeduplicate();
}
void HtraceMemParser::DmaMemDeduplicate() const
{
    auto dmaMemData = traceDataCache_->GetDmaMemData();
    auto dmaCount = dmaMemData->Size();
    if (dmaCount <= 1) {
        return;
    }

    std::vector<std::pair<size_t, size_t>> dataByTs;
    size_t start = 0;
    auto ts = dmaMemData->TimeStampData()[0];
    for (auto i = 0; i < dmaCount; ++i) {
        auto tsTmp = dmaMemData->TimeStampData()[i];
        if (tsTmp != ts) {
            dataByTs.emplace_back(std::make_pair(start, i - 1));
            start = i;
            ts = tsTmp;
        }
    }
    dataByTs.emplace_back(std::make_pair(start, dmaCount - 1));

    for (const auto& iterator : dataByTs) {
        /* L1 map (key = ino, value = L2 map)
           L2 map (key = ipid, value = pair(index, mem_process_type)) */
        std::map<uint32_t, std::map<uint64_t, std::pair<uint64_t, mem_process_type>>> inoMap;
        std::map<uint32_t /*ino*/, mem_process_type> processTypeMap;
        for (auto i = iterator.first; i <= iterator.second; ++i) {
            auto ino = dmaMemData->Inos()[i];
            auto ipid = dmaMemData->Ipids()[i];
            auto& pidMap = inoMap[ino];
            if (pidMap.find(ipid) != pidMap.end()) {
                dmaMemData->SetFlag(i, MEM_DEDUPLICATE_FLAG_DUP_SAME_PROCESS);
            } else {
                auto processType = GetMemProcessType(ipid);
                pidMap.emplace(ipid, std::make_pair(i, processType));
                if (processTypeMap[ino] < processType) {
                    processTypeMap[ino] = processType;
                }
            }
        }

        for (const auto& item : inoMap) {
            auto maxPidType = processTypeMap[item.first];
            auto& pidMap = item.second;
            for (const auto& pidItem : pidMap) {
                if (pidItem.second.second < maxPidType) {
                    dmaMemData->SetFlag(pidItem.second.first, MEM_DEDUPLICATE_FLAG_DUP_DIFF_PROCESS);
                }
            }
        }
    }
}

void HtraceMemParser::ParseGpuProcessMemInfo(const ProtoReader::MemoryData_Reader* tracePacket,
                                             uint64_t timeStamp) const
{
    if (tracePacket->has_gpumemoryinfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_GPU_PROCESS_MEM, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->gpumemoryinfo(); i; ++i) {
        ProtoReader::GpuMemoryInfo_Reader GpuMemoryInfo(i->ToBytes().data_, i->ToBytes().size_);
        DataIndex gpuNameId = traceDataCache_->GetDataIndex(GpuMemoryInfo.gpu_name().ToStdString());
        uint64_t allGpuSize = GpuMemoryInfo.all_gpu_size();
        if (GpuMemoryInfo.has_gpu_process_info()) {
            for (auto j = GpuMemoryInfo.gpu_process_info(); j; ++j) {
                ProtoReader::GpuProcessInfo_Reader GpuProcessInfo(j->ToBytes().data_, j->ToBytes().size_);
                std::string addr = GpuProcessInfo.addr().ToStdString();
                uint32_t pid = GpuProcessInfo.pid();
                uint32_t tid = GpuProcessInfo.tid();
                auto itid = streamFilters_->processFilter_->GetOrCreateThreadWithPid(tid, pid);
                auto ipid = streamFilters_->processFilter_->GetOrCreateInternalPid(timeStamp, pid);
                uint64_t usedGpuSize = GpuProcessInfo.used_gpu_size();

                traceDataCache_->GetGpuProcessMemData()->AppendNewData(timeStamp, gpuNameId, allGpuSize, addr, ipid,
                                                                       itid, usedGpuSize);
            }
        }
    }
}

void HtraceMemParser::ParseGpuWindowMemInfo(const ProtoReader::MemoryData_Reader* tracePacket, uint64_t timeStamp) const
{
    if (tracePacket->has_gpudumpinfo()) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_GPU_WINDOW_MEM, STAT_EVENT_RECEIVED);
    }
    for (auto i = tracePacket->gpudumpinfo(); i; ++i) {
        ProtoReader::GpuDumpInfo_Reader GpuDumpInfo(i->ToBytes().data_, i->ToBytes().size_);
        DataIndex windowNameId = traceDataCache_->GetDataIndex(GpuDumpInfo.window_name().ToStdString());
        uint64_t windowId = GpuDumpInfo.id();
        uint64_t purgeableSize = GpuDumpInfo.gpu_purgeable_size();
        if (GpuDumpInfo.has_gpu_detail_info()) {
            for (auto j = GpuDumpInfo.gpu_detail_info(); j; ++j) {
                ProtoReader::GpuDetailInfo_Reader GpuDetailInfo(j->ToBytes().data_, j->ToBytes().size_);
                DataIndex moduleNameId = traceDataCache_->GetDataIndex(GpuDetailInfo.module_name().ToStdString());
                if (GpuDetailInfo.has_gpu_sub_info()) {
                    for (auto k = GpuDetailInfo.gpu_sub_info(); k; ++k) {
                        ProtoReader::GpuSubInfo_Reader GpuSubInfo(k->ToBytes().data_, k->ToBytes().size_);
                        DataIndex categoryNameId =
                            traceDataCache_->GetDataIndex(GpuSubInfo.category_name().ToStdString());
                        uint64_t size = GpuSubInfo.size();
                        uint32_t entryNum = GpuSubInfo.entry_num();
                        traceDataCache_->GetGpuWindowMemData()->AppendNewData(timeStamp, windowNameId, windowId,
                                                                              moduleNameId, categoryNameId, size,
                                                                              entryNum, purgeableSize);
                    }
                }
            }
        }
    }
}

void HtraceMemParser::ParseMemoryConfig(HtraceDataSegment& seg)
{
    ProtoReader::MemoryConfig_Reader memConfigData(seg.protoData.data_, seg.protoData.size_);
    if (memConfigData.has_pid()) {
        bool parseError = false;
        auto itor = memConfigData.pid(&parseError);
        if (parseError) {
            TS_LOGE("Parse pid in MemoryConfig function failed!!!");
            return;
        }
        while (itor) {
            int32_t pid = *itor;
            auto ipid = streamFilters_->processFilter_->GetOrCreateInternalPid(seg.timeStamp, pid);
            traceDataCache_->GetTraceConfigData()->AppendNewData("memory_config", "ipid", std::to_string(ipid));
            itor++;
        }
    }
}

void HtraceMemParser::Finish()
{
    if (traceDataCache_->traceStartTime_ == INVALID_UINT64 || traceDataCache_->traceEndTime_ == 0) {
        traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
    } else {
        TS_LOGI("mem data time is not updated, maybe this trace file has other data");
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
