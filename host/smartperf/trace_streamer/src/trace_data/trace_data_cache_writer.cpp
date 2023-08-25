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

#include "trace_data_cache_writer.h"
#include "log.h"
namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
TraceDataCacheWriter::~TraceDataCacheWriter() {}
InternalPid TraceDataCacheWriter::GetProcessInternalPid(uint32_t pid)
{
    internalProcessesData_.emplace_back(pid);
    return static_cast<InternalPid>(internalProcessesData_.size() - 1);
}
Process* TraceDataCacheWriter::GetProcessData(InternalPid internalPid)
{
    TS_ASSERT(internalPid < internalProcessesData_.size());
    return &internalProcessesData_[internalPid];
}

uint32_t TraceDataCacheWriter::AppendNewProcessData(uint32_t pid, const std::string& name, uint64_t startTs)
{
    internalProcessesData_.emplace_back(pid);
    auto& process = internalProcessesData_.back();
    process.cmdLine_ = name;
    process.startT_ = startTs;
    return internalProcessesData_.size() - 1;
}

InternalTid TraceDataCacheWriter::NewInternalThread(uint32_t tid)
{
    internalThreadsData_.emplace_back(tid);
    return static_cast<InternalTid>(internalThreadsData_.size() - 1);
}
Thread* TraceDataCacheWriter::GetThreadData(InternalTid internalTid)
{
    if (internalTid >= internalThreadsData_.size()) {
        return nullptr;
    }
    return &internalThreadsData_[internalTid];
}

void TraceDataCacheWriter::UpdateTraceTime(uint64_t timeStamp)
{
    if (timeStamp) {
        traceStartTime_ = std::min(traceStartTime_, timeStamp);
        traceEndTime_ = std::max(traceEndTime_, timeStamp);
    }
}

void TraceDataCacheWriter::MixTraceTime(uint64_t timestampMin, uint64_t timestampMax)
{
    if (timestampMin == std::numeric_limits<uint64_t>::max() || timestampMax == 0 || timestampMin == timestampMax) {
        return;
    }
    if ((timestampMax < traceStartTime_ || timestampMin > traceEndTime_) &&
        (traceStartTime_ != std::numeric_limits<uint64_t>::max() && traceEndTime_ != 0)) {
        return;
    }
    if ((timestampMin == timestampMax - 1) && traceStartTime_ != std::numeric_limits<uint64_t>::max() &&
        traceEndTime_) {
        return;
    }
    if (traceStartTime_ != std::numeric_limits<uint64_t>::max()) {
        traceStartTime_ = std::max(traceStartTime_, timestampMin);
    } else {
        traceStartTime_ = timestampMin;
    }
    if (traceEndTime_) {
        traceEndTime_ = std::min(traceEndTime_, timestampMax);
    } else {
        traceEndTime_ = timestampMax;
    }
}
CallStack* TraceDataCacheWriter::GetInternalSlicesData()
{
    return &callstackData_;
}
CallStack* TraceDataCacheWriter::GetIrqData()
{
    return &irqData_;
}

Filter* TraceDataCacheWriter::GetFilterData()
{
    return &filterData_;
}

Raw* TraceDataCacheWriter::GetRawData()
{
    return &rawData_;
}

Measure* TraceDataCacheWriter::GetMeasureData()
{
    return &measureData_;
}

Measure* TraceDataCacheWriter::GetSysMemMeasureData()
{
    return &sysMemMeasureData_;
}
Measure* TraceDataCacheWriter::GetProcessMeasureData()
{
    return &processMeasureData_;
}

ThreadStateData* TraceDataCacheWriter::GetThreadStateData()
{
    return &threadStateData_;
}

SchedSlice* TraceDataCacheWriter::GetSchedSliceData()
{
    return &schedSliceData_;
}

CpuMeasureFilter* TraceDataCacheWriter::GetCpuMeasuresData()
{
    return &cpuMeasureData_;
}

ThreadMeasureFilter* TraceDataCacheWriter::GetThreadMeasureFilterData()
{
    return &threadMeasureFilterData_;
}

ThreadMeasureFilter* TraceDataCacheWriter::GetThreadFilterData()
{
    return &threadFilterData_;
}

Instants* TraceDataCacheWriter::GetInstantsData()
{
    return &instantsData_;
}

ProcessMeasureFilter* TraceDataCacheWriter::GetProcessFilterData()
{
    return &processFilterData_;
}

ProcessMeasureFilter* TraceDataCacheWriter::GetProcessMeasureFilterData()
{
    return &processMeasureFilterData_;
}

ClockEventData* TraceDataCacheWriter::GetClockEventFilterData()
{
    return &clockEventFilterData_;
}

ClkEventData* TraceDataCacheWriter::GetClkEventFilterData()
{
    return &clkEventFilterData_;
}
StatAndInfo* TraceDataCacheWriter::GetStatAndInfo()
{
    return &stat_;
}

MetaData* TraceDataCacheWriter::GetMetaData()
{
    return &metaData_;
}

