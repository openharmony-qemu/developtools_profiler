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
#include <malloc.h>
#include <string>
#include <sys/time.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unordered_map>
#include <unordered_set>
#include "dfx_regs_get.h"
#include "common.h"
#include "hook_common.h"
#include "hook_socket_client.h"
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

using namespace OHOS::HiviewDFX;
using namespace OHOS::Developtools::NativeDaemon;

static pthread_key_t g_disableHookFlag = 10000;
static pthread_key_t g_hookTid;
static pthread_key_t g_updateThreadNameCount = 10000;
static pthread_once_t g_onceFlag;
namespace {
static std::atomic<uint64_t> g_mallocTimes = 0;
static std::atomic<int> g_sharedMemCount = 1;
static std::atomic<uint16_t> g_tagId = 0;

enum class MISC_TYPE : uint32_t {
    JS_STACK_DATA = 1,
};

#ifdef PERFORMANCE_DEBUG
static std::atomic<uint64_t> g_timeCost = 0;
static std::atomic<uint64_t> g_dataCounts = 0;
constexpr int PRINT_INTERVAL = 5000;
constexpr uint64_t S_TO_NS = 1000 * 1000 * 1000;
#endif

using OHOS::Developtools::NativeDaemon::buildArchType;
static std::shared_ptr<HookSocketClient> g_hookClient {nullptr};
static Sampling g_sampler;
std::recursive_timed_mutex g_ClientMutex;
std::recursive_timed_mutex g_FilterMapMutex;
std::mutex g_tagMapMutex;
std::mutex g_tagIdMutex;
std::mutex g_usableSizeMapMutex;
std::atomic<const MallocDispatchType*> g_dispatch {nullptr};
constexpr int UPDATE_THEARD_NAME = 1000;
static std::atomic<pid_t> g_hookPid = 0;
static std::atomic<bool> g_hookReady = false;
static ClientConfig g_ClientConfig = {0};
static uint32_t g_maxSize = INT_MAX;
static std::unordered_map<std::string, uint32_t> g_memTagMap;
static std::unordered_map<size_t, size_t> g_mallocUsableSizeMap;
constexpr int PID_STR_SIZE = 4;
constexpr int STATUS_LINE_SIZE = 512;
constexpr int PID_NAMESPACE_ID = 1; // 1: pid is 1 after pid namespace used
constexpr int FD_PATH_LENGTH = 64;
constexpr int MIN_SAMPLER_INTERVAL = 1;
constexpr int FIRST_HASH = 16;
constexpr int SECOND_HASH = 13;
constexpr int THRESHOLD = 256;
constexpr int DIVIDE_VAL = 64;
constexpr uintptr_t MAX_UNWIND_ADDR_RANGE = 16 * 1024;
//5: fp mode is used, response_library_mode maximum stack depth
#if defined(__aarch64__)
constexpr int RESPONSE_LIBRARY_MODE_DEPTH = 5;
constexpr int TEMP_IP = 100;
#endif
static bool g_isPidChanged = false;
static struct mallinfo2 g_miStart = {0};
std::vector<std::pair<uint64_t, uint64_t>> g_filterStaLibRange;
std::atomic<Range> targetedRange;
constexpr int MAX_BITPOOL_SIZE = 1000 * 1024;
std::shared_ptr<WholeAddrHandler> g_wholeAddrHandler = nullptr;
std::shared_ptr<MidAddrHandler> g_midPartHandler = nullptr;
std::shared_ptr<LowAddrHandler> g_addrHandler = nullptr;

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
        NameData tnameData = {{{{0}}}};
        tnameData.tid = static_cast<uint32_t>(GetCurThreadId());
        tnameData.type = THREAD_NAME_MSG;
        prctl(PR_GET_NAME, tnameData.name);
        ret = client->SendStackWithPayload(&tnameData,
                                           sizeof(BaseStackRawData) + strlen(tnameData.name) + 1, nullptr, 0);
        if (!ret) {
            return ret;
        }
    }
    pthread_setspecific(g_updateThreadNameCount,
                        reinterpret_cast<void *>(updateCount == UPDATE_THEARD_NAME ? 0 : updateCount + 1));
    return ret;
}

uint32_t inline __attribute__((always_inline)) GetTagId(std::shared_ptr<HookSocketClient>& client, const char* tagName)
{
    if (tagName == nullptr || strlen(tagName) > MAX_HOOK_PATH) {
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
        NameData tagData = {{{{0}}}};
        tagData.type = MEMORY_TAG;
        tagData.tagId = tagId;
        strcpy_s(tagData.name, MAX_HOOK_PATH + 1, tagName);
        if (client != nullptr) {
            client->SendStackWithPayload(&tagData, sizeof(BaseStackRawData) + strlen(tagName) + 1, nullptr, 0);
        }
    }
    return tagId;
}

uint16_t inline __attribute__((always_inline)) GetMmapTagId(std::shared_ptr<HookSocketClient>& client,
                                                     const char* tagName)
{
    if (tagName == nullptr || strlen(tagName) > MAX_HOOK_PATH) {
        return 0;
    }

    std::unique_lock<std::mutex> lock(g_tagIdMutex);
    NameData tagData = {{{{0}}}};
    tagData.type = MEMORY_TAG;
    ++g_tagId;
    if (g_tagId.load() == 0) {
        ++g_tagId;
    }
    tagData.tagId = g_tagId.load();
    lock.unlock();
    if (strcpy_s(tagData.name, MAX_HOOK_PATH + 1, tagName) != EOK) {
        return 0;
    }
    if (client != nullptr) {
        client->SendStackWithPayload(&tagData, sizeof(BaseStackRawData) + strlen(tagName) + 1, nullptr, 0);
    }
    return tagData.tagId;
}

static bool IsPidChanged(void);

/* 返回值：true：该size大小需要过滤，不记录trace信息； fasle：不需要过滤，按正常流程记录trace信息 */
static bool SimplifiedFilter(void* ptr, size_t mallcoSize)
{
    if (g_ClientConfig.largestSize == 0 || g_ClientConfig.secondLargestSize == 0) {
        return false;
    }
    if (mallcoSize >= g_ClientConfig.sampleInterval) {
        return false;
    }
    size_t usableSize = 0;
    if (mallcoSize == 0) {
        /* hook_free */
        usableSize = malloc_usable_size(ptr);
    } else {
        std::unique_lock<std::mutex> lock(g_usableSizeMapMutex);
        auto it = g_mallocUsableSizeMap.find(mallcoSize);
        if (it == g_mallocUsableSizeMap.end()) {
            usableSize = malloc_usable_size(ptr);
            g_mallocUsableSizeMap[mallcoSize] = usableSize;
        } else {
            usableSize = it->second;
        }
        lock.unlock();
    }

    if (usableSize >= g_ClientConfig.sampleInterval) {
        return false;
    }

    if ((usableSize == g_ClientConfig.largestSize) ||
        (usableSize == g_ClientConfig.secondLargestSize) ||
        (usableSize == g_ClientConfig.maxGrowthSize)) {
            return false;
    }

    return true;
}

void* MallocHookStart(void* disableHookCallback)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    if (g_hookReady) {
        return nullptr;
    }
    PROFILER_LOG_INFO(LOG_CORE, "MallocHookStart begin!");
    g_wholeAddrHandler = std::make_shared<WholeAddrHandler>();
    g_midPartHandler = std::make_shared<MidAddrHandler>();
    g_addrHandler = std::make_shared<LowAddrHandler>();
    g_midPartHandler->SetSuccessor(std::move(g_wholeAddrHandler));
    g_addrHandler->SetSuccessor(std::move(g_midPartHandler));
    g_wholeAddrHandler = nullptr;
    g_midPartHandler = nullptr;
    g_mallocTimes = 0;
    g_hookClient.reset();
    g_hookPid = GetRealPid();
    ParseSelfMaps(g_filterStaLibRange);
    if (g_hookClient != nullptr) {
        return nullptr;
    } else {
        g_ClientConfig.Reset();
        g_sampler.Reset();
        g_hookClient = std::make_shared<HookSocketClient>(g_hookPid.load(), &g_ClientConfig, &g_sampler,
                                                          &targetedRange, &g_sharedMemCount,
                                                          reinterpret_cast<void (*)()>(disableHookCallback));
    }
    g_hookReady = true;
    return nullptr;
}

static void InitHookTidKey()
{
    if (pthread_key_create(&g_hookTid, nullptr) != 0) {
        return;
    }
    pthread_setspecific(g_hookTid, nullptr);
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
    if (pthread_create(&threadStart, nullptr, MallocHookStart,
                       reinterpret_cast<void *>(disableHookCallback))) {
        return false;
    }
    pthread_detach(threadStart);
    if (!InitTheadKey()) {
        return false;
    }
    return true;
}

void* ohos_release_on_end(void*)
{
    std::lock_guard<std::recursive_timed_mutex> guard(g_ClientMutex);
    if (!g_hookReady) {
        return nullptr;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ohos_release_on_end begin!");
    if (g_hookClient != nullptr) {
        if (g_hookClient->GetNmdType() == 1) {
            g_hookClient->SendNmdInfo();
        }
        g_hookClient->SendEndMsg();
        g_hookClient->Flush();
    }
    g_addrHandler = nullptr;
    g_hookClient = nullptr;
    g_ClientConfig.Reset();
    g_sharedMemCount = 1;
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
    auto range = targetedRange.load();
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
        if (g_ClientConfig.responseLibraryMode) {
            if (++count >= RESPONSE_LIBRARY_MODE_DEPTH || !FilterStandardSoIp(ip)) {
                break;
            }
        } else {
            ips[depth++] = ip > 0x4 ? ip - 0x4 : ip; // adjust pc in Arm64 architecture
        }

        if ((!g_ClientConfig.targetSoName.empty()) && CheckTargetLibIp(ip)) {
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
    if (g_ClientConfig.responseLibraryMode) {
        ips[0] = ip > 0x4 ? ip - 0x4 : ip;
        depth = 1;
    }
    if ((!g_ClientConfig.targetSoName.empty()) && filterTarget) {
        depth = 0;
    }
    return depth;
}

uint64_t getJsChainId()
{
    if (g_ClientConfig.arktsConfig.jsStackReport > 0) {
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

void* hook_malloc(void* (*fn)(size_t), size_t size)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(size);
    }
    if (!g_hookReady) {
        return ret;
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

    if (SimplifiedFilter(ret, size) ||
        (g_ClientConfig.sampleInterval > MIN_SAMPLER_INTERVAL && g_sampler.StartSampling(size) == 0)) {
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
        return ret;
    }

    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    if (!UpdateThreadName(holder)) {
        return ret;
    }
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
        if (fpStackDepth == 0) {
            return ret;
        }
        rawdata.jsChainId = getJsChainId();
#endif
    } else {
        stackSize = GetStackSize(stackPtr, rawdata);
    }
    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = ret;
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_ClientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        addrHandler->AddAllocAddr(reinterpret_cast<uint64_t>(ret));
    }
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize,
                                 reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    g_mallocTimes++;
#ifdef PERFORMANCE_DEBUG
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    g_dataCounts += stackSize;
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
    return ret;
}

void* hook_aligned_alloc(void* (*fn)(size_t, size_t), size_t align, size_t len)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(align, len);
    }
    if (!g_hookReady) {
        return ret;
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return ret;
    }
    if (!ohos_set_filter_size(len, ret)) {
        return ret;
    }

#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    if (SimplifiedFilter(ret, len) ||
        (g_ClientConfig.sampleInterval > MIN_SAMPLER_INTERVAL && g_sampler.StartSampling(len) == 0)) { //0 not sampling
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_aligned_allocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %"
                PRIu64"\n", g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(),
                g_timeCost.load() / g_mallocTimes.load());
        }
#endif
        return ret;
    }

    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    if (!UpdateThreadName(holder)) {
        return ret;
    }
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
        if (fpStackDepth == 0) {
            return ret;
        }
        rawdata.jsChainId = getJsChainId();
#endif
    } else {
        stackSize = GetStackSize(stackPtr, rawdata);
    }

    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = len;
    rawdata.addr = ret;
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_ClientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        addrHandler->AddAllocAddr(reinterpret_cast<uint64_t>(ret));
    }
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize,
                                 reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    g_mallocTimes++;
#ifdef PERFORMANCE_DEBUG
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    g_dataCounts += stackSize;
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_aligned_allocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %"
            PRIu64"\n", g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(),
            g_timeCost.load() / g_mallocTimes.load());
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
    if (!g_hookReady) {
        return pRet;
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return pRet;
    }
    if (!ohos_set_filter_size(number * size, pRet)) {
        return pRet;
    }

#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    if (SimplifiedFilter(pRet, number * size) ||
        (g_ClientConfig.sampleInterval > MIN_SAMPLER_INTERVAL && g_sampler.StartSampling(size * number) == 0)) {
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
        return pRet;
    }
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return pRet;
    }
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
        if (fpStackDepth == 0) {
            return pRet;
        }
        rawdata.jsChainId = getJsChainId();
#endif
    } else {
        stackSize = GetStackSize(stackPtr, rawdata);
    }

    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = number * size;
    rawdata.addr = pRet;
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_ClientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        addrHandler->AddAllocAddr(reinterpret_cast<uint64_t>(pRet));
    }
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize,
                                 reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
    g_mallocTimes++;
#ifdef PERFORMANCE_DEBUG
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
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
    if (!g_hookReady) {
        return pRet;
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return pRet;
    }
    if (!ohos_set_filter_size(size, pRet)) {
        return pRet;
    }

#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    if (SimplifiedFilter(pRet, size) ||
        (g_ClientConfig.sampleInterval > MIN_SAMPLER_INTERVAL && g_sampler.StartSampling(size) == 0)) {
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
        return pRet;
    }
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return pRet;
    }
    StackRawData rawdata = {{{{0}}}};
    StackRawData freeData = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
        if (fpStackDepth == 0) {
            return pRet;
        }
#endif
    } else {
        stackSize = GetStackSize(stackPtr, rawdata);
    }

    rawdata.type = MALLOC_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = pRet;
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_ClientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        addrHandler->AddAllocAddr(reinterpret_cast<uint64_t>(pRet));
    }
    int realSize = 0;
    int freeRealSize = 0;
    freeData.type = FREE_MSG;
    freeData.pid = rawdata.pid;
    freeData.tid = rawdata.tid;
    freeData.mallocSize = 0;
    freeData.addr = ptr;
    freeData.ts = rawdata.ts;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
        freeRealSize = sizeof(BaseStackRawData);
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
        freeRealSize = realSize;
    }
    // 0: Don't unwind the freeData
    holder->SendStackWithPayload(&freeData, freeRealSize, nullptr, 0,
                                 reinterpret_cast<uint64_t>(freeData.addr) % g_sharedMemCount);
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize,
                                 reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
#ifdef PERFORMANCE_DEBUG
    g_mallocTimes++;
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
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
    if (g_ClientConfig.statisticsInterval > 0) {
        if (!free_func) {
            return;
        }
        if ((!g_hookReady) || g_ClientConfig.mallocDisable || IsPidChanged()) {
            free_func(p);
            return;
        }
#ifdef PERFORMANCE_DEBUG
        struct timespec start = {};
        clock_gettime(CLOCK_REALTIME, &start);
#endif
        std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
        auto addrHandler = weakHandler.lock();
        if ((g_ClientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
            if (!addrHandler->CheckAddr(reinterpret_cast<uint64_t>(p))) {
                free_func(p);
#ifdef PERFORMANCE_DEBUG
                g_mallocTimes++;
                struct timespec end = {};
                clock_gettime(CLOCK_REALTIME, &end);
                g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
                if (g_mallocTimes % PRINT_INTERVAL == 0) {
                    PROFILER_LOG_ERROR(LOG_CORE,
                        "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64
                        " mean cost = %" PRIu64"\n", g_mallocTimes.load(), g_timeCost.load(),
                        g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
                }
#endif
                return;
            }
        }
        std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
        auto holder = weakClient.lock();
        if ((holder != nullptr) && p) {
            holder->SendStackWithPayload(&p, sizeof(void*), nullptr, 0,
                                         reinterpret_cast<uint64_t>(p) % g_sharedMemCount);
        }
        free_func(p);
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
        return;
    }
    if ((!g_hookReady) && free_func) {
        free_func(p);
        return;
    }
    struct timespec freeTime = {};
    clock_gettime(g_ClientConfig.clockId, &freeTime);
    if (free_func) {
        free_func(p);
    }
    if (g_ClientConfig.mallocDisable || IsPidChanged()) {
        return;
    }
    std::weak_ptr<AddressHandler> weakHandler = g_addrHandler;
    auto addrHandler = weakHandler.lock();
    if ((g_ClientConfig.sampleInterval >= THRESHOLD) && (addrHandler != nullptr)) {
        if (!addrHandler->CheckAddr(reinterpret_cast<uint64_t>(p))) {
            return;
        }
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return;
    }
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    rawdata.ts = freeTime;
    if (g_ClientConfig.freeStackData) {
        if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
            fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
            if (fpStackDepth == 0) {
                return;
            }
            rawdata.jsChainId = getJsChainId();
#endif
        } else {
            stackSize = GetStackSize(stackPtr, rawdata);
        }
    }
    rawdata.type = FREE_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = 0;
    rawdata.addr = p;
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize,
                                 reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
}

inline void SendMmapFileRawData(int prot, int flags, off_t offset, const char* filePath,
                                const StackRawData& rawdata, std::shared_ptr<HookSocketClient>& holder)
{
    NameData curRawdata = {{{{0}}}};
    curRawdata.addr = rawdata.addr;
    curRawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    curRawdata.mallocSize = rawdata.mallocSize;
    curRawdata.mmapArgs.offset = offset;
    curRawdata.type = MMAP_FILE_TYPE;
    if (prot & PROT_EXEC) {
        curRawdata.mmapArgs.flags |= PROT_EXEC;
    }
    size_t len = strlen(filePath) + 1;
    if ((flags & MAP_FIXED) && (g_ClientConfig.responseLibraryMode) && (IsLegalSoName(filePath)) &&
        (strstr(filePath, "ld-musl") != NULL || strstr(filePath, "libc++") != NULL)) {
        std::lock_guard<std::recursive_timed_mutex> guard(g_FilterMapMutex);
        ParseEvent(filePath, g_filterStaLibRange, curRawdata);
    }
    if ((flags & MAP_FIXED) && (IsLegalSoName(filePath)) && (!g_ClientConfig.targetSoName.empty()) &&
        strstr(filePath, g_ClientConfig.targetSoName.c_str()) != NULL) {
        uint64_t soStart = reinterpret_cast<uint64_t>(curRawdata.addr);
        uint64_t soEnd = soStart + static_cast<uint64_t>(curRawdata.mallocSize);
        auto range = targetedRange.load();
        if (range.start == 0 && range.end == 0) {
            range.start = soStart;
            range.end = soEnd;
        }
        if (range.start > soStart) {
            range.start = soStart;
        } else if (range.end < soEnd) {
            range.end = soEnd;
        }
        targetedRange.store(range);
    }
    if (strncpy_s(curRawdata.name, MAX_HOOK_PATH + 1, filePath, len) != EOK) {
        return;
    }
    curRawdata.name[len - 1] = '\0';
    if (flags & MAP_FIXED) {
        curRawdata.mmapArgs.flags |= MAP_FIXED;
    }
    holder->SendStackWithPayload(&curRawdata, sizeof(BaseStackRawData) + len, nullptr, 0);
}

void* hook_mmap(void*(*fn)(void*, size_t, int, int, int, off_t),
    void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    void* ret = nullptr;
    if (fn) {
        ret = fn(addr, length, prot, flags, fd, offset);
    }
    if (!g_hookReady) {
        return ret;
    }
    if (g_ClientConfig.largestSize > 0) {
        if ((fd <= 0) || IsPidChanged()) {
            return ret;
        }
    } else {
        if (g_ClientConfig.mmapDisable || IsPidChanged()) {
            return ret;
        }
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    if ((fd < 0 && offset == 0) && g_ClientConfig.sampleInterval > MIN_SAMPLER_INTERVAL
        && g_sampler.StartSampling(length) == 0) {
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
        return ret;
    }
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int stackSize = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
        fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
        rawdata.jsChainId = getJsChainId();
#endif
    } else {
        stackSize = GetStackSize(stackPtr, rawdata);
    }

    rawdata.type = MMAP_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = length;
    rawdata.addr = ret;
    if (fd >= 0) {
        rawdata.type = MMAP_FILE_PAGE_MSG;
        char path[FD_PATH_LENGTH] = {0};
        char fileName[MAX_HOOK_PATH + 1] = {0};
        if (snprintf_s(path, FD_PATH_LENGTH, FD_PATH_LENGTH - 1, "/proc/self/fd/%d", fd) < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "hook_mmap snprintf_s error");
            return nullptr;
        }
        ssize_t len = readlink(path, fileName, sizeof(fileName) - 1);
        if (len != -1) {
            fileName[len] = '\0';
            SendMmapFileRawData(prot, flags, offset, fileName, rawdata, holder);
            char* p = strrchr(fileName, '/');
            if (p != nullptr) {
                rawdata.tagId = GetMmapTagId(holder, &fileName[p - fileName + 1]);
            } else {
                rawdata.tagId = GetMmapTagId(holder, fileName);
            }
        }
    }
    if (!UpdateThreadName(holder)) {
        return ret;
    }
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        if (fpStackDepth == 0) {
            return ret;
        }
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize);
#ifdef PERFORMANCE_DEBUG
    g_mallocTimes++;
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
    return ret;
}

int hook_munmap(int(*fn)(void*, size_t), void* addr, size_t length)
{
    if ((!g_hookReady) && fn) {
        int ret = fn(addr, length);
        return ret;
    }
    int ret = -1;
    struct timespec unmapTime = {};
    clock_gettime(g_ClientConfig.clockId, &unmapTime);
    if (fn) {
        ret = fn(addr, length);
    }
    if (g_ClientConfig.mmapDisable || IsPidChanged()) {
        return ret;
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    int stackSize = 0;
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    if (!g_ClientConfig.targetSoName.empty()) {
        uint64_t addrval = reinterpret_cast<uint64_t>(addr);
        auto range = targetedRange.load();
        if (addrval < range.end && addrval >= range.start) {
            range.end = 0;
            range.start = 0;
            targetedRange.store(range);
        }
    }
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int fpStackDepth = 0;
    rawdata.ts = unmapTime;
    if (g_ClientConfig.munmapStackData) {
        if (g_ClientConfig.fpunwind && g_ClientConfig.statisticsInterval == 0) {
#ifdef __aarch64__
            fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
            if (fpStackDepth == 0) {
                return ret;
            }
            rawdata.jsChainId = getJsChainId();
#endif
        } else {
            stackSize = GetStackSize(stackPtr, rawdata);
        }
    }

    rawdata.type = MUNMAP_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = length;
    rawdata.addr = addr;
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize);
#ifdef PERFORMANCE_DEBUG
    g_mallocTimes++;
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
    return ret;
}

