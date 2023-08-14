/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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

#include <atomic>
#include <climits>
#include <dlfcn.h>
#include <fcntl.h>
#include <string>
#include <sys/time.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unordered_map>
#include <unordered_set>
#include "common.h"
#include "hook_common.h"
#include "hook_socket_client.h"
#include "musl_preinit_common.h"
#include "parameter.h"
#include "stack_writer.h"
#include "runtime_stack_range.h"
#include "get_thread_id.h"
#include "hook_client.h"
#include <sys/mman.h>

static pthread_key_t g_disableHookFlag;
static pthread_key_t g_hookTid;
static pthread_key_t g_updateThreadNameCount;
namespace {
static std::atomic<uint64_t> g_timeCost = 0;
static std::atomic<uint64_t> g_mallocTimes = 0;
static std::atomic<uint64_t> g_dataCounts = 0;
using OHOS::Developtools::NativeDaemon::buildArchType;
static std::shared_ptr<HookSocketClient> g_hookClient;
std::recursive_timed_mutex g_ClientMutex;
std::mutex g_tagMapMutex;
std::atomic<const MallocDispatchType*> g_dispatch {nullptr};
constexpr int TIMEOUT_MSEC = 2000;
constexpr int PRINT_INTERVAL = 5000;
constexpr int UPDATE_THEARD_NAME = 1000;
constexpr uint64_t S_TO_NS = 1000 * 1000 * 1000;
static pid_t g_hookPid = 0;
static ClientConfig g_ClientConfig = {0};
static uint32_t g_maxSize = INT_MAX;
static std::unordered_map<std::string, uint32_t> g_memTagMap;
constexpr int PID_STR_SIZE = 4;
constexpr int STATUS_LINE_SIZE = 512;
constexpr int PID_NAMESPACE_ID = 1; // 1: pid is 1 after pid namespace used
constexpr int FD_PATH_LENGTH = 64;
static bool g_isPidChanged = false;
const MallocDispatchType* GetDispatch()
{
    return g_dispatch.load(std::memory_order_relaxed);
}

bool InititalizeIPC()
{
    return true;
}
void FinalizeIPC() {}

int ConvertPid(char* buf)
{
    int count = 0;
    char pidBuf[11] = {0}; /* 11: 32 bits to the maximum length of a string */
    char *str = buf;
    while (*str != '\0') {
        if ((*str >= '0') && (*str <= '9') && (static_cast<unsigned long>(count) < sizeof(pidBuf) - 1)) {
            pidBuf[count] = *str;
            count++;
            str++;
            continue;
        }

        if (count > 0) {
            break;
        }
        str++;
    }
    return atoi(pidBuf);
}

pid_t GetRealPid(void)
{
    const char *path = "/proc/self/status";
    char buf[STATUS_LINE_SIZE] = {0};
    FILE *fp = fopen(path, "r");
    CHECK_NOTNULL(fp, -1, "fopen fail");
    while (!feof(fp)) {
        if (fgets(buf, STATUS_LINE_SIZE, fp) == nullptr) {
            fclose(fp);
            return -1;
        }
        if (strncmp(buf, "Pid:", PID_STR_SIZE) == 0) {
            break;
        }
    }
    (void)fclose(fp);
    return static_cast<pid_t>(ConvertPid(buf));
}
}  // namespace

pid_t inline __attribute__((always_inline)) GetCurThreadId()
{
    if (pthread_getspecific(g_hookTid) == nullptr) {
        pthread_setspecific(g_hookTid, reinterpret_cast<void *>(GetThreadId()));
    }
    return reinterpret_cast<long>((pthread_getspecific(g_hookTid)));
}

bool inline __attribute__((always_inline)) UpdateThreadName(std::shared_ptr<HookSocketClient>& client)
{
    long updateCount = reinterpret_cast<long>(pthread_getspecific(g_updateThreadNameCount));
    bool ret = true;
    if (updateCount == 0) {
        StackRawData tnameData = {{{{0}}}};
        tnameData.tid = static_cast<uint32_t>(GetCurThreadId());
        tnameData.type = THREAD_NAME_MSG;
        prctl(PR_GET_NAME, tnameData.name);
        ret = client->SendStackWithPayload(&tnameData,
                                           sizeof(BaseStackRawData) + strlen(tnameData.name) + 1, nullptr, 0);
        if (!ret) {
            HILOG_DEBUG(LOG_CORE, "Send thread name failed!");
            return ret;
        }
    }
    pthread_setspecific(g_updateThreadNameCount,
                        reinterpret_cast<void *>(updateCount == UPDATE_THEARD_NAME ? 0 : updateCount + 1));
    return ret;
}

