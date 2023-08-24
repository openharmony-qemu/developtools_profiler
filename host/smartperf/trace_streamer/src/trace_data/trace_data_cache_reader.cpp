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

#include "trace_data_cache_reader.h"
#include "log.h"
namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
TraceDataCacheReader::~TraceDataCacheReader() {}
const std::string& TraceDataCacheReader::GetDataFromDict(DataIndex id) const
{
    return dataDict_.GetDataFromDict(id);
}

const std::deque<Process>& TraceDataCacheReader::GetConstProcessData() const
{
    return internalProcessesData_;
}
const Process& TraceDataCacheReader::GetConstProcessData(InternalPid internalPid) const
{
    TS_ASSERT(internalPid < internalProcessesData_.size());
    return internalProcessesData_[internalPid];
}
const Thread& TraceDataCacheReader::GetConstThreadData(InternalTid internalTid) const
{
    TS_ASSERT(internalTid < internalThreadsData_.size());
    return internalThreadsData_[internalTid];
}

const std::deque<Thread>& TraceDataCacheReader::GetConstThreadData() const
{
    return internalThreadsData_;
}
const CallStack& TraceDataCacheReader::GetConstInternalSlicesData() const
{
    return callstackData_;
}
const CallStack& TraceDataCacheReader::GetConstIrqData() const
{
    return irqData_;
}
const Filter& TraceDataCacheReader::GetConstFilterData() const
{
    return filterData_;
}
const Raw& TraceDataCacheReader::GetConstRawTableData() const
{
    return rawData_;
}
const Measure& TraceDataCacheReader::GetConstMeasureData() const
{
    return measureData_;
}
const Measure& TraceDataCacheReader::GetConstSysMemMeasureData() const
{
    return sysMemMeasureData_;
}
const Measure& TraceDataCacheReader::GetConstProcessMeasureData() const
{
    return processMeasureData_;
}

const ThreadMeasureFilter& TraceDataCacheReader::GetConstThreadMeasureFilterData() const
{
    return threadMeasureFilterData_;
}
const ThreadStateData& TraceDataCacheReader::GetConstThreadStateData() const
{
    return threadStateData_;
}
const SchedSlice& TraceDataCacheReader::GetConstSchedSliceData() const
{
    return schedSliceData_;
}
const CpuMeasureFilter& TraceDataCacheReader::GetConstCpuMeasureData() const
{
    return cpuMeasureData_;
}
const ThreadMeasureFilter& TraceDataCacheReader::GetConstThreadFilterData() const
{
    return threadFilterData_;
}
const Instants& TraceDataCacheReader::GetConstInstantsData() const
{
    return instantsData_;
}
const ProcessMeasureFilter& TraceDataCacheReader::GetConstProcessFilterData() const
{
    return processFilterData_;
}
const ProcessMeasureFilter& TraceDataCacheReader::GetConstProcessMeasureFilterData() const
{
    return processMeasureFilterData_;
}

const ClockEventData& TraceDataCacheReader::GetConstClockEventFilterData() const
{
    return clockEventFilterData_;
}
const ClkEventData& TraceDataCacheReader::GetConstClkEventFilterData() const
{
    return clkEventFilterData_;
}
const std::string& TraceDataCacheReader::GetConstSchedStateData(uint64_t rowId) const
{
    TS_ASSERT(statusString_.find(rowId) != statusString_.end());
    return statusString_.at(rowId);
}
uint64_t TraceDataCacheReader::TraceStartTime() const
{
    return traceStartTime_;
}
uint64_t TraceDataCacheReader::TraceEndTime() const
{
    return traceEndTime_;
}

const StatAndInfo& TraceDataCacheReader::GetConstStatAndInfo() const
{
    return stat_;
}
const MetaData& TraceDataCacheReader::GetConstMetaData() const
{
    return metaData_;
}

const SymbolsData& TraceDataCacheReader::GetConstSymbolsData() const
{
    return symbolsData_;
}

const LogInfo& TraceDataCacheReader::GetConstHilogData() const
{
    return hilogData_;
}

const NativeHook& TraceDataCacheReader::GetConstNativeHookData() const
{
    return nativeHookData_;
}

const NativeHookFrame& TraceDataCacheReader::GetConstNativeHookFrameData() const
{
    return nativeHookFrameData_;
}
const NativeHookStatistic& TraceDataCacheReader::GetConstNativeHookStatisticData() const
{
    return nativeHookStatisticData_;
}
const Hidump& TraceDataCacheReader::GetConstHidumpData() const
{
    return hidumpData_;
}

