/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
#include "hook_socket_client.h"
#include "async_stack.h"
#include "runtime_stack_range.h"
#include "common.h"
#include "hook_common.h"
#include "hook_client.h"
#include "unix_socket_client.h"
#include "share_memory_allocator.h"
#include "mem_leak_detector.h"
#include "logging.h"
#include "musl_preinit_common.h"
#include "sampling.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>

extern "C" {
int32_t hook_fill_data(int, void*, size_t, size_t, void*, size_t);
void hook_data_dump(void*, size_t);
void hook_alive_addr_dump(void*, size_t);
}

namespace {
constexpr int ONLY_NMD_TYPE = 2;
constexpr int SIMP_NMD = 3;
std::atomic<bool> g_disableHook = true;
constexpr uint32_t MEMCHECK_DETAILINFO_MAXSIZE = 102400;

struct OptArg {
    size_t pos;
    char *buf;
};

#if defined(target_cpu_arm64)
extern "C" size_t libc_gwp_asan_collect_allocations_by_time_range(uint64_t end_time,
    uintptr_t *buffer, size_t max_count, size_t deep);
#endif

} // namespace

using namespace OHOS::Developtools::NativeDaemon;
static std::string GetRealTime()
{
    time_t now = time(nullptr);
    tm tm;
    const int timeLength = 64;
    char stampStr[timeLength] = {0};

    if (localtime_r(&now, &tm) == nullptr || strftime(stampStr, timeLength, "%Y/%m/%d %H:%M:%S", &tm) == 0) {
        return "error time format!";
    }
    return std::string(stampStr);
}

static void NmdWriteStat(void *arg, const char *buf)
{
    struct OptArg *opt = static_cast<struct OptArg*>(arg);
    std::string getNmdTime = std::to_string(getpid()) + " " + GetRealTime() + "\n";
    size_t nmdTimeLen = getNmdTime.size();
    if (strncpy_s(opt->buf + opt->pos, MEMCHECK_DETAILINFO_MAXSIZE - opt->pos,
                  getNmdTime.c_str(), nmdTimeLen) != EOK) {
        return;
    }
    opt->pos += nmdTimeLen;

    size_t len = strlen(buf);
    if (len + opt->pos + 1 > MEMCHECK_DETAILINFO_MAXSIZE) {
        return;
    }
    if (strncpy_s(opt->buf + opt->pos, MEMCHECK_DETAILINFO_MAXSIZE - opt->pos, buf, len) != EOK) {
        return;
    }
    opt->pos += len;
}

HookSocketClient::HookSocketClient(int pid, ClientConfig *config,
                                   std::vector<std::pair<uint64_t, uint64_t>>* targetLibRange,
                                   std::atomic<int>* memCount,
                                   void (*disableHookCallback)())
    : pid_(pid), config_(config), targetLibRange_(targetLibRange),
      sharedMemCount_(memCount), disableHookCallback_(disableHookCallback)
{
    g_disableHook = true;
    int sharedMemCount = (config_ != nullptr && config_->offlineSymbolization) ? SHARED_MEMORY_NUM : 1;
    smbFds_.reserve(sharedMemCount);
    eventFds_.reserve(sharedMemCount);
    stackWriterList_.reserve(sharedMemCount);
    unixSocketClient_ = nullptr;
    serviceName_ = "HookService";
    connectState_ = Connect(DEFAULT_UNIX_SOCKET_HOOK_FULL_PATH);
}

bool HookSocketClient::GetConnectState()
{
    return connectState_.load() && (unixSocketClient_->GetClientState() != CLIENT_STAT_THREAD_EXITED);
}

HookSocketClient::~HookSocketClient()
{
    RestoreAsyncStackType();
    for (size_t i = 0; i < stackWriterList_.size(); ++i) {
        if (stackWriterList_[i]) {
            stackWriterList_[i]->Flush();
        }
    }
    stackWriterList_.clear();
    eventFds_.clear();
    smbFds_.clear();
    unixSocketClient_ = nullptr;
    g_disableHook = true;
}

bool HookSocketClient::Connect(const std::string addrname)
{
    if (unixSocketClient_ != nullptr) {
        return false;
    }
    unixSocketClient_ = std::make_shared<UnixSocketClient>();
    if (!unixSocketClient_->Connect(addrname, *this, disableHookCallback_)) {
        unixSocketClient_ = nullptr;
        return false;
    }

    if (!unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t *>(&pid_), sizeof(pid_))) {
        return false;
    }
    return true;
}

void HookSocketClient::HandleFileCacheMode(SocketContext &context)
{
    std::shared_ptr<StackWriter> stackWriter = std::make_shared<StackWriter>(
        "hook_file_cache", config_->shareMemorySize, config_->fileCacheBufferSize, -1,
        config_->isBlocked, config_->isSaMode, config_->enableFileCacheMode);
    stackWriter->PrepareCacheFileWriter([this](uint16_t fileIndex, uint16_t eventType, int fd) -> bool {
        if (unixSocketClient_ == nullptr) {
            return false;
        }
        HookCacheFileMsg cacheFileMsg(pid_, fileIndex, eventType);
        if (!unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t *>(&cacheFileMsg), sizeof(cacheFileMsg))) {
            return false;
        }
        if (eventType == CACHE_FILE_READY) {
            return fd >= 0 && unixSocketClient_->SendFileDescriptor(fd);
        }
        return true;
    });
    for (uint16_t fileIndex = 0; fileIndex < FILE_CACHE_FD_COUNT; ++fileIndex) {
        int cacheFd = context.ReceiveFileDiscriptor();
        if (cacheFd >= 0) {
            fdsan_exchange_owner_tag(cacheFd, 0, PROFILER_SUBSYSTEM);
        }
        if (!stackWriter->AddFileCacheDescriptor(fileIndex, cacheFd)) {
            if (cacheFd >= 0) {
                fdsan_close_with_tag(cacheFd, PROFILER_SUBSYSTEM);
            }
            return;
        }
        if (cacheFd >= 0) {
            fdsan_exchange_owner_tag(cacheFd, PROFILER_SUBSYSTEM, 0);
        }
    }
    stackWriterList_.push_back(stackWriter);
    PROFILER_LOG_INFO(LOG_CORE, "HookSocketClient::ProtocolProc use file cache mode");
    if (sharedMemCount_ != nullptr) {
        *sharedMemCount_ = 1;
    }
}

bool HookSocketClient::HandleFileCacheMsg(SocketContext &context, const HookCacheFileMsg& cacheFileMsg)
{
    if (cacheFileMsg.msgType != CACHE_FILE_CREATED || stackWriterList_.empty() || stackWriterList_[0] == nullptr) {
        return false;
    }
    int cacheFd = context.ReceiveFileDiscriptor();
    if (cacheFd >= 0) {
        fdsan_exchange_owner_tag(cacheFd, 0, PROFILER_SUBSYSTEM);
    }
    if (stopRequested_.load()) {
        if (cacheFd >= 0) {
            fdsan_close_with_tag(cacheFd, PROFILER_SUBSYSTEM);
        }
        return true;
    }
    if (!stackWriterList_[0]->AddFileCacheDescriptor(cacheFileMsg.fileIndex, cacheFd)) {
        if (cacheFd >= 0) {
            fdsan_close_with_tag(cacheFd, PROFILER_SUBSYSTEM);
        }
        return false;
    }
    if (cacheFd >= 0) {
        fdsan_exchange_owner_tag(cacheFd, PROFILER_SUBSYSTEM, 0);
    }
    return true;
}

bool HookSocketClient::HandleBackpressureState(SocketContext &context, const int8_t *buf, uint32_t size)
{
    if (buf == nullptr || size != sizeof(BackpressureStateMsg) || config_->offlineSymbolization) {
        return false;
    }
    
    BackpressureStateMsg state;
    if (memcpy_s(&state, sizeof(state), buf, size) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleBackpressureState memcpy_s failed");
        return false;
    }
    
    if (!stackWriterList_.empty() && stackWriterList_[0] != nullptr) {
        stackWriterList_[0]->UpdateBackpressureState(state);
    }
    
    PROFILER_LOG_INFO(LOG_CORE, "HandleBackpressureState: pid=%d, queueDepth=%u, delay=%uns",
        state.pid, state.queueDepth, state.recommendedDelayNs);
    return true;
}