uint32_t inline __attribute__((always_inline)) GetTagId(std::shared_ptr<HookSocketClient>& client, const char* tagName)
{
    if (tagName == nullptr) {
        return 0;
    }
    uint32_t tagId = 0;
    bool isNewTag = false;
    std::unique_lock<std::mutex> lock(g_tagMapMutex);
    auto it = g_memTagMap.find(tagName);
    if (it == g_memTagMap.end()) {
        isNewTag = true;
        tagId = g_memTagMap.size() + 1;
        g_memTagMap[tagName] = tagId;
    } else {
        tagId = it->second;
    }
    lock.unlock();
    if (isNewTag) {
        StackRawData tagData = {{{{0}}}};
        tagData.type = MEMORY_TAG;
        tagData.tagId = tagId;
        if (strcpy_s(tagData.name, PATH_MAX + 1, tagName) != 0) {
            HILOG_ERROR(LOG_CORE, "Set tag name failed");
        }
        if (client != nullptr) {
            client->SendStackWithPayload(&tagData, sizeof(BaseStackRawData) + strlen(tagName) + 1, nullptr, 0);
        }
    }
    return tagId;
}

static bool IsPidChanged(void);

bool ohos_malloc_hook_on_start(void)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    COMMON::PrintMallinfoLog("before hook(byte) => ");
    g_hookPid = GetRealPid();
    g_mallocTimes = 0;
    if (g_hookClient != nullptr) {
        HILOG_INFO(LOG_CORE, "hook already started");
        return true;
    } else {
        g_ClientConfig.Reset();
        g_hookClient = std::make_shared<HookSocketClient>(g_hookPid, &g_ClientConfig);
    }
    pthread_key_create(&g_disableHookFlag, nullptr);
    pthread_setspecific(g_disableHookFlag, nullptr);
    pthread_key_create(&g_hookTid, nullptr);
    pthread_setspecific(g_hookTid, nullptr);
    pthread_key_create(&g_updateThreadNameCount, nullptr);
    pthread_setspecific(g_updateThreadNameCount, reinterpret_cast<void *>(0));
    HILOG_INFO(LOG_CORE, "ohos_malloc_hook_on_start");
    GetMainThreadRuntimeStackRange();
    constexpr int paramBufferLen = 128;
    char paramOutBuf[paramBufferLen] = {0};
    int ret = GetParameter("persist.hiviewdfx.profiler.mem.filter", "", paramOutBuf, paramBufferLen);
    if (ret > 0) {
        int min = 0;
        int max = 0;
        if (sscanf_s(paramOutBuf, "%d,%d", &min, &max) == 2) { // 2: two parameters.
            g_maxSize = max > 0 ? max : INT_MAX;
            g_ClientConfig.filterSize = min > 0 ? min : 0;
        }
        HILOG_INFO(LOG_CORE, "persist.hiviewdfx.profiler.mem.filter %s, min %d, max %d",
                   paramOutBuf, g_ClientConfig.filterSize, g_maxSize);
    }
    return true;
}

void* ohos_release_on_end(void*)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    g_hookClient = nullptr;
    pthread_key_delete(g_disableHookFlag);
    pthread_key_delete(g_hookTid);
    pthread_key_delete(g_updateThreadNameCount);
    g_ClientConfig.Reset();
    HILOG_INFO(LOG_CORE, "ohos_malloc_hook_on_end, mallocTimes :%" PRIu64, g_mallocTimes.load());
    COMMON::PrintMallinfoLog("after hook(byte) => ");
    return nullptr;
}

bool ohos_malloc_hook_on_end(void)
{
    if (g_hookClient != nullptr) {
        g_hookClient->Flush();
    }
    pthread_t threadEnd;
    if (pthread_create(&threadEnd, nullptr, ohos_release_on_end, nullptr)) {
        HILOG_INFO(LOG_CORE, "create ohos_release_on_end fail");
        return false;
    }
    pthread_detach(threadEnd);
    return true;
}

#if defined(__aarch64__)
static int inline __attribute__((always_inline)) FpUnwind(int maxDepth, uint64_t *ip, int stackSize)
{
    void **startfp = (void **)__builtin_frame_address(0);
    void **fp = startfp;
    int depth = 0;
    while (depth < maxDepth) {
        void **nextFp = (void **)*fp;
        if (nextFp <= fp) {
            break;
        }
        if (((nextFp - startfp) * sizeof(void *)) > static_cast<unsigned long>(stackSize)) {
            break;
        }
        fp = nextFp;
        if (*(reinterpret_cast<unsigned long *>(fp + 1)) == 0) {
            break;
        }
        ip[depth++] = *(reinterpret_cast<unsigned long *>(fp + 1));
    }
    return depth;
}
#endif

void* hook_malloc(void* (*fn)(size_t), size_t size)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(size);
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return ret;
    }
    if (!ohos_set_filter_size(size, ret)) {
        return ret;
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    if (!UpdateThreadName(holder)) {
        return ret;
    }
    StackRawData rawdata = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
        stackSize = 0;
#endif
    } else {
        unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
        unw_context_t context;
        unw_getcontext(&context);
#if defined(__arm__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                     sizeof(context.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
#elif defined(__aarch64__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                     sizeof(context.uc_mcontext.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
        regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
        regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
        stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
    }
    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = ret;
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    g_mallocTimes++;
#ifdef PERFORMANCE_DEBUG
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    g_dataCounts += stackSize;
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        HILOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
    return ret;
}

void* hook_valloc(void* (*fn)(size_t), size_t size)
{
    void* pRet = nullptr;
    if (fn) {
        pRet = fn(size);
    }
    return pRet;
}

void* hook_calloc(void* (*fn)(size_t, size_t), size_t number, size_t size)
{
    void* pRet = nullptr;
    if (fn) {
        pRet = fn(number, size);
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return pRet;
    }
    if (!ohos_set_filter_size(number * size, pRet)) {
        return pRet;
    }

    StackRawData rawdata = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
        stackSize = 0;
#endif
    } else {
        unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
        unw_context_t context;
        unw_getcontext(&context);
#if defined(__arm__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                     sizeof(context.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
#elif defined(__aarch64__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                     sizeof(context.uc_mcontext.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
        regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
        regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
        stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
    }

    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = number * size;
    rawdata.addr = pRet;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder != nullptr) {
        int realSize = 0;
        if (g_ClientConfig.fpunwind) {
            realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
        } else {
            realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
        }
        holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    }
    g_mallocTimes++;
    return pRet;
}

void* hook_memalign(void* (*fn)(size_t, size_t), size_t align, size_t bytes)
{
    void* pRet = nullptr;
    if (fn) {
        pRet = fn(align, bytes);
    }
    return pRet;
}

void* hook_realloc(void* (*fn)(void*, size_t), void* ptr, size_t size)
{
    void* pRet = nullptr;
    if (fn) {
        pRet = fn(ptr, size);
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return pRet;
    }
    if (!ohos_set_filter_size(size, pRet)) {
        return pRet;
    }

    StackRawData rawdata = {{{{0}}}};
    StackRawData freeData = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
        stackSize = 0;
        if (g_ClientConfig.freeStackData) {
            (void)memcpy_s(freeData.ip, sizeof(freeData.ip) / sizeof(uint64_t),
                           rawdata.ip, sizeof(rawdata.ip) / sizeof(uint64_t));
        }
#endif
    } else {
        unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
        unw_context_t context;
        unw_getcontext(&context);
#if defined(__arm__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                     sizeof(context.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
#elif defined(__aarch64__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                     sizeof(context.uc_mcontext.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
        regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
        regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
        stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        if (g_ClientConfig.freeStackData) {
            (void)memcpy_s(freeData.regs, sizeof(freeData.regs) / sizeof(char),
                           rawdata.regs, sizeof(rawdata.regs) / sizeof(char));
        }
    }

    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = pRet;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder != nullptr) {
        int realSize = 0;
        freeData.type = FREE_MSG;
        freeData.pid = rawdata.pid;
        freeData.tid = rawdata.tid;
        freeData.mallocSize = 0;
        freeData.addr = ptr;
        freeData.ts = rawdata.ts;
        if (g_ClientConfig.fpunwind) {
            realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
        } else {
            realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
        }
        holder->SendStackWithPayload(&freeData, sizeof(BaseStackRawData), nullptr, 0); // 0: Don't unwind the freeData
        holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    }
    return pRet;
}