const PerfCallChain& TraceDataCacheReader::GetConstPerfCallChainData() const
{
    return perfCallChain_;
}
const PerfFiles& TraceDataCacheReader::GetConstPerfFilesData() const
{
    return perfFiles_;
}
const PerfSample& TraceDataCacheReader::GetConstPerfSampleData() const
{
    return perfSample_;
}
const PerfThread& TraceDataCacheReader::GetConstPerfThreadData() const
{
    return perfThread_;
}
const PerfReport& TraceDataCacheReader::GetConstPerfReportData() const
{
    return perfReport_;
}
const SysCall& TraceDataCacheReader::GetConstSysCallData() const
{
    return sysCallData_;
}
const ArgSet& TraceDataCacheReader::GetConstArgSetData() const
{
    return argSet_;
}
const TaskPoolInfo& TraceDataCacheReader::GetConstTaskPoolData() const
{
    return taskPoolInfo_;
}
const JsHeapFiles& TraceDataCacheReader::GetConstJsHeapFilesData() const
{
    return jsHeapFilesData_;
}
const JsHeapEdges& TraceDataCacheReader::GetConstJsHeapEdgesData() const
{
    return jsHeapEdgesData_;
}
const JsHeapInfo& TraceDataCacheReader::GetConstJsHeapInfoData() const
{
    return jsHeapInfoData_;
}
const JsHeapLocation& TraceDataCacheReader::GetConstJsHeapLocationData() const
{
    return jsHeapLocationData_;
}
const JsHeapNodes& TraceDataCacheReader::GetConstJsHeapNodesData() const
{
    return jsHeapNodesData_;
}
const JsHeapSample& TraceDataCacheReader::GetConstJsHeapSampleData() const
{
    return jsHeapSampleData_;
}
const JsHeapString& TraceDataCacheReader::GetConstJsHeapStringData() const
{
    return jsHeapStringData_;
}
const JsHeapTraceFuncInfo& TraceDataCacheReader::GetConstJsHeapTraceFuncInfoData() const
{
    return jsHeapTraceFuncInfoData_;
}
const JsHeapTraceNode& TraceDataCacheReader::GetConstJsHeapTraceNodeData() const
{
    return jsHeapTraceNodeData_;
}
const JsCpuProfilerNode& TraceDataCacheReader::GetConstJsCpuProfilerNodeData() const
{
    return jsCpuProfilerNodeData_;
}
const JsCpuProfilerSample& TraceDataCacheReader::GetConstJsCpuProfilerSampleData() const
{
    return jsCpuProfilerSampleData_;
}

const JsConfig& TraceDataCacheReader::GetConstJsConfigData() const
{
    return jsConfigData_;
}

const DataType& TraceDataCacheReader::GetConstDataTypeData() const
{
    return dataType_;
}

const SysMeasureFilter& TraceDataCacheReader::GetConstSysMeasureFilterData() const
{
    return sysEvent_;
}
const NetDetailData& TraceDataCacheReader::GetConstNetworkData() const
{
    return networkData_;
}
const CpuUsageDetailData& TraceDataCacheReader::GetConstCpuUsageInfoData() const
{
    return cpuUsageData_;
}
const DiskIOData& TraceDataCacheReader::GetConstDiskIOData() const
{
    return diskIOData_;
}
const LiveProcessDetailData& TraceDataCacheReader::GetConstLiveProcessData() const
{
    return liveProcessDetailData_;
}
const FileSystemSample& TraceDataCacheReader::GetConstFileSystemSample() const
{
    return fileSamplingTableData_;
}
const EbpfCallStackData& TraceDataCacheReader::GetConstEbpfCallStackData() const
{
    return ebpfCallStackData_;
}
const PagedMemorySampleData& TraceDataCacheReader::GetConstPagedMemorySampleData() const
{
    return PagedMemorySampleData_;
}
#if WITH_EBPF_HELP
const EbpfProcessMaps& TraceDataCacheReader::GetConstEbpfProcessMaps() const
{
    return ebpfProcessMaps_;
}
const EbpfElf& TraceDataCacheReader::GetConstEbpfElf() const
{
    return ebpfElf_;
}
const EbpfElfSymbol& TraceDataCacheReader::GetConstEbpfElfSymbol() const
{
    return ebpfElfSymbol_;
}
#endif
const AppNames& TraceDataCacheReader::GetConstAppNamesData() const
{
    return appNames_;
}
const SysEventMeasureData& TraceDataCacheReader::GetConstSyseventMeasureData() const
{
    return sysEventMeasureData_;
}
const TraceConfigData& TraceDataCacheReader::GetConstTraceConfigData() const
{
    return traceConfigData_;
}
const DeviceStateData& TraceDataCacheReader::GetConstDeviceStateData() const
{
    return deviceStateData_;
}
const SmapsData& TraceDataCacheReader::GetConstSmapsData() const
{
    return smapsData_;
}
const BioLatencySampleData& TraceDataCacheReader::GetConstBioLatencySampleData() const
{
    return bioLatencySampleData_;
}

const ClockSnapshotData& TraceDataCacheReader::GetConstClockSnapshotData() const
{
    return clockSnapshotData_;
}

const DataSourceClockIdData& TraceDataCacheReader::GetConstDataSourceClockIdData() const
{
    return dataSourceClockIdData_;
}
const FrameSlice& TraceDataCacheReader::GetConstFameSliceData() const
{
    return frameSliceData_;
}
const FrameMaps& TraceDataCacheReader::GetConstFameMapsData() const
{
    return frameMapsData_;
}
const GPUSlice& TraceDataCacheReader::GetConstGPUSliceData() const
{
    return gpuSliceData_;
}
const AppStartup& TraceDataCacheReader::GetConstAppStartupData() const
{
    return appStartupData_;
}
const SoStaticInitalization& TraceDataCacheReader::GetConstStaticInitalizationData() const
{
    return staticInitalizationData_;
}
const Animation& TraceDataCacheReader::GetConstAnimation() const
{
    return animation_;
}
const DeviceInfo& TraceDataCacheReader::GetConstDeviceInfo() const
{
    return deviceInfo_;
}
const DynamicFrame& TraceDataCacheReader::GetConstDynamicFrame() const
{
    return dynamicFrame_;
}
const AshMemData& TraceDataCacheReader::GetConstAshMemData() const
{
    return ashMemData_;
}
const DmaMemData& TraceDataCacheReader::GetConstDmaMemData() const
{
    return dmaMemData_;
}
const GpuProcessMemData& TraceDataCacheReader::GetConstGpuProcessMemData() const
{
    return gpuProcessMemData_;
}
const GpuWindowMemData& TraceDataCacheReader::GetConstGpuWindowMemData() const
{
    return gpuWindowMemData_;
}
} // namespace TraceStreamer
} // namespace SysTuning