bool HookSocketClient::HandleSimplifiedMemConfigChanged(SocketContext &context, const int8_t *buf, uint32_t size)
{
    (void)context;
    if (buf == nullptr || size != sizeof(SimplifiedMemConfigMsg)) {
        return false;
    }
    SimplifiedMemConfigMsg msg;
    if (memcpy_s(&msg, sizeof(msg), buf, size) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleSimplifiedMemConfigChanged memcpy_s failed");
        return false;
    }
    maxGrowthSize_ = msg.maxGrowthSize;
    if (config_ != nullptr) {
        config_->maxGrowthSize = msg.maxGrowthSize;
    }
    g_mutableClientConfig.maxGrowthSize.store(msg.maxGrowthSize, std::memory_order_relaxed);
    PROFILER_LOG_INFO(LOG_CORE, "HookSocketClient::ProtocolProc update, ts1.1 = %d, ts1.2 = %d, ts2 = %d, ts3 = %d",
        largestSize_, secondLargestSize_, maxGrowthSize_, sampleInterval_);
    return true;
}

bool HookSocketClient::HandleSimplifiedNmdDumpRequest(SocketContext &context)
{
    int nmdRecvFd = context.ReceiveFileDiscriptor();
    if (nmdRecvFd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleSimplifiedNmdDumpRequest fail, recv fd err");
        return false;
    }
    void *nmdBuf = malloc(MEMCHECK_DETAILINFO_MAXSIZE);
    if (nmdBuf == nullptr) {
        close(nmdRecvFd);
        return false;
    }
    struct OptArg opt = {0, reinterpret_cast<char *>(nmdBuf) };
    malloc_stats_print(NmdWriteStat, &opt, "s");
    if (!WriteDataToFd(nmdRecvFd, reinterpret_cast<const uint8_t *>(opt.buf), strlen(opt.buf), false)) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleSimplifiedNmdDumpRequest: failed to write data");
        close(nmdRecvFd);
        free(nmdBuf);
        return false;
    }
    close(nmdRecvFd);
    free(nmdBuf);
    if (unixSocketClient_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "unixSocketClient_ is NULL");
        return false;
    }
    uint32_t cmd = DUMP_SIMP_NMD_DONE_CMD;
    if (!unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t*>(&cmd), sizeof(cmd))) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleSimplifiedNmdDumpRequest: failed to send cmd, errno: %d", errno);
        return false;
    }
    return true;
}

bool HookSocketClient::HandleControlCommand(uint32_t cmd)
{
    if (cmd == SNAPSHOT_REQUEST_CMD) {
#ifdef OHOS_ENABLE_SAMPLER
        je_malloc_sampler_alloc_report_all();
#endif
        return true;
    }
    if (cmd == FINAL_SNAPSHOT_REQUEST_CMD) {
#ifdef OHOS_ENABLE_SAMPLER
        je_malloc_sampler_data_fill_hook_reg(nullptr);
        je_malloc_sampler_alloc_report_all();
#endif
        return true;
    }
    if (cmd != STOP_HOOK_CMD) {
        return true;
    }
    PROFILER_LOG_INFO(LOG_CORE, "HandleControlCommand: received STOP_HOOK_CMD, pid %d", pid_);
    bool expected = false;
    if (stopRequested_.compare_exchange_strong(expected, true,
        std::memory_order_release, std::memory_order_relaxed)) {
        PROFILER_LOG_INFO(LOG_CORE, "HandleControlCommand: trigger hook end, pid %d", pid_);
        __set_hook_flag(false);
        (void)ohos_malloc_hook_on_end();
        __set_hook_flag(true);
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "HandleControlCommand: stop already requested, pid %d", pid_);
    }
    return true;
}

