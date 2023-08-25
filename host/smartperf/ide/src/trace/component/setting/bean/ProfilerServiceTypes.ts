/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

export interface ProfilerSessionConfigBufferConfig {
  pages: number;
  policy: ProfilerSessionConfigBufferConfigPolicy;
}

export enum ProfilerSessionConfigBufferConfigPolicy {
  RECYCLE = 0,
  FLATTEN = 1,
  UNRECOGNIZED = 2,
}

export interface ProfilerSessionConfig {
  buffers: ProfilerSessionConfigBufferConfig[];
  sessionMode: ProfilerSessionConfigMode;
  /** for OFFLINE mode, result file path */
  resultFile: string;
  /** for OFFLINE mode, result file max size in KB */
  resultMaxSize: number;
  /** for OFFLINE mode, sample duration in ms */
  sampleDuration: number;
  /** if set to non-zero value, session will auto-destroyed after CreateSession in ms */
  keepAliveTime: number;
}

export enum ProfilerSessionConfigMode {
  /** OFFLINE - save all plugin results to result file. */
  OFFLINE = 0,
  /** ONLINE - push all plugin results to host PC with streamed FetchDataResponse. */
  ONLINE = 1,
  UNRECOGNIZED = -1,
}

export interface TracePluginConfig {
  /** kernel event set */
  ftraceEvents: string[];
  /** bytrace event set */
  hitraceCategories: string[];
  /** bytrace app set */
  hitraceApps: string[];
  /** kernel trace buffer size */
  bufferSizeKb: number;
  /** time interval in milliseconds to notify service process */
  flushIntervalMs: number;
  /** buffer water mark threshold to notify service process */
  flushThresholdKb: number;
  /** parse /proc/kallsyms or not */
  parseKsyms: boolean;
  /** value for trace_clock */
  clock: string;
  /** time interval in milliseconds to read kernel trace buffer */
  tracePeriodMs: number;
  /** raw data file prefix for debug */
  rawDataPrefix: string;
  /** time duration in millisconds for trace actions */
  traceDurationMs: number;
  /** enable debug options */
  debugOn: boolean;
  hitraceTime: number;
}

export interface CreateSessionRequest {
  requestId: number;
  sessionConfig: ProfilerSessionConfig | undefined;
  pluginConfigs: ProfilerPluginConfig<any>[];
}

export interface ProfilerPluginConfig<T> {
  pluginName: string;
  sampleInterval?: number;
  configData: T;
  protobuf_serialize?: boolean;
}

export interface FileSystemConfig {
  cmdLine: string;
  outfileName: string;
}

export interface MemoryConfig {
  /** set true to report process list */
  reportProcessTree: boolean;
  /** set true to report memory counter from /proc/meminfo */
  reportSysmemMemInfo: boolean;
  /** set required counter list of system meminfo, eg:MemTotal, MemFree, etc. */
  sysMeminfoCounters: SysMeminfoType[];
  /** set true to report memory counter from /proc/vmstat */
  reportSysmemVmemInfo: boolean;
  /** set required counter list of virtual system meminfo, eg:nr_free_pages, nr_anon_pages, etc. */
  sysVmeminfoCounters: SysVMeminfoType[];
  /** set true to report process meminfo from /proc/${pid}/stat */
  reportProcessMemInfo: boolean;
  /** set true to report application memory usage summary, eg:java heap memory, native heap, stack memory, etc. */
  reportAppMemInfo: boolean;
  /**
   * set true to report application memory by memory service, otherwise,
   * application memory will count up by /proc/${pid}/smaps information
   */
  reportAppMemByMemoryService: boolean;
  /** set required pid list */
  pid: number[];
  /** set true to report smaps meminfo from /proc/${pid}/smaps */
  reportSmapsMemInfo?: boolean;
  /** set true to report ashmem meminfo from /proc/purgeable_ashmem_trigger*/
  reportPurgeableAshmemInfo?: boolean;
  /** set true to report DMA meminfo from /proc/process_dmabuf_info. */
  reportDmaMemInfo?: boolean;
  /** set true to report gpu meminfo from /proc/gpu_memory. */
  reportGpuMemInfo?: boolean;
  /** set true to report gpu info from hidumper. */
  reportGpuDumpInfo?: boolean;
}