size_t hook_malloc_usable_size(size_t (*fn)(void*), void* ptr)
{
    size_t ret = 0;
    if (fn) {
        ret = fn(ptr);
    }

    return ret;
}

void hook_free(void (*free_func)(void*), void* p)
{
    if (free_func) {
        free_func(p);
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return;
    }
    StackRawData rawdata = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.freeStackData) {
        if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
            stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
            fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
            stackSize = 0;
#endif
        } else {
            unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
            unw_context_t context;
            unw_getcontext(&context);
#if defined(__arm__)
            if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                         sizeof(context.regs)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
            }
#elif defined(__aarch64__)
            if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                         sizeof(context.uc_mcontext.regs)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
            }
            regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
            regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
            stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
        }
    }

    rawdata.type = FREE_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = 0;
    rawdata.addr = p;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder != nullptr) {
        int realSize = 0;
        if (g_ClientConfig.fpunwind) {
            realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
        } else {
            realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
        }
        holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    }
}

inline void SendMmapFileRawData(int prot, int flags, off_t offset, const std::string& filePath,
                                const StackRawData& rawdata, std::shared_ptr<HookSocketClient>& holder)
{
    StackRawData curRawdata = {{{{0}}}};
    curRawdata.addr = rawdata.addr;
    curRawdata.pid = static_cast<uint32_t>(g_hookPid);
    curRawdata.mallocSize = rawdata.mallocSize;
    curRawdata.mmapArgs.offset = offset;
    curRawdata.type = OHOS::Developtools::NativeDaemon::MMAP_FILE_TYPE;
    if (prot & PROT_EXEC) {
        curRawdata.mmapArgs.flags |= PROT_EXEC;
    }
    if (flags & MAP_FIXED) {
        curRawdata.mmapArgs.flags |= MAP_FIXED;
        curRawdata.name[0] = '\0';
        holder->SendStackWithPayload(&curRawdata, sizeof(BaseStackRawData) + 1, nullptr, 0);
    } else {
        size_t len = strlen(filePath.c_str()) + 1;
        (void)strncpy_s(curRawdata.name, PATH_MAX + 1, filePath.c_str(), len);
        holder->SendStackWithPayload(&curRawdata, sizeof(BaseStackRawData) + len, nullptr, 0);
    }
}

void* hook_mmap(void*(*fn)(void*, size_t, int, int, int, off_t),
    void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(addr, length, prot, flags, fd, offset);
    }
    if (g_ClientConfig.mmapDisable || IsPidChanged()) {
        return ret;
    }
    StackRawData rawdata = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
        stackSize = 0;
#endif
    } else {
        unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
        unw_context_t context;
        unw_getcontext(&context);
#if defined(__arm__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                     sizeof(context.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
#elif defined(__aarch64__)
        if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                     sizeof(context.uc_mcontext.regs)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        }
        regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
        regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
        stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
    }

    rawdata.type = MMAP_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = length;
    rawdata.addr = ret;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    if (fd >= 0) {
        rawdata.type = MMAP_FILE_PAGE_MSG;
        char path[FD_PATH_LENGTH] = {0};
        char fileName[PATH_MAX + 1] = {0};
        (void)snprintf_s(path, FD_PATH_LENGTH, FD_PATH_LENGTH - 1, "/proc/self/fd/%d", fd);
        ssize_t len = readlink(path, fileName, sizeof(fileName) - 1);
        if (len != -1) {
            fileName[len] = '\0';
            SendMmapFileRawData(prot, flags, offset, fileName, rawdata, holder);
            char* p = strrchr(fileName, '/');
            if (p != nullptr) {
                rawdata.tagId = GetTagId(holder, &fileName[p - fileName + 1]);
            } else {
                rawdata.tagId = GetTagId(holder, fileName);
            }
        } else {
            HILOG_ERROR(LOG_CORE, "Set mmap fd linked file name failed!");
        }
    }
    if (!UpdateThreadName(holder)) {
        return ret;
    }
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    return ret;
}