void HookSocketClient::HandleSharedMemoryMode(SocketContext &context)
{
    // shared memory mode
    int sharedMemCount = config_->offlineSymbolization ? SHARED_MEMORY_NUM : 1;
    for (int i = 0; i < sharedMemCount; ++i) {
        int smbfd = context.ReceiveFileDiscriptor();
        int eventfd = context.ReceiveFileDiscriptor();
        smbFds_.push_back(smbfd);
        eventFds_.push_back(eventfd);
        std::string smbName = "hooknativesmb_" + std::to_string(pid_) + ":" + std::to_string(i);
        std::shared_ptr<StackWriter> stackWriter = std::make_shared<StackWriter>(smbName, config_->shareMemorySize,
            smbfd, eventfd, config_->isBlocked, config_->isSaMode, false);
        stackWriterList_.push_back(stackWriter);
    }
    if (sharedMemCount_ != nullptr) {
        *sharedMemCount_ = sharedMemCount;
    }
}

void HookSocketClient::HandleCommonConfig()
{
    nmdType_ = config_->nmdType;
    g_disableHook = false;
    largestSize_ = config_->largestSize;
    secondLargestSize_ = config_->secondLargestSize;
    maxGrowthSize_ = config_->maxGrowthSize;
    g_mutableClientConfig.maxGrowthSize.store(config_->maxGrowthSize, std::memory_order_relaxed);
    sampleInterval_ = config_->sampleInterval;
    PROFILER_LOG_INFO(LOG_CORE, "HookSocketClient::ProtocolProc, ts1.1 = %d, ts1.2 = %d, ts2 = %d, ts3 = %d",
        largestSize_, secondLargestSize_, maxGrowthSize_, sampleInterval_);
}

void HookSocketClient::LogReceivedConfig()
{
    PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::ProtocolProc get memtraceEnable: %d and traceMask: %" PRIu64,
                       config_->memtraceEnable, static_cast<uint64_t>(config_->traceMask));
    SetAsyncStack();
    for (size_t i = 0; i < GPU_RANGE_COUNT * 2; ++i) {
        PROFILER_LOG_ERROR(LOG_CORE,
            "HookSocketClient::ProtocolProc get gpu range: %" PRIu64 ", %" PRIu64 ", %" PRIu64
            ", %" PRIu64 ", %" PRIu64 ",",
            config_->gpuRange.gpuVk[i], config_->gpuRange.gpuGlesImage[i],
            config_->gpuRange.gpuGlesBuffer[i], config_->gpuRange.gpuClImage[i],
            config_->gpuRange.gpuClBuffer[i]);
    }
    config_->maxStackDepth = config_->maxStackDepth > MAX_UNWIND_DEPTH ? MAX_UNWIND_DEPTH : config_->maxStackDepth;
    std::string configStr = config_->ToString();
    (void)configStr;
}

void HookSocketClient::PrepareWriters(SocketContext &context)
{
    if (config_->enableFileCacheMode) {
        HandleFileCacheMode(context);
    } else {
        HandleSharedMemoryMode(context);
    }
    if (!config_->asyncStackEnable) {
        return;
    }
    int tableFd = context.ReceiveFileDiscriptor();
    uint32_t uniqueStackSize = NormalizeUniqueStackTableSize(config_->uniqueStackTableSize);
    std::shared_ptr<StackWriter> stackWriter = std::make_shared<StackWriter>("unique_stack_table",
        uniqueStackSize, tableFd, -1, false, false);
    stackWriterList_.push_back(stackWriter);
}

void HookSocketClient::ReceiveMemLeakFd(SocketContext &context)
{
    if (config_->memLeakFormat == MemLeakFormat::TYPE_DISABLE) {
        return;
    }
    memLeakRecvFd_ = context.ReceiveFileDiscriptor();
    if (memLeakRecvFd_ >= 0) {
        fdsan_exchange_owner_tag(memLeakRecvFd_, 0, PROFILER_SUBSYSTEM);
    }
    PROFILER_LOG_INFO(LOG_CORE, "ReceiveMemLeakFd fd=%d", memLeakRecvFd_);
}

void HookSocketClient::SendOptionalReports()
{
    HandleCommonConfig();
    if (config_->onlyMemLeak) {
        if (!GetAndSendMemLeakInfo()) {
            PROFILER_LOG_ERROR(LOG_CORE, "SendOptionalReports mem leak report failed");
        }
    }
    if (config_->allocatorSamplingEnable) {
#ifdef OHOS_ENABLE_SAMPLER
        je_malloc_sampler_data_fill_hook_reg(hook_fill_data);
        je_malloc_sampler_data_dump_hook_reg(hook_data_dump);
        je_malloc_sampler_alloc_report_hook_reg(hook_alive_addr_dump);
#endif
    }
    if (nmdType_ == 0 || nmdType_ == ONLY_NMD_TYPE) {
        SendNmdInfo();
    } else if (nmdType_ == SIMP_NMD) {
        SendSimplifiedNmdInfo();
    }
#if defined(target_cpu_arm64)
    if (config_->gwpAsanHoldTime > 0) {
        SendGwpAsanInfo();
    }
#endif
}

bool HookSocketClient::ParseTargetMapsIfNeeded()
{
    if (config_ == nullptr) {
        return false;
    }
    if (config_->targetSoName.empty()) {
        return true;
    }
    std::lock_guard<std::recursive_timed_mutex> guard(g_FilterMapMutex);
    if (targetLibRange_ != nullptr && ParseTargetedMaps(*targetLibRange_, config_->targetSoName)) {
        return true;
    }
    PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::ProtocolProc ParseTargetedMaps failed!");
    return false;
}

bool HookSocketClient::ApplyClientConfig(SocketContext &context, const ClientConfig& clientConfig)
{
    if (config_ == nullptr) {
        return false;
    }
    *config_ = clientConfig;
    EnableArkTsHookIfNeeded(config_->traceMask);
    LogReceivedConfig();
    PrepareWriters(context);
    ReceiveMemLeakFd(context);
    SendOptionalReports();
    return ParseTargetMapsIfNeeded();
}

bool HookSocketClient::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (buf == nullptr) {
        return false;
    }
    if (size == sizeof(BackpressureStateMsg) && config_ != nullptr && config_->enableFileCacheMode) {
        return HandleBackpressureState(context, buf, size);
    }
    if (size == sizeof(HookCacheFileMsg) && config_ != nullptr && config_->enableFileCacheMode) {
        HookCacheFileMsg cacheFileMsg;
        if (memcpy_s(&cacheFileMsg, sizeof(cacheFileMsg), buf, size) != EOK) {
            return false;
        }
        return HandleFileCacheMsg(context, cacheFileMsg);
    }
    if (size == sizeof(SimplifiedMemConfigMsg)) {
        return HandleSimplifiedMemConfigChanged(context, buf, size);
    }
    if (size == sizeof(uint32_t)) {
        uint32_t cmd = 0;
        if (memcpy_s(&cmd, sizeof(cmd), buf, size) != EOK) {
            return false;
        }
        if (cmd == DUMP_SIMP_NMD_CMD) {
            return HandleSimplifiedNmdDumpRequest(context);
        }
        return HandleControlCommand(cmd);
    }
    if (size != sizeof(ClientConfigIpc)) {
        return true;
    }
    ClientConfigIpc clientConfigIpc;
    if (memcpy_s(&clientConfigIpc, sizeof(clientConfigIpc), buf, size) != EOK) {
        return false;
    }
    ClientConfig clientConfig;
    ConvertClientConfigFromIpc(clientConfigIpc, clientConfig);
    return ApplyClientConfig(context, clientConfig);
}

bool HookSocketClient::GetAndSendMemLeakInfo()
{
    if (config_->memLeakFormat == MemLeakFormat::TYPE_DISABLE) {
        return false;
    }

    MemoryLeakDetector memLeakDetector;
    std::string data;
    if (!memLeakDetector.GetMemLeakInfo(static_cast<int>(config_->memLeakFormat),
        static_cast<size_t>(config_->memLeakLimit), data)) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetAndSendMemLeakInfo GetMemLeakInfo failed");
        return false;
    }
    return WriteMemLeakInfo(data) && SendMemLeakDone();
}

bool HookSocketClient::WriteMemLeakInfo(const std::string& data)
{
    if (memLeakRecvFd_ < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteMemLeakInfo invalid fd %d", memLeakRecvFd_);
        return false;
    }
    size_t writtenSize = 0;
    while (writtenSize < data.size()) {
        ssize_t written = write(memLeakRecvFd_, data.data() + writtenSize, data.size() - writtenSize);
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written <= 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "WriteMemLeakInfo write failed, errno=%d", errno);
            fdsan_close_with_tag(memLeakRecvFd_, PROFILER_SUBSYSTEM);
            memLeakRecvFd_ = -1;
            return false;
        }
        writtenSize += static_cast<size_t>(written);
    }
    PROFILER_LOG_INFO(LOG_CORE, "WriteMemLeakInfo wrote %zu bytes to fd %d", writtenSize, memLeakRecvFd_);
    if (fsync(memLeakRecvFd_) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteMemLeakInfo fsync failed, errno=%d", errno);
    }
    fdsan_close_with_tag(memLeakRecvFd_, PROFILER_SUBSYSTEM);
    memLeakRecvFd_ = -1;
    return true;
}

bool HookSocketClient::SendMemLeakDone()
{
    if (unixSocketClient_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "SendMemLeakDone socket missing");
        return false;
    }
    uint32_t cmd = MEM_LEAK_DONE_CMD;
    bool ret = unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t*>(&cmd), sizeof(cmd));
    PROFILER_LOG_INFO(LOG_CORE, "SendMemLeakDone result %d", ret);
    return ret;
}

bool HookSocketClient::SendStack(const void* data, size_t size)
{
    if (stackWriterList_.size() == 0 || stackWriterList_[0] == nullptr || unixSocketClient_ == nullptr) {
        return false;
    }

    if (!unixSocketClient_->SendHeartBeat()) {
        return false;
    }

    stackWriterList_[0]->WriteTimeout(data, size);
    stackWriterList_[0]->Flush();

    return true;
}

bool HookSocketClient::SendStackWithPayload(const void* data, size_t size, const void* payload,
    size_t payloadSize, int smbIndex)
{
    if (smbIndex + 1 > static_cast<int>(stackWriterList_.size())) {
        return false;
    }
    if (g_disableHook || unixSocketClient_ == nullptr) {
        return false;
    }
    if (unixSocketClient_->GetClientState() == CLIENT_STAT_THREAD_EXITED) {
        return false;
    }
    std::shared_ptr<StackWriter> stackWriter = stackWriterList_[smbIndex];
    if (!stackWriter) {
        return false;
    }
    bool ret = stackWriter->WriteWithPayloadTimeout(data, size, payload, payloadSize,
                                                    std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret && config_->isBlocked) {
        return false;
    }
    if (!stackWriter->IsFileCacheMode() && stackWriter->PrepareFlush()) {
        stackWriter->Flush();
    }
    return true;
}

bool HookSocketClient::SendStackWithExtraData(const void* data, size_t size, const void* payload,
    size_t payloadSize, ExtraDataInfo& extraDataInfo, int smbIndex)
{
    if (smbIndex + 1 > static_cast<int>(stackWriterList_.size())) {
        return false;
    }
    if (g_disableHook || unixSocketClient_ == nullptr) {
        return false;
    }
    if (unixSocketClient_->GetClientState() == CLIENT_STAT_THREAD_EXITED) {
        return false;
    }
    std::shared_ptr<StackWriter> stackWriter = stackWriterList_[smbIndex];
    if (!stackWriter) {
        return false;
    }
    bool ret = stackWriter->WriteWithExtraDataTimeout(data, size, payload, payloadSize,
        extraDataInfo.firstExtra, extraDataInfo.firstExtraSize,
        extraDataInfo.secondExtra, extraDataInfo.secondExtraSize,
        std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret && config_->isBlocked) {
        return false;
    }
    if (!stackWriter->IsFileCacheMode() && stackWriter->PrepareFlush()) {
        stackWriter->Flush();
    }
    return true;
}

bool HookSocketClient::SendRawData(const void* data, size_t size, int smbIndex)
{
    if (smbIndex + 1 > static_cast<int>(stackWriterList_.size())) {
        return false;
    }
    if (g_disableHook || unixSocketClient_ == nullptr) {
        return false;
    }
    if (unixSocketClient_->GetClientState() == CLIENT_STAT_THREAD_EXITED) {
        return false;
    }
    std::shared_ptr<StackWriter> stackWriter = stackWriterList_[smbIndex];
    if (!stackWriter) {
        return false;
    }
    long ret = stackWriter->WriteSync(data, size,
                                       std::bind(&HookSocketClient::PeerIsConnected, this));
    if (ret <= 0 && config_->isBlocked) {
        return false;
    }
    if (stackWriter->PrepareFlush()) {
        stackWriter->Flush();
    }
    return ret > 0;
}

