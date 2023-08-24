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

#ifndef TRACE_DATA_CACHE_READER_H
#define TRACE_DATA_CACHE_READER_H

#include "log.h"
#include "trace_data_cache_base.h"
#include "trace_stdtype.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
class TraceDataCacheReader : virtual public TraceDataCacheBase {
public:
    TraceDataCacheReader() = default;
    TraceDataCacheReader(const TraceDataCacheReader&) = delete;
    TraceDataCacheReader& operator=(const TraceDataCacheReader&) = delete;
    ~TraceDataCacheReader() override;

public:
    const std::string& GetDataFromDict(DataIndex id) const;
    const Process& GetConstProcessData(InternalPid internalPid) const;
    const std::deque<Process>& GetConstProcessData() const;
    const Thread& GetConstThreadData(InternalTid internalTid) const;
    const std::deque<Thread>& GetConstThreadData() const;
    const CallStack& GetConstInternalSlicesData() const;
    const CallStack& GetConstIrqData() const;
    const Filter& GetConstFilterData() const;
    const Raw& GetConstRawTableData() const;
    const Measure& GetConstMeasureData() const;
    const Measure& GetConstSysMemMeasureData() const;
    const Measure& GetConstProcessMeasureData() const;
    const ThreadMeasureFilter& GetConstThreadMeasureFilterData() const;
    const ThreadStateData& GetConstThreadStateData() const;
    const SchedSlice& GetConstSchedSliceData() const;
    const CpuMeasureFilter& GetConstCpuMeasureData() const;
    const ThreadMeasureFilter& GetConstThreadFilterData() const;
    const Instants& GetConstInstantsData() const;
    const ProcessMeasureFilter& GetConstProcessFilterData() const;
    const ProcessMeasureFilter& GetConstProcessMeasureFilterData() const;
    const ClockEventData& GetConstClockEventFilterData() const;
    const ClkEventData& GetConstClkEventFilterData() const;
    const std::string& GetConstSchedStateData(uint64_t rowId) const;
    uint64_t TraceStartTime() const;
    uint64_t TraceEndTime() const;
    const StatAndInfo& GetConstStatAndInfo() const;
    const MetaData& GetConstMetaData() const;
    const SymbolsData& GetConstSymbolsData() const;
    const SysCall& GetConstSysCallData() const;
    const LogInfo& GetConstHilogData() const;
    const NativeHook& GetConstNativeHookData() const;
    const NativeHookFrame& GetConstNativeHookFrameData() const;
    const NativeHookStatistic& GetConstNativeHookStatisticData() const;
    const Hidump& GetConstHidumpData() const;
    const PerfCallChain& GetConstPerfCallChainData() const;
    const PerfFiles& GetConstPerfFilesData() const;
    const PerfSample& GetConstPerfSampleData() const;
    const PerfThread& GetConstPerfThreadData() const;
    const PerfReport& GetConstPerfReportData() const;
    const ArgSet& GetConstArgSetData() const;
    const DataType& GetConstDataTypeData() const;
    const SysMeasureFilter& GetConstSysMeasureFilterData() const;
    const NetDetailData& GetConstNetworkData() const;
    const CpuUsageDetailData& GetConstCpuUsageInfoData() const;
    const DiskIOData& GetConstDiskIOData() const;
    const LiveProcessDetailData& GetConstLiveProcessData() const;
    const FileSystemSample& GetConstFileSystemSample() const;
    const DeviceStateData& GetConstDeviceStateData() const;
    const EbpfCallStackData& GetConstEbpfCallStackData() const;
    const PagedMemorySampleData& GetConstPagedMemorySampleData() const;
#ifdef WITH_EBPF_HELP
    const EbpfProcessMaps& GetConstEbpfProcessMaps() const;
    const EbpfElf& GetConstEbpfElf() const;
    const EbpfElfSymbol& GetConstEbpfElfSymbol() const;
#endif
    const AppNames& GetConstAppNamesData() const;
    const SysEventMeasureData& GetConstSyseventMeasureData() const;
    const TraceConfigData& GetConstTraceConfigData() const;
    const SmapsData& GetConstSmapsData() const;
    const BioLatencySampleData& GetConstBioLatencySampleData() const;
    const ClockSnapshotData& GetConstClockSnapshotData() const;
    const DataSourceClockIdData& GetConstDataSourceClockIdData() const;
    const FrameSlice& GetConstFameSliceData() const;
    const FrameMaps& GetConstFameMapsData() const;
    const GPUSlice& GetConstGPUSliceData() const;
    const TaskPoolInfo& GetConstTaskPoolData() const;
    const JsHeapFiles& GetConstJsHeapFilesData() const;
    const JsHeapEdges& GetConstJsHeapEdgesData() const;
    const JsHeapInfo& GetConstJsHeapInfoData() const;
    const JsHeapLocation& GetConstJsHeapLocationData() const;
    const JsHeapNodes& GetConstJsHeapNodesData() const;
    const JsHeapSample& GetConstJsHeapSampleData() const;
    const JsHeapString& GetConstJsHeapStringData() const;
    const JsHeapTraceFuncInfo& GetConstJsHeapTraceFuncInfoData() const;
    const JsHeapTraceNode& GetConstJsHeapTraceNodeData() const;
    const JsCpuProfilerNode& GetConstJsCpuProfilerNodeData() const;
    const JsCpuProfilerSample& GetConstJsCpuProfilerSampleData() const;
    const JsConfig& GetConstJsConfigData() const;
    const AppStartup& GetConstAppStartupData() const;
    const SoStaticInitalization& GetConstStaticInitalizationData() const;
    const Animation& GetConstAnimation() const;
    const DeviceInfo& GetConstDeviceInfo() const;
    const DynamicFrame& GetConstDynamicFrame() const;
    const AshMemData& GetConstAshMemData() const;
    const DmaMemData& GetConstDmaMemData() const;
    const GpuProcessMemData& GetConstGpuProcessMemData() const;
    const GpuWindowMemData& GetConstGpuWindowMemData() const;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif
