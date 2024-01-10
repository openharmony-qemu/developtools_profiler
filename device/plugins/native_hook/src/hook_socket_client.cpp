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
#include "logging.h"
#include "sampling.h"

namespace {
constexpr int FLUSH_FLAG = 20;
std::atomic<uint64_t> g_flushCount = 0;
std::atomic<bool> g_disableHook = false;
} // namespace

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
        PROFILER_LOG_INFO(LOG_CORE, "~HookSocketClient Flush()");
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
    CHECK_TRUE(size == sizeof(ClientConfig), true, "HookSocketClient::config config size not match = %u\n", size);
    *config_ = *reinterpret_cast<ClientConfig *>(const_cast<int8_t*>(buf));
    config_->maxStackDepth  = config_->maxStackDepth > MAX_UNWIND_DEPTH ? MAX_UNWIND_DEPTH : config_->maxStackDepth;
    std::string configStr = config_->ToString();
    PROFILER_LOG_INFO(LOG_CORE, "recv hook client config:%s\n", configStr.c_str());
    sampler_->InitSampling(config_->sampleInterval);
    PROFILER_LOG_INFO(LOG_CORE, "%s sample interval %" PRIu64 "", __func__, sampler_->GetSampleInterval());
    smbFd_ = context.ReceiveFileDiscriptor();
    eventFd_ = context.ReceiveFileDiscriptor();
    std::string smbName = "hooknativesmb_" + std::to_string(pid_);
    stackWriter_ = std::make_shared<StackWriter>(smbName, config_->shareMemorySize,
        smbFd_, eventFd_, config_->isBlocked);
    struct mallinfo2 mi = mallinfo2();
    COMMON::PrintMallinfoLog("stackWriter init(byte) => ", mi);
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
        HILOG_INFO(LOG_CORE, "%s %p", __func__, disableHookCallback_);
        if (disableHookCallback_) {
            disableHookCallback_();
        }
    }
}

bool HookSocketClient::PeerIsConnected()
{
    return !unixSocketClient_->IsConnected();
}