int hook_munmap(int(*fn)(void*, size_t), void* addr, size_t length)
{
    int ret = -1;
    if (fn) {
        ret = fn(addr, length);
    }
    if (g_ClientConfig.mmapDisable || IsPidChanged()) {
        return ret;
    }
    int stackSize = 0;
    StackRawData rawdata = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.munmapStackData) {
        if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
            stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
            fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
            stackSize = 0;
#endif
        } else {
            unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
            unw_context_t context;
            unw_getcontext(&context);
#if defined(__arm__)
            if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                         sizeof(context.regs)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
            }
#elif defined(__aarch64__)
            if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                         sizeof(context.uc_mcontext.regs)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
            }
            regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
            regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
            stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
        }
    }

    rawdata.type = MUNMAP_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = length;
    rawdata.addr = addr;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder != nullptr) {
        int realSize = 0;
        if (g_ClientConfig.fpunwind) {
            realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
        } else {
            realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
        }
        holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    }
    return ret;
}

int hook_prctl(int(*fn)(int, ...),
    int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
    int ret = -1;
    if (fn) {
        ret = fn(option, arg2, arg3, arg4, arg5);
    }
    if (reinterpret_cast<char*>(arg5) == nullptr || IsPidChanged()) {
        return ret;
    }
    if (option == PR_SET_VMA && arg2 == PR_SET_VMA_ANON_NAME) {
        StackRawData rawdata = {{{{0}}}};
        clock_gettime(g_ClientConfig.clockId, &rawdata.ts);
        rawdata.type = PR_SET_VMA_MSG;
        rawdata.pid = static_cast<uint32_t>(g_hookPid);
        rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
        rawdata.mallocSize = arg4;
        rawdata.addr = reinterpret_cast<void*>(arg3);

        std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
        auto holder = weakClient.lock();
        if (strcpy_s(rawdata.name, PATH_MAX + 1, reinterpret_cast<char*>(arg5))) {
            HILOG_ERROR(LOG_CORE, "Set tag name failed");
        }
        if (holder != nullptr) {
            holder->SendStackWithPayload(&rawdata,
                                         sizeof(rawdata) + strlen(reinterpret_cast<char*>(arg5)), nullptr, 0);
        }
    }
    return ret;
}

void hook_memtrace(void* addr, size_t size, const char* tag, bool isUsing)
{
    if (!g_ClientConfig.memtraceEnable || IsPidChanged()) {
        return;
    }
    int stackSize = 0;
    StackRawData rawdata = {{{{0}}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (isUsing) {
        if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
            stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
            fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip, stackSize);
            stackSize = 0;
#endif
        } else {
            unsigned long* regs = reinterpret_cast<unsigned long*>(&(rawdata.regs));
            unw_context_t context;
            unw_getcontext(&context);
#if defined(__arm__)
            if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.regs),
                         sizeof(context.regs)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
            }
#elif defined(__aarch64__)
            if (memcpy_s(regs, sizeof(rawdata.regs), reinterpret_cast<char*>(context.uc_mcontext.regs),
                         sizeof(context.uc_mcontext.regs)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
            }
            regs[RegisterGetSP(buildArchType)] = context.uc_mcontext.sp;
            regs[RegisterGetIP(buildArchType)] = context.uc_mcontext.pc;
#endif
            stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
        }
    }
    rawdata.type = isUsing ? MEMORY_USING_MSG : MEMORY_UNUSING_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = addr;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    rawdata.tagId = isUsing ? GetTagId(holder, tag) : 0;
    if (holder != nullptr) {
        int realSize = 0;
        if (g_ClientConfig.fpunwind) {
            realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
        } else {
            realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
        }
        holder->SendStackWithPayload(&rawdata, realSize, stackptr, stackSize);
    }
}