export function sysVMeminfoTypeFromJSON(object: any): SysVMeminfoType {
  switch (object) {
    case 0:
    case 'VMEMINFO_UNSPECIFIED':
      return SysVMeminfoType.VMEMINFO_UNSPECIFIED;
    case 1:
    case 'VMEMINFO_NR_FREE_PAGES':
      return SysVMeminfoType.VMEMINFO_NR_FREE_PAGES;
    case 2:
    case 'VMEMINFO_NR_ALLOC_BATCH':
      return SysVMeminfoType.VMEMINFO_NR_ALLOC_BATCH;
    case 3:
    case 'VMEMINFO_NR_INACTIVE_ANON':
      return SysVMeminfoType.VMEMINFO_NR_INACTIVE_ANON;
    case 4:
    case 'VMEMINFO_NR_ACTIVE_ANON':
      return SysVMeminfoType.VMEMINFO_NR_ACTIVE_ANON;
    case 5:
    case 'VMEMINFO_NR_INACTIVE_FILE':
      return SysVMeminfoType.VMEMINFO_NR_INACTIVE_FILE;
    case 6:
    case 'VMEMINFO_NR_ACTIVE_FILE':
      return SysVMeminfoType.VMEMINFO_NR_ACTIVE_FILE;
    case 7:
    case 'VMEMINFO_NR_UNEVICTABLE':
      return SysVMeminfoType.VMEMINFO_NR_UNEVICTABLE;
    case 8:
    case 'VMEMINFO_NR_MLOCK':
      return SysVMeminfoType.VMEMINFO_NR_MLOCK;
    case 9:
    case 'VMEMINFO_NR_ANON_PAGES':
      return SysVMeminfoType.VMEMINFO_NR_ANON_PAGES;
    case 10:
    case 'VMEMINFO_NR_MAPPED':
      return SysVMeminfoType.VMEMINFO_NR_MAPPED;
    case 11:
    case 'VMEMINFO_NR_FILE_PAGES':
      return SysVMeminfoType.VMEMINFO_NR_FILE_PAGES;
    case 12:
    case 'VMEMINFO_NR_DIRTY':
      return SysVMeminfoType.VMEMINFO_NR_DIRTY;
    case 13:
    case 'VMEMINFO_NR_WRITEBACK':
      return SysVMeminfoType.VMEMINFO_NR_WRITEBACK;
    case 14:
    case 'VMEMINFO_NR_SLAB_RECLAIMABLE':
      return SysVMeminfoType.VMEMINFO_NR_SLAB_RECLAIMABLE;
    case 15:
    case 'VMEMINFO_NR_SLAB_UNRECLAIMABLE':
      return SysVMeminfoType.VMEMINFO_NR_SLAB_UNRECLAIMABLE;
    case 16:
    case 'VMEMINFO_NR_PAGE_TABLE_PAGES':
      return SysVMeminfoType.VMEMINFO_NR_PAGE_TABLE_PAGES;
    case 17:
    case 'VMEMINFO_NR_KERNEL_STACK':
      return SysVMeminfoType.VMEMINFO_NR_KERNEL_STACK;
    case 18:
    case 'VMEMINFO_NR_OVERHEAD':
      return SysVMeminfoType.VMEMINFO_NR_OVERHEAD;
    case 19:
    case 'VMEMINFO_NR_UNSTABLE':
      return SysVMeminfoType.VMEMINFO_NR_UNSTABLE;
    case 20:
    case 'VMEMINFO_NR_BOUNCE':
      return SysVMeminfoType.VMEMINFO_NR_BOUNCE;
    case 21:
    case 'VMEMINFO_NR_VMSCAN_WRITE':
      return SysVMeminfoType.VMEMINFO_NR_VMSCAN_WRITE;
    case 22:
    case 'VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM':
      return SysVMeminfoType.VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM;
    case 23:
    case 'VMEMINFO_NR_WRITEBACK_TEMP':
      return SysVMeminfoType.VMEMINFO_NR_WRITEBACK_TEMP;
    case 24:
    case 'VMEMINFO_NR_ISOLATED_ANON':
      return SysVMeminfoType.VMEMINFO_NR_ISOLATED_ANON;
    case 25:
    case 'VMEMINFO_NR_ISOLATED_FILE':
      return SysVMeminfoType.VMEMINFO_NR_ISOLATED_FILE;
    case 26:
    case 'VMEMINFO_NR_SHMEM':
      return SysVMeminfoType.VMEMINFO_NR_SHMEM;
    case 27:
    case 'VMEMINFO_NR_DIRTIED':
      return SysVMeminfoType.VMEMINFO_NR_DIRTIED;
    case 28:
    case 'VMEMINFO_NR_WRITTEN':
      return SysVMeminfoType.VMEMINFO_NR_WRITTEN;
    case 29:
    case 'VMEMINFO_NR_PAGES_SCANNED':
      return SysVMeminfoType.VMEMINFO_NR_PAGES_SCANNED;
    case 30:
    case 'VMEMINFO_WORKINGSET_REFAULT':
      return SysVMeminfoType.VMEMINFO_WORKINGSET_REFAULT;
    case 31:
    case 'VMEMINFO_WORKINGSET_ACTIVATE':
      return SysVMeminfoType.VMEMINFO_WORKINGSET_ACTIVATE;
    case 32:
    case 'VMEMINFO_WORKINGSET_NODERECLAIM':
      return SysVMeminfoType.VMEMINFO_WORKINGSET_NODERECLAIM;
    case 33:
    case 'VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES':
      return SysVMeminfoType.VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES;
    case 34:
    case 'VMEMINFO_NR_FREE_CMA':
      return SysVMeminfoType.VMEMINFO_NR_FREE_CMA;
    case 35:
    case 'VMEMINFO_NR_SWAPCACHE':
      return SysVMeminfoType.VMEMINFO_NR_SWAPCACHE;
    case 36:
    case 'VMEMINFO_NR_DIRTY_THRESHOLD':
      return SysVMeminfoType.VMEMINFO_NR_DIRTY_THRESHOLD;
    case 37:
    case 'VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD':
      return SysVMeminfoType.VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD;
    case 38:
    case 'VMEMINFO_PGPGIN':
      return SysVMeminfoType.VMEMINFO_PGPGIN;
    case 39:
    case 'VMEMINFO_PGPGOUT':
      return SysVMeminfoType.VMEMINFO_PGPGOUT;
    case 40:
    case 'VMEMINFO_PGPGOUTCLEAN':
      return SysVMeminfoType.VMEMINFO_PGPGOUTCLEAN;
    case 41:
    case 'VMEMINFO_PSWPIN':
      return SysVMeminfoType.VMEMINFO_PSWPIN;
    case 42:
    case 'VMEMINFO_PSWPOUT':
      return SysVMeminfoType.VMEMINFO_PSWPOUT;
    case 43:
    case 'VMEMINFO_PGALLOC_DMA':
      return SysVMeminfoType.VMEMINFO_PGALLOC_DMA;
    case 44:
    case 'VMEMINFO_PGALLOC_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGALLOC_NORMAL;
    case 45:
    case 'VMEMINFO_PGALLOC_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGALLOC_MOVABLE;
    case 46:
    case 'VMEMINFO_PGFREE':
      return SysVMeminfoType.VMEMINFO_PGFREE;
    case 47:
    case 'VMEMINFO_PGACTIVATE':
      return SysVMeminfoType.VMEMINFO_PGACTIVATE;
    case 48:
    case 'VMEMINFO_PGDEACTIVATE':
      return SysVMeminfoType.VMEMINFO_PGDEACTIVATE;
    case 49:
    case 'VMEMINFO_PGFAULT':
      return SysVMeminfoType.VMEMINFO_PGFAULT;
    case 50:
    case 'VMEMINFO_PGMAJFAULT':
      return SysVMeminfoType.VMEMINFO_PGMAJFAULT;
    case 51:
    case 'VMEMINFO_PGREFILL_DMA':
      return SysVMeminfoType.VMEMINFO_PGREFILL_DMA;
    case 52:
    case 'VMEMINFO_PGREFILL_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGREFILL_NORMAL;
    case 53:
    case 'VMEMINFO_PGREFILL_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGREFILL_MOVABLE;
    case 54:
    case 'VMEMINFO_PGSTEAL_KSWAPD_DMA':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_KSWAPD_DMA;
    case 55:
    case 'VMEMINFO_PGSTEAL_KSWAPD_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_KSWAPD_NORMAL;
    case 56:
    case 'VMEMINFO_PGSTEAL_KSWAPD_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_KSWAPD_MOVABLE;
    case 57:
    case 'VMEMINFO_PGSTEAL_DIRECT_DMA':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_DIRECT_DMA;
    case 58:
    case 'VMEMINFO_PGSTEAL_DIRECT_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_DIRECT_NORMAL;
    case 59:
    case 'VMEMINFO_PGSTEAL_DIRECT_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_DIRECT_MOVABLE;
    case 60:
    case 'VMEMINFO_PGSCAN_KSWAPD_DMA':
      return SysVMeminfoType.VMEMINFO_PGSCAN_KSWAPD_DMA;
    case 61:
    case 'VMEMINFO_PGSCAN_KSWAPD_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGSCAN_KSWAPD_NORMAL;
    case 62:
    case 'VMEMINFO_PGSCAN_KSWAPD_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGSCAN_KSWAPD_MOVABLE;
    case 63:
    case 'VMEMINFO_PGSCAN_DIRECT_DMA':
      return SysVMeminfoType.VMEMINFO_PGSCAN_DIRECT_DMA;
    case 64:
    case 'VMEMINFO_PGSCAN_DIRECT_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGSCAN_DIRECT_NORMAL;
    case 65:
    case 'VMEMINFO_PGSCAN_DIRECT_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGSCAN_DIRECT_MOVABLE;
    case 66:
    case 'VMEMINFO_PGSCAN_DIRECT_THROTTLE':
      return SysVMeminfoType.VMEMINFO_PGSCAN_DIRECT_THROTTLE;
    case 67:
    case 'VMEMINFO_PGINODESTEAL':
      return SysVMeminfoType.VMEMINFO_PGINODESTEAL;
    case 68:
    case 'VMEMINFO_SLABS_SCANNED':
      return SysVMeminfoType.VMEMINFO_SLABS_SCANNED;
    case 69:
    case 'VMEMINFO_KSWAPD_INODESTEAL':
      return SysVMeminfoType.VMEMINFO_KSWAPD_INODESTEAL;
    case 70:
    case 'VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY':
      return SysVMeminfoType.VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY;
    case 71:
    case 'VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY':
      return SysVMeminfoType.VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY;
    case 72:
    case 'VMEMINFO_PAGEOUTRUN':
      return SysVMeminfoType.VMEMINFO_PAGEOUTRUN;
    case 73:
    case 'VMEMINFO_ALLOCSTALL':
      return SysVMeminfoType.VMEMINFO_ALLOCSTALL;
    case 74:
    case 'VMEMINFO_PGROTATED':
      return SysVMeminfoType.VMEMINFO_PGROTATED;
    case 75:
    case 'VMEMINFO_DROP_PAGECACHE':
      return SysVMeminfoType.VMEMINFO_DROP_PAGECACHE;
    case 76:
    case 'VMEMINFO_DROP_SLAB':
      return SysVMeminfoType.VMEMINFO_DROP_SLAB;
    case 77:
    case 'VMEMINFO_PGMIGRATE_SUCCESS':
      return SysVMeminfoType.VMEMINFO_PGMIGRATE_SUCCESS;
    case 78:
    case 'VMEMINFO_PGMIGRATE_FAIL':
      return SysVMeminfoType.VMEMINFO_PGMIGRATE_FAIL;
    case 79:
    case 'VMEMINFO_COMPACT_MIGRATE_SCANNED':
      return SysVMeminfoType.VMEMINFO_COMPACT_MIGRATE_SCANNED;
    case 80:
    case 'VMEMINFO_COMPACT_FREE_SCANNED':
      return SysVMeminfoType.VMEMINFO_COMPACT_FREE_SCANNED;
    case 81:
    case 'VMEMINFO_COMPACT_ISOLATED':
      return SysVMeminfoType.VMEMINFO_COMPACT_ISOLATED;
    case 82:
    case 'VMEMINFO_COMPACT_STALL':
      return SysVMeminfoType.VMEMINFO_COMPACT_STALL;
    case 83:
    case 'VMEMINFO_COMPACT_FAIL':
      return SysVMeminfoType.VMEMINFO_COMPACT_FAIL;
    case 84:
    case 'VMEMINFO_COMPACT_SUCCESS':
      return SysVMeminfoType.VMEMINFO_COMPACT_SUCCESS;
    case 85:
    case 'VMEMINFO_COMPACT_DAEMON_WAKE':
      return SysVMeminfoType.VMEMINFO_COMPACT_DAEMON_WAKE;
    case 86:
    case 'VMEMINFO_UNEVICTABLE_PGS_CULLED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_CULLED;
    case 87:
    case 'VMEMINFO_UNEVICTABLE_PGS_SCANNED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_SCANNED;
    case 88:
    case 'VMEMINFO_UNEVICTABLE_PGS_RESCUED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_RESCUED;
    case 89:
    case 'VMEMINFO_UNEVICTABLE_PGS_MLOCKED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_MLOCKED;
    case 90:
    case 'VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED;
    case 91:
    case 'VMEMINFO_UNEVICTABLE_PGS_CLEARED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_CLEARED;
    case 92:
    case 'VMEMINFO_UNEVICTABLE_PGS_STRANDED':
      return SysVMeminfoType.VMEMINFO_UNEVICTABLE_PGS_STRANDED;
    case 93:
    case 'VMEMINFO_NR_ZSPAGES':
      return SysVMeminfoType.VMEMINFO_NR_ZSPAGES;
    case 94:
    case 'VMEMINFO_NR_ION_HEAP':
      return SysVMeminfoType.VMEMINFO_NR_ION_HEAP;
    case 95:
    case 'VMEMINFO_NR_GPU_HEAP':
      return SysVMeminfoType.VMEMINFO_NR_GPU_HEAP;
    case 96:
    case 'VMEMINFO_ALLOCSTALL_DMA':
      return SysVMeminfoType.VMEMINFO_ALLOCSTALL_DMA;
    case 97:
    case 'VMEMINFO_ALLOCSTALL_MOVABLE':
      return SysVMeminfoType.VMEMINFO_ALLOCSTALL_MOVABLE;
    case 98:
    case 'VMEMINFO_ALLOCSTALL_NORMAL':
      return SysVMeminfoType.VMEMINFO_ALLOCSTALL_NORMAL;
    case 99:
    case 'VMEMINFO_COMPACT_DAEMON_FREE_SCANNED':
      return SysVMeminfoType.VMEMINFO_COMPACT_DAEMON_FREE_SCANNED;
    case 100:
    case 'VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED':
      return SysVMeminfoType.VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED;
    case 101:
    case 'VMEMINFO_NR_FASTRPC':
      return SysVMeminfoType.VMEMINFO_NR_FASTRPC;
    case 102:
    case 'VMEMINFO_NR_INDIRECTLY_RECLAIMABLE':
      return SysVMeminfoType.VMEMINFO_NR_INDIRECTLY_RECLAIMABLE;
    case 103:
    case 'VMEMINFO_NR_ION_HEAP_POOL':
      return SysVMeminfoType.VMEMINFO_NR_ION_HEAP_POOL;
    case 104:
    case 'VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE':
      return SysVMeminfoType.VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE;
    case 105:
    case 'VMEMINFO_NR_SHADOW_CALL_STACK_BYTES':
      return SysVMeminfoType.VMEMINFO_NR_SHADOW_CALL_STACK_BYTES;
    case 106:
    case 'VMEMINFO_NR_SHMEM_HUGEPAGES':
      return SysVMeminfoType.VMEMINFO_NR_SHMEM_HUGEPAGES;
    case 107:
    case 'VMEMINFO_NR_SHMEM_PMDMAPPED':
      return SysVMeminfoType.VMEMINFO_NR_SHMEM_PMDMAPPED;
    case 108:
    case 'VMEMINFO_NR_UNRECLAIMABLE_PAGES':
      return SysVMeminfoType.VMEMINFO_NR_UNRECLAIMABLE_PAGES;
    case 109:
    case 'VMEMINFO_NR_ZONE_ACTIVE_ANON':
      return SysVMeminfoType.VMEMINFO_NR_ZONE_ACTIVE_ANON;
    case 110:
    case 'VMEMINFO_NR_ZONE_ACTIVE_FILE':
      return SysVMeminfoType.VMEMINFO_NR_ZONE_ACTIVE_FILE;
    case 111:
    case 'VMEMINFO_NR_ZONE_INACTIVE_ANON':
      return SysVMeminfoType.VMEMINFO_NR_ZONE_INACTIVE_ANON;
    case 112:
    case 'VMEMINFO_NR_ZONE_INACTIVE_FILE':
      return SysVMeminfoType.VMEMINFO_NR_ZONE_INACTIVE_FILE;
    case 113:
    case 'VMEMINFO_NR_ZONE_UNEVICTABLE':
      return SysVMeminfoType.VMEMINFO_NR_ZONE_UNEVICTABLE;
    case 114:
    case 'VMEMINFO_NR_ZONE_WRITE_PENDING':
      return SysVMeminfoType.VMEMINFO_NR_ZONE_WRITE_PENDING;
    case 115:
    case 'VMEMINFO_OOM_KILL':
      return SysVMeminfoType.VMEMINFO_OOM_KILL;
    case 116:
    case 'VMEMINFO_PGLAZYFREE':
      return SysVMeminfoType.VMEMINFO_PGLAZYFREE;
    case 117:
    case 'VMEMINFO_PGLAZYFREED':
      return SysVMeminfoType.VMEMINFO_PGLAZYFREED;
    case 118:
    case 'VMEMINFO_PGREFILL':
      return SysVMeminfoType.VMEMINFO_PGREFILL;
    case 119:
    case 'VMEMINFO_PGSCAN_DIRECT':
      return SysVMeminfoType.VMEMINFO_PGSCAN_DIRECT;
    case 120:
    case 'VMEMINFO_PGSCAN_KSWAPD':
      return SysVMeminfoType.VMEMINFO_PGSCAN_KSWAPD;
    case 121:
    case 'VMEMINFO_PGSKIP_DMA':
      return SysVMeminfoType.VMEMINFO_PGSKIP_DMA;
    case 122:
    case 'VMEMINFO_PGSKIP_MOVABLE':
      return SysVMeminfoType.VMEMINFO_PGSKIP_MOVABLE;
    case 123:
    case 'VMEMINFO_PGSKIP_NORMAL':
      return SysVMeminfoType.VMEMINFO_PGSKIP_NORMAL;
    case 124:
    case 'VMEMINFO_PGSTEAL_DIRECT':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_DIRECT;
    case 125:
    case 'VMEMINFO_PGSTEAL_KSWAPD':
      return SysVMeminfoType.VMEMINFO_PGSTEAL_KSWAPD;
    case 126:
    case 'VMEMINFO_SWAP_RA':
      return SysVMeminfoType.VMEMINFO_SWAP_RA;
    case 127:
    case 'VMEMINFO_SWAP_RA_HIT':
      return SysVMeminfoType.VMEMINFO_SWAP_RA_HIT;
    case 128:
    case 'VMEMINFO_WORKINGSET_RESTORE':
      return SysVMeminfoType.VMEMINFO_WORKINGSET_RESTORE;
    case -1:
    case 'UNRECOGNIZED':
    default:
      return SysVMeminfoType.UNRECOGNIZED;
  }
}