int hook_prctl(int(*fn)(int, ...),
    int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
    int ret = -1;
    if (fn) {
        ret = fn(option, arg2, arg3, arg4, arg5);
    }
    if ((!g_hookReady) || reinterpret_cast<char*>(arg5) == nullptr || IsPidChanged() || g_ClientConfig.mmapDisable) {
        return ret;
    }
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return ret;
    }
    if (option == PR_SET_VMA && arg2 == PR_SET_VMA_ANON_NAME) {
#ifdef PERFORMANCE_DEBUG
        struct timespec start = {};
        clock_gettime(CLOCK_REALTIME, &start);
#endif
        NameData rawdata = {{{{0}}}};
        clock_gettime(g_ClientConfig.clockId, &rawdata.ts);
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
        holder->SendStackWithPayload(&rawdata, sizeof(BaseStackRawData) + tagLen, nullptr, 0,
                                     reinterpret_cast<uint64_t>(rawdata.addr) % g_sharedMemCount);
#ifdef PERFORMANCE_DEBUG
        g_mallocTimes++;
        struct timespec end = {};
        clock_gettime(CLOCK_REALTIME, &end);
        g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
        if (g_mallocTimes % PRINT_INTERVAL == 0) {
            PROFILER_LOG_ERROR(LOG_CORE,
                "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
                g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
        }
#endif
    }
    return ret;
}

void hook_memtrace(void* addr, size_t size, const char* tag, bool isUsing)
{
    if (!g_hookReady || !g_ClientConfig.memtraceEnable || IsPidChanged()) {
        return;
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif
    std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return;
    }
    int stackSize = 0;
    StackRawData rawdata = {{{{0}}}};
    uintptr_t stackPtr = 0;
    int fpStackDepth = 0;
    clock_gettime(g_ClientConfig.clockId, &rawdata.ts);

    if (isUsing) {
        if (g_ClientConfig.fpunwind) {
#ifdef __aarch64__
            fpStackDepth = FpUnwind(g_ClientConfig.maxStackDepth, rawdata.ip);
            if (fpStackDepth == 0) {
                return;
            }
            rawdata.jsChainId = getJsChainId();
#endif
        } else {
            stackSize = GetStackSize(stackPtr, rawdata);
        }
    }
    rawdata.type = isUsing ? MEMORY_USING_MSG : MEMORY_UNUSING_MSG;
    rawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    rawdata.tid = static_cast<uint32_t>(GetCurThreadId());
    rawdata.mallocSize = size;
    rawdata.addr = addr;
    rawdata.tagId = isUsing ? GetTagId(holder, tag) : 0;
    int realSize = 0;
    if (g_ClientConfig.fpunwind) {
        realSize = sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
    } else {
        realSize = sizeof(BaseStackRawData) + sizeof(rawdata.regs);
    }
    holder->SendStackWithPayload(&rawdata, realSize, reinterpret_cast<void *>(stackPtr), stackSize);
#ifdef PERFORMANCE_DEBUG
    g_mallocTimes++;
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    g_timeCost += (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec);
    if (g_mallocTimes % PRINT_INTERVAL == 0) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "g_mallocTimes %" PRIu64" cost time = %" PRIu64" copy data bytes = %" PRIu64" mean cost = %" PRIu64"\n",
            g_mallocTimes.load(), g_timeCost.load(), g_dataCounts.load(), g_timeCost.load() / g_mallocTimes.load());
    }
#endif
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
        g_isPidChanged = (g_hookPid.load() != 0 && g_hookPid.load() != pid);
    }
    return g_isPidChanged;
}

bool ohos_malloc_hook_send_hook_misc_data(uint64_t id, const char* stackPtr, size_t stackSize, uint32_t type)
{
    if (type == static_cast<uint32_t>(MISC_TYPE::JS_STACK_DATA)) {
        BaseStackRawData rawdata = {};
        rawdata.jsChainId = id;
        rawdata.type = JS_STACK_MSG;
        std::weak_ptr<HookSocketClient> weakClient = g_hookClient;
        auto holder = weakClient.lock();
        bool result = true;
        if (holder != nullptr) {
            for (int i = 0; i < g_sharedMemCount; ++i) {
                result &= (holder->SendStackWithPayload(&rawdata, sizeof(BaseStackRawData), stackPtr, stackSize, i));
            }
            return result;
        }
    }
    return false;
}

void* ohos_malloc_hook_get_hook_config()
{
    return &g_ClientConfig.arktsConfig;
}