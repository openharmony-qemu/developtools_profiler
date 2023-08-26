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

#ifndef TRACE_DATA_CACHE_BASE_H
#define TRACE_DATA_CACHE_BASE_H

#include <array>
#include <deque>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "trace_stdtype.h"
#include "ts_common.h"
namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
class TraceDataCacheBase {
public:
    TraceDataCacheBase();
    TraceDataCacheBase(const TraceDataCacheBase&) = delete;
    TraceDataCacheBase& operator=(const TraceDataCacheBase&) = delete;
    virtual ~TraceDataCacheBase() = default;

public:
    size_t ThreadSize() const
    {
        return internalThreadsData_.size();
    }
    size_t ProcessSize() const
    {
        return internalProcessesData_.size();
    }

    void UpdataZeroThreadInfo();
    size_t DataDictSize() const
    {
        return dataDict_.Size();
    }
    void UpdateTraceRange()
    {
        metaData_.SetTraceDuration((traceEndTime_ - traceStartTime_) / SEC_TO_NS);
    }
    uint64_t GetThreadStateValue(const std::string& status) const
    {
        if (threadStatus2Value_.count(status)) {
            return threadStatus2Value_.at(status);
        }
        return INVALID_UINT64;
    }
    DataIndex GetDataIndex(std::string_view str);
    DataIndex GetConstDataIndex(std::string_view str) const;
    std::map<uint64_t, std::string> statusString_ = {{TASK_RUNNABLE, "R"},
                                                     {TASK_INTERRUPTIBLE, "S"},
                                                     {TASK_UNINTERRUPTIBLE, "D"},
                                                     {TASK_UNINTERRUPTIBLE_IO, "D-IO"},
                                                     {TASK_UNINTERRUPTIBLE_NIO, "D-NIO"},
                                                     {TASK_RUNNING, "Running"},
                                                     {TASK_INTERRUPTED, "T"},
                                                     {TASK_TRACED, "t"},
                                                     {TASK_EXIT_DEAD, "X"},
                                                     {TASK_ZOMBIE, "Z"},
                                                     {TASK_CLONE, "I"},
                                                     {TASK_KILLED, "I"},
                                                     {TASK_DK, "DK"},
                                                     {TASK_DK_IO, "DK-IO"},
                                                     {TASK_DK_NIO, "DK-NIO"},
                                                     {TASK_TRACED_KILL, "TK"},
                                                     {TASK_WAKEKILL, "K"},
                                                     {TASK_PARKED, "P"},
                                                     {TASK_FOREGROUND, "R+"},
                                                     {TASK_RUNNABLE_BINDER, "R-B"},
                                                     {TASK_MAX, "S"},
                                                     {TASK_INVALID, "U"}};
    std::map<std::string, uint64_t> threadStatus2Value_ = {};
    uint64_t traceStartTime_ = std::numeric_limits<uint64_t>::max();
    uint64_t traceEndTime_ = 0;

    Raw rawData_;
    ThreadStateData threadStateData_;
    Instants instantsData_;

    Filter filterData_;
    ProcessMeasureFilter processMeasureFilterData_;
    ClockEventData clockEventFilterData_;
    ClkEventData clkEventFilterData_;
    ProcessMeasureFilter processFilterData_;
    ThreadMeasureFilter threadMeasureFilterData_;
    ThreadMeasureFilter threadFilterData_;
    DataDict dataDict_;

    SchedSlice schedSliceData_;
    CallStack callstackData_;
    CallStack irqData_;
    LogInfo hilogData_;
    NativeHook nativeHookData_;
    NativeHookFrame nativeHookFrameData_;
    NativeHookStatistic nativeHookStatisticData_;
    Hidump hidumpData_;
    PerfSample perfSample_;
    PerfCallChain perfCallChain_;
    PerfThread perfThread_;
    PerfFiles perfFiles_;
    PerfReport perfReport_;

    std::deque<Process> internalProcessesData_ = {};
    std::deque<Thread> internalThreadsData_ = {};

    Measure measureData_;
    Measure sysMemMeasureData_;
    Measure processMeasureData_;
    CpuMeasureFilter cpuMeasureData_;

    StatAndInfo stat_;
    MetaData metaData_;
    SymbolsData symbolsData_;
    SysCall sysCallData_;
    ArgSet argSet_;
    DataType dataType_;
    SysMeasureFilter sysEvent_;
    NetDetailData networkData_;
    NetDetailData networkDetailData_;
    CpuUsageDetailData cpuUsageData_;
    DiskIOData diskIOData_;
    LiveProcessDetailData liveProcessDetailData_;
    FileSystemSample fileSamplingTableData_;
    EbpfCallStackData ebpfCallStackData_;
    PagedMemorySampleData PagedMemorySampleData_;
#if WITH_EBPF_HELP
    EbpfProcessMaps ebpfProcessMaps_;
    EbpfElf ebpfElf_;
    EbpfElfSymbol ebpfElfSymbol_;
#endif
    AppNames appNames_;
    SysEventMeasureData sysEventMeasureData_;
    DeviceStateData deviceStateData_;
    TraceConfigData traceConfigData_;
    SmapsData smapsData_;
    BioLatencySampleData bioLatencySampleData_;
    ClockSnapshotData clockSnapshotData_;
    DataSourceClockIdData dataSourceClockIdData_;
    FrameSlice frameSliceData_;
    FrameMaps frameMapsData_;
    GPUSlice gpuSliceData_;
    TaskPoolInfo taskPoolInfo_;
    JsHeapFiles jsHeapFilesData_;
    JsHeapEdges jsHeapEdgesData_;
    JsHeapInfo jsHeapInfoData_;
    JsHeapLocation jsHeapLocationData_;
    JsHeapNodes jsHeapNodesData_;
    JsHeapSample jsHeapSampleData_;
    JsHeapString jsHeapStringData_;
    JsHeapTraceFuncInfo jsHeapTraceFuncInfoData_;
    JsHeapTraceNode jsHeapTraceNodeData_;
    JsCpuProfilerNode jsCpuProfilerNodeData_;
    JsCpuProfilerSample jsCpuProfilerSampleData_;
    JsConfig jsConfigData_;
    AppStartup appStartupData_;
    SoStaticInitalization staticInitalizationData_;
    Animation animation_;
    DeviceInfo deviceInfo_;
    DynamicFrame dynamicFrame_;
    AshMemData ashMemData_;
    DmaMemData dmaMemData_;
    GpuProcessMemData gpuProcessMemData_;
    GpuWindowMemData gpuWindowMemData_;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_DATA_CACHE_BASE_H
