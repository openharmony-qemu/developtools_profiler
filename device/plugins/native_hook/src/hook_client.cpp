/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <unordered_set>
#include "common.h"
#include "hook_common.h"
#include "hook_socket_client.h"
#include "musl_preinit_common.h"
#include "parameter.h"
#include "stack_writer.h"
#include "runtime_stack_range.h"
#include "register.h"
#include "virtual_runtime.h"
#include "get_thread_id.h"
#include "hook_client.h"

static pthread_key_t g_disableHookFlag;
static pthread_key_t g_hookTid;
namespace {
static std::atomic<uint64_t> g_timeCost = 0;
static std::atomic<uint64_t> g_mallocTimes = 0;
static std::atomic<uint64_t> g_dataCounts = 0;
using OHOS::Developtools::NativeDaemon::buildArchType;
static std::shared_ptr<HookSocketClient> g_hookClient;
std::recursive_timed_mutex g_ClientMutex;
std::atomic<const MallocDispatchType*> g_dispatch {nullptr};
constexpr int TIMEOUT_MSEC = 2000;
constexpr int PRINT_INTERVAL = 5000;
constexpr uint64_t S_TO_NS = 1000 * 1000 * 1000;
static pid_t g_hookPid = 0;
static ClientConfig g_ClientConfig = {0};
static uint32_t g_minSize = 0;
static uint32_t g_maxSize = INT_MAX;
static std::unordered_set<void*> g_mallocIgnoreSet;
constexpr int PID_STR_SIZE = 4;
constexpr int STATUS_LINE_SIZE = 512;
constexpr int PID_NAMESPACE_ID = 1; // 1: pid is 1 after pid namespace used
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
    unsigned long count = 0;
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
    if (fp == nullptr) {
        return -1;
    }
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
        pthread_setspecific(g_hookTid, reinterpret_cast<void *>(get_thread_id()));
    }
    return reinterpret_cast<long>((pthread_getspecific(g_hookTid)));
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
        g_hookClient = std::make_shared<HookSocketClient>(g_hookPid, &g_ClientConfig);
    }
    pthread_key_create(&g_disableHookFlag, nullptr);
    pthread_setspecific(g_disableHookFlag, nullptr);
    pthread_key_create(&g_hookTid, nullptr);
    pthread_setspecific(g_hookTid, nullptr);
    HILOG_INFO(LOG_CORE, "ohos_malloc_hook_on_start");
    GetMainThreadRuntimeStackRange();
    g_minSize = g_ClientConfig.filterSize_;
    constexpr int paramBufferLen = 128;
    char paramOutBuf[paramBufferLen] = {0};
    int ret = GetParameter("persist.hiviewdfx.profiler.mem.filter", "", paramOutBuf, paramBufferLen);
    if (ret > 0) {
        int min = 0;
        int max = 0;
        if (sscanf_s(paramOutBuf, "%d,%d", &min, &max) == 2) { // 2: two parameters.
            g_maxSize = max > 0 ? max : INT_MAX;
            g_minSize = min > 0 ? min : 0;
        }
        HILOG_INFO(LOG_CORE, "persist.hiviewdfx.profiler.mem.filter %s, min %d, max %d", paramOutBuf, g_minSize,
                   g_maxSize);
    }
    return true;
}

void* ohos_release_on_end(void*)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    g_hookClient = nullptr;
    pthread_key_delete(g_disableHookFlag);
    pthread_key_delete(g_hookTid);
    g_mallocIgnoreSet.clear();
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
static void inline __attribute__((always_inline)) FpUnwind(int maxDepth, uint64_t *ip, unsigned long stackSize)
{
    void **startfp = (void **)__builtin_frame_address(0);
    void **fp = startfp;
    for (int i = 0; i < maxDepth + 1; i++) {
        ip[i] = *(reinterpret_cast<unsigned long *>(fp + 1));
        void **nextFp = (void **)*fp;
        if (nextFp <= fp) {
            break;
        }
        if (((nextFp - startfp) * sizeof(void *)) > static_cast<unsigned long>(stackSize)) {
            break;
        }
        fp = nextFp;
    }
}
#endif

