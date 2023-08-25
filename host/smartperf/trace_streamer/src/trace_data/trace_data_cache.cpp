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

#include "trace_data_cache.h"
#include "animation_table.h"
#include "app_startup_table.h"
#include "appname_table.h"
#include "args_table.h"
#include "bio_latency_sample_table.h"
#include "callstack_table.h"
#include "clk_event_filter_table.h"
#include "clock_event_filter_table.h"
#include "clock_snapshot_table.h"
#include "cpu_measure_filter_table.h"
#include "cpu_usage_info_table.h"
#include "data_dict_table.h"
#include "data_type_table.h"
#include "datasource_clockid_table.h"
#include "device_info_table.h"
#include "device_state_table.h"
#include "disk_io_table.h"
#include "dynamic_frame_table.h"
#include "ebpf_callstack_table.h"
#if WITH_EBPF_HELP
#include "ebpf_elf_symbol_table.h"
#include "ebpf_elf_table.h"
#include "ebpf_process_maps_table.h"
#endif
#include "file_system_sample_table.h"
#include "filter_table.h"
#include "frame_maps_table.h"
#include "frame_slice_table.h"
#include "gpu_slice_table.h"
#include "js_config_table.h"
#include "js_cpu_profiler_node_table.h"
#include "js_cpu_profiler_sample_table.h"
#include "js_heap_edges_table.h"
#include "js_heap_files_table.h"
#include "js_heap_info_table.h"
#include "js_heap_location_table.h"
#include "js_heap_nodes_table.h"
#include "js_heap_sample_table.h"
#include "js_heap_string_table.h"
#include "js_heap_trace_function_info_table.h"
#include "js_heap_trace_node_table.h"
#include "hidump_table.h"
#include "instants_table.h"
#include "irq_table.h"
#include "live_process_table.h"
#include "log_table.h"
#include "measure_filter_table.h"
#include "measure_table.h"
#include "meta_table.h"
#include "native_hook_frame_table.h"
#include "native_hook_statistic_table.h"
#include "native_hook_table.h"
#include "native_hook_statistic_table.h"
#include "network_table.h"
#include "paged_memory_sample_table.h"
#include "perf_call_chain_table.h"
#include "perf_files_table.h"
#include "perf_report_table.h"
#include "perf_sample_table.h"
#include "perf_thread_table.h"
#include "process_filter_table.h"
#include "process_measure_filter_table.h"
#include "process_table.h"
#include "range_table.h"
#include "raw_table.h"
#include "sched_slice_table.h"
#include "smaps_table.h"
#include "span_join.h"
#include "sqlite3.h"
#include "stat_table.h"
#include "so_static_initalization_table.h"
#include "symbols_table.h"
#include "sysevent_measure_table.h"
#include "system_call_table.h"
#include "system_event_filter_table.h"
#include "table_base.h"
#include "task_pool_table.h"
#include "thread_filter_table.h"
#include "thread_state_table.h"
#include "thread_table.h"
#include "trace_config_table.h"
#include "memory_ashmem_table.h"
#include "memory_dma_table.h"
#include "memory_process_gpu_table.h"
#include "memory_window_gpu_table.h"

