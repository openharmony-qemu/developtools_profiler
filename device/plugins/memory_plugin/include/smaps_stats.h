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

#ifndef SMAPS_STATS_H
#define SMAPS_STATS_H

#include <array>
#include <cinttypes>
#include <cstdio>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sys/mman.h>

#include "logging.h"
#include "memory_plugin_result.pb.h"

struct MemUsageInfo {
    uint64_t vss;
    uint64_t rss;
    uint64_t pss;
    uint64_t uss;

    uint64_t swap;
    uint64_t swapPss;

    uint64_t privateClean;
    uint64_t privateDirty;
    uint64_t sharedClean;
    uint64_t sharedDirty;
};

class StatsInfo {
public:
    int pss_;
    int swappablePss_;
    int rss_;
    int privateDirty_;
    int sharedDirty_;
    int privateClean_;
    int sharedClean_;
    int swappedOut_;
    int swappedOutPss_;

    StatsInfo()
        : pss_(0),
        swappablePss_(0),
        rss_(0),
        privateDirty_(0),
        sharedDirty_(0),
        privateClean_(0),
        sharedClean_(0),
        swappedOut_(0),
        swappedOutPss_(0)
    {
    }

    StatsInfo(uint64_t swapablePss, const MemUsageInfo& usage)
        : pss_(usage.pss),
        swappablePss_(swapablePss),
        rss_(usage.rss),
        privateDirty_(usage.privateDirty),
        sharedDirty_(usage.sharedDirty),
        privateClean_(usage.privateClean),
        sharedClean_(usage.sharedClean),
        swappedOut_(usage.swap),
        swappedOutPss_(usage.swapPss)
    {
    }
    ~StatsInfo() {}

    void operator+=(const StatsInfo &S)
    {
        pss_ += S.pss_;
        swappablePss_ += S.swappablePss_;
        rss_ += S.rss_;
        privateDirty_ += S.privateDirty_;
        sharedDirty_ += S.sharedDirty_;
        privateClean_ += S.privateClean_;
        sharedClean_ += S.sharedClean_;
        swappedOut_ += S.swappedOut_;
        swappedOutPss_ += S.swappedOutPss_;
    }
};

enum NumType {
    FIFTH_FIELD = 5,
    HEX_BASE = 16,
    DEC_BASE = 10,
};

struct MapPiecesInfo {
    uint64_t startAddr;
    uint64_t endAddr;

    std::string name;
};

struct SmapsHeadInfo {
    std::string startAddrStr;
    std::string endAddrStr;
    std::string permission;
    std::string path;
    int64_t iNode = -1;
};

enum VmemifoType {
    VMHEAP_NULL = -2,
    VMHEAP_NEEDFIX = -1,
    VMHEAP_UNKNOWN,
    VMHEAP_SENSITIVE_VM,
    VMHEAP_NATIVE,

    VMHEAP_SENSITIVE_VM_OTHER,
    VMHEAP_STACK,
    VMHEAP_CURSOR,
    VMHEAP_ASHMEM,
    VMHEAP_GL_DEV,
    VMHEAP_UNKNOWN_DEV,
    VMHEAP_SO,
    VMHEAP_JAR,
    VMHEAP_TTF,
    VMHEAP_SENSITIVE_JVBIN,
    VMHEAP_OAT,
    VMHEAP_HRT,
    VMHEAP_UNKNOWN_MAP,
    VMHEAP_GRAPHICS,
    VMHEAP_GL,
    VMHEAP_OTHER_MEMTRACK,

    // extra sections (heap).
    VMHEAP_SENSITIVE_VM_NORMAL,
    VMHEAP_SENSITIVE_VM_LARGE,
    VMHEAP_SENSITIVE_VM_ZYGOTE,
    VMHEAP_SENSITIVE_VM_NON_MOVING,

    // other extra sections.
    VMHEAP_SENSITIVE_VM_OTHER_LINEARALLOC,
    VMHEAP_SENSITIVE_VM_OTHER_ACCOUNTING,
    VMHEAP_SENSITIVE_VM_OTHER_ZYGOTE_CODE_CACHE,
    VMHEAP_SENSITIVE_VM_OTHER_APP_CODE_CACHE,
    VMHEAP_SENSITIVE_VM_OTHER_COMPILER_METADATA,
    VMHEAP_SENSITIVE_VM_OTHER_INDIRECT_REFERENCE_TABLE,

