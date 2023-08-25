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

#ifndef TRACE_STREAMER_CONFIG_H
#define TRACE_STREAMER_CONFIG_H
#include <map>
#include <string>
#include "ts_common.h"
namespace SysTuning {
namespace TraceCfg {
// all supported events should be defined here
#define CPU_IDEL_INVALID_KEY 4294967295
#define CPU_IDEL_INVALID_VALUE 4
enum SupportedTraceEventType {
    TRACE_EVENT_START = 0,
    TRACE_EVENT_BINDER_TRANSACTION = TRACE_EVENT_START,
    TRACE_EVENT_BINDER_TRANSACTION_RECEIVED,
    TRACE_EVENT_BINDER_TRANSACTION_ALLOC_BUF,
    TRACE_EVENT_BINDER_TRANSACTION_LOCK,
    TRACE_EVENT_BINDER_TRANSACTION_LOCKED,
    TRACE_EVENT_BINDER_TRANSACTION_UNLOCK,
    TRACE_EVENT_SCHED_SWITCH,
    TRACE_EVENT_SCHED_BLOCKED_REASON,
    TRACE_EVENT_TASK_RENAME,
    TRACE_EVENT_TASK_NEWTASK,
    TRACE_EVENT_TRACING_MARK_WRITE,
    TRACE_EVENT_PRINT,
    TRACE_EVENT_SCHED_WAKEUP,
    TRACE_EVENT_SCHED_WAKING,
    TRACE_EVENT_CPU_IDLE,
    TRACE_EVENT_CPU_FREQUENCY,
    TRACE_EVENT_CPU_FREQUENCY_LIMITS,
    TRACE_EVENT_SUSPEND_RESUME,
    TRACE_EVENT_WORKQUEUE_EXECUTE_START,
    TRACE_EVENT_WORKQUEUE_EXECUTE_END,
    TRACE_EVENT_CLOCK_SET_RATE,
    TRACE_EVENT_CLOCK_ENABLE,
    TRACE_EVENT_CLOCK_DISABLE,
    TRACE_EVENT_CLK_SET_RATE,
    TRACE_EVENT_CLK_ENABLE,
    TRACE_EVENT_CLK_DISABLE,
    TRACE_EVENT_SYS_ENTRY,
    TRACE_EVENT_SYS_EXIT,
    TRACE_EVENT_REGULATOR_SET_VOLTAGE,
    TRACE_EVENT_REGULATOR_SET_VOLTAGE_COMPLETE,
    TRACE_EVENT_REGULATOR_DISABLE,
    TRACE_EVENT_REGULATOR_DISABLE_COMPLETE,
    TRACE_EVENT_IPI_ENTRY,
    TRACE_EVENT_IPI_EXIT,
    TRACE_EVENT_IRQ_HANDLER_ENTRY,
    TRACE_EVENT_IRQ_HANDLER_EXIT,
    TRACE_EVENT_SOFTIRQ_RAISE,
    TRACE_EVENT_SOFTIRQ_ENTRY,
    TRACE_EVENT_SOFTIRQ_EXIT,
    TRACE_EVENT_OOM_SCORE_ADJ_UPDATE,
    TRACE_EVENT_SCHED_WAKEUP_NEW,
    TRACE_EVENT_PROCESS_EXIT,
    TRACE_EVENT_PROCESS_FREE,
    TRACE_EVENT_CLOCK_SYNC,
    TRACE_EVENT_SIGNAL_GENERATE,
    TRACE_EVENT_SIGNAL_DELIVER,
    TRACE_EVENT_BLOCK_BIO_BACKMERGE,
    TRACE_EVENT_BLOCK_BIO_BOUNCE,
    TRACE_EVENT_BLOCK_BIO_COMPLETE,
    TRACE_EVENT_BLOCK_BIO_FRONTMERGE,
    TRACE_EVENT_BLOCK_BIO_QUEUE,
    TRACE_EVENT_BLOCK_BIO_REMAP,
    TRACE_EVENT_BLOCK_DIRTY_BUFFER,
    TRACE_EVENT_BLOCK_GETRQ,
    TRACE_EVENT_BLOCK_PLUG,
    TRACE_EVENT_BLOCK_RQ_COMPLETE,
    TRACE_EVENT_BLOCK_RQ_INSERT,
    TRACE_EVENT_BLOCK_RQ_REMAP,
    TRACE_EVENT_BLOCK_RQ_ISSUE,
    TRACE_MEMORY,
    TRACE_HILOG,
    TRACE_HIDUMP_FPS,
    TRACE_NATIVE_HOOK_MALLOC,
    TRACE_NATIVE_HOOK_FREE,
    TRACE_NATIVE_HOOK_MMAP,
    TRACE_NATIVE_HOOK_MUNMAP,
    TRACE_NATIVE_HOOK_RECORD_STATISTICS,
    TRACE_NATIVE_HOOK_MEMTAG,
    TRACE_SYS_MEMORY,
    TRACE_SYS_VIRTUAL_MEMORY,
    TRACE_DISKIO,
    TRACE_PROCESS,
    TRACE_CPU_USAGE,
    TRACE_NETWORK,
    TRACE_JS_MEMORY,
    TRACE_PERF,
    TRACE_EVENT_EBPF,
    TRACE_EVENT_EBPF_FILE_SYSTEM,
    TRACE_EVENT_EBPF_PAGED_MEMORY,
    TRACE_EVENT_EBPF_BIO_LATENCY,
    TRACE_HISYSEVENT,
    TRACE_SMAPS,
    TRACE_VSYNC,
    TRACE_ONVSYNC,
    TRACE_FRAMEQUEUE,
    TRACE_ASHMEM,
    TRACE_DMAMEM,
    TRACE_GPU_PROCESS_MEM,
    TRACE_GPU_WINDOW_MEM,
    TRACE_EVENT_OTHER,
    TRACE_EVENT_MAX
};
enum MemInfoType {
    MEM_VM_SIZE,
    MEM_VM_RSS,
    MEM_VM_ANON,
    MEM_RSS_FILE,
    MEM_RSS_SHMEM,
    MEM_VM_SWAP,
    MEM_VM_LOCKED,
    MEM_VM_HWM,
    MEM_OOM_SCORE_ADJ,
    MEM_PURG_SUM,
    MEM_PURG_PIN,
    MEM_GL_PSS,
    MEM_MAX
};
enum StatType {
    STAT_EVENT_START = 0,
    STAT_EVENT_RECEIVED = STAT_EVENT_START,
    STAT_EVENT_DATA_LOST,
    STAT_EVENT_NOTMATCH,
    STAT_EVENT_NOTSUPPORTED,
    STAT_EVENT_DATA_INVALID,
    STAT_EVENT_MAX
};

// there maybe some error while parser trace msgs, here defined the error levels
enum StatSeverityLevel {
    STAT_SEVERITY_LEVEL_START = 0,
    STAT_SEVERITY_LEVEL_INFO = STAT_SEVERITY_LEVEL_START,
    STAT_SEVERITY_LEVEL_WARN,
    STAT_SEVERITY_LEVEL_ERROR,
    STAT_SEVERITY_LEVEL_FATAL,
    STAT_SEVERITY_LEVEL_MAX
};

// the supported metadata
enum MetaDataItem {
    METADATA_ITEM_START = 0,
    METADATA_ITEM_DATASIZE = METADATA_ITEM_START,
    METADATA_ITEM_PARSETOOL_NAME,
    METADATA_ITEM_PARSERTOOL_VERSION,
    METADATA_ITEM_PARSERTOOL_PUBLISH_DATETIME,
    METADATA_ITEM_SOURCE_FILENAME,
    METADATA_ITEM_OUTPUT_FILENAME,
    METADATA_ITEM_PARSERTIME, // the data time while the data parsed
    METADATA_ITEM_TRACE_DURATION,
    METADATA_ITEM_SOURCE_DATETYPE, // proto-based-trace or txt-based-trace
    METADATA_ITEM_MAX
};

class TraceStreamerConfig {
public:
    TraceStreamerConfig();
    ~TraceStreamerConfig() = default;
    void PrintInfo() const;
    static uint32_t GetStateValue(uint32_t state)
    {
        return (state > CPU_IDEL_INVALID_VALUE ? 0 : (state + 1));
    }

public:
    std::map<SupportedTraceEventType, std::string> eventNameMap_ = {};
    std::map<StatType, std::string> eventErrorDescMap_ = {};
    std::map<StatSeverityLevel, std::string> serverityLevelDescMap_ = {};
    // different msg may have STAT_EVENT_MAX types of exception when parse, and they have different error level
    // if you think some error level should be improve or depress, you can edit this map
    std::map<SupportedTraceEventType, std::map<StatType, StatSeverityLevel>> eventParserStatSeverityDescMap_ = {};
    // process mem info desc
    std::map<MemInfoType, std::string> memNameMap_ = {};
    // sys memorty info desc
    std::map<uint32_t, std::string> sysMemNameMap_ = {};
    // sys virtual memorty info desc
    std::map<uint32_t, std::string> sysVirtualMemNameMap_ = {};

private:
    void InitEventNameMap();
    void InitSysMemMap();
    void InitSysVmemMap();
    void InitSecurityMap();
    // all supported events should be defined here, these str can be find in text-based trace
    const std::string TRACE_ACTION_BINDER_TRANSACTION = "binder_transaction";
    const std::string TRACE_ACTION_BINDER_TRANSACTION_RECEIVED = "binder_transaction_received";
    const std::string TRACE_ACTION_SCHED_SWITCH = "sched_switch";
    const std::string TRACE_ACTION_SCHED_BLOCKED_REASON = "sched_blocked_reason";
    const std::string TRACE_ACTION_TASK_RENAME = "task_rename";
    const std::string TRACE_ACTION_TASK_NEWTASK = "task_newtask";
    const std::string TRACE_ACTION_TRACING_MARK_WRITE = "tracing_mark_write";
    const std::string TRACE_ACTION_PRINT = "print";
    const std::string TRACE_ACTION_SCHED_WAKEUP = "sched_wakeup";
    const std::string TRACE_ACTION_SCHED_WAKING = "sched_waking";
    const std::string TRACE_ACTION_CPU_IDLE = "cpu_idle";
    const std::string TRACE_ACTION_CPU_FREQUENCY = "cpu_frequency";
    const std::string TRACE_ACTION_CPU_FREQUENCY_LIMITS = "cpu_frequency_limits";
    const std::string TRACE_ACTION_SUSPEND_RESUME = "suspend_resume";
    const std::string TRACE_ACTION_WORKQUEUE_EXECUTE_START = "workqueue_execute_start";
    const std::string TRACE_ACTION_WORKQUEUE_EXECUTE_END = "workqueue_execute_end";