namespace SysTuning {
namespace TraceStreamer {
TraceDataCache::TraceDataCache()
{
    InitDB();
}

TraceDataCache::~TraceDataCache() {}

void TraceDataCache::InitDB()
{
    if (dbInited_) {
        return;
    }
#ifdef USE_VTABLE
    TableBase::TableDeclare<AnimationTable>(*db_, this, "animation");
    TableBase::TableDeclare<DynamicFrameTable>(*db_, this, "dynamic_frame");
    TableBase::TableDeclare<ProcessTable>(*db_, this, "process");
    TableBase::TableDeclare<SchedSliceTable>(*db_, this, "sched_slice");
    TableBase::TableDeclare<CallStackTable>(*db_, this, "callstack");
    TableBase::TableDeclare<ThreadTable>(*db_, this, "thread");
    TableBase::TableDeclare<ThreadStateTable>(*db_, this, "thread_state");
    TableBase::TableDeclare<ThreadFilterTable>(*db_, this, "thread_filter");
    TableBase::TableDeclare<ProcessFilterTable>(*db_, this, "process_filter");
    TableBase::TableDeclare<MeasureFilterTable>(*db_, this, "measure_filter");
    TableBase::TableDeclare<IrqTable>(*db_, this, "irq");
    TableBase::TableDeclare<DataDictTable>(*db_, this, "data_dict");
    TableBase::TableDeclare<RawTable>(*db_, this, "raw");
    TableBase::TableDeclare<SymbolsTable>(*db_, this, "symbols");
    TableBase::TableDeclare<DataTypeTable>(*db_, this, "data_type");
    TableBase::TableDeclare<HidumpTable>(*db_, this, "hidump");
    TableBase::TableDeclare<NativeHookTable>(*db_, this, "native_hook");
    TableBase::TableDeclare<NativeHookFrameTable>(*db_, this, "native_hook_frame");
    TableBase::TableDeclare<NativeHookStatisticTable>(*db_, this, "native_hook_statistic");
    TableBase::TableDeclare<SpanJoin>(*db_, this, "span_join");

    // no id
    TableBase::TableDeclare<DeviceInfoTable>(*db_, this, "device_info");
    TableBase::TableDeclare<InstantsTable>(*db_, this, "instant");
    TableBase::TableDeclare<MeasureTable>(*db_, this, "measure");
    TableBase::TableDeclare<MeasureTable>(*db_, this, "sys_mem_measure");
    TableBase::TableDeclare<MeasureTable>(*db_, this, "process_measure");
    TableBase::TableDeclare<RangeTable>(*db_, this, "trace_range");
    TableBase::TableDeclare<StatTable>(*db_, this, "stat");
    TableBase::TableDeclare<SystemCallTable>(*db_, this, "syscall");
    TableBase::TableDeclare<MetaTable>(*db_, this, "meta");
    TableBase::TableDeclare<LogTable>(*db_, this, "log");
    TableBase::TableDeclare<NetworkTable>(*db_, this, "network");

    // id is not real id
    TableBase::TableDeclare<CpuMeasureFilterTable>(*db_, this, "cpu_measure_filter");
    TableBase::TableDeclare<FilterTable>(*db_, this, "measure_filter");
    TableBase::TableDeclare<ProcessMeasureFilterTable>(*db_, this, "process_measure_filter");
    TableBase::TableDeclare<ClockEventFilterTable>(*db_, this, "clock_event_filter");
    TableBase::TableDeclare<ClkEventFilterTable>(*db_, this, "clk_event_filter");
    TableBase::TableDeclare<TaskPoolTable>(*db_, this, "task_pool");
    TableBase::TableDeclare<JsHeapFilesTable>(*db_, this, "js_heap_files");
    TableBase::TableDeclare<JsHeapEdgesTable>(*db_, this, "js_heap_edges");
    TableBase::TableDeclare<JsHeapInfoTable>(*db_, this, "js_heap_info");
    TableBase::TableDeclare<JsHeapLocationTable>(*db_, this, "js_heap_location");
    TableBase::TableDeclare<JsHeapNodesTable>(*db_, this, "js_heap_nodes");
    TableBase::TableDeclare<JsHeapSampleTable>(*db_, this, "js_heap_sample");
    TableBase::TableDeclare<JsHeapStringTable>(*db_, this, "js_heap_string");
    TableBase::TableDeclare<JsHeapTraceFunctionInfoTable>(*db_, this, "js_heap_trace_function_info");
    TableBase::TableDeclare<JsHeapTraceNodeTable>(*db_, this, "js_heap_trace_node");
    TableBase::TableDeclare<JsCpuProfilerNodeTable>(*db_, this, "js_cpu_profiler_node");
    TableBase::TableDeclare<JsCpuProfilerSampleTable>(*db_, this, "js_cpu_profiler_sample");
    TableBase::TableDeclare<JsConfigTable>(*db_, this, "js_config");
    TableBase::TableDeclare<ArgsTable>(*db_, this, "args");

    TableBase::TableDeclare<SystemEventFilterTable>(*db_, this, "sys_event_filter");
    TableBase::TableDeclare<DiskIOTable>(*db_, this, "diskio");
    TableBase::TableDeclare<CpuUsageInfoTable>(*db_, this, "cpu_usage");
    TableBase::TableDeclare<LiveProcessTable>(*db_, this, "live_process");
    TableBase::TableDeclare<FileSystemSampleTable>(*db_, this, "file_system_sample");
    TableBase::TableDeclare<EbpfCallStackTable>(*db_, this, "ebpf_callstack");
    TableBase::TableDeclare<PagedMemorySampleTable>(*db_, this, "paged_memory_sample");
#if WITH_EBPF_HELP
    TableBase::TableDeclare<EbpfProcessMapsTable>(*db_, this, "ebpf_process_maps");
    TableBase::TableDeclare<EbpfElfTable>(*db_, this, "ebpf_elf");
    TableBase::TableDeclare<EbpfElfSymbolTable>(*db_, this, "ebpf_elf_symbol");
#endif
    TableBase::TableDeclare<AppnameTable>(*db_, this, "app_name");
    TableBase::TableDeclare<SysEventMeasureTable>(*db_, this, "hisys_event_measure");
    TableBase::TableDeclare<TraceConfigTable>(*db_, this, "trace_config");
    TableBase::TableDeclare<DeviceStateTable>(*db_, this, "device_state");
    TableBase::TableDeclare<SmapsTable>(*db_, this, "smaps");
    TableBase::TableDeclare<BioLatencySampleTable>(*db_, this, "bio_latency_sample");
    TableBase::TableDeclare<DataSourceClockIdTableTable>(*db_, this, "datasource_clockid");
    TableBase::TableDeclare<ClockSnapShotTable>(*db_, this, "clock_snapshot");
    TableBase::TableDeclare<FrameSliceTable>(*db_, this, "frame_slice");
    TableBase::TableDeclare<FrameMapsTable>(*db_, this, "frame_maps");
    TableBase::TableDeclare<GPUSliceTable>(*db_, this, "gpu_slice");
    TableBase::TableDeclare<AppStartupTable>(*db_, this, "app_startup");
    TableBase::TableDeclare<SoStaticInitalizationTable>(*db_, this, "static_initalize");
    TableBase::TableDeclare<MemoryAshMemTable>(*db_, this, "memory_ashmem");
    TableBase::TableDeclare<MemoryDmaTable>(*db_, this, "memory_dma");
    TableBase::TableDeclare<MemoryProcessGpuTable>(*db_, this, "memory_process_gpu");
    TableBase::TableDeclare<MemoryWindowGpuTable>(*db_, this, "memory_window_gpu");

    TableBase::TableDeclare<PerfReportTable>(*db_, this, "perf_report");
    TableBase::TableDeclare<PerfSampleTable>(*db_, this, "perf_sample");
    TableBase::TableDeclare<PerfCallChainTable>(*db_, this, "perf_callchain");
    TableBase::TableDeclare<PerfThreadTable>(*db_, this, "perf_thread");
    TableBase::TableDeclare<PerfFilesTable>(*db_, this, "perf_files");
#else
    TableBase::TableDeclare<AnimationTable>(*db_, this, "_animation");
    TableBase::TableDeclare<DynamicFrameTable>(*db_, this, "_dynamic_frame");
    TableBase::TableDeclare<ProcessTable>(*db_, this, "_process");
    TableBase::TableDeclare<SchedSliceTable>(*db_, this, "_sched_slice");
    TableBase::TableDeclare<CallStackTable>(*db_, this, "_callstack");
    TableBase::TableDeclare<ThreadTable>(*db_, this, "_thread");
    TableBase::TableDeclare<ThreadStateTable>(*db_, this, "_thread_state");
    TableBase::TableDeclare<ThreadFilterTable>(*db_, this, "_thread_filter");
    TableBase::TableDeclare<ProcessFilterTable>(*db_, this, "_process_filter");
    TableBase::TableDeclare<MeasureFilterTable>(*db_, this, "_measure_filter");
    TableBase::TableDeclare<IrqTable>(*db_, this, "_irq");
    TableBase::TableDeclare<DataDictTable>(*db_, this, "_data_dict");
    TableBase::TableDeclare<RawTable>(*db_, this, "_raw");
    TableBase::TableDeclare<SymbolsTable>(*db_, this, "_symbols");
    TableBase::TableDeclare<DataTypeTable>(*db_, this, "_data_type");
    TableBase::TableDeclare<HidumpTable>(*db_, this, "_hidump");
    TableBase::TableDeclare<NativeHookTable>(*db_, this, "_native_hook");
    TableBase::TableDeclare<NativeHookFrameTable>(*db_, this, "_native_hook_frame");
    TableBase::TableDeclare<NativeHookStatisticTable>(*db_, this, "_native_hook_statistic");
    TableBase::TableDeclare<SpanJoin>(*db_, this, "_span_join");

    // no id
    TableBase::TableDeclare<DeviceInfoTable>(*db_, this, "_device_info");
    TableBase::TableDeclare<InstantsTable>(*db_, this, "_instant");
    TableBase::TableDeclare<MeasureTable>(*db_, this, "_measure");
    TableBase::TableDeclare<MeasureTable>(*db_, this, "_sys_mem_measure");
    TableBase::TableDeclare<MeasureTable>(*db_, this, "_process_measure");
    TableBase::TableDeclare<RangeTable>(*db_, this, "_trace_range");
    TableBase::TableDeclare<StatTable>(*db_, this, "_stat");
    TableBase::TableDeclare<SystemCallTable>(*db_, this, "_syscall");
    TableBase::TableDeclare<MetaTable>(*db_, this, "_meta");
    TableBase::TableDeclare<LogTable>(*db_, this, "_log");
    TableBase::TableDeclare<NetworkTable>(*db_, this, "_network");

    // id is not real id
    TableBase::TableDeclare<CpuMeasureFilterTable>(*db_, this, "_cpu_measure_filter");
    TableBase::TableDeclare<FilterTable>(*db_, this, "_measure_filter");
    TableBase::TableDeclare<ProcessMeasureFilterTable>(*db_, this, "_process_measure_filter");
    TableBase::TableDeclare<ClockEventFilterTable>(*db_, this, "_clock_event_filter");
    TableBase::TableDeclare<ClkEventFilterTable>(*db_, this, "_clk_event_filter");
    TableBase::TableDeclare<TaskPoolTable>(*db_, this, "_task_pool");
    TableBase::TableDeclare<JsHeapFilesTable>(*db_, this, "_js_heap_files");
    TableBase::TableDeclare<JsHeapEdgesTable>(*db_, this, "_js_heap_edges");
    TableBase::TableDeclare<JsHeapInfoTable>(*db_, this, "_js_heap_info");
    TableBase::TableDeclare<JsHeapLocationTable>(*db_, this, "_js_heap_location");
    TableBase::TableDeclare<JsHeapNodesTable>(*db_, this, "_js_heap_nodes");
    TableBase::TableDeclare<JsHeapSampleTable>(*db_, this, "_js_heap_sample");
    TableBase::TableDeclare<JsHeapStringTable>(*db_, this, "_js_heap_string");
    TableBase::TableDeclare<JsHeapTraceFunctionInfoTable>(*db_, this, "_js_heap_trace_function_info");
    TableBase::TableDeclare<JsHeapTraceNodeTable>(*db_, this, "_js_heap_trace_node");
    TableBase::TableDeclare<JsCpuProfilerNodeTable>(*db_, this, "_js_cpu_Perfiler_Node");
    TableBase::TableDeclare<JsCpuProfilerSampleTable>(*db_, this, "_js_cpu_Perfiler_Sample");
    TableBase::TableDeclare<JsConfigTable>(*db_, this, "_js_config");
    TableBase::TableDeclare<ArgsTable>(*db_, this, "_args");
    TableBase::TableDeclare<SystemEventFilterTable>(*db_, this, "_sys_event_filter");
    TableBase::TableDeclare<DiskIOTable>(*db_, this, "_diskio");
    TableBase::TableDeclare<CpuUsageInfoTable>(*db_, this, "_cpu_usage");
    TableBase::TableDeclare<LiveProcessTable>(*db_, this, "_live_process");
    TableBase::TableDeclare<FileSystemSampleTable>(*db_, this, "_file_system_sample");
    TableBase::TableDeclare<EbpfCallStackTable>(*db_, this, "_ebpf_callstack");
    TableBase::TableDeclare<PagedMemorySampleTable>(*db_, this, "_paged_memory_sample");
    TableBase::TableDeclare<SmapsTable>(*db_, this, "_smaps");
    TableBase::TableDeclare<BioLatencySampleTable>(*db_, this, "_bio_latency_sample");
    TableBase::TableDeclare<DataSourceClockIdTableTable>(*db_, this, "_datasource_clockid");
    TableBase::TableDeclare<ClockSnapShotTable>(*db_, this, "_clock_snapshot");
    TableBase::TableDeclare<FrameSliceTable>(*db_, this, "_frame_slice");
    TableBase::TableDeclare<FrameMapsTable>(*db_, this, "_frame_maps");
    TableBase::TableDeclare<GPUSliceTable>(*db_, this, "_gpu_slice");
    TableBase::TableDeclare<AppStartupTable>(*db_, this, "_app_startup");
    TableBase::TableDeclare<SoStaticInitalizationTable>(*db_, this, "_static_initalize");
    TableBase::TableDeclare<MemoryAshMemTable>(*db_, this, "_memory_ashmem");
    TableBase::TableDeclare<MemoryDmaTable>(*db_, this, "_memory_dma");
    TableBase::TableDeclare<MemoryProcessGpuTable>(*db_, this, "_memory_process_gpu");
    TableBase::TableDeclare<MemoryWindowGpuTable>(*db_, this, "_memory_window_gpu");
#if WITH_EBPF_HELP
    TableBase::TableDeclare<EbpfProcessMapsTable>(*db_, this, "_ebpf_process_maps");
    TableBase::TableDeclare<EbpfElfTable>(*db_, this, "_ebpf_elf");
    TableBase::TableDeclare<EbpfElfSymbolTable>(*db_, this, "_ebpf_elf_symbol");
#endif
    TableBase::TableDeclare<AppnameTable>(*db_, this, "_app_name");
    TableBase::TableDeclare<SysEventMeasureTable>(*db_, this, "_hisys_event_measure");
    TableBase::TableDeclare<DeviceStateTable>(*db_, this, "_device_state");
    TableBase::TableDeclare<TraceConfigTable>(*db_, this, "_trace_config");
    TableBase::TableDeclare<PerfReportTable>(*db_, this, "_perf_report");
    TableBase::TableDeclare<PerfSampleTable>(*db_, this, "_perf_sample");
    TableBase::TableDeclare<PerfCallChainTable>(*db_, this, "_perf_callchain");
    TableBase::TableDeclare<PerfThreadTable>(*db_, this, "_perf_thread");
    TableBase::TableDeclare<PerfFilesTable>(*db_, this, "_perf_files");
#endif
    dbInited_ = true;
}
bool TraceDataCache::AnimationTraceEnabled()
{
    return animationTraceEnabled_;
}
void TraceDataCache::UpdateAnimationTraceStatus(bool status)
{
    animationTraceEnabled_ = status;
}
bool TraceDataCache::TaskPoolTraceEnabled()
{
    return taskPoolTraceEnabled_;
}
void TraceDataCache::UpdateTaskPoolTraceStatus(bool status)
{
    taskPoolTraceEnabled_ = status;
}
bool TraceDataCache::AppStartTraceEnabled()
{
    return appStartTraceEnabled_;
}
void TraceDataCache::UpdateAppStartTraceStatus(bool status)
{
    appStartTraceEnabled_ = status;
}
bool TraceDataCache::BinderRunnableTraceEnabled()
{
    return binderRunnableTraceEnabled_;
}
void TraceDataCache::UpdateBinderRunnableTraceStatus(bool status)
{
    binderRunnableTraceEnabled_ = status;
}
} // namespace TraceStreamer
} // namespace SysTuning
