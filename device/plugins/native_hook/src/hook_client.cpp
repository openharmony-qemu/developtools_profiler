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

#include <climits>
#include <cstdint>
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <malloc.h>
#include <new>
#include <pthread.h>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <sys/prctl.h>
#include <unordered_map>
#include <unordered_set>
#include "dfx_regs_get.h"
#include "common.h"
#include "logging.h"
#include "musl_preinit_common.h"
#include "parameter.h"
#include "stack_writer.h"
#include "runtime_stack_range.h"
#include "get_thread_id.h"
#include "hook_client.h"
#include <sys/mman.h>
#include "sampling.h"
#include "hitrace/trace.h"
#include "address_handler.h"
#include "hook_guard.h"
#include "hook_client_holder.h"
#include "rawdata_builder.h"
#include "performance_tracker.h"
#ifdef HAVE_HILOG
#include "async_stack.h"
#endif

using namespace OHOS::HiviewDFX;
using namespace OHOS::Developtools::NativeDaemon;

static pthread_key_t g_disableHookFlag = 10000;
static pthread_key_t g_hookTid;
pthread_key_t g_updateThreadNameCount = 10000;
static pthread_once_t g_onceFlag;

// Global variables that need to be exported
std::recursive_timed_mutex g_ClientMutex;
std::recursive_timed_mutex g_FilterMapMutex;
std::mutex g_tagMapMutex;
std::mutex g_tagIdMutex;
std::mutex g_usableSizeMapMutex;
std::atomic<const MallocDispatchType*> g_dispatch {nullptr};
std::atomic<pid_t> g_hookPid = 0;
std::atomic<bool> g_hookReady = false;
ClientConfig g_clientConfig = {0};
std::atomic<bool> g_isPidChanged = false;
std::unordered_map<size_t, size_t> g_mallocUsableSizeMap;
std::shared_ptr<AddressHandler> g_addrHandler = nullptr;
std::vector<std::pair<uint64_t, uint64_t>> g_filterStaLibRange;
std::atomic<Range> g_targetedRange;
std::shared_ptr<HookSocketClient> g_hookClient {nullptr};
std::atomic<int> g_sharedMemCount = 1;

#if defined(__aarch64__)
std::shared_ptr<AddressHandler> g_wholeAddrHandler = nullptr;
std::shared_ptr<AddressHandler> g_midPartHandler = nullptr;
#endif

namespace {
static std::atomic<uint16_t> g_tagId = RESTRACE_TYPE_COUNT + 1;

enum class MISC_TYPE : uint32_t {
    JS_STACK_DATA = 1,
};

struct StackParam {
    bool needCollect = true;
    bool needFpunwind = true;
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    bool needJsStack = true;
};

using OHOS::Developtools::NativeDaemon::buildArchType;
static uint32_t g_maxSize = INT_MAX;
static std::unordered_map<std::string, uint32_t> g_memTagMap;
constexpr int PID_STR_SIZE = 4;
constexpr int STATUS_LINE_SIZE = 512;
constexpr int PID_NAMESPACE_ID = 1; // 1: pid is 1 after pid namespace used
constexpr int FD_PATH_LENGTH = 64;
constexpr int MIN_SAMPLER_INTERVAL = 1;
constexpr int THRESHOLD = 256;
constexpr uintptr_t MAX_UNWIND_ADDR_RANGE = 16 * 1024;
//5: fp mode is used, response_library_mode maximum stack depth
#if defined(__aarch64__)
constexpr int RESPONSE_LIBRARY_MODE_DEPTH = 5;
#endif
constexpr int MAX_BITPOOL_SIZE = 1000 * 1024;
constexpr uint64_t PROFILER_TYPE = 1ULL << 1;
static pthread_key_t g_sampleKey = 10000;

static void DestroySampleKey(void* value)
{
    delete static_cast<Sampling*>(value);
}

const MallocDispatchType* GetDispatch()
{
    return g_dispatch.load(std::memory_order_relaxed);
}

bool InititalizeIPC()
{
    return true;
}
void FinalizeIPC() {}

int ConvertPid(char* buf, size_t len)
{
    UNUSED_PARAMETER(len);
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
    while (fp != nullptr && !feof(fp)) {
        if (fgets(buf, STATUS_LINE_SIZE, fp) == nullptr) {
            fclose(fp);
            return -1;
        }
        if (strncmp(buf, "Pid:", PID_STR_SIZE) == 0) {
            break;
        }
    }
    (void)fclose(fp);
    return static_cast<pid_t>(ConvertPid(buf, sizeof(buf)));
}
}  // namespace