void HookSocketClient::Flush()
{
    for (size_t i = 0; i < stackWriterList_.size(); ++i) {
        if (stackWriterList_[i] == nullptr || unixSocketClient_ == nullptr) {
            return;
        }
        stackWriterList_[i]->Flush();
    }
}

void HookSocketClient::DisableHook()
{
    bool expected = false;
    if (g_disableHook.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed)) {
        RestoreAsyncStackType();
        HILOG_BASE_INFO(LOG_CORE, "%s", __func__);
        if (disableHookCallback_) {
            disableHookCallback_();
        }
    }
}

void HookSocketClient::RestoreAsyncStackType()
{
#ifdef __aarch64__
    if (asyncFlagConfigured_) {
        DfxSetAsyncStackType(asyncFlagBackup_);
        SetAsyncStackMode(MODE_LAST_STACKTRACE);
        asyncFlagConfigured_ = false;
    }
#endif
}

bool HookSocketClient::PeerIsConnected()
{
    return !unixSocketClient_->IsConnected();
}

bool HookSocketClient::SendNmdInfo()
{
    if ((!config_->printNmd) || (stackWriterList_.size() == 0)) {
        return false;
    }
    void* nmdBuf = malloc(MEMCHECK_DETAILINFO_MAXSIZE);
    if (nmdBuf == nullptr) {
        return false;
    }
    struct OptArg opt = {0, reinterpret_cast<char*>(nmdBuf) };
    malloc_stats_print(NmdWriteStat, &opt, "a");
    StackRawData rawdata = {{{{0}}}};
    rawdata.type = NMD_MSG;
    if (!stackWriterList_[0]) {
        free(nmdBuf);
        return false;
    }
    bool ret = stackWriterList_[0]->WriteWithPayloadTimeout(&rawdata, sizeof(BaseStackRawData),
                                                            reinterpret_cast<int8_t *>(opt.buf), strlen(opt.buf) + 1,
                                                            std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret) {
        free(nmdBuf);
        return false;
    }
    stackWriterList_[0]->Flush();
    free(nmdBuf);
    return true;
}

bool HookSocketClient::SendSimplifiedNmdInfo()
{
    if ((!config_->printNmd) || (stackWriterList_.size() == 0)) {
        return false;
    }
    void* nmdBuf = malloc(MEMCHECK_DETAILINFO_MAXSIZE);
    if (nmdBuf == nullptr) {
        return false;
    }
    struct OptArg opt = {0, reinterpret_cast<char*>(nmdBuf) };
    malloc_stats_print(NmdWriteStat, &opt, "s");
    StackRawData rawdata = {{{{0}}}};
    rawdata.type = NMD_MSG;
    if (!stackWriterList_[0]) {
        free(nmdBuf);
        return false;
    }
    bool ret = stackWriterList_[0]->WriteWithPayloadTimeout(&rawdata, sizeof(BaseStackRawData),
                                                            reinterpret_cast<int8_t *>(opt.buf), strlen(opt.buf) + 1,
                                                            std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret) {
        free(nmdBuf);
        return false;
    }
    stackWriterList_[0]->Flush();
    free(nmdBuf);
    return true;
}

