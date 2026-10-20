/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef HOOK_COMMON_H
#define HOOK_COMMON_H

#if defined(HAVE_LIBUNWIND) && HAVE_LIBUNWIND
// for libunwind.h empty struct has size 0 in c, size 1 in c++
#define UNW_EMPTY_STRUCT uint8_t unused;
#include <libunwind.h>
#endif

#include "register.h"
#include "utilities.h"
#include "async_context_manager.h"
#include <memory_trace.h>
#include <unistd.h>
#include <algorithm>
#include <limits.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define MAX_THREAD_NAME (32)
#define MAX_UNWIND_DEPTH (100)
#define MAX_ASYNC_CHAIN_LAYERS (17)

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
constexpr int STACK_DATA_SIZE = 400000;
constexpr int STACK_DATA_SIZE_DWARF = 40000;
constexpr int SPEED_UP_THRESHOLD = STACK_DATA_SIZE / 2;
constexpr int SLOW_DOWN_THRESHOLD = STACK_DATA_SIZE / 4;
constexpr int32_t MIN_STACK_DEPTH = 6;
constexpr static uint32_t DEFAULT_UNIQUE_STACK_TABLE_SIZE = 8 * 1024 * 1024;
constexpr static uint32_t MAX_UNIQUE_STACK_TABLE_SIZE = 256 * 1024 * 1024;
constexpr static uint32_t ONE_MEGABYTE = 1024 * 1024;
inline uint32_t NormalizeUniqueStackTableSize(uint32_t size)
{
    if (size == 0 || size <= DEFAULT_UNIQUE_STACK_TABLE_SIZE) {
        return DEFAULT_UNIQUE_STACK_TABLE_SIZE;
    }
    return std::min(size, MAX_UNIQUE_STACK_TABLE_SIZE);
}
// filter two layers of dwarf stack in libnative_hook.z.so
const size_t FILTER_STACK_DEPTH = 2;
const size_t MAX_CALL_FRAME_UNWIND_SIZE = MAX_UNWIND_DEPTH + FILTER_STACK_DEPTH;
const size_t GPU_TYPE_COUNT = 5;
const uint32_t TAG_ID_PENDING = UINT32_MAX;  // Special marker for dynamic tagId generation
const size_t RESTRACE_TYPE_COUNT = 26;
const size_t GPU_RANGE_COUNT = 2;
const size_t GPU_VK_INDEX = 0;
const size_t GPU_GLES_IMAGE_INDEX = 1;
const size_t GPU_GLES_BUFFER_INDEX = 2;
const size_t GPU_CL_IMAGE_INDEX = 3;
const size_t GPU_CL_BUFFER_INDEX = 4;
const size_t FD_OPEN_INDEX = 5;
const size_t FD_EPOLL_INDEX = 6;
const size_t FD_EVENTFD_INDEX = 7;
const size_t FD_SOCKET_INDEX = 8;
const size_t FD_PIPE_INDEX = 9;
const size_t FD_DUP_INDEX = 10;
const size_t FD_MASK_INDEX = 11;
const size_t THREAD_PTHREAD_INDEX = 12;
const size_t THREAD_MASK_INDEX = 13;
const size_t SO_MASK_INDEX = 14;
const size_t ARKTS_HEAP_MASK_INDEX = 15;
const size_t JS_HEAP_MASK_INDEX = 16;
const size_t KOTLIN_HEAP_MASK_INDEX = 17;
const size_t RN_HERMES_HEAP_INDEX = 18;
const size_t DMABUF_MASK_INDEX = 19;
const size_t ARK_GLOBAL_HANDLE_INDEX = 20;
const size_t ARK_LOCAL_HANDLE_INDEX = 21;
const size_t VMA_ARKWEB_INDEX = 22;
const size_t DART_HEAP_INDEX = 23;
const size_t ARKTS_STATIC_HEAP_MASK_INDEX = 24;
const size_t CHROMIUM_HEAP_INDEX = 25;
const size_t VMA_CHROMIUM_INDEX = 26;
// dlopen function minimum stack depth
const int32_t DLOPEN_MIN_UNWIND_DEPTH = 5;
// default max js stack depth
const int32_t DEFAULT_MAX_JS_STACK_DEPTH = 10;
constexpr int SHARED_MEMORY_NUM = 3;
constexpr uint64_t DWARF_ERROR_ID = 999999;
constexpr uint64_t SIZE_MASK = 0xFFFFFF0000000000;
const std::string PRELOAD_APP_IDENTIFY = "preload_appname:";
constexpr uint32_t MIN_FILE_CACHE_MEMORY_SIZE = 16;
constexpr uint32_t MAX_FILE_CACHE_MEMORY_SIZE = 32;
constexpr uint32_t DEFAULT_FILE_CACHE_MEMORY_SIZE = 32;
constexpr uint32_t MB_TO_BYTES = 1024 * 1024;
}
}
}

