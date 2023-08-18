/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MEMORY_DATA_PLUGIN_H
#define MEMORY_DATA_PLUGIN_H

#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <iomanip>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>

#include "logging.h"
#include "memory_plugin_config.pb.h"
#include "memory_plugin_result.pb.h"
#include "smaps_stats.h"

struct Proto2StrMapping {
    int protobufid;
    const char* procstr;
};

constexpr Proto2StrMapping meminfoMapping[] = {
    {SysMeminfoType::PMEM_UNSPECIFIED, "MemUnspecified"},
    {SysMeminfoType::PMEM_MEM_TOTAL, "MemTotal"},
    {SysMeminfoType::PMEM_MEM_FREE, "MemFree"},
    {SysMeminfoType::PMEM_MEM_AVAILABLE, "MemAvailable"},
    {SysMeminfoType::PMEM_BUFFERS, "Buffers"},
    {SysMeminfoType::PMEM_CACHED, "Cached"},
    {SysMeminfoType::PMEM_SWAP_CACHED, "SwapCached"},
    {SysMeminfoType::PMEM_ACTIVE, "Active"},
    {SysMeminfoType::PMEM_INACTIVE, "Inactive"},
    {SysMeminfoType::PMEM_ACTIVE_ANON, "Active(anon)"},
    {SysMeminfoType::PMEM_INACTIVE_ANON, "Inactive(anon)"},
    {SysMeminfoType::PMEM_ACTIVE_FILE, "Active(file)"},
    {SysMeminfoType::PMEM_INACTIVE_FILE, "Inactive(file)"},
    {SysMeminfoType::PMEM_UNEVICTABLE, "Unevictable"},
    {SysMeminfoType::PMEM_MLOCKED, "Mlocked"},
    {SysMeminfoType::PMEM_SWAP_TOTAL, "SwapTotal"},
    {SysMeminfoType::PMEM_SWAP_FREE, "SwapFree"},
    {SysMeminfoType::PMEM_DIRTY, "Dirty"},
    {SysMeminfoType::PMEM_WRITEBACK, "Writeback"},
    {SysMeminfoType::PMEM_ANON_PAGES, "AnonPages"},
    {SysMeminfoType::PMEM_MAPPED, "Mapped"},
    {SysMeminfoType::PMEM_SHMEM, "Shmem"},
    {SysMeminfoType::PMEM_SLAB, "Slab"},
    {SysMeminfoType::PMEM_SLAB_RECLAIMABLE, "SReclaimable"},
    {SysMeminfoType::PMEM_SLAB_UNRECLAIMABLE, "SUnreclaim"},
    {SysMeminfoType::PMEM_KERNEL_STACK, "KernelStack"},
    {SysMeminfoType::PMEM_PAGE_TABLES, "PageTables"},
    {SysMeminfoType::PMEM_COMMIT_LIMIT, "CommitLimit"},
    {SysMeminfoType::PMEM_COMMITED_AS, "Committed_AS"},
    {SysMeminfoType::PMEM_VMALLOC_TOTAL, "VmallocTotal"},
    {SysMeminfoType::PMEM_VMALLOC_USED, "VmallocUsed"},
    {SysMeminfoType::PMEM_VMALLOC_CHUNK, "VmallocChunk"},
    {SysMeminfoType::PMEM_CMA_TOTAL, "CmaTotal"},
    {SysMeminfoType::PMEM_CMA_FREE, "CmaFree"},
    {SysMeminfoType::PMEM_KERNEL_RECLAIMABLE, "KReclaimable"},
};

