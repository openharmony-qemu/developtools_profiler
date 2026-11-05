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

#ifndef HOOK_CONTAINER_PROTOCOL_H
#define HOOK_CONTAINER_PROTOCOL_H

#include <unistd.h>
#include <algorithm>
#include <limits.h>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>
#include <securec.h>

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
namespace Container {

const uint8_t CONTAINER_MAX_UNWIND_DEPTH = 100;
const size_t CONTAINER_MAX_ASYNC_CHAIN_LAYERS = 17;
const size_t GPU_RANGE_COUNT = 2;
constexpr uint32_t DEFAULT_FILE_CACHE_MEMORY_SIZE = 32;
constexpr uint32_t MB_TO_BYTES = 1024 * 1024;

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

constexpr uint32_t STOP_HOOK_CMD = 0xADD50005;

enum CacheFileMsgType {
    CACHE_FILE_CREATED,
    CACHE_FILE_READY,
    CACHE_FILE_STOP,
};

constexpr uint16_t FILE_CACHE_FD_COUNT = 3;
constexpr uint16_t INVALID_CACHE_FILE_INDEX = UINT16_MAX;

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

struct alignas(8) AsyncStackData : public BaseStackRawData { // 8 is 8 bit
    uint64_t syncStackId;
    uint64_t asyncStackId;
    uint32_t nestedAsyncCount;
    uint64_t nestedAsyncIds[CONTAINER_MAX_ASYNC_CHAIN_LAYERS];
    uint64_t nestedAsyncTypes[CONTAINER_MAX_ASYNC_CHAIN_LAYERS];
};

struct alignas(8) StackRawData: public BaseStackRawData { // 8 is 8 bit
    union {
        char regs[MAX_REG_SIZE];
        uint64_t ip[CONTAINER_MAX_UNWIND_DEPTH];
    };
};

struct alignas(8) NameData: public BaseStackRawData { // 8 is 8 bit
    char name[MAX_HOOK_PATH + 1] {0};
};

struct alignas(8) ArkTsClientConfig { // 8 is 8 bit
        int32_t jsStackReport = 0;
        uint8_t maxJsStackDepth = 0;
        bool jsFpunwind = false;
        char filterNapiName[64] = {""};
};

struct alignas(8) GpuRange { // 8 is 8 bit
    uint64_t gpuVk [OHOS::Developtools::NativeDaemon::Container::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuGlesImage [OHOS::Developtools::NativeDaemon::Container::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuGlesBuffer [OHOS::Developtools::NativeDaemon::Container::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuClImage [OHOS::Developtools::NativeDaemon::Container::GPU_RANGE_COUNT * 2] = {0};
    uint64_t gpuClBuffer [OHOS::Developtools::NativeDaemon::Container::GPU_RANGE_COUNT * 2] = {0};
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
        memLeakLimit = UINT32_MAX;
        onlyMemLeak = false;
        asyncNestingDepth = 0;
        asyncTaskStackDepth = 0;
        arkuiMemTraceEnable = false;
        discardDestroyedTraces = false;
        mmapSpecificAddrOnly = false;
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
            << ", jsStackReport: "
            << arktsConfig.jsStackReport << ", maxJsStackDepth: "
            << std::to_string(arktsConfig.maxJsStackDepth) << ", filterNapiName: ";
        size_t napiLen = 0;
        while (napiLen < sizeof(arktsConfig.filterNapiName) && arktsConfig.filterNapiName[napiLen] != '\0') {
            ++napiLen;
        }
        ss << std::string(arktsConfig.filterNapiName, napiLen)
            << ", jsFpunwind: " << arktsConfig.jsFpunwind
            << ", largestSize: " << largestSize  << ", secondLargestSize: " << secondLargestSize
            << ", maxGrowthSize: " << maxGrowthSize << ", offline:" << offlineSymbolization
            << ", filterLibListSize: " << filterLibList.size()
            << ", enableFileCacheMode: " << enableFileCacheMode << ", fileCacheBufferSize:" << fileCacheBufferSize;
        return ss.str();
    }

    void ResetGpuRange(GpuRange& gpuRange)
    {
        for (size_t i = 0; i < OHOS::Developtools::NativeDaemon::Container::GPU_RANGE_COUNT * 2; i++) { //2: double
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
        config.filterLibList.emplace_back(ReadClientConfigString(ipc.filterLibList[index],
            sizeof(ipc.filterLibList[index])));
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

} //namespace Container
} //namespace NativeDaemon
} //namespace Developtools
} //namespace OHOS

#endif // HOOK_CONTAINER_PROTOCOL_H