constexpr size_t MAX_REG_SIZE = sizeof(uint64_t)
    * OHOS::Developtools::NativeDaemon::PERF_REG_ARM64_MAX;
constexpr size_t MAX_HOOK_PATH = 256;
constexpr size_t CLIENT_CONFIG_TARGET_SO_NAME_SIZE = PATH_MAX;
constexpr size_t CLIENT_CONFIG_FILTER_LIB_NAME_SIZE = MAX_HOOK_PATH + 1;
constexpr size_t CLIENT_CONFIG_FILTER_LIB_COUNT = 16;
constexpr uint64_t MMAP_FILE_SIZE = 64 * 1024 * 1024;
// 4-byte dataSize header: uint32 covers 64M-4 max data; single aligned store is
// tear-free on arm64, shrinking the crash-tearing window from 8B to 4B
constexpr size_t DATA_SIZE_HEADER_LEN = 4;
constexpr size_t CRASH_ZERO_THRESHOLD = 100;
constexpr uint32_t FILE_CACHE_READ_BUFFER_SIZE = 4 * 1024 * 1024;
constexpr uint32_t MAX_CACHE_DATA_SIZE = static_cast<uint32_t>(MMAP_FILE_SIZE) -
    static_cast<uint32_t>(DATA_SIZE_HEADER_LEN);

enum {
    MALLOCDISABLE = (1u << 0),
    MMAPDISABLE = (1u << 1),
    FREEMSGSTACK = (1u << 2),
    MUNMAPMSGSTACK = (1u << 3),
    FPUNWIND = (1u << 4),
    BLOCKED = (1u << 5),
    MEMTRACE_ENABLE = (1u << 6),
    ASYNCSTACK_ENABLE = (1u << 7),
};

enum {
    MALLOC_MSG = 0,
    FREE_MSG,
    MMAP_MSG,
    MMAP_FILE_PAGE_MSG,
    MUNMAP_MSG,
    MEMORY_USING_MSG,
    MEMORY_UNUSING_MSG,
    MEMORY_TAG,
    THREAD_NAME_MSG,
    PR_SET_VMA_MSG,
    JS_STACK_MSG,
    MMAP_FILE_TYPE,
    NMD_MSG,
    GWP_ASAN_MSG,
    END_MSG,
    FREE_MSG_SIMP,
    MALLOC_ARKTS,
    FREE_ARKTS,
    MEM_LEAK_MSG,
    UNKNOWN,
};

enum MemLeakFormat : uint32_t {
    TYPE_TXT = 0,       // A normal text format(default)
    TYPE_VECTOR = 1,    // A vector-type text
    TYPE_SNAPSHOT = 2,  // A JSON-based format
    TYPE_DISABLE = 0xFFFFFFFF,
};
constexpr uint32_t MEM_LEAK_LIMIT_DEFAULT = UINT32_MAX;

constexpr uint32_t SNAPSHOT_REQUEST_CMD = 0xADD50001;
constexpr uint32_t ALIVE_ADDR_SNAPSHOT_MARKER = 0xADD50002;
constexpr uint32_t ALIVE_ADDR_SNAPSHOT_END_MARKER = 0xADD50003;
constexpr uint32_t FINAL_SNAPSHOT_REQUEST_CMD = 0xADD50004;
constexpr uint32_t STOP_HOOK_CMD = 0xADD50005;
constexpr uint32_t MEM_LEAK_DONE_CMD = 0xADD50006;
constexpr uint32_t DUMP_SIMP_NMD_CMD = 0xADD50007;
constexpr uint32_t DUMP_SIMP_NMD_DONE_CMD = 0xADD50008;

enum SymbolType {
    NATIVE_SYMBOL,
    JS_SYMBOL,
};

enum CacheFileMsgType {
    CACHE_FILE_CREATED,
    CACHE_FILE_READY,
    CACHE_FILE_STOP,
};

constexpr uint16_t FILE_CACHE_FD_COUNT = 3;
constexpr uint16_t INVALID_CACHE_FILE_INDEX = UINT16_MAX;

struct Range {
    uint64_t start;
    uint64_t end;
};

struct alignas(8) MmapFileRawData { // 8 is 8 bit
    off_t offset;
    uint32_t flags;
};

struct alignas(8) BaseStackRawData { // 8 is 8 bit
    union {
        struct timespec ts;
        MmapFileRawData mmapArgs;
    };
    void* addr;
    void* newAddr;
    size_t mallocSize;
    uint64_t jsChainId;
    uint32_t pid;
    uint32_t tid;
    uint16_t type;
    uint32_t tagId;
    uint32_t nodeType;
    uint64_t nodeId;
};

inline bool TsLess(const struct timespec& a, const struct timespec& b)
{
    return std::tie(a.tv_sec, a.tv_nsec) < std::tie(b.tv_sec, b.tv_nsec);
}

inline bool IsArkGlobalHandleRecord(const BaseStackRawData* data)
{
    return data->tagId == OHOS::Developtools::NativeDaemon::ARK_GLOBAL_HANDLE_INDEX + 1;
}

inline bool ShouldSkipRecord(const BaseStackRawData* data,
    const std::unordered_map<void*, struct timespec>& liveData)
{
    auto iter = liveData.find(data->addr);
    return iter == liveData.end() || TsLess(data->ts, iter->second);
}

inline bool IsAllocRecord(const BaseStackRawData* data)
{
    return data->type == MALLOC_MSG || data->type == MEMORY_USING_MSG;
}

inline bool IsFreeRecord(const BaseStackRawData* data)
{
    return data->type == FREE_MSG || data->type == MEMORY_UNUSING_MSG;
}

inline bool InOrphanFreeWhitelist(const BaseStackRawData* data,
    const std::unordered_map<void*, struct timespec>& whitelist)
{
    auto it = whitelist.find(data->addr);
    return it != whitelist.end() &&
           data->ts.tv_sec == it->second.tv_sec &&
           data->ts.tv_nsec == it->second.tv_nsec;
}

inline bool IsBaseStackRawDataRecord(const int8_t* data, uint32_t size)
{
    if (size < sizeof(BaseStackRawData)) {
        return false;
    }
    uint32_t marker = *reinterpret_cast<const uint32_t*>(data);
    return marker != ALIVE_ADDR_SNAPSHOT_MARKER && marker != ALIVE_ADDR_SNAPSHOT_END_MARKER;
}

struct alignas(8) AsyncStackData : public BaseStackRawData { // 8 is 8 bit
    uint64_t syncStackId;
    uint64_t asyncStackId;
    uint32_t nestedAsyncCount;
    uint64_t nestedAsyncIds[MAX_ASYNC_CHAIN_LAYERS];
    uint64_t nestedAsyncTypes[MAX_ASYNC_CHAIN_LAYERS];
};

struct alignas(8) StackRawData: public BaseStackRawData { // 8 is 8 bit
    union {
        char regs[MAX_REG_SIZE];
        uint64_t ip[MAX_UNWIND_DEPTH];
    };
};

struct alignas(8) NameData: public BaseStackRawData { // 8 is 8 bit
    char name[MAX_HOOK_PATH + 1] {0};
};

struct alignas(8) ExtraDataInfo {
    ExtraDataInfo() : firstExtra(nullptr), firstExtraSize(0), secondExtra(nullptr), secondExtraSize(0) {}
    ExtraDataInfo(const int8_t* first, uint32_t firstSize, const int8_t* second, uint32_t secondSize)
        : firstExtra(first), firstExtraSize(firstSize), secondExtra(second), secondExtraSize(secondSize) {}
    const int8_t* firstExtra = nullptr;
    uint32_t firstExtraSize = 0;
    const int8_t* secondExtra = nullptr;
    uint32_t secondExtraSize = 0;
};

struct alignas(8) ReportEventBaseData { // 8 is 8 bit
    ReportEventBaseData(const BaseStackRawData* baseData, uint32_t id = 0)
    {
        ts = baseData->ts;
        addr = reinterpret_cast<uint64_t>(baseData->addr);
        type = baseData->type;
        tagId = baseData->tagId;
        mallocSize = baseData->mallocSize;
        tid = baseData->tid;
        stackMapId = id;
    }
    struct timespec ts;
    uint64_t addr : 40;
    uint64_t type : 8;
    uint32_t tagId;
    uint32_t mallocSize;
    uint32_t tid;
    uint32_t stackMapId;
    uint32_t nodeType = 0;
    uint64_t nodeId = 0;
};

struct alignas(8) ArkTsClientConfig { // 8 is 8 bit
        int32_t jsStackReport = 0;
        uint8_t maxJsStackDepth = 0;
        bool jsFpunwind = false;
        char filterNapiName[64] = {""};
};

struct alignas(8) GpuRange { // 8 is 8 bit
    uint64_t gpuVk [OHOS::Developtools::NativeDaemon::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuGlesImage [OHOS::Developtools::NativeDaemon::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuGlesBuffer [OHOS::Developtools::NativeDaemon::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuClImage [OHOS::Developtools::NativeDaemon::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuClBuffer [OHOS::Developtools::NativeDaemon::GPU_RANGE_COUNT * 2] = {0};
};

struct alignas(8) ClientConfig { // 8 is 8 bit
    void Reset()
    {
        filterSize = -1;
        shareMemorySize = 0;
        clockId = CLOCK_REALTIME;
        maxStackDepth = 0;
        uniqueStackTableSize = 0;
        statisticsInterval = 0;
        sampleInterval = 0;
        mallocDisable = false;
        mmapDisable = false;
        freeStackData = false;
        munmapStackData = false;
        fpunwind = false;
        isBlocked = false;
        memtraceEnable = false;
        asyncStackEnable = false;
        asyncFlag = 0;
        nestedAsyncStackEnable = false;
        responseLibraryMode = false;
        allocatorSamplingEnable = false;
        freeEventOnlyAddrEnable = false;
        printNmd = false;
        nmdType = -1;
        isSaMode = false;
        offlineSymbolization = false;
        arktsConfig.jsStackReport = 0;
        arktsConfig.maxJsStackDepth = 0;
        arktsConfig.jsFpunwind = false;
        arktsConfig.filterNapiName[0] = '\0';
        traceMask = 0;
        ResetGpuRange(gpuRange);
        largestSize = 0;
        secondLargestSize = 0;
        maxGrowthSize = 0;
        targetSoName = "";
        filterLibList.clear();
        enableFileCacheMode = false;
        fileCacheBufferSize = 0;
        memLeakFormat = MemLeakFormat::TYPE_DISABLE;
        memLeakLimit = MEM_LEAK_LIMIT_DEFAULT;
        onlyMemLeak = false;
        asyncNestingDepth = 0;
        asyncTaskStackDepth = 0;
        arkuiMemTraceEnable = false;
        discardDestroyedTraces = false;
        mmapSpecificAddrOnly = false;
        anonMmapOnly = false;
    }

    std::string ToString()
    {
        std::stringstream ss;
        ss << "filterSize:" << filterSize << ", shareMemorySize:" << shareMemorySize
            << ", clockId:" << clockId << ", maxStackDepth:" << std::to_string(maxStackDepth)
            << ", mallocDisable:" << mallocDisable << ", mmapDisable:" << mmapDisable
            << ", freeStackData:" << freeStackData << ", munmapStackData:" << munmapStackData
            << ", fpunwind:" << fpunwind << ", isBlocked:" << isBlocked << ", memtraceEnable:" << memtraceEnable
            << ", asyncStackEnable:" << asyncStackEnable << ", asyncFlag:" << asyncFlag
            << ", asyncNestingDepth:" << asyncNestingDepth << ", asyncTaskStackDepth:" << asyncTaskStackDepth
            << ", sampleInterval: " << sampleInterval << ", responseLibraryMode: " << responseLibraryMode
            << ", allocatorSamplingEnable: " << allocatorSamplingEnable
            << ", uniqueStackTableSize: " << uniqueStackTableSize
            << ", freeEventOnlyAddrEnable: " << freeEventOnlyAddrEnable
            << ", arkuiMemTraceEnable: " << arkuiMemTraceEnable
            << ", discardDestroyedTraces: " << discardDestroyedTraces
            << ", mmapSpecificAddrOnly: " << mmapSpecificAddrOnly
            << ", anonMmapOnly: " << anonMmapOnly
            << ", jsStackReport: "
            << arktsConfig.jsStackReport << ", maxJsStackDepth: "
            << std::to_string(arktsConfig.maxJsStackDepth) << ", filterNapiName: "
            << arktsConfig.filterNapiName << ", jsFpunwind: " << arktsConfig.jsFpunwind
            << ", largestSize: " << largestSize  << ", secondLargestSize: " << secondLargestSize
            << ", maxGrowthSize: " << maxGrowthSize << ", offline:" << offlineSymbolization
            << ", filterLibListSize: " << filterLibList.size()
            << ", enableFileCacheMode: " << enableFileCacheMode << ", fileCacheBufferSize:" << fileCacheBufferSize;
        return ss.str();
    }

    void ResetGpuRange(GpuRange& gpuRange)
    {
        for (size_t i = 0; i < OHOS::Developtools::NativeDaemon::GPU_RANGE_COUNT * 2; i++) { //2: double
            gpuRange.gpuVk[i] = 0;
            gpuRange.gpuGlesImage[i] = 0;
            gpuRange.gpuGlesBuffer[i] = 0;
            gpuRange.gpuClImage[i] = 0;
            gpuRange.gpuClBuffer[i] = 0;
        }
    }

    int32_t filterSize = -1;
    uint32_t shareMemorySize = 0;
    uint32_t uniqueStackTableSize = 0;
    uint32_t sampleInterval = 0;
    uint32_t statisticsInterval = 0;
    clockid_t clockId = CLOCK_REALTIME;
    uint8_t maxStackDepth = 0;
    bool mallocDisable = false;
    bool mmapDisable = false;
    bool freeStackData = false;
    bool munmapStackData = false;
    bool fpunwind = false;
    bool isBlocked = false;
    bool memtraceEnable = false;
    bool asyncStackEnable = false;
    uint64_t asyncFlag = 0;
    bool nestedAsyncStackEnable = false;
    bool responseLibraryMode = false;
    bool allocatorSamplingEnable = false;
    bool freeEventOnlyAddrEnable = false;
    bool printNmd = false;
    int nmdType = -1;
    bool isSaMode = false;
    bool offlineSymbolization = false;
    ArkTsClientConfig arktsConfig = {0};
    unsigned long long traceMask = 0;
    GpuRange gpuRange = {};
    uint32_t largestSize = 0;
    uint32_t secondLargestSize = 0;
    uint32_t maxGrowthSize = 0;
    std::string targetSoName;
    std::vector<std::string> filterLibList;
    uint32_t gwpAsanHoldTime = 0;
    uint32_t gwpAsanMaxEntries = 0;
    uint32_t gwpAsanMaxStackDepth = 0;
    bool enableFileCacheMode = false;
    uint32_t fileCacheBufferSize = 0;
    uint32_t memLeakLimit = 0;
    MemLeakFormat memLeakFormat = MemLeakFormat::TYPE_DISABLE;
    bool onlyMemLeak = false;
    uint32_t asyncNestingDepth = 0;
    uint32_t asyncTaskStackDepth = 0;
    bool arkuiMemTraceEnable = false;
    bool discardDestroyedTraces = false;
    bool mmapSpecificAddrOnly = false;
    bool anonMmapOnly = false;
};

struct alignas(8) ClientConfigIpc { // 8 is 8 bit
    int32_t filterSize = -1;
    uint32_t shareMemorySize = 0;
    uint32_t uniqueStackTableSize = 0;
    uint32_t sampleInterval = 0;
    uint32_t statisticsInterval = 0;
    clockid_t clockId = CLOCK_REALTIME;
    uint8_t maxStackDepth = 0;
    bool mallocDisable = false;
    bool mmapDisable = false;
    bool freeStackData = false;
    bool munmapStackData = false;
    bool fpunwind = false;
    bool isBlocked = false;
    bool memtraceEnable = false;
    bool asyncStackEnable = false;
    uint64_t asyncFlag = 0;
    bool nestedAsyncStackEnable = false;
    bool responseLibraryMode = false;
    bool allocatorSamplingEnable = false;
    bool freeEventOnlyAddrEnable = false;
    bool printNmd = false;
    int nmdType = -1;
    bool isSaMode = false;
    bool offlineSymbolization = false;
    ArkTsClientConfig arktsConfig = {0};
    unsigned long long traceMask = 0;
    GpuRange gpuRange = {};
    uint32_t largestSize = 0;
    uint32_t secondLargestSize = 0;
    uint32_t maxGrowthSize = 0;
    char targetSoName[CLIENT_CONFIG_TARGET_SO_NAME_SIZE] = {0};
    uint32_t filterLibCount = 0;
    char filterLibList[CLIENT_CONFIG_FILTER_LIB_COUNT][CLIENT_CONFIG_FILTER_LIB_NAME_SIZE] = {{0}};
    uint32_t gwpAsanHoldTime = 0;
    uint32_t gwpAsanMaxEntries = 0;
    uint32_t gwpAsanMaxStackDepth = 0;
    bool enableFileCacheMode = false;
    uint32_t fileCacheBufferSize = 0;
    uint32_t memLeakLimit = 0;
    MemLeakFormat memLeakFormat = MemLeakFormat::TYPE_DISABLE;
    bool onlyMemLeak = false;
    uint32_t asyncNestingDepth = 0;
    uint32_t asyncTaskStackDepth = 0;
    bool arkuiMemTraceEnable = false;
    bool discardDestroyedTraces = false;
    bool mmapSpecificAddrOnly = false;
    bool anonMmapOnly = false;
};

inline bool CopyClientConfigString(char* dest, size_t destSize, const std::string& src)
{
    if (dest == nullptr || destSize == 0 || src.size() >= destSize) {
        return false;
    }
    dest[0] = '\0';
    if (src.empty()) {
        return true;
    }
    return strncpy_s(dest, destSize, src.c_str(), src.size()) == EOK;
}

inline std::string ReadClientConfigString(const char* src, size_t srcSize)
{
    if (src == nullptr || srcSize == 0) {
        return "";
    }
    size_t len = 0;
    while (len < srcSize && src[len] != '\0') {
        ++len;
    }
    return std::string(src, len);
}

inline void CopyClientConfigBaseToIpc(const ClientConfig& config, ClientConfigIpc& ipc)
{
    ipc.filterSize = config.filterSize;
    ipc.shareMemorySize = config.shareMemorySize;
    ipc.uniqueStackTableSize = config.uniqueStackTableSize;
    ipc.sampleInterval = config.sampleInterval;
    ipc.statisticsInterval = config.statisticsInterval;
    ipc.clockId = config.clockId;
    ipc.maxStackDepth = config.maxStackDepth;
    ipc.mallocDisable = config.mallocDisable;
    ipc.mmapDisable = config.mmapDisable;
    ipc.freeStackData = config.freeStackData;
    ipc.munmapStackData = config.munmapStackData;
    ipc.fpunwind = config.fpunwind;
    ipc.isBlocked = config.isBlocked;
    ipc.memtraceEnable = config.memtraceEnable;
    ipc.asyncStackEnable = config.asyncStackEnable;
    ipc.asyncFlag = config.asyncFlag;
}

inline void CopyClientConfigExtToIpc(const ClientConfig& config, ClientConfigIpc& ipc)
{
    ipc.nestedAsyncStackEnable = config.nestedAsyncStackEnable;
    ipc.responseLibraryMode = config.responseLibraryMode;
    ipc.allocatorSamplingEnable = config.allocatorSamplingEnable;
    ipc.freeEventOnlyAddrEnable = config.freeEventOnlyAddrEnable;
    ipc.printNmd = config.printNmd;
    ipc.nmdType = config.nmdType;
    ipc.isSaMode = config.isSaMode;
    ipc.offlineSymbolization = config.offlineSymbolization;
    ipc.arktsConfig = config.arktsConfig;
    ipc.traceMask = config.traceMask;
    ipc.gpuRange = config.gpuRange;
    ipc.largestSize = config.largestSize;
    ipc.secondLargestSize = config.secondLargestSize;
    ipc.maxGrowthSize = config.maxGrowthSize;
    ipc.arkuiMemTraceEnable = config.arkuiMemTraceEnable;
    ipc.discardDestroyedTraces = config.discardDestroyedTraces;
    ipc.mmapSpecificAddrOnly = config.mmapSpecificAddrOnly;
    ipc.anonMmapOnly = config.anonMmapOnly;
}

inline void CopyClientConfigTailToIpc(const ClientConfig& config, ClientConfigIpc& ipc)
{
    ipc.gwpAsanHoldTime = config.gwpAsanHoldTime;
    ipc.gwpAsanMaxEntries = config.gwpAsanMaxEntries;
    ipc.gwpAsanMaxStackDepth = config.gwpAsanMaxStackDepth;
    ipc.enableFileCacheMode = config.enableFileCacheMode;
    ipc.fileCacheBufferSize = config.fileCacheBufferSize;
    ipc.memLeakLimit = config.memLeakLimit;
    ipc.memLeakFormat = config.memLeakFormat;
    ipc.onlyMemLeak = config.onlyMemLeak;
}

inline void CopyClientConfigAsyncToIpc(const ClientConfig& config, ClientConfigIpc& ipc)
{
    ipc.asyncNestingDepth = config.asyncNestingDepth;
    ipc.asyncTaskStackDepth = config.asyncTaskStackDepth;
}

inline bool CopyClientConfigTextToIpc(const ClientConfig& config, ClientConfigIpc& ipc)
{
    if (!CopyClientConfigString(ipc.targetSoName, sizeof(ipc.targetSoName), config.targetSoName)) {
        return false;
    }
    if (config.filterLibList.size() > CLIENT_CONFIG_FILTER_LIB_COUNT) {
        return false;
    }
    ipc.filterLibCount = static_cast<uint32_t>(config.filterLibList.size());
    for (size_t index = 0; index < config.filterLibList.size(); ++index) {
        if (!CopyClientConfigString(ipc.filterLibList[index], sizeof(ipc.filterLibList[index]),
            config.filterLibList[index])) {
            return false;
        }
    }
    return true;
}

inline bool ConvertClientConfigToIpc(const ClientConfig& config, ClientConfigIpc& ipc)
{
    ipc = ClientConfigIpc();
    CopyClientConfigBaseToIpc(config, ipc);
    CopyClientConfigExtToIpc(config, ipc);
    CopyClientConfigTailToIpc(config, ipc);
    CopyClientConfigAsyncToIpc(config, ipc);
    return CopyClientConfigTextToIpc(config, ipc);
}

inline void CopyClientConfigBaseFromIpc(const ClientConfigIpc& ipc, ClientConfig& config)
{
    config.filterSize = ipc.filterSize;
    config.shareMemorySize = ipc.shareMemorySize;
    config.uniqueStackTableSize = ipc.uniqueStackTableSize;
    config.sampleInterval = ipc.sampleInterval;
    config.statisticsInterval = ipc.statisticsInterval;
    config.clockId = ipc.clockId;
    config.maxStackDepth = ipc.maxStackDepth;
    config.mallocDisable = ipc.mallocDisable;
    config.mmapDisable = ipc.mmapDisable;
    config.freeStackData = ipc.freeStackData;
    config.munmapStackData = ipc.munmapStackData;
    config.fpunwind = ipc.fpunwind;
    config.isBlocked = ipc.isBlocked;
    config.memtraceEnable = ipc.memtraceEnable;
    config.asyncStackEnable = ipc.asyncStackEnable;
    config.asyncFlag = ipc.asyncFlag;
}

inline void CopyClientConfigExtFromIpc(const ClientConfigIpc& ipc, ClientConfig& config)
{
    config.nestedAsyncStackEnable = ipc.nestedAsyncStackEnable;
    config.responseLibraryMode = ipc.responseLibraryMode;
    config.allocatorSamplingEnable = ipc.allocatorSamplingEnable;
    config.freeEventOnlyAddrEnable = ipc.freeEventOnlyAddrEnable;
    config.printNmd = ipc.printNmd;
    config.nmdType = ipc.nmdType;
    config.isSaMode = ipc.isSaMode;
    config.offlineSymbolization = ipc.offlineSymbolization;
    config.arktsConfig = ipc.arktsConfig;
    config.traceMask = ipc.traceMask;
    config.gpuRange = ipc.gpuRange;
    config.largestSize = ipc.largestSize;
    config.secondLargestSize = ipc.secondLargestSize;
    config.maxGrowthSize = ipc.maxGrowthSize;
    config.arkuiMemTraceEnable = ipc.arkuiMemTraceEnable;
    config.discardDestroyedTraces = ipc.discardDestroyedTraces;
    config.mmapSpecificAddrOnly = ipc.mmapSpecificAddrOnly;
    config.anonMmapOnly = ipc.anonMmapOnly;
}

inline void CopyClientConfigTailFromIpc(const ClientConfigIpc& ipc, ClientConfig& config)
{
    config.gwpAsanHoldTime = ipc.gwpAsanHoldTime;
    config.gwpAsanMaxEntries = ipc.gwpAsanMaxEntries;
    config.gwpAsanMaxStackDepth = ipc.gwpAsanMaxStackDepth;
    config.enableFileCacheMode = ipc.enableFileCacheMode;
    config.fileCacheBufferSize = ipc.fileCacheBufferSize;
    config.memLeakLimit = ipc.memLeakLimit;
    config.memLeakFormat = ipc.memLeakFormat;
    config.onlyMemLeak = ipc.onlyMemLeak;
}

inline void CopyClientConfigAsyncFromIpc(const ClientConfigIpc& ipc, ClientConfig& config)
{
    config.asyncNestingDepth = ipc.asyncNestingDepth;
    config.asyncTaskStackDepth = ipc.asyncTaskStackDepth;
}

inline void CopyClientConfigTextFromIpc(const ClientConfigIpc& ipc, ClientConfig& config)
{
    config.targetSoName = ReadClientConfigString(ipc.targetSoName, sizeof(ipc.targetSoName));
    config.filterLibList.clear();
    size_t count = std::min(static_cast<size_t>(ipc.filterLibCount), CLIENT_CONFIG_FILTER_LIB_COUNT);
    for (size_t index = 0; index < count; ++index) {
        config.filterLibList.emplace_back(ReadClientConfigString(ipc.filterLibList[index], sizeof(ipc.filterLibList[index])));
    }
}

inline void ConvertClientConfigFromIpc(const ClientConfigIpc& ipc, ClientConfig& config)
{
    config.Reset();
    CopyClientConfigBaseFromIpc(ipc, config);
    CopyClientConfigExtFromIpc(ipc, config);
    CopyClientConfigTailFromIpc(ipc, config);
    CopyClientConfigAsyncFromIpc(ipc, config);
    CopyClientConfigTextFromIpc(ipc, config);
}
struct alignas(8) HookCacheFileMsg {
    HookCacheFileMsg() : hookPid(-1), fileIndex(0), msgType(CACHE_FILE_CREATED) {}
    HookCacheFileMsg(int32_t pid, uint16_t index, uint16_t type) : hookPid(pid), fileIndex(index), msgType(type) {}
    int32_t hookPid = -1;
    uint16_t fileIndex = 0;
    uint16_t msgType = CACHE_FILE_CREATED;
};

inline bool ReadCacheFileHeader(int fd, uint32_t& dataSize)
{
    dataSize = 0;
    if (fd < 0 || lseek(fd, 0, SEEK_SET) < 0 ||
        read(fd, &dataSize, DATA_SIZE_HEADER_LEN) != static_cast<ssize_t>(DATA_SIZE_HEADER_LEN)) {
        dataSize = 0;
        return false;
    }
    return true;
}

// resets dataSize to 0 (crash fallback mode) when it exceeds the physical maximum
inline bool SanitizeCacheDataSize(uint32_t& dataSize)
{
    if (dataSize > MAX_CACHE_DATA_SIZE) {
        dataSize = 0;
        return false;
    }
    return true;
}

inline bool IsBufferAllZero(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    return std::all_of(data, data + size, [](uint8_t byte) { return byte == 0; });
}

inline bool IsNextCacheDataAllZero(int fd)
{
    off_t curPos = fd < 0 ? -1 : lseek(fd, 0, SEEK_CUR);
    if (curPos < 0) {
        return true;
    }
    uint8_t probe[CRASH_ZERO_THRESHOLD] = {0};
    ssize_t probed = pread(fd, probe, CRASH_ZERO_THRESHOLD, curPos);
    return probed <= 0 || IsBufferAllZero(probe, static_cast<size_t>(probed));
}

struct CacheFileReadContext {
    int fd = -1;
    uint8_t* readBuffer = nullptr;
    uint32_t bytesToRead = 0;
    bool isCrashFile = false;
    uint32_t remainSize = 0;
    uint64_t readfileSize = 0;
    bool readDone = false;
};

// max retry count for GetRecordHeadWithRetry when totalSize==0 in crash mode
constexpr uint32_t MAX_ZERO_HEAD_RETRY = 5;

constexpr uint32_t BACKPRESSURE_LOW_WATERMARK = 5;
constexpr uint32_t BACKPRESSURE_MID_WATERMARK = 10;
constexpr uint32_t BACKPRESSURE_HIGH_WATERMARK = 20;
constexpr uint32_t BACKPRESSURE_MIN_DELAY_NS = 0;
constexpr uint32_t BACKPRESSURE_MID_DELAY_NS = 20 * 1000;
constexpr uint32_t BACKPRESSURE_HIGH_DELAY_NS = 100 * 1000;
constexpr uint32_t BACKPRESSURE_SAMPLE_SIZE = 4;
constexpr uint32_t BACKPRESSURE_VOLATILE_SIZE = 10;

struct BackpressureConfig {
    uint32_t lowWatermark = BACKPRESSURE_LOW_WATERMARK;
    uint32_t midWatermark = BACKPRESSURE_MID_WATERMARK;
    uint32_t highWatermark = BACKPRESSURE_HIGH_WATERMARK;
    uint32_t minDelayNs = BACKPRESSURE_MIN_DELAY_NS;
    uint32_t midDelayNs = BACKPRESSURE_MID_DELAY_NS;
    uint32_t highDelayNs = BACKPRESSURE_HIGH_DELAY_NS;
    uint32_t stateBroadcastIntervalMs = 100;
    uint32_t monitorIntervalMs = 1000;
};

struct alignas(8) BackpressureStateMsg {
    int32_t pid = -1;
    uint32_t queueDepth = 0;
    uint32_t recommendedDelayNs = 0;
};

inline uint64_t GetBackpressureCurrentTimeMs()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

struct alignas(8) SimplifiedMemConfigMsg {
    int32_t pid = -1;
    uint32_t largestSize = 0;
    uint32_t secondLargestSize = 0;
    uint32_t maxGrowthSize = 0;
    uint32_t sampleSize = 0;
};

#endif // HOOK_COMMON_H