constexpr Proto2StrMapping vmeminfoMapping[] = {
    {SysVMeminfoType::VMEMINFO_UNSPECIFIED, "unspecified"},
    {SysVMeminfoType::VMEMINFO_NR_FREE_PAGES, "nr_free_pages"},
    {SysVMeminfoType::VMEMINFO_NR_ALLOC_BATCH, "nr_alloc_batch"},
    {SysVMeminfoType::VMEMINFO_NR_INACTIVE_ANON, "nr_inactive_anon"},
    {SysVMeminfoType::VMEMINFO_NR_ACTIVE_ANON, "nr_active_anon"},
    {SysVMeminfoType::VMEMINFO_NR_INACTIVE_FILE, "nr_inactive_file"},
    {SysVMeminfoType::VMEMINFO_NR_ACTIVE_FILE, "nr_active_file"},
    {SysVMeminfoType::VMEMINFO_NR_UNEVICTABLE, "nr_unevictable"},
    {SysVMeminfoType::VMEMINFO_NR_MLOCK, "nr_mlock"},
    {SysVMeminfoType::VMEMINFO_NR_ANON_PAGES, "nr_anon_pages"},
    {SysVMeminfoType::VMEMINFO_NR_MAPPED, "nr_mapped"},
    {SysVMeminfoType::VMEMINFO_NR_FILE_PAGES, "nr_file_pages"},
    {SysVMeminfoType::VMEMINFO_NR_DIRTY, "nr_dirty"},
    {SysVMeminfoType::VMEMINFO_NR_WRITEBACK, "nr_writeback"},
    {SysVMeminfoType::VMEMINFO_NR_SLAB_RECLAIMABLE, "nr_slab_reclaimable"},
    {SysVMeminfoType::VMEMINFO_NR_SLAB_UNRECLAIMABLE, "nr_slab_unreclaimable"},
    {SysVMeminfoType::VMEMINFO_NR_PAGE_TABLE_PAGES, "nr_page_table_pages"},
    {SysVMeminfoType::VMEMINFO_NR_KERNEL_STACK, "nr_kernel_stack"},
    {SysVMeminfoType::VMEMINFO_NR_OVERHEAD, "nr_overhead"},
    {SysVMeminfoType::VMEMINFO_NR_UNSTABLE, "nr_unstable"},
    {SysVMeminfoType::VMEMINFO_NR_BOUNCE, "nr_bounce"},
    {SysVMeminfoType::VMEMINFO_NR_VMSCAN_WRITE, "nr_vmscan_write"},
    {SysVMeminfoType::VMEMINFO_NR_VMSCAN_IMMEDIATE_RECLAIM, "nr_vmscan_immediate_reclaim"},
    {SysVMeminfoType::VMEMINFO_NR_WRITEBACK_TEMP, "nr_writeback_temp"},
    {SysVMeminfoType::VMEMINFO_NR_ISOLATED_ANON, "nr_isolated_anon"},
    {SysVMeminfoType::VMEMINFO_NR_ISOLATED_FILE, "nr_isolated_file"},
    {SysVMeminfoType::VMEMINFO_NR_SHMEM, "nr_shmem"},
    {SysVMeminfoType::VMEMINFO_NR_DIRTIED, "nr_dirtied"},
    {SysVMeminfoType::VMEMINFO_NR_WRITTEN, "nr_written"},
    {SysVMeminfoType::VMEMINFO_NR_PAGES_SCANNED, "nr_pages_scanned"},
    {SysVMeminfoType::VMEMINFO_WORKINGSET_REFAULT, "workingset_refault"},
    {SysVMeminfoType::VMEMINFO_WORKINGSET_ACTIVATE, "workingset_activate"},
    {SysVMeminfoType::VMEMINFO_WORKINGSET_NODERECLAIM, "workingset_nodereclaim"},
    {SysVMeminfoType::VMEMINFO_NR_ANON_TRANSPARENT_HUGEPAGES, "nr_anon_transparent_hugepages"},
    {SysVMeminfoType::VMEMINFO_NR_FREE_CMA, "nr_free_cma"},
    {SysVMeminfoType::VMEMINFO_NR_SWAPCACHE, "nr_swapcache"},
    {SysVMeminfoType::VMEMINFO_NR_DIRTY_THRESHOLD, "nr_dirty_threshold"},
    {SysVMeminfoType::VMEMINFO_NR_DIRTY_BACKGROUND_THRESHOLD, "nr_dirty_background_threshold"},
    {SysVMeminfoType::VMEMINFO_PGPGIN, "pgpgin"},
    {SysVMeminfoType::VMEMINFO_PGPGOUT, "pgpgout"},
    {SysVMeminfoType::VMEMINFO_PGPGOUTCLEAN, "pgpgoutclean"},
    {SysVMeminfoType::VMEMINFO_PSWPIN, "pswpin"},
    {SysVMeminfoType::VMEMINFO_PSWPOUT, "pswpou"},
    {SysVMeminfoType::VMEMINFO_PGALLOC_DMA, "pgalloc_dma"},
    {SysVMeminfoType::VMEMINFO_PGALLOC_NORMAL, "pgalloc_normal"},
    {SysVMeminfoType::VMEMINFO_PGALLOC_MOVABLE, "pgalloc_movable"},
    {SysVMeminfoType::VMEMINFO_PGFREE, "pgfree"},
    {SysVMeminfoType::VMEMINFO_PGACTIVATE, "pgactivate"},
    {SysVMeminfoType::VMEMINFO_PGDEACTIVATE, "pgdeactivate"},
    {SysVMeminfoType::VMEMINFO_PGFAULT, "pgfault"},
    {SysVMeminfoType::VMEMINFO_PGMAJFAULT, "pgmajfault"},
    {SysVMeminfoType::VMEMINFO_PGREFILL_DMA, "pgrefill_dma"},
    {SysVMeminfoType::VMEMINFO_PGREFILL_NORMAL, "pgrefill_normal"},
    {SysVMeminfoType::VMEMINFO_PGREFILL_MOVABLE, "pgrefill_movable"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_KSWAPD_DMA, "pgsteal_kswapd_dma"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_KSWAPD_NORMAL, "pgsteal_kswapd_normal"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_KSWAPD_MOVABLE, "pgsteal_kswapd_movable"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_DIRECT_DMA, "pgsteal_direct_dma"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_DIRECT_NORMAL, "pgsteal_direct_normal"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_DIRECT_MOVABLE, "pgsteal_direct_movable"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_KSWAPD_DMA, "pgscan_kswapd_dma"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_KSWAPD_NORMAL, "pgscan_kswapd_normal"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_KSWAPD_MOVABLE, "pgscan_kswapd_movable"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_DIRECT_DMA, "pgscan_direct_dma"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_DIRECT_NORMAL, "pgscan_direct_normal"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_DIRECT_MOVABLE, "pgscan_direct_movable"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_DIRECT_THROTTLE, "pgscan_direct_throttle"},
    {SysVMeminfoType::VMEMINFO_PGINODESTEAL, "pginodesteal"},
    {SysVMeminfoType::VMEMINFO_SLABS_SCANNED, "slabs_scanned"},
    {SysVMeminfoType::VMEMINFO_KSWAPD_INODESTEAL, "kswapd_inodesteal"},
    {SysVMeminfoType::VMEMINFO_KSWAPD_LOW_WMARK_HIT_QUICKLY, "kswapd_low_wmark_hit_quickly"},
    {SysVMeminfoType::VMEMINFO_KSWAPD_HIGH_WMARK_HIT_QUICKLY, "kswapd_high_wmark_hit_quickly"},
    {SysVMeminfoType::VMEMINFO_PAGEOUTRUN, "pageoutrun"},
    {SysVMeminfoType::VMEMINFO_ALLOCSTALL, "allocstall"},
    {SysVMeminfoType::VMEMINFO_PGROTATED, "pgrotated"},
    {SysVMeminfoType::VMEMINFO_DROP_PAGECACHE, "drop_pagecache"},
    {SysVMeminfoType::VMEMINFO_DROP_SLAB, "drop_slab"},
    {SysVMeminfoType::VMEMINFO_PGMIGRATE_SUCCESS, "pgmigrate_success"},
    {SysVMeminfoType::VMEMINFO_PGMIGRATE_FAIL, "pgmigrate_fail"},
    {SysVMeminfoType::VMEMINFO_COMPACT_MIGRATE_SCANNED, "compact_migrate_scanned"},
    {SysVMeminfoType::VMEMINFO_COMPACT_FREE_SCANNED, "compact_free_scanned"},
    {SysVMeminfoType::VMEMINFO_COMPACT_ISOLATED, "compact_isolated"},
    {SysVMeminfoType::VMEMINFO_COMPACT_STALL, "compact_stall"},
    {SysVMeminfoType::VMEMINFO_COMPACT_FAIL, "compact_fail"},
    {SysVMeminfoType::VMEMINFO_COMPACT_SUCCESS, "compact_success"},
    {SysVMeminfoType::VMEMINFO_COMPACT_DAEMON_WAKE, "compact_daemon_wake"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_CULLED, "unevictable_pgs_culled"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_SCANNED, "unevictable_pgs_scanned"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_RESCUED, "unevictable_pgs_rescued"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_MLOCKED, "unevictable_pgs_mlocked"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_MUNLOCKED, "unevictable_pgs_munlocked"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_CLEARED, "unevictable_pgs_cleared"},
    {SysVMeminfoType::VMEMINFO_UNEVICTABLE_PGS_STRANDED, "unevictable_pgs_stranded"},
    {SysVMeminfoType::VMEMINFO_NR_ZSPAGES, "nr_zspages"},
    {SysVMeminfoType::VMEMINFO_NR_ION_HEAP, "nr_ion_heap"},
    {SysVMeminfoType::VMEMINFO_NR_GPU_HEAP, "nr_gpu_heap"},
    {SysVMeminfoType::VMEMINFO_ALLOCSTALL_DMA, "allocstall_dma"},
    {SysVMeminfoType::VMEMINFO_ALLOCSTALL_MOVABLE, "allocstall_movable"},
    {SysVMeminfoType::VMEMINFO_ALLOCSTALL_NORMAL, "allocstall_normal"},
    {SysVMeminfoType::VMEMINFO_COMPACT_DAEMON_FREE_SCANNED, "compact_daemon_free_scanned"},
    {SysVMeminfoType::VMEMINFO_COMPACT_DAEMON_MIGRATE_SCANNED, "compact_daemon_migrate_scanned"},
    {SysVMeminfoType::VMEMINFO_NR_FASTRPC, "nr_fastrpc"},
    {SysVMeminfoType::VMEMINFO_NR_INDIRECTLY_RECLAIMABLE, "nr_indirectly_reclaimable"},
    {SysVMeminfoType::VMEMINFO_NR_ION_HEAP_POOL, "nr_ion_heap_pool"},
    {SysVMeminfoType::VMEMINFO_NR_KERNEL_MISC_RECLAIMABLE, "nr_kernel_misc_reclaimable"},
    {SysVMeminfoType::VMEMINFO_NR_SHADOW_CALL_STACK_BYTES, "nr_shadow_call_stack_bytes"},
    {SysVMeminfoType::VMEMINFO_NR_SHMEM_HUGEPAGES, "nr_shmem_hugepages"},
    {SysVMeminfoType::VMEMINFO_NR_SHMEM_PMDMAPPED, "nr_shmem_pmdmapped"},
    {SysVMeminfoType::VMEMINFO_NR_UNRECLAIMABLE_PAGES, "nr_unreclaimable_pages"},
    {SysVMeminfoType::VMEMINFO_NR_ZONE_ACTIVE_ANON, "nr_zone_active_anon"},
    {SysVMeminfoType::VMEMINFO_NR_ZONE_ACTIVE_FILE, "nr_zone_active_file"},
    {SysVMeminfoType::VMEMINFO_NR_ZONE_INACTIVE_ANON, "nr_zone_inactive_anon"},
    {SysVMeminfoType::VMEMINFO_NR_ZONE_INACTIVE_FILE, "nr_zone_inactive_file"},
    {SysVMeminfoType::VMEMINFO_NR_ZONE_UNEVICTABLE, "nr_zone_unevictable"},
    {SysVMeminfoType::VMEMINFO_NR_ZONE_WRITE_PENDING, "nr_zone_write_pending"},
    {SysVMeminfoType::VMEMINFO_OOM_KILL, "oom_kill"},
    {SysVMeminfoType::VMEMINFO_PGLAZYFREE, "pglazyfree"},
    {SysVMeminfoType::VMEMINFO_PGLAZYFREED, "pglazyfreed"},
    {SysVMeminfoType::VMEMINFO_PGREFILL, "pgrefill"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_DIRECT, "pgscan_direct"},
    {SysVMeminfoType::VMEMINFO_PGSCAN_KSWAPD, "pgscan_kswapd"},
    {SysVMeminfoType::VMEMINFO_PGSKIP_DMA, "pgskip_dma"},
    {SysVMeminfoType::VMEMINFO_PGSKIP_MOVABLE, "pgskip_movable"},
    {SysVMeminfoType::VMEMINFO_PGSKIP_NORMAL, "pgskip_normal"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_DIRECT, "pgsteal_direct"},
    {SysVMeminfoType::VMEMINFO_PGSTEAL_KSWAPD, "pgsteal_kswapd"},
    {SysVMeminfoType::VMEMINFO_SWAP_RA, "swap_ra"},
    {SysVMeminfoType::VMEMINFO_SWAP_RA_HIT, "swap_ra_hit"},
    {SysVMeminfoType::VMEMINFO_WORKINGSET_RESTORE, "workingset_restore"},
};