export enum SysVMeminfoType {
  UNRECOGNIZED = 'UNRECOGNIZED',
  VMEMINFO_UNSPECIFIED = 'VMEMINFO_UNSPECIFIED',
  VMEMINFO_NR_FREE_PAGES = 'VMEMINFO_NR_FREE_PAGES',
  VMEMINFO_NR_ALLOC_BATCH = 'VMEMINFO_NR_ALLOC_BATCH',
  VMEMINFO_NR_INACTIVE_ANON = 'VMEMINFO_NR_INACTIVE_ANON',
  VMEMINFO_NR_ACTIVE_ANON = 'VMEMINFO_NR_ACTIVE_ANON',
  VMEMINFO_NR_INACTIVE_FILE = 'VMEMINFO_NR_INACTIVE_FILE',
  VMEMINFO_NR_ACTIVE_FILE = 'VMEMINFO_NR_ACTIVE_FILE',
  VMEMINFO_NR_UNEVICTABLE = 'VMEMINFO_NR_UNEVICTABLE',
  VMEMINFO_NR_MLOCK = 'VMEMINFO_NR_MLOCK',
  VMEMINFO_NR_ANON_PAGES = 'VMEMINFO_NR_ANON_PAGES',
  VMEMINFO_NR_MAPPED = 'VMEMINFO_NR_MAPPED',
  VMEMINFO_NR_FILE_PAGES = 'VMEMINFO_NR_FILE_PAGES',
  VMEMINFO_NR_DIRTY = 'VMEMINFO_NR_DIRTY',
  VMEMINFO_NR_WRITEBACK = 'VMEMINFO_NR_WRITEBACK',
  VMEMINFO_NR_SLAB_RECLAIMABLE = 'VMEMINFO_NR_SLAB_RECLAIMABLE',
  VMEMINFO_NR_SLAB_UNRECLAIMABLE = 'VMEMINFO_NR_SLAB_UNRECLAIMABLE',
  VMEMINFO_NR_PAGE_TABLE_PAGES = 'VMEMINFO_NR_PAGE_TABLE_PAGES',
  VMEMINFO_NR_KERNEL_STACK = 'VMEMINFO_NR_KERNEL_STACK',
  VMEMINFO_NR_OVERHEAD = 'VMEMINFO_NR_OVERHEAD',
  VMEMINFO_NR_UNSTABLE = 'VMEMINFO_NR_UNSTABLE',
  VMEMINFO_NR_BOUNCE = 'VMEMINFO_NR_BOUNCE',
  VMEMINFO_NR_VMSCAN_WRITE = 'VMEMINFO_NR_VMSCAN_WRITE',
  VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM = 'VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM',
  VMEMINFO_NR_WRITEBACK_TEMP = 'VMEMINFO_NR_WRITEBACK_TEMP',
  VMEMINFO_NR_ISOLATED_ANON = 'VMEMINFO_NR_ISOLATED_ANON',
  VMEMINFO_NR_ISOLATED_FILE = 'VMEMINFO_NR_ISOLATED_FILE',
  VMEMINFO_NR_SHMEM = 'VMEMINFO_NR_SHMEM',
  VMEMINFO_NR_DIRTIED = 'VMEMINFO_NR_DIRTIED',
  VMEMINFO_NR_WRITTEN = 'VMEMINFO_NR_WRITTEN',
  VMEMINFO_NR_PAGES_SCANNED = 'VMEMINFO_NR_PAGES_SCANNED',
  VMEMINFO_WORKINGSET_REFAULT = 'VMEMINFO_WORKINGSET_REFAULT',
  VMEMINFO_WORKINGSET_ACTIVATE = 'VMEMINFO_WORKINGSET_ACTIVATE',
  VMEMINFO_WORKINGSET_NODERECLAIM = 'VMEMINFO_WORKINGSET_NODERECLAIM',
  VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES = 'VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES',
  VMEMINFO_NR_FREE_CMA = 'VMEMINFO_NR_FREE_CMA',
  VMEMINFO_NR_SWAPCACHE = 'VMEMINFO_NR_SWAPCACHE',
  VMEMINFO_NR_DIRTY_THRESHOLD = 'VMEMINFO_NR_DIRTY_THRESHOLD',
  VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD = 'VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD',
  VMEMINFO_PGPGIN = 'VMEMINFO_PGPGIN',
  VMEMINFO_PGPGOUT = 'VMEMINFO_PGPGOUT',
  VMEMINFO_PGPGOUTCLEAN = 'VMEMINFO_PGPGOUTCLEAN',
  VMEMINFO_PSWPIN = 'VMEMINFO_PSWPIN',
  VMEMINFO_PSWPOUT = 'VMEMINFO_PSWPOUT',
  VMEMINFO_PGALLOC_DMA = 'VMEMINFO_PGALLOC_DMA',
  VMEMINFO_PGALLOC_NORMAL = 'VMEMINFO_PGALLOC_NORMAL',
  VMEMINFO_PGALLOC_MOVABLE = 'VMEMINFO_PGALLOC_MOVABLE',
  VMEMINFO_PGFREE = 'VMEMINFO_PGFREE',
  VMEMINFO_PGACTIVATE = 'VMEMINFO_PGACTIVATE',
  VMEMINFO_PGDEACTIVATE = 'VMEMINFO_PGDEACTIVATE',
  VMEMINFO_PGFAULT = 'VMEMINFO_PGFAULT',
  VMEMINFO_PGMAJFAULT = 'VMEMINFO_PGMAJFAULT',
  VMEMINFO_PGREFILL_DMA = 'VMEMINFO_PGREFILL_DMA',
  VMEMINFO_PGREFILL_NORMAL = 'VMEMINFO_PGREFILL_NORMAL',
  VMEMINFO_PGREFILL_MOVABLE = 'VMEMINFO_PGREFILL_MOVABLE',
  VMEMINFO_PGSTEAL_KSWAPD_DMA = 'VMEMINFO_PGSTEAL_KSWAPD_DMA',
  VMEMINFO_PGSTEAL_KSWAPD_NORMAL = 'VMEMINFO_PGSTEAL_KSWAPD_NORMAL',
  VMEMINFO_PGSTEAL_KSWAPD_MOVABLE = 'VMEMINFO_PGSTEAL_KSWAPD_MOVABLE',
  VMEMINFO_PGSTEAL_DIRECT_DMA = 'VMEMINFO_PGSTEAL_DIRECT_DMA',
  VMEMINFO_PGSTEAL_DIRECT_NORMAL = 'VMEMINFO_PGSTEAL_DIRECT_NORMAL',
  VMEMINFO_PGSTEAL_DIRECT_MOVABLE = 'VMEMINFO_PGSTEAL_DIRECT_MOVABLE',
  VMEMINFO_PGSCAN_KSWAPD_DMA = 'VMEMINFO_PGSCAN_KSWAPD_DMA',
  VMEMINFO_PGSCAN_KSWAPD_NORMAL = 'VMEMINFO_PGSCAN_KSWAPD_NORMAL',
  VMEMINFO_PGSCAN_KSWAPD_MOVABLE = 'VMEMINFO_PGSCAN_KSWAPD_MOVABLE',
  VMEMINFO_PGSCAN_DIRECT_DMA = 'VMEMINFO_PGSCAN_DIRECT_DMA',
  VMEMINFO_PGSCAN_DIRECT_NORMAL = 'VMEMINFO_PGSCAN_DIRECT_NORMAL',
  VMEMINFO_PGSCAN_DIRECT_MOVABLE = 'VMEMINFO_PGSCAN_DIRECT_MOVABLE',
  VMEMINFO_PGSCAN_DIRECT_THROTTLE = 'VMEMINFO_PGSCAN_DIRECT_THROTTLE',
  VMEMINFO_PGINODESTEAL = 'VMEMINFO_PGINODESTEAL',
  VMEMINFO_SLABS_SCANNED = 'VMEMINFO_SLABS_SCANNED',
  VMEMINFO_KSWAPD_INODESTEAL = 'VMEMINFO_KSWAPD_INODESTEAL',
  VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY = 'VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY',
  VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY = 'VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY',
  VMEMINFO_PAGEOUTRUN = 'VMEMINFO_PAGEOUTRUN',
  VMEMINFO_ALLOCSTALL = 'VMEMINFO_ALLOCSTALL',
  VMEMINFO_PGROTATED = 'VMEMINFO_PGROTATED',
  VMEMINFO_DROP_PAGECACHE = 'VMEMINFO_DROP_PAGECACHE',
  VMEMINFO_DROP_SLAB = 'VMEMINFO_DROP_SLAB',
  VMEMINFO_PGMIGRATE_SUCCESS = 'VMEMINFO_PGMIGRATE_SUCCESS',
  VMEMINFO_PGMIGRATE_FAIL = 'VMEMINFO_PGMIGRATE_FAIL',
  VMEMINFO_COMPACT_MIGRATE_SCANNED = 'VMEMINFO_COMPACT_MIGRATE_SCANNED',
  VMEMINFO_COMPACT_FREE_SCANNED = 'VMEMINFO_COMPACT_FREE_SCANNED',
  VMEMINFO_COMPACT_ISOLATED = 'VMEMINFO_COMPACT_ISOLATED',
  VMEMINFO_COMPACT_STALL = 'VMEMINFO_COMPACT_STALL',
  VMEMINFO_COMPACT_FAIL = 'VMEMINFO_COMPACT_FAIL',
  VMEMINFO_COMPACT_SUCCESS = 'VMEMINFO_COMPACT_SUCCESS',
  VMEMINFO_COMPACT_DAEMON_WAKE = 'VMEMINFO_COMPACT_DAEMON_WAKE',
  VMEMINFO_UNEVICTABLE_PGS_CULLED = 'VMEMINFO_UNEVICTABLE_PGS_CULLED',
  VMEMINFO_UNEVICTABLE_PGS_SCANNED = 'VMEMINFO_UNEVICTABLE_PGS_SCANNED ',
  VMEMINFO_UNEVICTABLE_PGS_RESCUED = 'VMEMINFO_UNEVICTABLE_PGS_RESCUED',
  VMEMINFO_UNEVICTABLE_PGS_MLOCKED = 'VMEMINFO_UNEVICTABLE_PGS_MLOCKED',
  VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED = 'VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED',
  VMEMINFO_UNEVICTABLE_PGS_CLEARED = 'VMEMINFO_UNEVICTABLE_PGS_CLEARED',
  VMEMINFO_UNEVICTABLE_PGS_STRANDED = 'VMEMINFO_UNEVICTABLE_PGS_STRANDED',
  VMEMINFO_NR_ZSPAGES = 'VMEMINFO_NR_ZSPAGES',
  VMEMINFO_NR_ION_HEAP = 'VMEMINFO_NR_ION_HEAP',
  VMEMINFO_NR_GPU_HEAP = 'VMEMINFO_NR_GPU_HEAP',
  VMEMINFO_ALLOCSTALL_DMA = 'VMEMINFO_ALLOCSTALL_DMA',
  VMEMINFO_ALLOCSTALL_MOVABLE = 'VMEMINFO_ALLOCSTALL_MOVABLE',
  VMEMINFO_ALLOCSTALL_NORMAL = 'VMEMINFO_ALLOCSTALL_NORMAL',
  VMEMINFO_COMPACT_DAEMON_FREE_SCANNED = 'VMEMINFO_COMPACT_DAEMON_FREE_SCANNED',
  VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED = 'VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED',
  VMEMINFO_NR_FASTRPC = 'VMEMINFO_NR_FASTRPC',
  VMEMINFO_NR_INDIRECTLY_RECLAIMABLE = 'VMEMINFO_NR_INDIRECTLY_RECLAIMABLE',
  VMEMINFO_NR_ION_HEAP_POOL = 'VMEMINFO_NR_ION_HEAP_POOL',
  VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE = 'VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE',
  VMEMINFO_NR_SHADOW_CALL_STACK_BYTES = 'VMEMINFO_NR_SHADOW_CALL_STACK_BYTES',
  VMEMINFO_NR_SHMEM_HUGEPAGES = 'VMEMINFO_NR_SHMEM_HUGEPAGES',
  VMEMINFO_NR_SHMEM_PMDMAPPED = 'VMEMINFO_NR_SHMEM_PMDMAPPED',
  VMEMINFO_NR_UNRECLAIMABLE_PAGES = 'VMEMINFO_NR_UNRECLAIMABLE_PAGES',
  VMEMINFO_NR_ZONE_ACTIVE_ANON = 'VMEMINFO_NR_ZONE_ACTIVE_ANON',
  VMEMINFO_NR_ZONE_ACTIVE_FILE = 'VMEMINFO_NR_ZONE_ACTIVE_FILE',
  VMEMINFO_NR_ZONE_INACTIVE_ANON = 'VMEMINFO_NR_ZONE_INACTIVE_ANON',
  VMEMINFO_NR_ZONE_INACTIVE_FILE = 'VMEMINFO_NR_ZONE_INACTIVE_FILE',
  VMEMINFO_NR_ZONE_UNEVICTABLE = 'VMEMINFO_NR_ZONE_UNEVICTABLE',
  VMEMINFO_NR_ZONE_WRITE_PENDING = 'VMEMINFO_NR_ZONE_WRITE_PENDING',
  VMEMINFO_OOM_KILL = 'VMEMINFO_OOM_KILL ',
  VMEMINFO_PGLAZYFREE = 'VMEMINFO_PGLAZYFREE',
  VMEMINFO_PGLAZYFREED = 'VMEMINFO_PGLAZYFREED',
  VMEMINFO_PGREFILL = 'VMEMINFO_PGREFILL',
  VMEMINFO_PGSCAN_DIRECT = 'VMEMINFO_PGSCAN_DIRECT',
  VMEMINFO_PGSCAN_KSWAPD = 'VMEMINFO_PGSCAN_KSWAPD',
  VMEMINFO_PGSKIP_DMA = 'VMEMINFO_PGSKIP_DMA',
  VMEMINFO_PGSKIP_MOVABLE = 'VMEMINFO_PGSKIP_MOVABLE',
  VMEMINFO_PGSKIP_NORMAL = 'VMEMINFO_PGSKIP_NORMAL',
  VMEMINFO_PGSTEAL_DIRECT = 'VMEMINFO_PGSTEAL_DIRECT',
  VMEMINFO_PGSTEAL_KSWAPD = 'VMEMINFO_PGSTEAL_KSWAPD',
  VMEMINFO_SWAP_RA = 'VMEMINFO_SWAP_RA',
  VMEMINFO_SWAP_RA_HIT = 'VMEMINFO_SWAP_RA_HIT',
  VMEMINFO_WORKINGSET_RESTORE = 'VMEMINFO_WORKINGSET_RESTORE',
}