SymbolsData* TraceDataCacheWriter::GetSymbolsData()
{
    return &symbolsData_;
}
SysCall* TraceDataCacheWriter::GetSysCallData()
{
    return &sysCallData_;
}
LogInfo* TraceDataCacheWriter::GetHilogData()
{
    return &hilogData_;
}

NativeHook* TraceDataCacheWriter::GetNativeHookData()
{
    return &nativeHookData_;
}

NativeHookFrame* TraceDataCacheWriter::GetNativeHookFrameData()
{
    return &nativeHookFrameData_;
}

NativeHookStatistic* TraceDataCacheWriter::GetNativeHookStatisticsData()
{
    return &nativeHookStatisticData_;
}
Hidump* TraceDataCacheWriter::GetHidumpData()
{
    return &hidumpData_;
}
PerfCallChain* TraceDataCacheWriter::GetPerfCallChainData()
{
    return &perfCallChain_;
}
PerfFiles* TraceDataCacheWriter::GetPerfFilesData()
{
    return &perfFiles_;
}
PerfSample* TraceDataCacheWriter::GetPerfSampleData()
{
    return &perfSample_;
}
PerfThread* TraceDataCacheWriter::GetPerfThreadData()
{
    return &perfThread_;
}
PerfReport* TraceDataCacheWriter::GetPerfReportData()
{
    return &perfReport_;
}
ArgSet* TraceDataCacheWriter::GetArgSetData()
{
    return &argSet_;
}

DataType* TraceDataCacheWriter::GetDataTypeData()
{
    return &dataType_;
}

SysMeasureFilter* TraceDataCacheWriter::GetSysMeasureFilterData()
{
    return &sysEvent_;
}
NetDetailData* TraceDataCacheWriter::GetNetworkData()
{
    return &networkData_;
}
NetDetailData* TraceDataCacheWriter::GetNetworkDetailData()
{
    return &networkDetailData_;
}
DiskIOData* TraceDataCacheWriter::GetDiskIOData()
{
    return &diskIOData_;
}

CpuUsageDetailData* TraceDataCacheWriter::GetCpuUsageInfoData()
{
    return &cpuUsageData_;
}
LiveProcessDetailData* TraceDataCacheWriter::GetLiveProcessData()
{
    return &liveProcessDetailData_;
}
FileSystemSample* TraceDataCacheWriter::GetFileSystemSample()
{
    return &fileSamplingTableData_;
}
EbpfCallStackData* TraceDataCacheWriter::GetEbpfCallStack()
{
    return &ebpfCallStackData_;
}
PagedMemorySampleData* TraceDataCacheWriter::GetPagedMemorySampleData()
{
    return &PagedMemorySampleData_;
}
#if WITH_EBPF_HELP
EbpfProcessMaps* TraceDataCacheWriter::GetEbpfProcessMaps()
{
    return &ebpfProcessMaps_;
}
EbpfElf* TraceDataCacheWriter::GetEbpfElf()
{
    return &ebpfElf_;
}
EbpfElfSymbol* TraceDataCacheWriter::GetEbpfElfSymbol()
{
    return &ebpfElfSymbol_;
}
#endif
AppNames* TraceDataCacheWriter::GetAppNamesData()
{
    return &appNames_;
}
SysEventMeasureData* TraceDataCacheWriter::GetSyseventMeasureData()
{
    return &sysEventMeasureData_;
}
DeviceStateData* TraceDataCacheWriter::GetDeviceStateData()
{
    return &deviceStateData_;
}
TraceConfigData* TraceDataCacheWriter::GetTraceConfigData()
{
    return &traceConfigData_;
}
SmapsData* TraceDataCacheWriter::GetSmapsData()
{
    return &smapsData_;
}
BioLatencySampleData* TraceDataCacheWriter::GetBioLatencySampleData()
{
    return &bioLatencySampleData_;
}

ClockSnapshotData* TraceDataCacheWriter::GetClockSnapshotData()
{
    return &clockSnapshotData_;
}
DataSourceClockIdData* TraceDataCacheWriter::GetDataSourceClockIdData()
{
    return &dataSourceClockIdData_;
}
FrameSlice* TraceDataCacheWriter::GetFrameSliceData()
{
    return &frameSliceData_;
}
FrameMaps* TraceDataCacheWriter::GetFrameMapsData()
{
    return &frameMapsData_;
}