inline void FillNodeInfo(BaseStackRawData& rawdata)
{
    uint32_t nodeType;
    uint64_t nodeId;
    if (getResTraceId(&nodeType, &nodeId)) {
        rawdata.nodeType = nodeType;
        rawdata.nodeId = nodeId;
    }
}

pid_t GetCurThreadId()
{
    if (pthread_getspecific(g_hookTid) == nullptr) {
        pthread_setspecific(g_hookTid, reinterpret_cast<void *>(GetThreadId()));
    }
    return reinterpret_cast<long>((pthread_getspecific(g_hookTid)));
}

uint16_t GetTagId(const char* tagName, unsigned long long mask = 0)
{
    if (tagName == nullptr || strlen(tagName) > MAX_HOOK_PATH) {
        return 0;
    }
    uint16_t predefinedId = HookGuard::GetPredefinedTagId(mask);
    if (predefinedId != 0) {
        return predefinedId;
    }
    std::unique_lock<std::mutex> lock(g_tagIdMutex);
    NameData tagData = {{{{0}}}};
    tagData.type = MEMORY_TAG;
    ++g_tagId;
    if (g_tagId.load() == 0) {
        g_tagId.store(RESTRACE_TYPE_COUNT + 1);
    }
    tagData.tagId = g_tagId.load();
    lock.unlock();
    if (strcpy_s(tagData.name, MAX_HOOK_PATH + 1, tagName) != EOK) {
        return 0;
    }
    HookClientHolder client(g_hookClient);
    client.SendStackWithPayload(&tagData, sizeof(BaseStackRawData) + strlen(tagName) + 1, nullptr, 0);
    return tagData.tagId;
}

void* MallocHookStart(void* disableHookCallback)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    if (HookGuard::IsReady()) {
        return nullptr;
    }
    PROFILER_LOG_INFO(LOG_CORE, "MallocHookStart begin!");
    g_hookClient.reset();
    g_hookPid = GetRealPid();
    ParseSelfMaps(g_filterStaLibRange);
    if (g_hookClient != nullptr) {
        return nullptr;
    } else {
        g_clientConfig.Reset();
        g_hookClient = std::make_shared<HookSocketClient>(g_hookPid.load(), &g_clientConfig,
                                                          &g_targetedRange, &g_sharedMemCount,
                                                          reinterpret_cast<void (*)()>(disableHookCallback));
        if (g_hookClient->GetConnectState()) {
            g_addrHandler = std::make_shared<LowAddrHandler>();
#ifdef __aarch64__
            g_wholeAddrHandler = std::make_shared<WholeAddrHandler>();
            g_midPartHandler = std::make_shared<MidAddrHandler>();
            g_midPartHandler->SetSuccessor(std::move(g_wholeAddrHandler));
            g_addrHandler->SetSuccessor(std::move(g_midPartHandler));
            g_wholeAddrHandler = nullptr;
            g_midPartHandler = nullptr;
#endif
            g_hookReady = true;
        } else {
            void (*callback)() = reinterpret_cast<void (*)()>(disableHookCallback);
            if (callback != nullptr) {
                callback();
            }
        }
    }
    return nullptr;
}

static void InitHookTidKey()
{
    if (pthread_key_create(&g_hookTid, nullptr) != 0) {
        return;
    }
    pthread_setspecific(g_hookTid, nullptr);
    if (g_sampleKey != 10000) { // 10000: initial value
        pthread_key_delete(g_sampleKey);
    }
    if (pthread_key_create(&g_sampleKey, DestroySampleKey) != 0) {
        return;
    }
    pthread_setspecific(g_sampleKey, nullptr);
}

static bool InitTheadKey()
{
    if (g_disableHookFlag != 10000) { // 10000: initial value
        pthread_key_delete(g_disableHookFlag);
    }
    if (pthread_key_create(&g_disableHookFlag, nullptr) != 0) {
        return false;
    }
    pthread_setspecific(g_disableHookFlag, nullptr);
    pthread_once(&g_onceFlag, InitHookTidKey);
    if (g_updateThreadNameCount != 10000) { // 10000: initial value
        pthread_key_delete(g_updateThreadNameCount);
    }
    if (pthread_key_create(&g_updateThreadNameCount, nullptr) != 0) {
        return false;
    }
    pthread_setspecific(g_updateThreadNameCount, reinterpret_cast<void *>(0));
    return true;
}

bool ohos_malloc_hook_on_start(void (*disableHookCallback)())
{
    pthread_t threadStart;
    if (pthread_create(&threadStart, nullptr, MallocHookStart, reinterpret_cast<void*>(disableHookCallback)) != 0) {
        disableHookCallback();
        return false;
    }
    pthread_detach(threadStart);
    if (!InitTheadKey()) {
        disableHookCallback();
        return false;
    }
    return true;
}

void* ohos_release_on_end(void*)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    if (!HookGuard::IsReady()) {
        return nullptr;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ohos_release_on_end begin!");
    if (g_hookClient != nullptr) {
        g_hookClient->SendEndMsg();
        PERF_PRINT_RESULTS();
        g_hookClient->Flush();
    }
    g_addrHandler = nullptr;
    g_hookClient = nullptr;
    g_clientConfig.Reset();
    g_sharedMemCount = 1;
    std::unique_lock<std::mutex> lock(g_tagMapMutex);
    g_memTagMap.clear();
    g_hookReady = false;
    return nullptr;
}

bool ohos_malloc_hook_on_end(void)
{
    pthread_t threadEnd;
    if (pthread_create(&threadEnd, nullptr, ohos_release_on_end, nullptr)) {
        return false;
    }
    pthread_detach(threadEnd);
    return true;
}

bool FilterStandardSoIp(uint64_t ip)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_FilterMapMutex);
    for (auto [soBegin, soEnd_]: g_filterStaLibRange) {
        if (ip >= soBegin && ip < soEnd_) {
            return true;
        }
    }
    return false;
}

bool CheckTargetLibIp(uint64_t ip)
{
    auto range = g_targetedRange.load();
    return (ip >= range.start && ip < range.end);
}

#if defined(__aarch64__)
static int inline __attribute__((always_inline)) FpUnwind(int maxDepth, uint64_t* ips)
{
    uintptr_t stackBottom = 0;
    uintptr_t stackTop = 0;
    uintptr_t stackPtr = reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
    int depth = 0;
    if (!GetRuntimeStackRange(stackPtr, stackBottom, stackTop, g_hookPid.load() == GetCurThreadId())) {
        return depth;
    }

    uintptr_t startFp = stackPtr;
    uintptr_t nextFp = *reinterpret_cast<uintptr_t*>(startFp);
    if (nextFp <= stackPtr) {
        return depth;
    }
    uintptr_t fp = nextFp; // skip current frame
    bool filterTarget = true;
    int count = 0;
    uint64_t ip = 0;
    while (depth < maxDepth && (fp - startFp < MAX_UNWIND_ADDR_RANGE)) {
        if (fp < stackBottom || fp >= stackTop - sizeof(uintptr_t)) {
            break;
        }
        ip = *reinterpret_cast<uintptr_t*>(fp + sizeof(uintptr_t));
        if (g_clientConfig.responseLibraryMode) {
            if (++count >= RESPONSE_LIBRARY_MODE_DEPTH || !FilterStandardSoIp(ip)) {
                break;
            }
        } else {
            ips[depth++] = ip > 0x4 ? ip - 0x4 : ip; // adjust pc in Arm64 architecture
        }

        if ((!g_clientConfig.targetSoName.empty()) && CheckTargetLibIp(ip)) {
            filterTarget = false;
        }
        nextFp = *reinterpret_cast<uintptr_t*>(fp);
        if (nextFp <= stackPtr) {
            depth -= 1;
            break;
        }
        if (fp == nextFp) {
            depth -= 1;
            break;
        }
        fp = nextFp;
    }
    if (g_clientConfig.responseLibraryMode) {
        ips[0] = ip > 0x4 ? ip - 0x4 : ip;
        depth = 1;
    }
    if ((!g_clientConfig.targetSoName.empty()) && filterTarget) {
        depth = 0;
    }
    return depth;
}

uint64_t getJsChainId()
{
    if (g_clientConfig.arktsConfig.jsStackReport > 0) {
            OHOS::HiviewDFX::HiTraceId hitraceId = OHOS::HiviewDFX::HiTraceChain::GetId();
            if (hitraceId.IsValid()) {
                return hitraceId.GetChainId();
            }
    }
    return 0;
}
#endif

static int inline __attribute__((always_inline)) GetStackSize(uintptr_t& stackPtr, StackRawData& rawdata)
{
    uintptr_t* regs = reinterpret_cast<uintptr_t*>(&(rawdata.regs));
    GetLocalRegs(regs);
    stackPtr = reinterpret_cast<uintptr_t>(regs[RegisterGetSP(buildArchType)]);
    uintptr_t stackBottom = 0;
    uintptr_t stackTop = 0;
    int stackSize = 0;
    if (!GetRuntimeStackRange(stackPtr, stackBottom, stackTop, g_hookPid.load() == GetCurThreadId())) {
        return stackSize;
    }
    stackSize = static_cast<int>(stackTop - stackPtr);
    return stackSize;
}

static bool inline __attribute__((always_inline)) CollectStackInfoWithAsyncData(
    StackRawData& rawdata, AsyncStackData& asyncData, StackParam &param)
{
    if (!param.needCollect) {
        return true;
    }
    if (g_clientConfig.fpunwind && param.needFpunwind) {
#ifdef __aarch64__
        if (g_clientConfig.asyncStackEnable) {
            if (g_clientConfig.maxStackDepth == 0) {
                return false;
            }
            asyncData.asyncStackId = DfxGetSubmitterStackId();
            asyncData.syncStackId = DfxCollectStackWithDepth(PROFILER_TYPE, g_clientConfig.maxStackDepth);
        } else {
            param.fpStackDepth = FpUnwind(g_clientConfig.maxStackDepth, rawdata.ip);
            if (param.fpStackDepth == 0) {
                return false;
            }
            if (param.needJsStack) {
                rawdata.jsChainId = getJsChainId();
            }
        }
#endif
    } else {
        param.stackSize = GetStackSize(param.stackPtr, rawdata);
    }
    return true;
}

static bool inline __attribute__((always_inline)) CollectStackInfo(StackRawData& rawdata, StackParam &param)
{
    if (!param.needCollect) {
        return true;
    }
    if (g_clientConfig.fpunwind && param.needFpunwind) {
#ifdef __aarch64__
        param.fpStackDepth = FpUnwind(g_clientConfig.maxStackDepth, rawdata.ip);
        if (param.fpStackDepth == 0) {
            return false;
        }
        rawdata.jsChainId = getJsChainId();
#endif
    } else {
        param.stackSize = GetStackSize(param.stackPtr, rawdata);
    }
    return true;
}

static void inline __attribute__((always_inline)) SetStackRawData(BaseStackRawData& baseData, uint16_t type,
    void* addr, size_t mallocSize)
{
    baseData.type = type;
    baseData.addr = addr;
    baseData.mallocSize = mallocSize;
    baseData.pid = static_cast<uint32_t>(g_hookPid.load());
    baseData.tid = static_cast<uint32_t>(GetCurThreadId());
    clock_gettime(g_clientConfig.clockId, &baseData.ts);
}

static void inline __attribute__((always_inline)) AddAllocAddr(void* pRet)
{
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_clientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        addrHandler->AddAllocAddr(reinterpret_cast<uint64_t>(pRet));
    }
}

void* __attribute__((noinline)) hook_malloc(void* (*fn)(size_t), size_t size)
{
    void* ret = fn ? fn(size) : nullptr;
    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMalloc()) {
        return ret;
    }
    if (!ohos_set_filter_size(size, ret)) {
        return ret;
    }

    if (HookGuard::ShouldFilterBySize(ret, size) || HookGuard::ShouldSample(size, g_sampleKey)) {
        return ret;
    }

    HookClientHolder client(g_hookClient);
    if (!client.IsValid() || !client.UpdateThreadName()) {
        return ret;
    }
    PERF_TRACK(OperatType::malloc);

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return ret;
    }

    AddAllocAddr(ret);
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    bool sendResult = false;
    if (isAsyncStack) {
        SetStackRawData(asyncData, MALLOC_MSG, ret, size);
        FillNodeInfo(asyncData);
        sendResult = client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    } else {
        SetStackRawData(rawdata, MALLOC_MSG, ret, size);
        FillNodeInfo(rawdata);
        sendResult = client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    }
    if (sendResult) {
        PERF_RECORD_DATA(param.stackSize, size);
    }
    return ret;
}

void* __attribute__((noinline)) hook_aligned_alloc(void* (*fn)(size_t, size_t), size_t align, size_t len)
{
    void* ret = fn ? fn(align, len) : nullptr;

    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMalloc()) {
        return ret;
    }
    if (!ohos_set_filter_size(len, ret)) {
        return ret;
    }

    if (HookGuard::ShouldFilterBySize(ret, len) || HookGuard::ShouldSample(len, g_sampleKey)) {
        return ret;
    }

    HookClientHolder client(g_hookClient);
    if (!client.IsValid() || !client.UpdateThreadName()) {
        return ret;
    }
    PERF_TRACK(OperatType::aligned_alloc);

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return ret;
    }
    AddAllocAddr(ret);
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    bool sendResult = false;
    if (isAsyncStack) {
        SetStackRawData(asyncData, MALLOC_MSG, ret, len);
        FillNodeInfo(asyncData);
        sendResult = client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    } else {
        SetStackRawData(rawdata, MALLOC_MSG, ret, len);
        FillNodeInfo(rawdata);
        sendResult = client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    }
    if (sendResult) {
        PERF_RECORD_DATA(param.stackSize, len);
    }
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

