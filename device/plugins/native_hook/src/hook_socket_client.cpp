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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
constexpr int FLUSH_FLAG = 20;
constexpr int ONLY_NMD_TYPE = 2;
constexpr int SIMP_NMD = 3;
std::atomic<bool> g_disableHook = true;
constexpr uint32_t MEMCHECK_DETAILINFO_MAXSIZE = 102400;

struct OptArg {
    size_t pos;
    char *buf;
};

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

HookSocketClient::HookSocketClient(int pid, ClientConfig *config, Sampling *sampler,
                                   std::atomic<Range>* targetedRange, std::atomic<int>* memCount,
                                   void (*disableHookCallback)())
    : pid_(pid), config_(config), sampler_(sampler), targetedRange_(targetedRange),
      sharedMemCount_(memCount), disableHookCallback_(disableHookCallback)
{
    g_disableHook = true;
    int sharedMemCount = (config_->offlineSymbolization) ? SHARED_MEMORY_NUM : 1;
    smbFds_.reserve(sharedMemCount);
    eventFds_.reserve(sharedMemCount);
    stackWriterList_.reserve(sharedMemCount);
    unixSocketClient_ = nullptr;
    serviceName_ = "HookService";
    Connect(DEFAULT_UNIX_SOCKET_HOOK_FULL_PATH);
}

HookSocketClient::~HookSocketClient()
{
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

    unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t *>(&pid_), sizeof(pid_));
    return true;
}

bool HookSocketClient::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (size != sizeof(ClientConfig)) {
        return true;
    }
    *config_ = *reinterpret_cast<ClientConfig *>(const_cast<int8_t*>(buf));
    config_->maxStackDepth  = config_->maxStackDepth > MAX_UNWIND_DEPTH ? MAX_UNWIND_DEPTH : config_->maxStackDepth;
    std::string configStr = config_->ToString();
    sampler_->InitSampling(config_->sampleInterval);
    int sharedMemCount = (config_->offlineSymbolization) ? SHARED_MEMORY_NUM : 1;
    for (int i = 0; i < sharedMemCount; ++i) {
        int smbfd = context.ReceiveFileDiscriptor();
        int eventfd = context.ReceiveFileDiscriptor();
        smbFds_.push_back(smbfd);
        eventFds_.push_back(eventfd);
        std::string smbName = "hooknativesmb_" + std::to_string(pid_) + ":" + std::to_string(i);
        std::shared_ptr<StackWriter> stackWriter = std::make_shared<StackWriter>(smbName, config_->shareMemorySize,
            smbfd, eventfd, config_->isBlocked, config_->isSaMode);
        stackWriterList_.push_back(stackWriter);
    }
    nmdType_ = config_->nmdType;
    g_disableHook = false;
    if (sharedMemCount_ != nullptr) {
        *sharedMemCount_ = sharedMemCount;
    }
    largestSize_ = config_->largestSize;
    secondLargestSize_ = config_->secondLargestSize;
    maxGrowthSize_ = config_->maxGrowthSize;
    sampleInterval_ = config_->sampleInterval;
    PROFILER_LOG_INFO(LOG_CORE, "HookSocketClient::ProtocolProc, ts1.1 = %d, ts1.2 = %d, ts2 = %d, ts3 = %d",
        largestSize_, secondLargestSize_, maxGrowthSize_, sampleInterval_);
    if (nmdType_ == 0 || nmdType_ == ONLY_NMD_TYPE) {
        SendNmdInfo();
    } else if (nmdType_ == SIMP_NMD) {
        SendSimplifiedNmdInfo();
    }
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
    if (stackWriter->PrepareFlush()) {
        stackWriter->Flush();
    }
    return true;
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
        HILOG_BASE_INFO(LOG_CORE, "%s", __func__);
        if (disableHookCallback_) {
            disableHookCallback_();
        }
    }
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
    if (stackWriterList_[0]) {
        stackWriterList_[0]->WriteWithPayloadTimeout(&rawdata, sizeof(BaseStackRawData),
                                                     reinterpret_cast<int8_t*>(opt.buf), strlen(opt.buf) + 1,
                                                     std::bind(&HookSocketClient::PeerIsConnected, this));
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
    if (stackWriterList_[0]) {
        stackWriterList_[0]->WriteWithPayloadTimeout(&rawdata, sizeof(BaseStackRawData),
                                                     reinterpret_cast<int8_t*>(opt.buf), strlen(opt.buf) + 1,
                                                     std::bind(&HookSocketClient::PeerIsConnected, this));
    }
    stackWriterList_[0]->Flush();
    free(nmdBuf);
    return true;
}

bool HookSocketClient::SendEndMsg()
{
    StackRawData rawdata = {{{{0}}}};
    rawdata.type = END_MSG;
    std::for_each(stackWriterList_.begin(), stackWriterList_.end(),
        [&rawdata](std::shared_ptr<StackWriter>& writer) {
        if (writer) {
            writer->WriteTimeout(&rawdata, sizeof(BaseStackRawData));
        }
    });
    return true;
}