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

#define MAX_THREAD_NAME (32)
#define MAX_UNWIND_DEPTH (100)

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
const int STACK_DATA_SIZE = 40000;
const int SPEED_UP_THRESHOLD = STACK_DATA_SIZE / 2;
const int SLOW_DOWN_THRESHOLD = STACK_DATA_SIZE / 4;
const int32_t MIN_STACK_DEPTH = 6;
// filter two layers of dwarf stack in libnative_hook.z.so
const size_t FILTER_STACK_DEPTH = 2;
const size_t MAX_CALL_FRAME_UNWIND_SIZE = MAX_UNWIND_DEPTH + FILTER_STACK_DEPTH;
// dlopen function minimum stack depth
const int32_t DLOPEN_MIN_UNWIND_DEPTH = 5;
const uint32_t  MMAP_FILE_TYPE = (1u << 8);
}
}
}

constexpr size_t MAX_REG_SIZE = sizeof(uint64_t)
    * OHOS::Developtools::NativeDaemon::PERF_REG_ARM64_MAX;

enum {
    MALLOCDISABLE = (1u << 0),
    MMAPDISABLE = (1u << 1),
    FREEMSGSTACK = (1u << 2),
    MUNMAPMSGSTACK = (1u << 3),
    FPUNWIND = (1u << 4),
    BLOCKED = (1u << 5),
    MEMTRACE_ENABLE = (1u << 6),
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
    size_t mallocSize;
    uint32_t pid;
    uint32_t tid;
    uint16_t type;
    uint16_t tagId;
};

struct alignas(8) StackRawData: public BaseStackRawData { // 8 is 8 bit
    union {
        char regs[MAX_REG_SIZE];
        uint64_t ip[MAX_UNWIND_DEPTH];
        char name[PATH_MAX + 1] {0};
    };
};

struct alignas(8) ClientConfig { // 8 is 8 bit
    void Reset()
    {
        filterSize = -1;
        shareMemroySize = 0;
        clockId = CLOCK_REALTIME;
        maxStackDepth = 0;
        mallocDisable = false;
        mmapDisable = false;
        freeStackData = false;
        munmapStackData = false;
        fpunwind = false;
        isBlocked = false;
        memtraceEnable = false;
    }

    std::string ToString()
    {
        std::stringstream ss;
        ss << "filterSize:" << filterSize << ", shareMemroySize:" << shareMemroySize
            << ", clockId:" << clockId << ", maxStackDepth:" << std::to_string(maxStackDepth)
            << ", mallocDisable:" << mallocDisable << ", mmapDisable:" << mmapDisable
            << ", freeStackData:" << freeStackData << ", munmapStackData:" << munmapStackData
            << ", fpunwind:" << fpunwind << ", isBlocked:" << isBlocked << ", memtraceEnable:" << memtraceEnable;
        return ss.str();
    }

    int32_t filterSize = -1;
    uint32_t shareMemroySize = 0;
    clockid_t clockId = CLOCK_REALTIME;
    uint8_t maxStackDepth = 0;
    bool mallocDisable = false;
    bool mmapDisable = false;
    bool freeStackData = false;
    bool munmapStackData = false;
    bool fpunwind = false;
    bool isBlocked = false;
    bool memtraceEnable = false;
};

struct StandaloneRawStack {
    BaseStackRawData* stackConext; // points to the foundation type data
    uint8_t* stackData;
    int8_t* data; // fp mode data is ip, dwarf mode data is regs
    uint32_t stackSize;
    uint8_t fpDepth; // fp mode fpDepth is ip depth, dwarf mode is invalid
};

#endif // HOOK_COMMON_H