void* __attribute__((noinline)) hook_calloc(void* (*fn)(size_t, size_t), size_t number, size_t size)
{
    void* pRet = fn ? fn(number, size) : nullptr;

    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMalloc()) {
        return pRet;
    }
    size_t totalSize = number * size;
    if (!ohos_set_filter_size(totalSize, pRet)) {
        return pRet;
    }

    if (HookGuard::ShouldFilterBySize(pRet, totalSize) || HookGuard::ShouldSample(totalSize, g_sampleKey)) {
        return pRet;
    }

    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return pRet;
    }
    PERF_TRACK(OperatType::calloc);

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return pRet;
    }
    AddAllocAddr(pRet);
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    bool sendResult = false;
    if (isAsyncStack) {
        SetStackRawData(asyncData, MALLOC_MSG, pRet, totalSize);
        FillNodeInfo(asyncData);
        sendResult = client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    } else {
        SetStackRawData(rawdata, MALLOC_MSG, pRet, totalSize);
        FillNodeInfo(rawdata);
        sendResult = client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    }
    if (sendResult) {
        PERF_RECORD_DATA(param.stackSize, totalSize);
    }
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

void* __attribute__((noinline)) hook_realloc(void* (*fn)(void*, size_t), void* ptr, size_t size)
{
    void* pRet = fn ? fn(ptr, size) : nullptr;

    if (!HookGuard::IsReady()) {
        return pRet;
    }
    if (HookGuard::ShouldSkipMalloc()) {
        return pRet;
    }
    if (!ohos_set_filter_size(size, pRet)) {
        return pRet;
    }

    if (HookGuard::ShouldFilterBySize(pRet, size) || HookGuard::ShouldSample(size, g_sampleKey)) {
        return pRet;
    }

    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return pRet;
    }

    struct timespec ts = {};
    clock_gettime(g_clientConfig.clockId, &ts);

    PERF_TRACK(OperatType::realloc);

    StackParam param;
    param.needJsStack = false;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    StackRawData freedata = {{{{0}}}};
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return pRet;
    }

    AddAllocAddr(pRet);

    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    int rawSize = HookGuard::CalculateRealSize(param.fpStackDepth, false);
    int freeRealSize = g_clientConfig.fpunwind ? sizeof(BaseStackRawData) : rawSize;
    bool sendResult = false;
    
    SetStackRawData(freedata, FREE_MSG, ptr, 0);
    freedata.ts = ts;
    client.SendStackWithPayload(&freedata, freeRealSize, nullptr, 0,
                                reinterpret_cast<uint64_t>(freedata.addr) % g_sharedMemCount);

    if (isAsyncStack) {
        SetStackRawData(asyncData, MALLOC_MSG, pRet, size);
        FillNodeInfo(asyncData);
        asyncData.ts = ts;
        sendResult = client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(asyncData.addr) % g_sharedMemCount);
    } else {
        SetStackRawData(rawdata, MALLOC_MSG, pRet, size);
        FillNodeInfo(rawdata);
        rawdata.ts = ts;
        sendResult = client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr),
            param.stackSize, reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    }

    if (sendResult) {
        PERF_RECORD_DATA(param.stackSize, size);
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

void HookFreeStatisticsInterval(void (*free_func)(void*), void* p)
{
    if (!free_func) {
        return;
    }
    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMalloc()) {
        free_func(p);
        return;
    }
    PERF_TRACK(OperatType::free);
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_clientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        if (!addrHandler->CheckAddr(reinterpret_cast<uint64_t>(p))) {
            free_func(p);
            return;
        }
    }

    HookClientHolder client(g_hookClient);
    if (client.IsValid() && p) {
        client.SendStackWithPayload(&p, sizeof(void*), nullptr, 0,
                                    reinterpret_cast<uint64_t>(p) % g_sharedMemCount);
    }
    free_func(p);
    return;
}

void __attribute__((noinline)) hook_free(void (*free_func)(void*), void* p)
{
    if (g_clientConfig.statisticsInterval > 0) {
        return HookFreeStatisticsInterval(free_func, p);
    }
    struct timespec freeTs = {};
    clock_gettime(g_clientConfig.clockId, &freeTs);
    if (free_func) {
        free_func(p);
    }

    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMalloc()) {
        return;
    }
    PERF_TRACK(OperatType::free);

    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_clientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        if (!addrHandler->CheckAddr(reinterpret_cast<uint64_t>(p))) {
            return;
        }
    }

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    param.needCollect = g_clientConfig.freeStackData;
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return;
    }
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    HookClientHolder client(g_hookClient);
    if (isAsyncStack) {
        SetStackRawData(asyncData, FREE_MSG, p, 0);
        asyncData.ts = freeTs;
        client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize,
                                    reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    } else {
        SetStackRawData(rawdata, FREE_MSG, p, 0);
        rawdata.ts = freeTs;
        client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize,
                                    reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    }
}