void* hook_malloc(void* (*fn)(size_t), size_t size)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(size);
    }
    if (g_ClientConfig.mallocDisable_ || IsPidChanged()) {
        return ret;
    }
    if (!ohos_set_filter_size(size, ret)) {
        return ret;
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif
    StackRawData rawdata = {{{0}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);

    if (g_ClientConfig.fpunwind_) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        FpUnwind(g_ClientConfig.maxStackDepth_, rawdata.ip, stackSize);
        stackSize = 0;
#endif
    } else {
        uint64_t* regs = reinterpret_cast<uint64_t*>(&(rawdata.regs));
#if defined(__arm__)
        asm volatile(
            "mov r3, r13\n"
            "mov r4, r15\n"
            "stmia %[base], {r3-r4}\n"
            : [ base ] "+r"(regs)
            :
            : "r3", "r4", "memory");
#elif defined(__aarch64__)
        asm volatile(
            "1:\n"
            "stp x28, x29, [%[base], #224]\n"
            "str x30, [%[base], #240]\n"
            "mov x12, sp\n"
            "adr x13, 1b\n"
            "stp x12, x13, [%[base], #248]\n"
            : [ base ] "+r"(regs)
            :
            : "x12", "x13", "memory");
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
    prctl(PR_GET_NAME, rawdata.tname);
    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock hook_malloc failed!");
        return ret;
    }

    if (g_hookClient != nullptr) {
        g_hookClient->SendStackWithPayload(&rawdata, sizeof(rawdata), stackptr, stackSize);
    }
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
    if (g_ClientConfig.mallocDisable_ || IsPidChanged()) {
        return pRet;
    }
    if (!ohos_set_filter_size(number * size, pRet)) {
        return pRet;
    }

    StackRawData rawdata = {{{0}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);

    if (g_ClientConfig.fpunwind_) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        FpUnwind(g_ClientConfig.maxStackDepth_, rawdata.ip, stackSize);
        stackSize = 0;
#endif
    } else {
        uint64_t* regs = reinterpret_cast<uint64_t*>(&(rawdata.regs));
#if defined(__arm__)
        asm volatile(
            "mov r3, r13\n"
            "mov r4, r15\n"
            "stmia %[base], {r3-r4}\n"
            : [ base ] "+r"(regs)
            :
            : "r3", "r4", "memory");
#elif defined(__aarch64__)
        asm volatile(
            "1:\n"
            "stp x28, x29, [%[base], #224]\n"
            "str x30, [%[base], #240]\n"
            "mov x12, sp\n"
            "adr x13, 1b\n"
            "stp x12, x13, [%[base], #248]\n"
            : [ base ] "+r"(regs)
            :
            : "x12", "x13", "memory");
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
    prctl(PR_GET_NAME, rawdata.tname);
    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock hook_calloc failed!");
        return pRet;
    }

    if (g_hookClient != nullptr) {
        g_hookClient->SendStackWithPayload(&rawdata, sizeof(rawdata), stackptr, stackSize);
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
    if (g_ClientConfig.mallocDisable_ || IsPidChanged()) {
        return pRet;
    }
    if (!ohos_set_filter_size(size, pRet)) {
        return pRet;
    }

    StackRawData rawdata = {{{0}}};
    StackRawData freeData = {{{0}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);

    if (g_ClientConfig.fpunwind_) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        FpUnwind(g_ClientConfig.maxStackDepth_, rawdata.ip, stackSize);
        stackSize = 0;
        if (g_ClientConfig.freeStackData_) {
            (void)memcpy_s(freeData.ip, sizeof(freeData.ip) / sizeof(uint64_t),
                           rawdata.ip, sizeof(rawdata.ip) / sizeof(uint64_t));
        }
#endif
    } else {
        uint64_t* regs = reinterpret_cast<uint64_t*>(&(rawdata.regs));
#if defined(__arm__)
        asm volatile(
            "mov r3, r13\n"
            "mov r4, r15\n"
            "stmia %[base], {r3-r4}\n"
            : [ base ] "+r"(regs)
            :
            : "r3", "r4", "memory");
#elif defined(__aarch64__)
        asm volatile(
            "1:\n"
            "stp x28, x29, [%[base], #224]\n"
            "str x30, [%[base], #240]\n"
            "mov x12, sp\n"
            "adr x13, 1b\n"
            "stp x12, x13, [%[base], #248]\n"
            : [ base ] "+r"(regs)
            :
            : "x12", "x13", "memory");
#endif
        stackptr = reinterpret_cast<const char*>(regs[RegisterGetSP(buildArchType)]);
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        if (g_ClientConfig.freeStackData_) {
            (void)memcpy_s(freeData.regs, sizeof(freeData.regs) / sizeof(char),
                           rawdata.regs, sizeof(rawdata.regs) / sizeof(char));
        }
    }
    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid);
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = pRet;
    prctl(PR_GET_NAME, rawdata.tname);
    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock hook_realloc failed!");
        return pRet;
    }

    if (g_hookClient != nullptr) {
        freeData.type = FREE_MSG;
        freeData.pid = rawdata.pid;
        freeData.tid = rawdata.tid;
        freeData.mallocSize = 0;
        freeData.addr = ptr;
        freeData.ts = rawdata.ts;
        (void)memcpy_s(freeData.tname, sizeof(freeData.tname) / sizeof(char),
                       rawdata.tname, sizeof(rawdata.tname) / sizeof(char));
        g_hookClient->SendStackWithPayload(&freeData, sizeof(freeData), nullptr, 0); // 0: Don't unwind the freeData
        g_hookClient->SendStackWithPayload(&rawdata, sizeof(rawdata), stackptr, stackSize);
    }
    g_mallocTimes++;
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
    if (g_ClientConfig.mallocDisable_ || IsPidChanged()) {
        return;
    }
    {
        std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
        auto record = g_mallocIgnoreSet.find(p);
        if (record != g_mallocIgnoreSet.end()) {
            g_mallocIgnoreSet.erase(record);
            return;
        }
    }
    StackRawData rawdata = {{{0}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);

    if (g_ClientConfig.freeStackData_) {
        if (g_ClientConfig.fpunwind_) {
#ifdef __aarch64__
            stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
            FpUnwind(g_ClientConfig.maxStackDepth_, rawdata.ip, stackSize);
            stackSize = 0;
#endif
        } else {
            uint64_t* regs = reinterpret_cast<uint64_t*>(&(rawdata.regs));
#if defined(__arm__)
            asm volatile(
                "mov r3, r13\n"
                "mov r4, r15\n"
                "stmia %[base], {r3-r4}\n"
                : [ base ] "+r"(regs)
                :
                : "r3", "r4", "memory");
#elif defined(__aarch64__)
            asm volatile(
                "1:\n"
                "stp x28, x29, [%[base], #224]\n"
                "str x30, [%[base], #240]\n"
                "mov x12, sp\n"
                "adr x13, 1b\n"
                "stp x12, x13, [%[base], #248]\n"
                : [ base ] "+r"(regs)
                :
                : "x12", "x13", "memory");
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
    prctl(PR_GET_NAME, rawdata.tname);

    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock hook_free failed!");
        return;
    }

    if (g_hookClient != nullptr) {
        g_hookClient->SendStackWithPayload(&rawdata, sizeof(rawdata), stackptr, stackSize);
    }
}

void* hook_mmap(void*(*fn)(void*, size_t, int, int, int, off_t),
    void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(addr, length, prot, flags, fd, offset);
    }
    if (g_ClientConfig.mmapDisable_ || IsPidChanged()) {
        return ret;
    }
    StackRawData rawdata = {{{0}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    int stackSize = 0;
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);

    if (g_ClientConfig.fpunwind_) {
#ifdef __aarch64__
        stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
        GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
        stackSize = stackendptr - stackptr;
        FpUnwind(g_ClientConfig.maxStackDepth_, rawdata.ip, stackSize);
        stackSize = 0;
#endif
    } else {
        uint64_t* regs = reinterpret_cast<uint64_t*>(&(rawdata.regs));
#if defined(__arm__)
        asm volatile(
            "mov r3, r13\n"
            "mov r4, r15\n"
            "stmia %[base], {r3-r4}\n"
            : [ base ] "+r"(regs)
            :
            : "r3", "r4", "memory");
#elif defined(__aarch64__)
        asm volatile(
            "1:\n"
            "stp x28, x29, [%[base], #224]\n"
            "str x30, [%[base], #240]\n"
            "mov x12, sp\n"
            "adr x13, 1b\n"
            "stp x12, x13, [%[base], #248]\n"
            : [ base ] "+r"(regs)
            :
            : "x12", "x13", "memory");
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
    prctl(PR_GET_NAME, rawdata.tname);

    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock hook_mmap failed!");
        return ret;
    }
    if (g_hookClient != nullptr) {
        g_hookClient->SendStackWithPayload(&rawdata, sizeof(rawdata), stackptr, stackSize);
    }
    return ret;
}

int hook_munmap(int(*fn)(void*, size_t), void* addr, size_t length)
{
    int ret = -1;
    if (fn) {
        ret = fn(addr, length);
    }
    if (g_ClientConfig.mmapDisable_ || IsPidChanged()) {
        return ret;
    }
    int stackSize = 0;
    StackRawData rawdata = {{{0}}};
    const char* stackptr = nullptr;
    const char* stackendptr = nullptr;
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);

    if (g_ClientConfig.munmapStackData_) {
        if (g_ClientConfig.fpunwind_) {
#ifdef __aarch64__
            stackptr = reinterpret_cast<const char*>(__builtin_frame_address(0));
            GetRuntimeStackEnd(stackptr, &stackendptr, g_hookPid, GetCurThreadId());  // stack end pointer
            stackSize = stackendptr - stackptr;
            FpUnwind(g_ClientConfig.maxStackDepth_, rawdata.ip, stackSize);
            stackSize = 0;
#endif
        } else {
            uint64_t* regs = reinterpret_cast<uint64_t*>(&(rawdata.regs));
#if defined(__arm__)
            asm volatile(
                "mov r3, r13\n"
                "mov r4, r15\n"
                "stmia %[base], {r3-r4}\n"
                : [ base ] "+r"(regs)
                :
                : "r3", "r4", "memory");
#elif defined(__aarch64__)
            asm volatile(
                "1:\n"
                "stp x28, x29, [%[base], #224]\n"
                "str x30, [%[base], #240]\n"
                "mov x12, sp\n"
                "adr x13, 1b\n"
                "stp x12, x13, [%[base], #248]\n"
                : [ base ] "+r"(regs)
                :
                : "x12", "x13", "memory");
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
    prctl(PR_GET_NAME, rawdata.tname);

    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock hook_munmap failed!");
        return ret;
    }
    if (g_hookClient != nullptr) {
        g_hookClient->SendStackWithPayload(&rawdata, sizeof(rawdata), stackptr, stackSize);
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
    if (IsPidChanged()) {
        return ret;
    }
    if (option == PR_SET_VMA && arg2 == PR_SET_VMA_ANON_NAME) {
        StackRawData rawdata = {{{0}}};
        clock_gettime(CLOCK_REALTIME, &rawdata.ts);
        rawdata.type = PR_SET_VMA_MSG;
        rawdata.pid = static_cast<uint32_t>(g_hookPid);
        rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
        rawdata.mallocSize = arg4;
        rawdata.addr = reinterpret_cast<void*>(arg3);
        size_t tagLen = strlen(reinterpret_cast<char*>(arg5)) + 1;
        if (memcpy_s(rawdata.tname, sizeof(rawdata.tname), reinterpret_cast<char*>(arg5), tagLen) != EOK) {
            HILOG_ERROR(LOG_CORE, "memcpy_s tag failed");
        }
        rawdata.tname[sizeof(rawdata.tname) - 1] = '\0';
        std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
        std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
        if (!lck.try_lock_until(timeout)) {
            HILOG_ERROR(LOG_CORE, "lock failed!");
            return ret;
        }
        if (g_hookClient != nullptr) {
            g_hookClient->SendStack(&rawdata, sizeof(rawdata));
        }
    }
    return ret;
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

void ohos_malloc_hook_memtag(void* addr, size_t size, char* tag, size_t tagLen)
{
    __set_hook_flag(false);
    if (IsPidChanged()) {
        return;
    }
    StackRawData rawdata = {{{0}}};
    clock_gettime(CLOCK_REALTIME, &rawdata.ts);
    rawdata.type = MEMORY_TAG;
    rawdata.pid = getpid();
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = addr;

    if (memcpy_s(rawdata.tname, sizeof(rawdata.tname), tag, tagLen) != EOK) {
        HILOG_ERROR(LOG_CORE, "memcpy_s tag failed");
    }
    rawdata.tname[sizeof(rawdata.tname) - 1] = '\0';

    std::unique_lock<std::recursive_timed_mutex> lck(g_ClientMutex, std::defer_lock);
    std::chrono::time_point<std::chrono::steady_clock> timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMEOUT_MSEC);
    if (!lck.try_lock_until(timeout)) {
        HILOG_ERROR(LOG_CORE, "lock failed!");
        return;
    }
    if (g_hookClient != nullptr) {
        g_hookClient->SendStack(&rawdata, sizeof(rawdata));
    }
    __set_hook_flag(true);
}

bool ohos_set_filter_size(size_t size, void* ret)
{
    if ((size < g_minSize) || (size > g_maxSize)) {
        std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
        g_mallocIgnoreSet.insert(ret);
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