    VMHEAP_SENSITIVE_JVBIN_BOOT_VDEX,
    VMHEAP_SENSITIVE_JVBIN_APP_SENSITIVE_JVBIN,
    VMHEAP_SENSITIVE_JVBIN_APP_VDEX,

    // App hrt, boot hrt.
    VMHEAP_HRT_APP,
    VMHEAP_HRT_BOOT,

    VMHEAP_NUM_HEAP,
    VMHEAP_NUM_EXCLUSIVE_HEAP = VMHEAP_OTHER_MEMTRACK + 1,
    VMHEAP_NUM_CORE_HEAP = VMHEAP_NATIVE + 1
};

enum OpsType {
    OPS_START = 1,
    OPS_END,
};

struct VmeminfoAreaMapping {
    int ops;
    const char* heapstr;
    int heapid[2];
};

constexpr VmeminfoAreaMapping g_vmaMemHeap[] = {
    {OpsType::OPS_START, "[heap]", {VmemifoType::VMHEAP_NATIVE, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "[stack", {VmemifoType::VMHEAP_STACK, VmemifoType::VMHEAP_NULL}},
};

// [anon:
constexpr VmeminfoAreaMapping g_vmaMemAnon[] = {
    {OpsType::OPS_START, "[anon:libc_malloc]", {VmemifoType::VMHEAP_NATIVE, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "[anon:native_heap:", {VmemifoType::VMHEAP_NATIVE, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "[anon:scudo:", {VmemifoType::VMHEAP_NATIVE, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "[anon:GWP-ASan", {VmemifoType::VMHEAP_NATIVE, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "[anon:stack_and_tls:", {VmemifoType::VMHEAP_STACK, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-LinearAlloc",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_LINEARALLOC}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-alloc space", {VmemifoType::VMHEAP_SENSITIVE_VM, VmemifoType::VMHEAP_SENSITIVE_VM_NORMAL}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-main space", {VmemifoType::VMHEAP_SENSITIVE_VM, VmemifoType::VMHEAP_SENSITIVE_VM_NORMAL}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-large object space",
     {VmemifoType::VMHEAP_SENSITIVE_VM, VmemifoType::VMHEAP_SENSITIVE_VM_LARGE}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-free list large object space",
     {VmemifoType::VMHEAP_SENSITIVE_VM, VmemifoType::VMHEAP_SENSITIVE_VM_LARGE}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-non moving space",
     {VmemifoType::VMHEAP_SENSITIVE_VM, VmemifoType::VMHEAP_SENSITIVE_VM_NON_MOVING}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-zygote space", {VmemifoType::VMHEAP_SENSITIVE_VM, VmemifoType::VMHEAP_SENSITIVE_VM_ZYGOTE}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-indirect ref",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_INDIRECT_REFERENCE_TABLE}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-jit-code-cache",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_APP_CODE_CACHE}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-data-code-cache",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_APP_CODE_CACHE}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-CompilerMetadata",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_COMPILER_METADATA}},
    {OpsType::OPS_START,
     "[anon:sensitive_vm-",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_ACCOUNTING}},
    {OpsType::OPS_START, "[anon:", {VmemifoType::VMHEAP_UNKNOWN, VmemifoType::VMHEAP_NULL}},
};

constexpr VmeminfoAreaMapping g_vmaMemFd[] = {
    {OpsType::OPS_START,
     "/memfd:jit-cache",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_APP_CODE_CACHE}},
    {OpsType::OPS_START,
     "/memfd:jit-zygote-cache",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_ZYGOTE_CODE_CACHE}},
};
// dev
constexpr VmeminfoAreaMapping g_vmaMemDev[] = {
    {OpsType::OPS_START, "/dev/kgsl-3d0", {VmemifoType::VMHEAP_GL_DEV, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "/dev/ashmem/CursorWindow", {VmemifoType::VMHEAP_CURSOR, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START,
     "/dev/ashmem/jit-zygote-cache",
     {VmemifoType::VMHEAP_SENSITIVE_VM_OTHER, VmemifoType::VMHEAP_SENSITIVE_VM_OTHER_ZYGOTE_CODE_CACHE}},
    {OpsType::OPS_START, "/dev/ashmem", {VmemifoType::VMHEAP_ASHMEM, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_START, "/dev/", {VmemifoType::VMHEAP_UNKNOWN_DEV, VmemifoType::VMHEAP_NULL}},
};

constexpr VmeminfoAreaMapping g_vmaMemSuffix[] = {
    {OpsType::OPS_END, ".so", {VmemifoType::VMHEAP_SO, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_END, ".so.1", {VmemifoType::VMHEAP_SO, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_END, ".jar", {VmemifoType::VMHEAP_JAR, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_END, ".ttf", {VmemifoType::VMHEAP_TTF, VmemifoType::VMHEAP_NULL}},
    {OpsType::OPS_END, ".oat", {VmemifoType::VMHEAP_OAT, VmemifoType::VMHEAP_NULL}},

    {OpsType::OPS_END,
     ".odex", {VmemifoType::VMHEAP_SENSITIVE_JVBIN, VmemifoType::VMHEAP_SENSITIVE_JVBIN_APP_SENSITIVE_JVBIN}},

    {OpsType::OPS_END, ".vdex", {VmemifoType::VMHEAP_SENSITIVE_JVBIN, VmemifoType::VMHEAP_NEEDFIX}},
    {OpsType::OPS_END, ".hrt", {VmemifoType::VMHEAP_HRT, VmemifoType::VMHEAP_NEEDFIX}},
    {OpsType::OPS_END, ".hrt]", {VmemifoType::VMHEAP_HRT, VmemifoType::VMHEAP_NEEDFIX}},
};

class SmapsStats {
public:
    SmapsStats() {}
    SmapsStats(const std::string path) : testpath_(path){};
    ~SmapsStats() {}

    using MatchFunc = std::function<bool(const std::string& name, const char* str)>;

    bool ParseMaps(int pid, ProcessMemoryInfo& processinfo, bool isReportApp, bool isReportSmaps);
    int GetProcessJavaHeap();
    int GetProcessNativeHeap();
    int GetProcessCode();
    int GetProcessStack();
    int GetProcessGraphics();
    int GetProcessPrivateOther();
    int GetProcessSystem();
private:
    std::array<StatsInfo, VMHEAP_NUM_HEAP> stats_;
    bool lastline_ = false;
    std::string testpath_;

    int GetTotalPrivateClean();
    int GetTotalPrivateDirty();
    int GetPrivate(int type);
    int GetTotalPss();
    int GetTotalSwappedOutPss();
    void ReviseStatsData();

    bool ReadVmemareasFile(const std::string& path, ProcessMemoryInfo& processinfo, bool isReportApp,
                           bool isReportSmaps);
    bool ParseMapHead(std::string& line, MapPiecesInfo& head, SmapsHeadInfo& smapsHeadInfo);
    bool SetMapAddrInfo(std::string& line, MapPiecesInfo& head);
    bool GetMemUsageField(std::string& line, MemUsageInfo& memusage);
    void CollectVmemAreasData(const MapPiecesInfo& mempic,
                              const MemUsageInfo& memusage,
                              uint64_t& prevEnd,
                              int& prevHeap);
    bool GetVmaIndex(std::string name, uint32_t namesz, int32_t heapIndex[2], bool& swappable);
    uint64_t GetSwapablepssValue(const MemUsageInfo& memusage, bool swappable);
    void SetVmemAreasData(int index, uint64_t swapablePss, const MemUsageInfo& usage);
    void HeapIndexFix(std::string name, const char* key, int32_t heapIndex[2]);
    bool GetVMAStuId(int ops,
                     std::string name,
                     const VmeminfoAreaMapping vma[],
                     int count,
                     int32_t heapIndex[2],
                     bool& swappable);
    std::string ParseCategory(const SmapsHeadInfo& smapsHeadInfo);
    bool GetCategoryFromMap(const std::string &name, std::string &group,
                            const std::map<std::string, std::string> &map, MatchFunc func);
    const std::map<std::string, std::string> beginMap_ = {
        {"[heap]", "native heap"}, {"[stack]", "stack"}, {"[anon:stack", "stack"},
        {"[anon:native_heap:", "native heap"}, {"[anon:ArkTS Heap]", "ark ts heap"},
        {"[anon:guard", "guard"}, {"/dev", "dev"}, {"[anon:signal_stack", "stack"},
        {"/dmabuf", "dmabuf"}, {"/data/storage", ".hap"}, {"[anon:libc_malloc", "native heap"},
        {"[anon:scudo", "native heap"}, {"[anon:GWP-ASan", "native heap"},
    };
    const std::map<std::string, std::string> endMap_ = {
        {".so", ".so"}, {".so.1", ".so"}, {".ttf", ".ttf"},
        {".db", ".db"}, {".db-shm", ".db"},
    };
    const std::string FILE_PAGE_TAG = "FilePage other";
    const std::string ANON_PAGE_TAG = "AnonPage other";
};

#endif