#if defined(target_cpu_arm64)
bool HookSocketClient::SendGwpAsanInfo()
{
    if (stackWriterList_.empty() || !stackWriterList_[0]) {
        return false;
    }
    size_t maxAllocations = config_->gwpAsanMaxEntries;
    uint64_t timespan = config_->gwpAsanHoldTime;
    size_t deep = config_->gwpAsanMaxStackDepth;
    uintptr_t *buffer = reinterpret_cast<uintptr_t *>(malloc(MEMCHECK_DETAILINFO_MAXSIZE - sizeof(size_t)));
    if (!buffer) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::SendGwpAsanInfo malloc failed!");
        return false;
    }
    size_t collected_count = libc_gwp_asan_collect_allocations_by_time_range(timespan, buffer,
        maxAllocations, deep);
    int8_t *payload = reinterpret_cast<int8_t *>(malloc(MEMCHECK_DETAILINFO_MAXSIZE));
    if (!payload) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::SendGwpAsanInfo malloc payload failed!");
        free(buffer);
        return false;
    }
    if (memcpy_s(payload, sizeof(size_t), &collected_count, sizeof(size_t)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::SendGwpAsanInfo memcpy_s collected_count failed!");
        free(buffer);
        free(payload);
        return false;
    }
    if (memcpy_s(payload + sizeof(size_t), MEMCHECK_DETAILINFO_MAXSIZE - sizeof(size_t), buffer,
                 MEMCHECK_DETAILINFO_MAXSIZE - sizeof(size_t)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::SendGwpAsanInfo memcpy_s buffer failed!");
        free(buffer);
        free(payload);
        return false;
    }
    free(buffer);
    StackRawData rawdata = {{{{0}}}};
    rawdata.type = GWP_ASAN_MSG;
    bool ret = stackWriterList_[0]->WriteWithPayloadTimeout(&rawdata, sizeof(BaseStackRawData),
                                                            payload, MEMCHECK_DETAILINFO_MAXSIZE,
                                                            std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret) {
        free(payload);
        return false;
    }
    stackWriterList_[0]->Flush();
    free(payload);
    return true;
}
#endif

bool HookSocketClient::SendEndMsg()
{
    StackRawData rawdata = {{{{0}}}};
    rawdata.type = END_MSG;
    if (g_disableHook || unixSocketClient_ == nullptr) {
        PROFILER_LOG_WARN(LOG_CORE, "SendEndMsg: hook disabled or client missing, pid %d", pid_);
        return false;
    }
    if (!config_->onlyMemLeak) {
        GetAndSendMemLeakInfo();
    }
    int sharedMemCount = *sharedMemCount_;
    PROFILER_LOG_INFO(LOG_CORE, "SendEndMsg: send END_MSG, pid %d, sharedMemCount %d, fileCacheMode %d",
        pid_, sharedMemCount, config_->enableFileCacheMode);
    for (int smbIndex = 0; smbIndex < sharedMemCount; ++smbIndex) {
        auto& writer = stackWriterList_[smbIndex];
        if (writer) {
            writer->EnterStopFileCacheMode();
            writer->WriteTimeout(&rawdata, sizeof(BaseStackRawData));
            writer->StopFileCache();
        }
    }
    return true;
}

void HookSocketClient::SetAsyncStack()
{
#ifdef __aarch64__
    if (config_->asyncStackEnable && config_->asyncFlag != 0) {
        asyncFlagBackup_ = DfxSetAsyncStackType(config_->asyncFlag);
        asyncFlagConfigured_ = true;
        if (config_->nestedAsyncStackEnable) {
            // asyncNestingDepth is nesting count, layer count is depth + 1
            constexpr uint32_t DEFAULT_MAX_ASYNC_CTX_COUNT = 64 * 1024;
            SetChainedAsyncStackConfig(
                config_->asyncNestingDepth + 1,
                config_->asyncTaskStackDepth,
                DEFAULT_MAX_ASYNC_CTX_COUNT);
            SetAsyncStackMode(MODE_CHAINED_STACKTRACE);
        }
        // the default mode is MODE_LAST_STACKTRACE.
    }
#endif
}

bool HookSocketClient::WriteDataToFd(int fd, const uint8_t *data, size_t size, bool needFlush)
{
    if (fd < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteDataToFd invalid fd %d", fd);
        return false;
    }
    size_t writtenSize = 0;
    while (writtenSize < size) {
        ssize_t written = write(fd, data + writtenSize, size - writtenSize);
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written <= 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "WriteDataToFd write failed, errno=%d", errno);
            return false;
        }
        writtenSize += static_cast<size_t>(written);
    }
    if (needFlush && fsync(fd) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "WriteDataToFd fsync failed, errno=%d", errno);
        return false;
    }
    return true;
}
