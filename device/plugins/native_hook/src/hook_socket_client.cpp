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
#include <malloc.h>
#include "hook_socket_client.h"

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
#include <cstring>
#include <iostream>

namespace {
constexpr int FLUSH_FLAG = 20;
std::atomic<uint64_t> g_flushCount = 0;
std::atomic<bool> g_disableHook = false;
constexpr uint32_t MEMCHECK_DETAILINFO_MAXSIZE = 102400;

struct OptArg {
    size_t pos;
    char *buf;
};

} // namespace


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

HookSocketClient::HookSocketClient(int pid, ClientConfig *config, Sampling *sampler, void (*disableHookCallback)())
    : pid_(pid), config_(config), sampler_(sampler), disableHookCallback_(disableHookCallback)
{
    smbFd_ = 0;
    eventFd_ = 0;
    unixSocketClient_ = nullptr;
    serviceName_ = "HookService";
    Connect(DEFAULT_UNIX_SOCKET_HOOK_FULL_PATH);
}

HookSocketClient::~HookSocketClient()
{
    if (stackWriter_) {
        stackWriter_->Flush();
    }
    unixSocketClient_ = nullptr;
    stackWriter_ = nullptr;
}

bool HookSocketClient::Connect(const std::string addrname)
{
    if (unixSocketClient_ != nullptr) {
        return false;
    }
    unixSocketClient_ = std::make_shared<UnixSocketClient>();
    if (!unixSocketClient_->Connect(addrname, *this)) {
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
    smbFd_ = context.ReceiveFileDiscriptor();
    eventFd_ = context.ReceiveFileDiscriptor();
    std::string smbName = "hooknativesmb_" + std::to_string(pid_);
    stackWriter_ = std::make_shared<StackWriter>(smbName, config_->shareMemorySize,
        smbFd_, eventFd_, config_->isBlocked);
    nmdType_ = config_->nmdType;
    if (nmdType_ == 0) {
        SendNmdInfo();
    }
    return true;
}

bool HookSocketClient::SendStack(const void* data, size_t size)
{
    if (stackWriter_ == nullptr || unixSocketClient_ == nullptr) {
        return false;
    }

    if (!unixSocketClient_->SendHeartBeat()) {
        return false;
    }

    stackWriter_->WriteTimeout(data, size);
    stackWriter_->Flush();

    return true;
}

bool HookSocketClient::SendStackWithPayload(const void* data, size_t size, const void* payload,
    size_t payloadSize)
{
    if (stackWriter_ == nullptr || unixSocketClient_ == nullptr || g_disableHook) {
        return false;
    } else if (unixSocketClient_->GetClientState() == CLIENT_STAT_THREAD_EXITED) {
        DisableHook();
        return false;
    }

    bool ret = stackWriter_->WriteWithPayloadTimeout(data, size, payload, payloadSize,
                                                     std::bind(&HookSocketClient::PeerIsConnected, this));
    if (!ret && config_->isBlocked) {
        DisableHook();
        return false;
    }
    ++g_flushCount;
    if (g_flushCount % FLUSH_FLAG == 0) {
        stackWriter_->Flush();
    }
    return true;
}

void HookSocketClient::Flush()
{
    if (stackWriter_ == nullptr || unixSocketClient_ == nullptr) {
        return;
    }
    stackWriter_->Flush();
}

void HookSocketClient::DisableHook()
{
    bool expected = false;
    if (g_disableHook.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed)) {
        HILOG_INFO(LOG_CORE, "%s", __func__);
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
    if (!config_->printNmd) {
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
    if (stackWriter_) {
        stackWriter_->WriteWithPayloadTimeout(&rawdata, sizeof(BaseStackRawData),
                                              reinterpret_cast<int8_t*>(opt.buf), strlen(opt.buf) + 1,
                                              std::bind(&HookSocketClient::PeerIsConnected, this));
    }
    free(nmdBuf);
    return true;
}

bool HookSocketClient::SendEndMsg()
{
    StackRawData rawdata = {{{{0}}}};
    rawdata.type = END_MSG;
    if (stackWriter_) {
        stackWriter_->WriteTimeout(&rawdata, sizeof(BaseStackRawData));
    }
    return true;
}