GPUSlice* TraceDataCacheWriter::GetGPUSliceData()
{
    return &gpuSliceData_;
}
TaskPoolInfo* TraceDataCacheWriter::GetTaskPoolData()
{
    return &taskPoolInfo_;
}
JsHeapFiles* TraceDataCacheWriter::GetJsHeapFilesData()
{
    return &jsHeapFilesData_;
}
JsHeapEdges* TraceDataCacheWriter::GetJsHeapEdgesData()
{
    return &jsHeapEdgesData_;
}
JsHeapInfo* TraceDataCacheWriter::GetJsHeapInfoData()
{
    return &jsHeapInfoData_;
}
JsHeapLocation* TraceDataCacheWriter::GetJsHeapLocationData()
{
    return &jsHeapLocationData_;
}
JsHeapNodes* TraceDataCacheWriter::GetJsHeapNodesData()
{
    return &jsHeapNodesData_;
}
JsHeapSample* TraceDataCacheWriter::GetJsHeapSampleData()
{
    return &jsHeapSampleData_;
}
JsHeapString* TraceDataCacheWriter::GetJsHeapStringData()
{
    return &jsHeapStringData_;
}
JsHeapTraceFuncInfo* TraceDataCacheWriter::GetJsHeapTraceFuncInfoData()
{
    return &jsHeapTraceFuncInfoData_;
}
JsHeapTraceNode* TraceDataCacheWriter::GetJsHeapTraceNodeData()
{
    return &jsHeapTraceNodeData_;
}
JsCpuProfilerNode* TraceDataCacheWriter::GetJsCpuProfilerNodeData()
{
    return &jsCpuProfilerNodeData_;
}
JsCpuProfilerSample* TraceDataCacheWriter::GetJsCpuProfilerSampleData()
{
    return &jsCpuProfilerSampleData_;
}
JsConfig* TraceDataCacheWriter::GetJsConfigData()
{
    return &jsConfigData_;
}
AppStartup* TraceDataCacheWriter::GetAppStartupData()
{
    return &appStartupData_;
}
SoStaticInitalization* TraceDataCacheWriter::GetStaticInitalizationData()
{
    return &staticInitalizationData_;
}
Animation* TraceDataCacheWriter::GetAnimation()
{
    return &animation_;
}
DeviceInfo* TraceDataCacheWriter::GetDeviceInfo()
{
    return &deviceInfo_;
}
DynamicFrame* TraceDataCacheWriter::GetDynamicFrame()
{
    return &dynamicFrame_;
}
AshMemData* TraceDataCacheWriter::GetAshMemData()
{
    return &ashMemData_;
}
DmaMemData* TraceDataCacheWriter::GetDmaMemData()
{
    return &dmaMemData_;
}
GpuProcessMemData* TraceDataCacheWriter::GetGpuProcessMemData()
{
    return &gpuProcessMemData_;
}
GpuWindowMemData* TraceDataCacheWriter::GetGpuWindowMemData()
{
    return &gpuWindowMemData_;
}

void TraceDataCacheWriter::Clear()
{
    rawData_.Clear();
    threadStateData_.Clear();
    instantsData_.Clear();

    filterData_.Clear();
    processMeasureFilterData_.Clear();
    clockEventFilterData_.Clear();
    clkEventFilterData_.Clear();
    processFilterData_.Clear();
    threadMeasureFilterData_.Clear();
    threadFilterData_.Clear();
    dataDict_.Clear();

    schedSliceData_.Clear();
    callstackData_.Clear();
    irqData_.Clear();
    hilogData_.Clear();
    nativeHookData_.Clear();
    nativeHookFrameData_.Clear();
    hidumpData_.Clear();

    internalProcessesData_.clear();
    internalThreadsData_.clear();

    measureData_.Clear();
    cpuMeasureData_.Clear();

    metaData_.Clear();
    symbolsData_.Clear();
    sysCallData_.Clear();
    argSet_.Clear();
    dataType_.Clear();
    sysEvent_.Clear();
    networkData_.Clear();
    networkDetailData_.Clear();
    perfSample_.Clear();
    perfCallChain_.Clear();
    perfThread_.Clear();
    perfFiles_.Clear();
    perfReport_.Clear();
    cpuUsageData_.Clear();
    diskIOData_.Clear();
    liveProcessDetailData_.Clear();
    fileSamplingTableData_.Clear();
    ebpfCallStackData_.Clear();
    PagedMemorySampleData_.Clear();
    jsHeapFilesData_.Clear();
    jsHeapEdgesData_.Clear();
    jsHeapInfoData_.Clear();
    jsHeapLocationData_.Clear();
    jsHeapNodesData_.Clear();
    jsHeapSampleData_.Clear();
    jsHeapStringData_.Clear();
    jsHeapTraceFuncInfoData_.Clear();
    jsHeapTraceNodeData_.Clear();
    jsCpuProfilerNodeData_.Clear();
    jsCpuProfilerSampleData_.Clear();
    jsConfigData_.Clear();
    appStartupData_.Clear();
    staticInitalizationData_.Clear();
    animation_.Clear();
    deviceInfo_.Clear();
    dynamicFrame_.Clear();

#if WITH_EBPF_HELP
    ebpfProcessMaps_.Clear();
    ebpfElf_.Clear();
    ebpfElfSymbol_.Clear();
#endif
    appNames_.Clear();
    sysEventMeasureData_.Clear();
    deviceStateData_.Clear();
    smapsData_.Clear();
    bioLatencySampleData_.Clear();
    taskPoolInfo_.Clear();
    ashMemData_.Clear();
    dmaMemData_.Clear();
    gpuProcessMemData_.Clear();
    gpuWindowMemData_.Clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