    const std::string TRACE_ACTION_CLOCK_SET_RATE = "clock_set_rate";
    const std::string TRACE_ACTION_CLOCK_ENABLE = "clock_enable";
    const std::string TRACE_ACTION_CLOCK_DISABLE = "clock_disable";
    const std::string TRACE_ACTION_CLK_SET_RATE = "clk_set_rate";
    const std::string TRACE_ACTION_CLK_ENABLE = "clk_enable";
    const std::string TRACE_ACTION_CLK_DISABLE = "clk_disable";
    const std::string TRACE_ACTION_SYS_ENTRY = "sys_enter";
    const std::string TRACE_ACTION_SYS_EXIT = "sys_exit";
    const std::string TRACE_ACTION_OOM_SCORE_ADJ_UPDATE = "oom_score_adj_update";
    const std::string TRACE_ACTION_REGULATOR_SET_VOLTAGE = "regulator_set_voltage";
    const std::string TRACE_ACTION_REGULATOR_SET_VOLTAGE_COMPLETE = "regulator_set_voltage_complete";
    const std::string TRACE_ACTION_REGULATOR_DISABLE = "regulator_disable";
    const std::string TRACE_ACTION_REGULATOR_DISABLE_COMPLETE = "regulator_disable_complete";
    const std::string TRACE_ACTION_IPI_ENTRY = "ipi_entry";
    const std::string TRACE_ACTION_IPI_EXIT = "ipi_exit";
    const std::string TRACE_ACTION_IRQ_HANDLER_ENTRY = "irq_handler_entry";
    const std::string TRACE_ACTION_IRQ_HANDLER_EXIT = "irq_handler_exit";
    const std::string TRACE_ACTION_SOFTIRQ_RAISE = "softirq_raise";
    const std::string TRACE_ACTION_SOFTIRQ_ENTRY = "softirq_entry";
    const std::string TRACE_ACTION_SOFTIRQ_EXIT = "softirq_exit";
    const std::string TRACE_ACTION_BINDER_TRANSACTION_ALLOC_BUF = "binder_transaction_alloc_buf";
    const std::string TRACE_ACTION_BINDER_TRANSACTION_LOCK = "binder_transaction_lock";
    const std::string TRACE_ACTION_BINDER_TRANSACTION_LOCKED = "binder_transaction_locked";
    const std::string TRACE_ACTION_BINDER_TRANSACTION_UNLOCK = "binder_transaction_unlock";
    const std::string TRACE_ACTION_SCHED_WAKEUP_NEW = "sched_wakeup_new";
    const std::string TRACE_ACTION_PROCESS_EXIT = "sched_process_exit";
    const std::string TRACE_ACTION_PROCESS_FREE = "sched_process_free";
    const std::string TRACE_ACTION_CLOCK_SYNC = "trace_event_clock_sync";
    const std::string TRACE_ACTION_MEMORY = "memory";
    const std::string TRACE_ACTION_HILOG = "hilog";
    const std::string TRACE_ACTION_HIDUMP_FPS = "hidump_fps";
    const std::string TRACE_ACTION_NATIVE_HOOK_MALLOC = "native_hook_malloc";
    const std::string TRACE_ACTION_NATIVE_HOOK_FREE = "native_hook_free";
    const std::string TRACE_ACTION_NATIVE_HOOK_MMAP = "native_hook_mmap";
    const std::string TRACE_ACTION_NATIVE_HOOK_MUNMAP = "native_hook_munmap";
    const std::string TRACE_ACTION_NATIVE_HOOK_RECORD_STATISTICS = "native_hook_record_statistics";
    const std::string TRACE_ACTION_NATIVE_HOOK_MEMTAG = "native_hook_memtag";
    const std::string TRACE_ACTION_SIGNAL_GENERATE = "signal_generate";
    const std::string TRACE_ACTION_SIGNAL_DELIVER = "signal_deliver";
    const std::string TRACE_ACTION_BLOCK_BIO_BACKMERGE = "trace_block_bio_backmerge";
    const std::string TRACE_ACTION_BLOCK_BIO_BOUNCE = "trace_block_bio_bounce";
    const std::string TRACE_ACTION_BLOCK_BIO_COMPLETE = "trace_block_bio_complete";
    const std::string TRACE_ACTION_BLOCK_BIO_FRONTMERGE = "trace_block_bio_frontmerge";
    const std::string TRACE_ACTION_BLOCK_BIO_QUEUE = "trace_bblock_bio_queue";
    const std::string TRACE_ACTION_BLOCK_BIO_REMAP = "trace_block_bio_remap";
    const std::string TRACE_ACTION_BLOCK_DIRTY_BUFFER = "trace_block_dirty_buffer";
    const std::string TRACE_ACTION_BLOCK_GETRQ = "trace_block_getrq";
    const std::string TRACE_ACTION_BLOCK_PLUG = "trace_block_plug";
    const std::string TRACE_ACTION_BLOCK_RQ_COMPLETE = "trace_block_rq_complete";
    const std::string TRACE_ACTION_BLOCK_RQ_INSERT = "trace_block_rq_insert";
    const std::string TRACE_ACTION_BLOCK_RQ_REMAP = "trace_block_rq_remap";
    const std::string TRACE_ACTION_BLOCK_RQ_ISSUE = "trace_block_rq_issue";