export enum SysMeminfoType {
  MEMINFO_UNSPECIFIED = 'PMEM_UNSPECIFIED',
  MEMINFO_MEM_TOTAL = 'PMEM_MEM_TOTAL',
  MEMINFO_MEM_FREE = 'PMEM_MEM_FREE',
  MEMINFO_MEM_AVAILABLE = 'PMEM_MEM_AVAILABLE',
  MEMINFO_BUFFERS = 'PMEM_BUFFERS',
  MEMINFO_CACHED = 'PMEM_CACHED',
  MEMINFO_SWAP_CACHED = 'PMEM_SWAP_CACHED',
  MEMINFO_ACTIVE = 'PMEM_ACTIVE',
  MEMINFO_INACTIVE = 'PMEM_INACTIVE',
  MEMINFO_ACTIVE_ANON = 'PMEM_ACTIVE_ANON',
  MEMINFO_INACTIVE_ANON = 'PMEM_INACTIVE_ANON',
  MEMINFO_ACTIVE_FILE = 'PMEM_ACTIVE_FILE',
  MEMINFO_INACTIVE_FILE = 'PMEM_INACTIVE_FILE',
  MEMINFO_UNEVICTABLE = 'PMEM_UNEVICTABLE',
  MEMINFO_MLOCKED = 'PMEM_MLOCKED',
  MEMINFO_SWAP_TOTAL = 'PMEM_SWAP_TOTAL',
  MEMINFO_SWAP_FREE = 'PMEM_SWAP_FREE',
  MEMINFO_DIRTY = 'PMEM_DIRTY',
  MEMINFO_WRITEBACK = 'PMEM_WRITEBACK',
  MEMINFO_ANON_PAGES = 'PMEM_ANON_PAGES',
  MEMINFO_MAPPED = 'PMEM_MAPPED',
  MEMINFO_SHMEM = 'PMEM_SHMEM',
  MEMINFO_SLAB = 'PMEM_SLAB',
  MEMINFO_SLAB_RECLAIMABLE = 'PMEM_SLAB_RECLAIMABLE',
  MEMINFO_SLAB_UNRECLAIMABLE = 'PMEM_SLAB_UNRECLAIMABLE',
  MEMINFO_KERNEL_STACK = 'PMEM_KERNEL_STACK',
  MEMINFO_PAGE_TABLES = 'PMEM_PAGE_TABLES',
  MEMINFO_COMMIT_LIMIT = 'PMEM_COMMIT_LIMIT',
  MEMINFO_COMMITED_AS = 'PMEM_COMMITED_AS',
  MEMINFO_VMALLOC_TOTAL = 'PMEM_VMALLOC_TOTAL',
  MEMINFO_VMALLOC_USED = 'PMEM_VMALLOC_USED',
  MEMINFO_VMALLOC_CHUNK = 'PMEM_VMALLOC_CHUNK',
  MEMINFO_CMA_TOTAL = 'PMEM_CMA_TOTAL',
  MEMINFO_CMA_FREE = 'PMEM_CMA_FREE',
  MEMINFO_KERNEL_RECLAIMABLE = 'PMEM_KERNEL_RECLAIMABLE',
  UNRECOGNIZED = 'UNRECOGNIZED',
}