static bool RefreshMmapRawData(StackRawData &rawdata, int prot, int flags, int fd, off_t offset, size_t length)
{
    if (fd < 0) {
        return true;
    }
    rawdata.type = MMAP_FILE_PAGE_MSG;
    char path[FD_PATH_LENGTH] = {0};
    char fileName[MAX_HOOK_PATH + 1] = {0};
    if (snprintf_s(path, FD_PATH_LENGTH, FD_PATH_LENGTH - 1, "/proc/self/fd/%d", fd) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "hook_mmap snprintf_s error");
        return false;
    }
    ssize_t len = readlink(path, fileName, sizeof(fileName) - 1);
    if (len == -1) {
        return true;
    }
    fileName[len] = '\0';
    if (g_clientConfig.memtraceEnable) {
        const bool isSoFile = strstr(fileName, ".so") != nullptr;
        const bool isAshmem = strstr(fileName, "/dev/ashmem") != nullptr;
        if ((isSoFile && (g_clientConfig.traceMask & RES_SO_MASK)) ||
            (isAshmem && (g_clientConfig.traceMask & RES_ASHMEM_MASK))) {
            if (!ohos_set_filter_size(length, nullptr)) {
                return false;
            }
            rawdata.type = MEMORY_USING_MSG;
            FillNodeInfo(rawdata);
        }
    }
    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return false;
    }
    client.SendMmapFileRawData(prot, flags, offset, fileName, rawdata);
    char* p = strrchr(fileName, '/');
    if (p != nullptr) {
        rawdata.tagId = GetTagId(&fileName[p - fileName + 1]);
    } else {
        rawdata.tagId = GetTagId(fileName);
    }
    return true;
}

void* __attribute__((noinline)) hook_mmap(void*(*fn)(void*, size_t, int, int, int, off_t),
    void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    void* ret = fn ? fn(addr, length, prot, flags, fd, offset) :  nullptr;

    if (!HookGuard::IsReady() || HookGuard::IsPidChanged()) {
        return ret;
    } else if (g_clientConfig.largestSize > 0 && fd <= 0) {
        return ret;
    } else if ((g_clientConfig.mmapDisable) && ((g_clientConfig.memtraceEnable ||
               (!g_clientConfig.mallocDisable)) && fd <= 0)) {
        return ret;
    }
    if ((fd < 0 && offset == 0) && HookGuard::ShouldSample(length, g_sampleKey)) {
        return ret;
    }
    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return ret;
    }

    PERF_TRACK(OperatType::mmap);
    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return ret;
    }
    SetStackRawData(rawdata, MMAP_MSG, ret, length);
    if (!RefreshMmapRawData(rawdata, prot, flags, fd, offset, length)) {
        return ret;
    }
    if (!client.UpdateThreadName()) {
        return ret;
    }
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    if (isAsyncStack) {
        *static_cast<BaseStackRawData*>(&asyncData) = *static_cast<BaseStackRawData*>(&rawdata);
        client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize);
    } else {
        client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize);
    }
    return ret;
}

int __attribute__((noinline)) hook_munmap(int(*fn)(void*, size_t), void* addr, size_t length)
{
    struct timespec unmapTime = {};
    clock_gettime(g_clientConfig.clockId, &unmapTime);
    int ret = fn ? fn(addr, length) : -1;

    if (!HookGuard::IsReady()) {
        return ret;
    }
    if (HookGuard::ShouldSkipMmap()) {
        return ret;
    }
    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return ret;
    }
    PERF_TRACK(OperatType::munmap);

    if (!g_clientConfig.targetSoName.empty()) {
        uint64_t addrval = reinterpret_cast<uint64_t>(addr);
        auto range = g_targetedRange.load();
        if (addrval < range.end && addrval >= range.start) {
            range.end = 0;
            range.start = 0;
            g_targetedRange.store(range);
        }
    }

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    param.needFpunwind = g_clientConfig.statisticsInterval == 0;
    param.needCollect = g_clientConfig.munmapStackData;
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return ret;
    }
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    if (isAsyncStack) {
        SetStackRawData(asyncData, MUNMAP_MSG, addr, length);
        asyncData.ts = unmapTime;
        client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize);
    } else {
        SetStackRawData(rawdata, MUNMAP_MSG, addr, length);
        rawdata.ts = unmapTime;
        client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize);
    }
    return ret;
}