    const std::string TRACE_ACTION_SYS_MEMORY = "sys_memory";
    const std::string TRACE_ACTION_SYS_VIRTUAL_MEMORY = "sys_virtual_memory";
    const std::string TRACE_ACTION_DISKIO = "trace_diskio";
    const std::string TRACE_ACTION_PROCESS = "trace_process";
    const std::string TRACE_ACTION_CPU_USAGE = "trace_cpu_usage";
    const std::string TRACE_ACTION_NETWORK = "trace_network";
    const std::string TRACE_ACTION_JS_MEMORY = "trace_js_memory";
    const std::string TRACE_ACTION_PERF = "trace_perf";
    const std::string TRACE_ACTION_EBPF = "trace_ebpf";
    const std::string TRACE_ACTION_EBPF_FILE_SYSTEM = "trace_ebpf_file_system";
    const std::string TRACE_ACTION_EBPF_PAGED_MEMORY = "trace_ebpf_paged_memory";
    const std::string TRACE_ACTION_EBPF_BIO_LATENCY = "trace_ebpf_bio_latency";
    const std::string TRACE_ACTION_HISYS_EVENT = "trace_hisys_event";
    const std::string TRACE_ACTION_SMAPS = "trace_smaps";
    const std::string TRACE_ACTION_VSYNC = "trace_vsync";
    const std::string TRACE_ACTION_ONVSYNC = "trace_on_vsync";
    const std::string TRACE_ACTION_FRAMEQUEUE = "trace_frame_queue";
    const std::string TRACE_ACTION_ASHMEM = "trace_ash_mem";
    const std::string TRACE_ACTION_DMAMEM = "trace_dma_mem";
    const std::string TRACE_ACTION_GPU_PROCESS_MEM = "trace_gpu_process_mem";
    const std::string TRACE_ACTION_GPU_WINDOW_MEM = "trace_gpu_window_mem";
    const std::string TRACE_ACTION_OTHER = "other";

