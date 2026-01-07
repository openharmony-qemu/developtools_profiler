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
#include <memory_trace.h>
#define MAX_THREAD_NAME (32)
#define MAX_UNWIND_DEPTH (100)

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
constexpr int STACK_DATA_SIZE = 400000;
constexpr int SPEED_UP_THRESHOLD = STACK_DATA_SIZE / 2;
constexpr int SLOW_DOWN_THRESHOLD = STACK_DATA_SIZE / 4;
constexpr int32_t MIN_STACK_DEPTH = 6;
constexpr static uint32_t UNIQUE_STACK_TABLE_SIZE = 8 * 1024 * 1024;
// filter two layers of dwarf stack in libnative_hook.z.so
const size_t FILTER_STACK_DEPTH = 2;
const size_t MAX_CALL_FRAME_UNWIND_SIZE = MAX_UNWIND_DEPTH + FILTER_STACK_DEPTH;
const size_t GPU_TYPE_COUNT = 5;
const size_t RESTRACE_TYPE_COUNT = 20;
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
const size_t KMP_HEAP_MASK_INDEX = 17;
const size_t RN_HEAP_MASK_INDEX = 18;
const size_t DMABUF_MASK_INDEX = 19;
const size_t ARK_GLOBAL_HANDLE_INDEX = 20;
// dlopen function minimum stack depth
const int32_t DLOPEN_MIN_UNWIND_DEPTH = 5;
// default max js stack depth
const int32_t DEFAULT_MAX_JS_STACK_DEPTH = 10;
constexpr int SHARED_MEMORY_NUM = 3;
constexpr uint64_t DWARF_ERROR_ID = 999999;
constexpr uint64_t SIZE_MASK = 0xFFFFFF0000000000;
}
}
}

constexpr size_t MAX_REG_SIZE = sizeof(uint64_t)
    * OHOS::Developtools::NativeDaemon::PERF_REG_ARM64_MAX;
constexpr size_t MAX_HOOK_PATH = 256;

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
    END_MSG,
    FREE_MSG_SIMP,
    MALLOC_ARKTS,
    FREE_ARKTS,
    UNKNOWN,
};

enum SymbolType {
    NATIVE_SYMBOL,
    JS_SYMBOL,
};

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
    uint16_t tagId;
    uint32_t nodeType;
    uint64_t nodeId;
};

struct alignas(8) AsyncStackData : public BaseStackRawData { // 8 is 8 bit
    uint64_t syncStackId;
    uint64_t asyncStackId;
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
    uint64_t tagId : 16;
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
        responseLibraryMode = false;
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
            << ", sampleInterval: " << sampleInterval << ", responseLibraryMode: " << responseLibraryMode
            << ", freeEventOnlyAddrEnable: " << freeEventOnlyAddrEnable << ", jsStackReport: "
            << arktsConfig.jsStackReport << ", maxJsStackDepth: "
            << std::to_string(arktsConfig.maxJsStackDepth) << ", filterNapiName: "
            << arktsConfig.filterNapiName << ", jsFpunwind: " << arktsConfig.jsFpunwind
            << ", largestSize: " << largestSize  << ", secondLargestSize: " << secondLargestSize
            << ", maxGrowthSize: " << maxGrowthSize << ", offline:" << offlineSymbolization;
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
    bool responseLibraryMode = false;
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
};
#endif // HOOK_COMMON_H