int hook_prctl(int(*fn)(int, ...),
    int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
    int ret = fn ? fn(option, arg2, arg3, arg4, arg5) : -1;

    if ((!HookGuard::IsReady()) || reinterpret_cast<char*>(arg5) == nullptr || HookGuard::ShouldSkipMmap()) {
        return ret;
    }
    if (option != PR_SET_VMA || arg2 != PR_SET_VMA_ANON_NAME) {
        return ret;
    }
    PERF_TRACK(OperatType::prctl);
    NameData rawdata = {{{{0}}}};
    clock_gettime(g_clientConfig.clockId, &rawdata.ts);
    rawdata.type = PR_SET_VMA_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = arg4;
    rawdata.addr = reinterpret_cast<void*>(arg3);
    size_t tagLen = strlen(reinterpret_cast<char*>(arg5)) + 1;
    if (memcpy_s(rawdata.name, sizeof(rawdata.name), reinterpret_cast<char*>(arg5), tagLen) != EOK) {
        HILOG_BASE_ERROR(LOG_CORE, "memcpy_s tag failed");
    }
    rawdata.name[sizeof(rawdata.name) - 1] = '\0';
    HookClientHolder client(g_hookClient);
    client.SendStackWithPayload(&rawdata, sizeof(BaseStackRawData) + tagLen, nullptr, 0,
                                reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    return ret;
}

void __attribute__((noinline)) hook_memtrace(void* addr, size_t size, const char* tag, bool isUsing)
{
    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMemtrace() || g_clientConfig.isSaMode) {
        return;
    }
    if (g_clientConfig.traceMask > 0) {
        return;
    }

    if (!ohos_set_filter_size(size, nullptr)) {
        return;
    }

    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return;
    }
    PERF_TRACK(OperatType::memtrace);

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    param.needCollect = isUsing;
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return;
    }

    uint16_t type = isUsing ? MEMORY_USING_MSG : MEMORY_UNUSING_MSG;
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    if (isAsyncStack) {
        SetStackRawData(asyncData, type, addr, size);
        FillNodeInfo(asyncData);
        asyncData.tagId = GetTagId(tag);
        client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize);
    } else {
        SetStackRawData(rawdata, type, addr, size);
        FillNodeInfo(rawdata);
        rawdata.tagId = GetTagId(tag);
        client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize);
    }
}

size_t FindFdSize(const std::string& fileName, uint32_t targetPid, uint32_t targetFd)
{
    std::ifstream file(fileName);
    if (!file) {
        PROFILER_LOG_ERROR(LOG_CORE, "caon not open errno=%d!!!", errno);
        return 0;
    }
    std::string line;
    int skip = 1;
    while (std::getline(file, line)) {
        if (skip > 0) {
            --skip;
            continue;
        }
        std::istringstream iss(line);
        std::string col1;
        uint32_t pid;
        uint32_t fd;
        size_t size;
        if (!(iss >> col1 >> pid >> fd >> size)) {
            continue;
        }
        if (pid == targetPid && fd == targetFd) {
            return size;
        }
    }
    return 0;
}

size_t GetDmaSize(void* addr, bool isUsing)
{
    if (!isUsing) {
        return 0;
    }
    char path[FD_PATH_LENGTH] = {0};
    char fileName[MAX_HOOK_PATH + 1] = {0};
    int fd = static_cast<int>(reinterpret_cast<intptr_t>(addr));

    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid fd value");
        return 0;
    }

    if (snprintf_s(path, FD_PATH_LENGTH, FD_PATH_LENGTH - 1, "/proc/self/fd/%d", fd) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "getDmaSize snprintf_s error");
        return 0;
    }
    
    ssize_t len = readlink(path, fileName, sizeof(fileName) - 1);
    if (len != -1) {
        fileName[len] = '\0';
        if (strstr(fileName, "anon_inode:dmabuf") != NULL) {
            std::string filePath = "/proc/" + std::to_string(g_hookPid.load()) + "/mm_dmabuf_info";
            size_t size = FindFdSize(filePath, g_hookPid.load(), fd);
            return size;
        }
    }
    return 0;
}

void __attribute__((noinline)) hook_restrace(unsigned long long mask,
    void* addr, size_t size, const char* tag, bool isUsing)
{
    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMemtrace()) {
        return;
    }

    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return;
    }

    unsigned long long combineVal = mask & g_clientConfig.traceMask;
    if (combineVal == 0) {
        return;
    }
    if ((g_clientConfig.isSaMode && isUsing) && (!HookGuard::CheckRestraceConditions(combineVal, size))) {
        return;
    }

    if (mask == RES_DMABUF_MASK) {
        size = GetDmaSize(addr, isUsing);
        if (size == 0) {
            return;
        }
    }

    if (((mask & (RES_FD_MASK | RES_THREAD_MASK)) == 0) && !ohos_set_filter_size(size, nullptr)) {
        return;
    }

    PERF_TRACK(OperatType::restrace);

    StackParam param;
    AsyncStackData asyncData {{{{0}}}};
    StackRawData rawdata = {{{{0}}}};
    param.needCollect = isUsing;
    if (!CollectStackInfoWithAsyncData(rawdata, asyncData, param)) {
        return;
    }
    uint16_t type = isUsing ? MEMORY_USING_MSG : MEMORY_UNUSING_MSG;
    const bool isAsyncStack = g_clientConfig.fpunwind && g_clientConfig.asyncStackEnable;
    int realSize = HookGuard::CalculateRealSize(param.fpStackDepth, isAsyncStack);
    if (isAsyncStack) {
        SetStackRawData(asyncData, type, addr, size);
        FillNodeInfo(asyncData);
        asyncData.tagId = GetTagId(tag, mask);
        client.SendStackWithPayload(&asyncData, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize,
                                    reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    } else {
        SetStackRawData(rawdata, type, addr, size);
        FillNodeInfo(rawdata);
        rawdata.tagId = GetTagId(tag, mask);
        client.SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void*>(param.stackPtr), param.stackSize,
                                    reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    }
}