bool ohos_malloc_hook_initialize(const MallocDispatchType*malloc_dispatch, bool*, const char*)
{
    g_dispatch.store(malloc_dispatch);
    InititalizeIPC();
    return true;
}
void ohos_malloc_hook_finalize(void)
{
    FinalizeIPC();
}

void* ohos_malloc_hook_malloc(size_t size)
{
    __set_hook_flag(false);
    void* ret = hook_malloc(GetDispatch()->malloc, size);
    __set_hook_flag(true);
    return ret;
}

void* ohos_malloc_hook_realloc(void* ptr, size_t size)
{
    __set_hook_flag(false);
    void* ret = hook_realloc(GetDispatch()->realloc, ptr, size);
    __set_hook_flag(true);
    return ret;
}

void* ohos_malloc_hook_calloc(size_t number, size_t size)
{
    __set_hook_flag(false);
    void* ret = hook_calloc(GetDispatch()->calloc, number, size);
    __set_hook_flag(true);
    return ret;
}

void* ohos_malloc_hook_valloc(size_t size)
{
    __set_hook_flag(false);
    void* ret = hook_valloc(GetDispatch()->valloc, size);
    __set_hook_flag(true);
    return ret;
}

void ohos_malloc_hook_free(void* p)
{
    __set_hook_flag(false);
    hook_free(GetDispatch()->free, p);
    __set_hook_flag(true);
}

void* ohos_malloc_hook_memalign(size_t alignment, size_t bytes)
{
    __set_hook_flag(false);
    void* ret = hook_memalign(GetDispatch()->memalign, alignment, bytes);
    __set_hook_flag(true);
    return ret;
}

size_t ohos_malloc_hook_malloc_usable_size(void* mem)
{
    __set_hook_flag(false);
    size_t ret = hook_malloc_usable_size(GetDispatch()->malloc_usable_size, mem);
    __set_hook_flag(true);
    return ret;
}

bool ohos_malloc_hook_get_hook_flag(void)
{
    return pthread_getspecific(g_disableHookFlag) == nullptr;
}

bool ohos_malloc_hook_set_hook_flag(bool flag)
{
    bool oldFlag = ohos_malloc_hook_get_hook_flag();
    if (flag) {
        pthread_setspecific(g_disableHookFlag, nullptr);
    } else {
        pthread_setspecific(g_disableHookFlag, reinterpret_cast<void *>(1));
    }
    return oldFlag;
}

void* ohos_malloc_hook_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    __set_hook_flag(false);
    void* ret = hook_mmap(GetDispatch()->mmap, addr, length, prot, flags, fd, offset);
    __set_hook_flag(true);
    return ret;
}

int ohos_malloc_hook_munmap(void* addr, size_t length)
{
    __set_hook_flag(false);
    int ret = hook_munmap(GetDispatch()->munmap, addr, length);
    __set_hook_flag(true);
    return ret;
}

void ohos_malloc_hook_memtrace(void* addr, size_t size, const char* tag, bool isUsing)
{
    __set_hook_flag(false);
    hook_memtrace(addr, size, tag, isUsing);
    __set_hook_flag(true);
}

int  ohos_malloc_hook_prctl(int option, unsigned long arg2, unsigned long arg3,
                            unsigned long arg4, unsigned long arg5)
{
    __set_hook_flag(false);
    int ret = hook_prctl((GetDispatch()->prctl), option, arg2, arg3, arg4, arg5);
    __set_hook_flag(true);
    return ret;
}

bool ohos_set_filter_size(size_t size, void* ret)
{
    if (g_ClientConfig.filterSize < 0 || size < static_cast<size_t>(g_ClientConfig.filterSize) || size > g_maxSize) {
        return false;
    }
    return true;
}

static bool IsPidChanged(void)
{
    if (g_isPidChanged) {
        return true;
    }
    int pid = getpid();
    // hap app after pid namespace used
    if (pid == PID_NAMESPACE_ID) {
        return false;
    } else {
        // native app & sa service
        g_isPidChanged = (g_hookPid != pid);
    }
    return g_isPidChanged;
}