struct ProcStatusMapping {
    int procid;
    const char* procstr;
};

enum StatusType {
    PRO_TGID = 1,
    PRO_NAME,
    PRO_VMSIZE,
    PRO_VMRSS,
    PRO_RSSANON,
    PRO_RSSFILE,
    PRO_RSSSHMEM,
    PRO_VMSWAP,
    PRO_VMLCK,
    PRO_VMHWM,
};

constexpr ProcStatusMapping procStatusMapping[] = {
    {StatusType::PRO_TGID, "Tgid"},         {StatusType::PRO_NAME, "Name"},       {StatusType::PRO_VMSIZE, "VmSize"},
    {StatusType::PRO_VMRSS, "VmRSS"},       {StatusType::PRO_RSSANON, "RssAnon"}, {StatusType::PRO_RSSFILE, "RssFile"},
    {StatusType::PRO_RSSSHMEM, "RssShmem"}, {StatusType::PRO_VMSWAP, "VmSwap"},   {StatusType::PRO_VMLCK, "VmLck"},
    {StatusType::PRO_VMHWM, "VmHWM"},
};

enum ErrorType {
    RET_NULL_ADDR,
    RET_IVALID_PID,
    RET_TGID_VALUE_NULL,
    RET_FAIL = -1,
    RET_SUCC = 0,
};