void hook_resTraceMove(unsigned long long mask, void* oldAddr, void* newAddr, size_t newSize)
{
    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMemtrace()) {
        return;
    }
    HookClientHolder client(g_hookClient);
    if (!client.IsValid()) {
        return;
    }
    if ((mask & g_clientConfig.traceMask) == 0) {
        return;
    }
    PERF_TRACK(OperatType::resTraceMove);

    StackRawData rawdata = {{{{0}}}};
    SetStackRawData(rawdata, MALLOC_ARKTS, oldAddr, newSize);
    rawdata.newAddr = newAddr;
    rawdata.tagId = HookGuard::GetPredefinedTagId(mask);
    
    uintptr_t stackPtr = 0;
    client.SendStackWithPayload(&rawdata, sizeof(BaseStackRawData), reinterpret_cast<void *>(stackPtr), 0,
                                reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
}

void hook_resTraceFreeRegion(unsigned long long mask, void* addr, size_t size)
{
    if (!HookGuard::IsReady() || HookGuard::ShouldSkipMemtrace()) {
        return;
    }

    if ((mask & g_clientConfig.traceMask) == 0) {
        return;
    }
    PERF_TRACK(OperatType::resTraceFreeRegion);
    
    StackRawData rawdata = {{{{0}}}};
    SetStackRawData(rawdata, FREE_ARKTS, addr, size);
    rawdata.tagId = HookGuard::GetPredefinedTagId(mask);

    uintptr_t stackPtr = 0;
    HookClientHolder client(g_hookClient);
    client.SendStackWithPayload(&rawdata, sizeof(BaseStackRawData), reinterpret_cast<void *>(stackPtr), 0,
                                reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
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

void ohos_malloc_hook_restrace(unsigned long long mask, void* addr, size_t size, const char* tag, bool isUsing)
{
    __set_hook_flag(false);
    hook_restrace(mask, addr, size, tag, isUsing);
    __set_hook_flag(true);
}

void ohos_malloc_hook_resTraceMove(unsigned long long mask, void* oldAddr, void* newAddr, size_t newSize)
{
    __set_hook_flag(false);
    hook_resTraceMove(mask, oldAddr, newAddr, newSize);
    __set_hook_flag(true);
}

void ohos_malloc_hook_resTraceFreeRegion(unsigned long long mask, void* addr, size_t size)
{
    __set_hook_flag(false);
    hook_resTraceFreeRegion(mask, addr, size);
    __set_hook_flag(true);
}

void* ohos_malloc_hook_aligned_alloc(size_t align, size_t len)
{
    __set_hook_flag(false);
    void* ret = hook_aligned_alloc(GetDispatch()->aligned_alloc, align, len);
    __set_hook_flag(true);
    return ret;
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
    return g_clientConfig.filterSize >= 0 && size >= static_cast<size_t>(g_clientConfig.filterSize)
        && size <= g_maxSize;
}

bool ohos_malloc_hook_send_hook_misc_data(uint64_t id, const char* stackPtr, size_t stackSize, uint32_t type)
{
    if (type == static_cast<uint32_t>(MISC_TYPE::JS_STACK_DATA) && !g_clientConfig.asyncStackEnable) {
        BaseStackRawData rawdata = {};
        rawdata.jsChainId = id;
        rawdata.type = JS_STACK_MSG;
        bool result = true;
        HookClientHolder client(g_hookClient);
        if (!client.IsValid()) {
            return false;
        }
        for (int i = 0; i < g_sharedMemCount; ++i) {
            result &= (client.SendStackWithPayload(&rawdata, sizeof(BaseStackRawData), stackPtr, stackSize, i));
        }
        return result;
    }
    return false;
}

void* ohos_malloc_hook_get_hook_config()
{
    return &g_clientConfig.arktsConfig;
}