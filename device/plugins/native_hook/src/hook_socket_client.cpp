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
#include "unix_socket_client.h"
#include "share_memory_allocator.h"
#include "logging.h"
#include "sampling.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
                                   std::atomic<Range>* targetedRange, std::atomic<int>* memCount,
                                   void (*disableHookCallback)())
    : pid_(pid), config_(config), targetedRange_(targetedRange),
      sharedMemCount_(memCount), disableHookCallback_(disableHookCallback)
{
    g_disableHook = true;
    int sharedMemCount = (config_->offlineSymbolization) ? SHARED_MEMORY_NUM : 1;
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
    // file cache mode enabled
    int eventfd = context.ReceiveFileDiscriptor();
    std::shared_ptr<StackWriter> stackWriter = std::make_shared<StackWriter>(
        "hook_file_cache", config_->shareMemorySize, config_->fileCacheBufferSize, eventfd,
        config_->isBlocked, config_->isSaMode, config_->enableFileCacheMode);
    stackWriter->PrepareCacheFileWriter([this](uint16_t fileIndex, uint16_t eventType, int fd) {
        if (unixSocketClient_ != nullptr && eventType == CACHE_FILE_CREATED) {
            // when a new cache file is craeted, send fd to native_daemon
            HookCacheFileMsg cacheFileMsg(pid_, fileIndex, CACHE_FILE_CREATED);
            unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t *>(&cacheFileMsg), sizeof(cacheFileMsg));
            unixSocketClient_->SendFileDescriptor(fd);
            PROFILER_LOG_INFO(LOG_CORE, "[%d] send cache file fd to native_daemon", pid_);
        }
    });
    stackWriterList_.push_back(stackWriter);
    PROFILER_LOG_INFO(LOG_CORE, "HookSocketClient::ProtocolProc use file cache mode");
    if (sharedMemCount_ != nullptr) {
        *sharedMemCount_ = 1;
    }
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
    sampleInterval_ = config_->sampleInterval;
    PROFILER_LOG_INFO(LOG_CORE, "HookSocketClient::ProtocolProc, ts1.1 = %d, ts1.2 = %d, ts2 = %d, ts3 = %d",
        largestSize_, secondLargestSize_, maxGrowthSize_, sampleInterval_);
}

bool HookSocketClient::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (size == sizeof(uint32_t)) {
        uint32_t cmd = *reinterpret_cast<const uint32_t*>(buf);
        if (cmd == SNAPSHOT_REQUEST_CMD) {
#ifdef OHOS_ENABLE_SAMPLER
            malloc_sampler_alloc_report_all();
#endif
            return true;
        } else if (cmd == FINAL_SNAPSHOT_REQUEST_CMD) {
#ifdef OHOS_ENABLE_SAMPLER
            malloc_sampler_data_fill_hook_reg(nullptr);
            malloc_sampler_alloc_report_all();
#endif
            return true;
        }
    }
    if (size != sizeof(ClientConfig)) {
        return true;
    }
    *config_ = *reinterpret_cast<ClientConfig *>(const_cast<int8_t*>(buf));
    PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::ProtocolProc get memtraceEnable: %d and traceMask: %" PRIu64,
                       config_->memtraceEnable, static_cast<uint64_t>(config_->traceMask));
#ifdef __aarch64__
    if (config_->asyncStackEnable && config_->asyncFlag != 0) {
        asyncFlagBackup_ = DfxSetAsyncStackType(config_->asyncFlag);
        asyncFlagConfigured_ = true;
    }
#endif
    for (size_t i = 0; i < GPU_RANGE_COUNT * 2; ++i) { // 2: vk and gles image
        PROFILER_LOG_ERROR(LOG_CORE,
            "HookSocketClient::ProtocolProc get gpu range: %" PRIu64 ", %" PRIu64 ", %" PRIu64
            ", %" PRIu64 ", %" PRIu64 ",",
            config_->gpuRange.gpuVk[i], config_->gpuRange.gpuGlesImage[i],
            config_->gpuRange.gpuGlesBuffer[i], config_->gpuRange.gpuClImage[i],
            config_->gpuRange.gpuClBuffer[i]);
    }
    config_->maxStackDepth  = config_->maxStackDepth > MAX_UNWIND_DEPTH ? MAX_UNWIND_DEPTH : config_->maxStackDepth;
    std::string configStr = config_->ToString();

    if (config_->enableFileCacheMode) {
        // file cache mode enabled
        HandleFileCacheMode(context);
    } else {
        // shared memory mode
        HandleSharedMemoryMode(context);
    }

    if (config_->asyncStackEnable) {
        int tableFd = context.ReceiveFileDiscriptor();
        uint32_t uniqueStackSize = NormalizeUniqueStackTableSize(config_->uniqueStackTableSize);
        std::shared_ptr<StackWriter> stackWriter = std::make_shared<StackWriter>("unique_stack_table",
            uniqueStackSize, tableFd, -1, false, false);
        stackWriterList_.push_back(stackWriter);
    }
    HandleCommonConfig();
    if (config_->allocatorSamplingEnable) {
#ifdef OHOS_ENABLE_SAMPLER
        malloc_sampler_data_fill_hook_reg(hook_fill_data);
        malloc_sampler_data_dump_hook_reg(hook_data_dump);
        malloc_sampler_alloc_report_hook_reg(hook_alive_addr_dump);
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
    if (!config_->targetSoName.empty() && !ParseTargetedMaps(*targetedRange_, config_->targetSoName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookSocketClient::ProtocolProc ParseTargetedMaps failed!");
        return false;
    }
    return true;
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
        DisableHook();
        return false;
    }
    std::shared_ptr<StackWriter> stackWriter = stackWriterList_[smbIndex];
    if (!stackWriter) {
        return false;
    }
    bool ret = stackWriter->WriteWithPayloadTimeout(data, size, payload, payloadSize,
                                                    std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret && config_->isBlocked) {
        DisableHook();
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
        DisableHook();
        return false;
    }
    std::shared_ptr<StackWriter> stackWriter = stackWriterList_[smbIndex];
    if (!stackWriter) {
        return false;
    }
    long ret = stackWriter->WriteSync(data, size,
                                       std::bind(&HookSocketClient::PeerIsConnected, this));
    if (ret <= 0 && config_->isBlocked) {
        DisableHook();
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
        DisableHook();
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
        DisableHook();
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
        DisableHook();
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
        return false;
    }
    std::for_each(stackWriterList_.begin(), stackWriterList_.end(),
        [&rawdata](std::shared_ptr<StackWriter>& writer) {
        if (writer) {
            writer->WriteTimeout(&rawdata, sizeof(BaseStackRawData));
            writer->StopFileCache();
        }
    });
    return true;
}