enum FileType {
    FILE_STATUS = 0,
    FILE_OOM,
    FILE_SMAPS,
};

struct ProcfdMapping {
    int procid;
    const char* file;
};

constexpr ProcfdMapping procfdMapping[] = {
    {FileType::FILE_STATUS, "status"},
    {FileType::FILE_OOM, "oom_score_adj"},
    {FileType::FILE_SMAPS, "smaps"},
};

class MemoryDataPlugin {
public:
    MemoryDataPlugin();
    ~MemoryDataPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* configData, uint32_t configSize);
    int Stop();
    void SetPath(char* path)
    {
        testpath_ = path;
    };
    void WriteProcesseList(MemoryData& data);
    void WriteProcinfoByPidfds(ProcessMemoryInfo* processinfo, int32_t pid);
    DIR* OpenDestDir(const char* dirPath);
    int32_t GetValidPid(DIR* dirp);
    // for test change static
    int ParseNumber(std::string line);

private:
    /* data */
    MemoryConfig protoConfig_;

    std::unique_ptr<uint8_t[]> buffer_;

    int meminfoFd_;
    int vmstatFd_;
    std::map<std::string, int> meminfoCounters_;
    std::map<std::string, int> vmstatCounters_;

    void InitProto2StrVector();
    std::vector<const char*> meminfoStrList_;
    std::vector<const char*> vmstatStrList_;
    // SmapsStats *
    void WriteVmstat(MemoryData& data);
    void WriteMeminfo(MemoryData& data);
    void WriteZramData(MemoryData& data);

    std::unordered_map<int32_t, std::vector<int>> pidFds_;
    std::vector<int32_t> seenPids_;
    char* testpath_;
    int32_t err_;
    int32_t ReadFile(int fd);
    std::string ReadFile(const std::string& path);
    std::vector<int> OpenProcPidFiles(int32_t pid);
    int32_t ReadProcPidFile(int32_t pid, const char* pFileName);
    void WriteProcessInfo(MemoryData& data, int32_t pid);
    void WriteOomInfo(ProcessMemoryInfo* processinfo, int32_t pid);
    void WriteProcess(ProcessMemoryInfo* processinfo, const char* pFile, uint32_t fileLen, int32_t pid);
    void WriteAppsummary(ProcessMemoryInfo* processinfo, SmapsStats& smapInfo);
    void SetProcessInfo(ProcessMemoryInfo* processinfo, int key, const char* word);
    bool StringToUll(const char* word, uint64_t& value);

    bool BufnCmp(const char* src, int srcLen, const char* key, int keyLen);
    bool addPidBySort(int32_t pid);
    int GetProcStatusId(const char* src, int srcLen);

    bool ParseMemInfo(const char* data, ProcessMemoryInfo* memoryInfo);
    bool GetMemInfoByMemoryService(uint32_t pid, ProcessMemoryInfo* memoryInfo);
    int InitMemVmemFd();
};

#endif