    const std::string MEM_INFO_VM_SIZE_DESC = "mem.vm.size";
    const std::string MEM_INFO_LOCKED_DESC = "mem.locked";
    const std::string MEM_INFO_RSS_DESC = "mem.rss";
    const std::string MEM_INFO_RSS_ANON_DESC = "mem.rss.anon";
    const std::string MEM_INFO_RSS_FILE_DESC = "mem.rss.file";
    const std::string MEM_INFO_RSS_SCHEM_DESC = "mem.rss.schem";
    const std::string MEM_INFO_SWAP_DESC = "mem.swap";
    const std::string MEM_INFO_VIRT_DESC = "mem.virt";
    const std::string MEM_INFO_HWM_DESC = "mem.hwm";
    const std::string MEM_INFO_SCORE_ADJ_DESC = "mm.oom_score_adj";
    const std::string MEM_INFO_PURG_SUM_DESC = "mem.purg_sum";
    const std::string MEM_INFO_PURG_PIN_DESC = "mem.purg_pin";
    const std::string MEM_INFO_GL_PSS_DESC = "mem.gl_pss";

    const std::string SYS_MEMINFO_UNSPECIFIED_DESC = "sys.mem.unspecified";
    const std::string SYS_MEMINFO_MEM_TOTAL_DESC = "sys.mem.total";
    const std::string SYS_MEMINFO_MEM_FREE_DESC = "sys.mem.free";
    const std::string SYS_MEMINFO_MEM_AVAILABLE_DESC = "sys.mem.avaiable";
    const std::string SYS_MEMINFO_BUFFERS_DESC = "sys.mem.buffers";
    const std::string SYS_MEMINFO_CACHED_DESC = "sys.mem.cached";
    const std::string SYS_MEMINFO_SWAP_CACHED_DESC = "sys.mem.swap.chard";
    const std::string SYS_MEMINFO_ACTIVE_DESC = "sys.mem.active";
    const std::string SYS_MEMINFO_INACTIVE_DESC = "sys.mem.inactive";
    const std::string SYS_MEMINFO_ACTIVE_ANON_DESC = "sys.mem.active.anon";
    const std::string SYS_MEMINFO_INACTIVE_ANON_DESC = "sys.mem.inactive.anon";
    const std::string SYS_MEMINFO_ACTIVE_FILE_DESC = "sys.mem.active_file";
    const std::string SYS_MEMINFO_INACTIVE_FILE_DESC = "sys.mem.inactive_file";
    const std::string SYS_MEMINFO_UNEVICTABLE_DESC = "sys.mem.unevictable";
    const std::string SYS_MEMINFO_MLOCKED_DESC = "sys.mem.mlocked";
    const std::string SYS_MEMINFO_SWAP_TOTAL_DESC = "sys.mem.swap.total";
    const std::string SYS_MEMINFO_SWAP_FREE_DESC = "sys.mem.swap.free";
    const std::string SYS_MEMINFO_DIRTY_DESC = "sys.mem.dirty";
    const std::string SYS_MEMINFO_WRITEBACK_DESC = "sys.mem.writeback";
    const std::string SYS_MEMINFO_ANON_PAGES_DESC = "sys.mem.anon.pages";
    const std::string SYS_MEMINFO_MAPPED_DESC = "sys.mem.mapped";
    const std::string SYS_MEMINFO_SHMEM_DESC = "sys.mem.shmem";
    const std::string SYS_MEMINFO_SLAB_DESC = "sys.mem.slab";
    const std::string SYS_MEMINFO_SLAB_RECLAIMABLE_DESC = "sys.mem.slab.reclaimable";
    const std::string SYS_MEMINFO_SLAB_UNRECLAIMABLE_DESC = "sys.mem.slab.unreclaimable";
    const std::string SYS_MEMINFO_KERNEL_STACK_DESC = "sys.mem.kernel.stack";
    const std::string SYS_MEMINFO_PAGE_TABLES_DESC = "sys.mem.page.tables";
    const std::string SYS_MEMINFO_COMMIT_LIMIT_DESC = "sys.mem.commit.limit";
    const std::string SYS_MEMINFO_COMMITED_AS_DESC = "sys.mem.commited.as";
    const std::string SYS_MEMINFO_VMALLOC_TOTAL_DESC = "sys.mem.vmalloc.total";
    const std::string SYS_MEMINFO_VMALLOC_USED_DESC = "sys.mem.vmalloc.used";
    const std::string SYS_MEMINFO_VMALLOC_CHUNK_DESC = "sys.mem.vmalloc.chunk";
    const std::string SYS_MEMINFO_CMA_TOTAL_DESC = "sys.mem.cma.total";
    const std::string SYS_MEMINFO_CMA_FREE_DESC = "sys.mem.cma.free";
    const std::string SYS_MEMINFO_KERNEL_RECLAIMABLE_DESC = "sys.mem.kernel.reclaimable";
    const std::string SYS_MEMINFO_ACTIVE_PURG_DESC = "sys.mem.active.purg";
    const std::string SYS_MEMINFO_INACTIVE_PURG_DESC = "sys.mem.inactive.purg";
    const std::string SYS_MEMINFO_PINED_PURG_DESC = "sys.mem.pined.purg";
    const std::string SYS_VMEMINFO_UNSPECIFIED_DESC = "sys.virtual.mem.unspecified";
    const std::string SYS_VMEMINFO_NR_FREE_PAGES_DESC = "sys.virtual.mem.nr.free.pages";
    const std::string SYS_VMEMINFO_NR_ALLOC_BATCH_DESC = "sys.virtual.mem.nr.alloc.batch";
    const std::string SYS_VMEMINFO_NR_INACTIVE_ANON_DESC = "sys.virtual.mem.nr.inactive.anon";
    const std::string SYS_VMEMINFO_NR_ACTIVE_ANON_DESC = "sys.virtual.mem.nr.active_anon";
    const std::string SYS_VMEMINFO_NR_INACTIVE_FILE_DESC = "sys.virtual.mem.nr.inactive.file";
    const std::string SYS_VMEMINFO_NR_ACTIVE_FILE_DESC = "sys.virtual.mem.nr.active_file";
    const std::string SYS_VMEMINFO_NR_UNEVICTABLE_DESC = "sys.virtual.mem.nr.unevictable";
    const std::string SYS_VMEMINFO_NR_MLOCK_DESC = "sys.virtual.mem.nr.mlock";
    const std::string SYS_VMEMINFO_NR_ANON_PAGES_DESC = "sys.virtual.mem.anon.pages";
    const std::string SYS_VMEMINFO_NR_MAPPED_DESC = "sys.virtual.mem.nr.mapped";
    const std::string SYS_VMEMINFO_NR_FILE_PAGES_DESC = "sys.virtual.mem.nr.file.pages";
    const std::string SYS_VMEMINFO_NR_DIRTY_DESC = "sys.virtual.mem.nr.dirty";
    const std::string SYS_VMEMINFO_NR_WRITEBACK_DESC = "sys.virtual.mem.nr.writeback";
    const std::string SYS_VMEMINFO_NR_SLAB_RECLAIMABLE_DESC = "sys.virtual.mem.nr.slab.reclaimable";
    const std::string SYS_VMEMINFO_NR_SLAB_UNRECLAIMABLE_DESC = "sys.virtual.mem.nr.slab.unreclaimable";
    const std::string SYS_VMEMINFO_NR_PAGE_TABLE_PAGES_DESC = "sys.virtual.mem.nr.page_table.pages";
    const std::string SYS_VMEMINFO_NR_KERNEL_STACK_DESC = "sys.virtual.mem.nr_kernel.stack";
    const std::string SYS_VMEMINFO_NR_OVERHEAD_DESC = "sys.virtual.mem.nr.overhead";
    const std::string SYS_VMEMINFO_NR_UNSTABLE_DESC = "sys.virtual.mem.nr.unstable";
    const std::string SYS_VMEMINFO_NR_BOUNCE_DESC = "sys.virtual.mem.nr.bounce";
    const std::string SYS_VMEMINFO_NR_VMSCAN_WRITE_DESC = "sys.virtual.mem.nr.vmscan.write";
    const std::string SYS_VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM_DESC = "sys.virtual.mem.nr.vmscan.immediate.reclaim";
    const std::string SYS_VMEMINFO_NR_WRITEBACK_TEMP_DESC = "sys.virtual.mem.nr.writeback_temp";
    const std::string SYS_VMEMINFO_NR_ISOLATED_ANON_DESC = "sys.virtual.mem.nr.isolated_anon";
    const std::string SYS_VMEMINFO_NR_ISOLATED_FILE_DESC = "sys.virtual.mem.nr.isolated_file";
    const std::string SYS_VMEMINFO_NR_SHMEM_DESC = "sys.virtual.mem.nr.shmem";
    const std::string SYS_VMEMINFO_NR_DIRTIED_DESC = "sys.virtual.mem.nr.dirtied";
    const std::string SYS_VMEMINFO_NR_WRITTEN_DESC = "sys.virtual.mem.nr.written";
    const std::string SYS_VMEMINFO_NR_PAGES_SCANNED_DESC = "sys.virtual.mem.nr.pages.scanned";
    const std::string SYS_VMEMINFO_WORKINGSET_REFAULT_DESC = "sys.virtual.mem.workingset.refault";
    const std::string SYS_VMEMINFO_WORKINGSET_ACTIVATE_DESC = "sys.virtual.mem.workingset.activate";
    const std::string SYS_VMEMINFO_WORKINGSET_NODERECLAIM_DESC = "sys.virtual.mem.workingset_nodereclaim";
    const std::string SYS_VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES_DESC = "sys.virtual.mem.nr_anon.transparent.hugepages";
    const std::string SYS_VMEMINFO_NR_FREE_CMA_DESC = "sys.virtual.mem.nr.free_cma";
    const std::string SYS_VMEMINFO_NR_SWAPCACHE_DESC = "sys.virtual.mem.nr.swapcache";
    const std::string SYS_VMEMINFO_NR_DIRTY_THRESHOLD_DESC = "sys.virtual.mem.nr.dirty.threshold";
    const std::string SYS_VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD_DESC = "sys.virtual.mem.nr.dirty.background.threshold";
    const std::string SYS_VMEMINFO_PGPGIN_DESC = "sys.virtual.mem.vmeminfo.pgpgin";
    const std::string SYS_VMEMINFO_PGPGOUT_DESC = "sys.virtual.mem.pgpgout";
    const std::string SYS_VMEMINFO_PGPGOUTCLEAN_DESC = "sys.virtual.mem.pgpgoutclean";
    const std::string SYS_VMEMINFO_PSWPIN_DESC = "sys.virtual.mem.pswpin";
    const std::string SYS_VMEMINFO_PSWPOUT_DESC = "sys.virtual.mem.pswpout";
    const std::string SYS_VMEMINFO_PGALLOC_DMA_DESC = "sys.virtual.mem.pgalloc.dma";
    const std::string SYS_VMEMINFO_PGALLOC_NORMAL_DESC = "sys.virtual.mem.pgalloc.normal";
    const std::string SYS_VMEMINFO_PGALLOC_MOVABLE_DESC = "sys.virtual.mem.pgalloc.movable";
    const std::string SYS_VMEMINFO_PGFREE_DESC = "sys.virtual.mem.pgfree";
    const std::string SYS_VMEMINFO_PGACTIVATE_DESC = "sys.virtual.mem.pgactivate";
    const std::string SYS_VMEMINFO_PGDEACTIVATE_DESC = "sys.virtual.mem.pgdeactivate";
    const std::string SYS_VMEMINFO_PGFAULT_DESC = "sys.virtual.mem.pgfault";
    const std::string SYS_VMEMINFO_PGMAJFAULT_DESC = "sys.virtual.mem.pgmajfault";
    const std::string SYS_VMEMINFO_PGREFILL_DMA_DESC = "sys.virtual.mem.pgrefill.dma";
    const std::string SYS_VMEMINFO_PGREFILL_NORMAL_DESC = "sys.virtual.mem.pgrefill.normal";
    const std::string SYS_VMEMINFO_PGREFILL_MOVABLE_DESC = "sys.virtual.mem.pgrefill.movable";
    const std::string SYS_VMEMINFO_PGSTEAL_KSWAPD_DMA_DESC = "sys.virtual.mem.pgsteal.kswapd.dma";
    const std::string SYS_VMEMINFO_PGSTEAL_KSWAPD_NORMAL_DESC = "sys.virtual.mem.pgsteal.kswapd.normal";
    const std::string SYS_VMEMINFO_PGSTEAL_KSWAPD_MOVABLE_DESC = "sys.virtual.mem.pgsteal.kswapd.movable";
    const std::string SYS_VMEMINFO_PGSTEAL_DIRECT_DMA_DESC = "sys.virtual.mem.pgsteal.direct.dma";
    const std::string SYS_VMEMINFO_PGSTEAL_DIRECT_NORMAL_DESC = "sys.virtual.mem.pgsteal.direct.normal";
    const std::string SYS_VMEMINFO_PGSTEAL_DIRECT_MOVABLE_DESC = "sys.virtual.mem.pgsteal_direct.movable";
    const std::string SYS_VMEMINFO_PGSCAN_KSWAPD_DMA_DESC = "sys.virtual.mem.pgscan.kswapd.dma";
    const std::string SYS_VMEMINFO_PGSCAN_KSWAPD_NORMAL_DESC = "sys.virtual.mem.pgscan_kswapd.normal";
    const std::string SYS_VMEMINFO_PGSCAN_KSWAPD_MOVABLE_DESC = "sys.virtual.mem.pgscan.kswapd.movable";
    const std::string SYS_VMEMINFO_PGSCAN_DIRECT_DMA_DESC = "sys.virtual.mem.pgscan.direct.dma";
    const std::string SYS_VMEMINFO_PGSCAN_DIRECT_NORMAL_DESC = "sys.virtual.mem.pgscan.direct.normal";
    const std::string SYS_VMEMINFO_PGSCAN_DIRECT_MOVABLE_DESC = "sys.virtual.mem.pgscan.direct.movable";
    const std::string SYS_VMEMINFO_PGSCAN_DIRECT_THROTTLE_DESC = "sys.virtual.mem.pgscan.direct.throttle";
    const std::string SYS_VMEMINFO_PGINODESTEAL_DESC = "sys.virtual.mem.pginodesteal";
    const std::string SYS_VMEMINFO_SLABS_SCANNED_DESC = "sys.virtual.mem.slabs_scanned";
    const std::string SYS_VMEMINFO_KSWAPD_INODESTEAL_DESC = "sys.virtual.mem.kswapd.inodesteal";
    const std::string SYS_VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY_DESC = "sys.virtual.mem.kswapd.low.wmark.hit.quickly";
    const std::string SYS_VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY_DESC = "sys.virtual.mem.high.wmark.hit.quickly";
    const std::string SYS_VMEMINFO_PAGEOUTRUN_DESC = "sys.virtual.mem.pageoutrun";
    const std::string SYS_VMEMINFO_ALLOCSTALL_DESC = "sys.virtual.mem.allocstall";
    const std::string SYS_VMEMINFO_PGROTATED_DESC = "sys.virtual.mem.pgrotated";
    const std::string SYS_VMEMINFO_DROP_PAGECACHE_DESC = "sys.virtual.mem.drop.pagecache";
    const std::string SYS_VMEMINFO_DROP_SLAB_DESC = "sys.virtual.mem.drop.slab";
    const std::string SYS_VMEMINFO_PGMIGRATE_SUCCESS_DESC = "sys.virtual.mem.pgmigrate.success";
    const std::string SYS_VMEMINFO_PGMIGRATE_FAIL_DESC = "sys.virtual.mem.pgmigrate.fail";
    const std::string SYS_VMEMINFO_COMPACT_MIGRATE_SCANNED_DESC = "sys.virtual.mem.compact.migrate.scanned";
    const std::string SYS_VMEMINFO_COMPACT_FREE_SCANNED_DESC = "sys.virtual.mem.compact.free.scanned";
    const std::string SYS_VMEMINFO_COMPACT_ISOLATED_DESC = "sys.virtual.mem.compact.isolated";
    const std::string SYS_VMEMINFO_COMPACT_STALL_DESC = "sys.virtual.mem.compact.stall";
    const std::string SYS_VMEMINFO_COMPACT_FAIL_DESC = "sys.virtual.mem.compact.fail";
    const std::string SYS_VMEMINFO_COMPACT_SUCCESS_DESC = "sys.virtual.mem.compact.success";
    const std::string SYS_VMEMINFO_COMPACT_DAEMON_WAKE_DESC = "sys.virtual.mem.compact.daemon.wake";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_CULLED_DESC = "sys.virtual.mem.unevictable.pgs.culled";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_SCANNED_DESC = "sys.virtual.mem.unevictable.pgs.scanned";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_RESCUED_DESC = "sys.virtual.mem.unevictable.pgs.rescued";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_MLOCKED_DESC = "sys.virtual.mem.unevictable.pgs.mlocked";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED_DESC = "sys.virtual.mem.unevictable.pgs.munlocked";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_CLEARED_DESC = "sys.virtual.mem.unevictable.pgs.cleared";
    const std::string SYS_VMEMINFO_UNEVICTABLE_PGS_STRANDED_DESC = "sys.virtual.mem.unevictable.pgs.stranded";
    const std::string SYS_VMEMINFO_NR_ZSPAGES_DESC = "sys.virtual.mem.nr.zspages";
    const std::string SYS_VMEMINFO_NR_ION_HEAP_DESC = "sys.virtual.mem.nr.ion.heap";
    const std::string SYS_VMEMINFO_NR_GPU_HEAP_DESC = "sys.virtual.mem.nr.gpu.heap";
    const std::string SYS_VMEMINFO_ALLOCSTALL_DMA_DESC = "sys.virtual.mem.allocstall.dma";
    const std::string SYS_VMEMINFO_ALLOCSTALL_MOVABLE_DESC = "sys.virtual.mem.allocstall.movable";
    const std::string SYS_VMEMINFO_ALLOCSTALL_NORMAL_DESC = "sys.virtual.mem.allocstall.normal";
    const std::string SYS_VMEMINFO_COMPACT_DAEMON_FREE_SCANNED_DESC = "sys.virtual.mem.compact_daemon.free.scanned";
    const std::string SYS_VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED_DESC =
        "sys.virtual.mem.compact.daemon.migrate.scanned";
    const std::string SYS_VMEMINFO_NR_FASTRPC_DESC = "sys.virtual.mem.nr.fastrpc";
    const std::string SYS_VMEMINFO_NR_INDIRECTLY_RECLAIMABLE_DESC = "sys.virtual.mem.nr.indirectly.reclaimable";
    const std::string SYS_VMEMINFO_NR_ION_HEAP_POOL_DESC = "sys.virtual.mem.nr_ion_heap_pool";
    const std::string SYS_VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE_DESC = "sys.virtual.mem.nr.kernel_misc.reclaimable";
    const std::string SYS_VMEMINFO_NR_SHADOW_CALL_STACK_BYTES_DESC = "sys.virtual.mem.nr.shadow_call.stack_bytes";
    const std::string SYS_VMEMINFO_NR_SHMEM_HUGEPAGES_DESC = "sys.virtual.mem.nr.shmem.hugepages";
    const std::string SYS_VMEMINFO_NR_SHMEM_PMDMAPPED_DESC = "sys.virtual.mem.nr.shmem.pmdmapped";
    const std::string SYS_VMEMINFO_NR_UNRECLAIMABLE_PAGES_DESC = "sys.virtual.mem.nr.unreclaimable.pages";
    const std::string SYS_VMEMINFO_NR_ZONE_ACTIVE_ANON_DESC = "sys.virtual.mem.nr.zone.active.anon";
    const std::string SYS_VMEMINFO_NR_ZONE_ACTIVE_FILE_DESC = "sys.virtual.mem.nr.zone.active.file";
    const std::string SYS_VMEMINFO_NR_ZONE_INACTIVE_ANON_DESC = "sys.virtual.mem.nr.zone.inactive_anon";
    const std::string SYS_VMEMINFO_NR_ZONE_INACTIVE_FILE_DESC = "sys.virtual.mem.nr.zone.inactive_file";
    const std::string SYS_VMEMINFO_NR_ZONE_UNEVICTABLE_DESC = "sys.virtual.mem.nr.zone.unevictable";
    const std::string SYS_VMEMINFO_NR_ZONE_WRITE_PENDING_DESC = "sys.virtual.mem.nr.zone.write_pending";
    const std::string SYS_VMEMINFO_OOM_KILL_DESC = "sys.virtual.mem.oom.kill";
    const std::string SYS_VMEMINFO_PGLAZYFREE_DESC = "sys.virtual.mem.pglazyfree";
    const std::string SYS_VMEMINFO_PGLAZYFREED_DESC = "sys.virtual.mem.pglazyfreed";
    const std::string SYS_VMEMINFO_PGREFILL_DESC = "sys.virtual.mem.pgrefill";
    const std::string SYS_VMEMINFO_PGSCAN_DIRECT_DESC = "sys.virtual.mem.pgscan.direct";
    const std::string SYS_VMEMINFO_PGSCAN_KSWAPD_DESC = "sys.virtual.mem.pgscan.kswapd";
    const std::string SYS_VMEMINFO_PGSKIP_DMA_DESC = "sys.virtual.mem.pgskip.dma";
    const std::string SYS_VMEMINFO_PGSKIP_MOVABLE_DESC = "sys.virtual.mem.pgskip.movable";
    const std::string SYS_VMEMINFO_PGSKIP_NORMAL_DESC = "sys.virtual.mem.pgskip.normal";
    const std::string SYS_VMEMINFO_PGSTEAL_DIRECT_DESC = "sys.virtual.mem.pgsteal.direct";
    const std::string SYS_VMEMINFO_PGSTEAL_KSWAPD_DESC = "sys.virtual.mem.pgsteal.kswapd";
    const std::string SYS_VMEMINFO_SWAP_RA_DESC = "sys.virtual.mem.swap.ra";
    const std::string SYS_VMEMINFO_SWAP_RA_HIT_DESC = "sys.virtual.mem.swap.ra.hit";
    const std::string SYS_VMEMINFO_WORKINGSET_RESTORE_DESC = "sys.virtual.mem.workingset.restore";

    const std::string TRACE_STAT_TYPE_RECEIVED_DESC = "received";
    const std::string TRACE_STAT_TYPE_LOST_DESC = "data_lost";
    const std::string TRACE_STAT_TYPE_NOTMATCH_DESC = "not_match";
    const std::string TRACE_STAT_TYPE_NOTSUPPORTED_DESC = "not_supported";
    const std::string TRACE_STAT_TYPE_DATA_INVALID_DESC = "invalid_data";

    const std::string STAT_SEVERITY_LEVEL_INFO_DESC = "info";
    const std::string STAT_SEVERITY_LEVEL_WARN_DESC = "warn";
    const std::string STAT_SEVERITY_LEVEL_ERROR_DESC = "error";
    const std::string STAT_SEVERITY_LEVEL_FATAL_DESC = "fatal";
};
} // namespace TraceCfg
} // namespace SysTuning
#endif