export function sysMeminfoTypeFromJSON(object: any): SysMeminfoType {
  switch (object) {
    case 0:
    case 'MEMINFO_UNSPECIFIED':
      return SysMeminfoType.MEMINFO_UNSPECIFIED;
    case 1:
    case 'MEMINFO_MEM_TOTAL':
      return SysMeminfoType.MEMINFO_MEM_TOTAL;
    case 2:
    case 'MEMINFO_MEM_FREE':
      return SysMeminfoType.MEMINFO_MEM_FREE;
    case 3:
    case 'MEMINFO_MEM_AVAILABLE':
      return SysMeminfoType.MEMINFO_MEM_AVAILABLE;
    case 4:
    case 'MEMINFO_BUFFERS':
      return SysMeminfoType.MEMINFO_BUFFERS;
    case 5:
    case 'MEMINFO_CACHED':
      return SysMeminfoType.MEMINFO_CACHED;
    case 6:
    case 'MEMINFO_SWAP_CACHED':
      return SysMeminfoType.MEMINFO_SWAP_CACHED;
    case 7:
    case 'MEMINFO_ACTIVE':
      return SysMeminfoType.MEMINFO_ACTIVE;
    case 8:
    case 'MEMINFO_INACTIVE':
      return SysMeminfoType.MEMINFO_INACTIVE;
    case 9:
    case 'MEMINFO_ACTIVE_ANON':
      return SysMeminfoType.MEMINFO_ACTIVE_ANON;
    case 10:
    case 'MEMINFO_INACTIVE_ANON':
      return SysMeminfoType.MEMINFO_INACTIVE_ANON;
    case 11:
    case 'MEMINFO_ACTIVE_FILE':
      return SysMeminfoType.MEMINFO_ACTIVE_FILE;
    case 12:
    case 'MEMINFO_INACTIVE_FILE':
      return SysMeminfoType.MEMINFO_INACTIVE_FILE;
    case 13:
    case 'MEMINFO_UNEVICTABLE':
      return SysMeminfoType.MEMINFO_UNEVICTABLE;
    case 14:
    case 'MEMINFO_MLOCKED':
      return SysMeminfoType.MEMINFO_MLOCKED;
    case 15:
    case 'MEMINFO_SWAP_TOTAL':
      return SysMeminfoType.MEMINFO_SWAP_TOTAL;
    case 16:
    case 'MEMINFO_SWAP_FREE':
      return SysMeminfoType.MEMINFO_SWAP_FREE;
    case 17:
    case 'MEMINFO_DIRTY':
      return SysMeminfoType.MEMINFO_DIRTY;
    case 18:
    case 'MEMINFO_WRITEBACK':
      return SysMeminfoType.MEMINFO_WRITEBACK;
    case 19:
    case 'MEMINFO_ANON_PAGES':
      return SysMeminfoType.MEMINFO_ANON_PAGES;
    case 20:
    case 'MEMINFO_MAPPED':
      return SysMeminfoType.MEMINFO_MAPPED;
    case 21:
    case 'MEMINFO_SHMEM':
      return SysMeminfoType.MEMINFO_SHMEM;
    case 22:
    case 'MEMINFO_SLAB':
      return SysMeminfoType.MEMINFO_SLAB;
    case 23:
    case 'MEMINFO_SLAB_RECLAIMABLE':
      return SysMeminfoType.MEMINFO_SLAB_RECLAIMABLE;
    case 24:
    case 'MEMINFO_SLAB_UNRECLAIMABLE':
      return SysMeminfoType.MEMINFO_SLAB_UNRECLAIMABLE;
    case 25:
    case 'MEMINFO_KERNEL_STACK':
      return SysMeminfoType.MEMINFO_KERNEL_STACK;
    case 26:
    case 'MEMINFO_PAGE_TABLES':
      return SysMeminfoType.MEMINFO_PAGE_TABLES;
    case 27:
    case 'MEMINFO_COMMIT_LIMIT':
      return SysMeminfoType.MEMINFO_COMMIT_LIMIT;
    case 28:
    case 'MEMINFO_COMMITED_AS':
      return SysMeminfoType.MEMINFO_COMMITED_AS;
    case 29:
    case 'MEMINFO_VMALLOC_TOTAL':
      return SysMeminfoType.MEMINFO_VMALLOC_TOTAL;
    case 30:
    case 'MEMINFO_VMALLOC_USED':
      return SysMeminfoType.MEMINFO_VMALLOC_USED;
    case 31:
    case 'MEMINFO_VMALLOC_CHUNK':
      return SysMeminfoType.MEMINFO_VMALLOC_CHUNK;
    case 32:
    case 'MEMINFO_CMA_TOTAL':
      return SysMeminfoType.MEMINFO_CMA_TOTAL;
    case 33:
    case 'MEMINFO_CMA_FREE':
      return SysMeminfoType.MEMINFO_CMA_FREE;
    case 34:
    case 'MEMINFO_KERNEL_RECLAIMABLE':
      return SysMeminfoType.MEMINFO_KERNEL_RECLAIMABLE;
    case -1:
    case 'UNRECOGNIZED':
    default:
      return SysMeminfoType.UNRECOGNIZED;
  }
}

export enum Type {
  TYPE_UNSPECIFIED = 0,
  HI3516 = 1,
  P40 = 2,
  UNRECOGNIZED = -1,
}

export interface HilogConfig {
  deviceType: Type;
  logLevel: Level;
  needClear: boolean;
}

export function levelFromJSON(object: any): Level {
  switch (object) {
    case 0:
    case 'LEVEL_UNSPECIFIED':
      return Level.LOG_UNSPECIFIED;
    case 1:
    case 'Error':
      return Level.LOG_ERROR;
    case 2:
    case 'Info':
      return Level.LOG_INFO;
    case 3:
    case 'Debug':
      return Level.LOG_DEBUG;
    case 4:
    case 'Warns':
      return Level.LOG_WARN;
    case -1:
    case 'UNRECOGNIZED':
    default:
      return Level.UNRECOGNIZED;
  }
}

export enum Level {
  LOG_UNSPECIFIED = 'LOG_UNSPECIFIED',
  LOG_ERROR = 'LOG_ERROR',
  LOG_INFO = 'LOG_INFO',
  LOG_DEBUG = 'LOG_DEBUG',
  LOG_WARN = 'LOG_WARN',
  UNRECOGNIZED = -1,
}

export interface NativeHookConfig {
  pid: number;
  saveFile: boolean;
  fileName: string;
  filterSize: number;
  smbPages: number;
  maxStackDepth: number;
  processName: string;
  stringCompressed: boolean;
  fpUnwind: boolean;
  blocked: boolean;
  recordAccurately?: boolean;
  offlineSymbolization?: boolean;
  callframeCompress?: boolean;
  startupMode?: boolean;
  statisticsInterval?: number;
}

export interface FpsConfig {
  reportFps: boolean;
}

export interface ProcessConfig {
  report_process_tree: boolean;
  report_cpu: boolean;
  report_diskio: boolean;
  report_pss: boolean;
}

export interface CpuConfig {
  pid: number;
  reportProcessInfo: boolean;
}

enum IoReportType {
  UNSPECIFIED = 'UNSPECIFIED',
  IO_REPORT = 'IO_REPORT',
  IO_REPORT_EX = 'IO_REPORT_EX',
}

export interface DiskioConfig {
  reportIoStats: string;
}

export interface NetworkConfig {
}

export interface HiperfPluginConfig {
  isRoot: boolean;
  outfileName: string;
  recordArgs: string;
}

export interface HiSystemEventConfig {
  msg: string;
  processName: string;
}

export interface ArkTSConfig {
  pid: number;
  type: number;
  interval: number;
  capture_numeric_value: boolean;
  track_allocations: boolean;
  enable_cpu_profiler: boolean;
  cpu_profiler_interval: number;
}
