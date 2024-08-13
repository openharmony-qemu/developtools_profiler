/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "network_profiler_socker_client.h"

#include "network_profiler_common.h"
#include "network_profiler.h"

namespace {
std::atomic<bool> g_disableHook = false;

} // namespace

namespace OHOS::Developtools::Profiler {
NetworkProfilerSocketClient::NetworkProfilerSocketClient(int pid, NetworkProfiler* profiler,
    void (*disableHookCallback)()) : pid_(pid), disableHookCallback_(disableHookCallback), profiler_(profiler)
{
    smbFd_ = 0;
    eventFd_ = 0;
    unixSocketClient_ = nullptr;
    serviceName_ = "NetworkProfilerService";
    Connect(NETWORK_PROFILER_UNIX_SOCKET_FULL_PATH, disableHookCallback_);
}

NetworkProfilerSocketClient::~NetworkProfilerSocketClient()
{
    if (writer_) {
        writer_->Flush();
    }
    unixSocketClient_ = nullptr;
    writer_ = nullptr;
}

bool NetworkProfilerSocketClient::Connect(const std::string addrname, void (*disableHookCallback)())
{
    if (unixSocketClient_ != nullptr) {
        return false;
    }
    unixSocketClient_ = std::make_shared<UnixSocketClient>();
    if (!unixSocketClient_->Connect(addrname, *this, disableHookCallback)) {
        unixSocketClient_ = nullptr;
        return false;
    }

    unixSocketClient_->SendHookConfig(reinterpret_cast<uint8_t *>(&pid_), sizeof(pid_));
    return true;
}

bool NetworkProfilerSocketClient::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf,
                                               const uint32_t size)
{
    CHECK_TRUE(size == sizeof(NetworkConfig), true, "NetworkProfilerSocketClient config size not match = %u\n", size);
    NetworkConfig* config = reinterpret_cast<NetworkConfig *>(const_cast<int8_t*>(buf));
    smbFd_ = context.ReceiveFileDiscriptor();
    eventFd_ = context.ReceiveFileDiscriptor();
    PROFILER_LOG_INFO(LOG_CORE, "network profiler client: ProtocolProc: smbFd: %d, eventFd: %d, shmSize: %d",
        smbFd_, eventFd_, config->shmSize);
    flushInterval_ = config->flushCount;
    std::string smbName = "networkProfilerSmb_" + std::to_string(pid_);
    profiler_->SetClockId(config->clock);
    writer_ = std::make_shared<NetworkProfilerWriter>(smbName, config->shmSize, smbFd_, eventFd_, config->block);
    profiler_->SetEnableFlag(true);
    profiler_->SetWaitingFlag(false);
    profiler_->SendCachedData();
    return true;
}

bool NetworkProfilerSocketClient::SendNetworkProfilerData(const void* data, size_t size,
                                                          const void* payload, size_t payloadSize)
{
    if (writer_ == nullptr || unixSocketClient_ == nullptr || g_disableHook) {
        return false;
    } else if (unixSocketClient_->GetClientState() == CLIENT_STAT_THREAD_EXITED) {
        DisableHook();
        return false;
    }

    bool ret = writer_->WriteWithPayloadTimeout(
        data,
        size,
        payload,
        payloadSize,
        std::bind(&NetworkProfilerSocketClient::PeerIsConnected, this));
    if (!ret) {
        DisableHook();
        return false;
    }
    writer_->Flush();
    return true;
}

void NetworkProfilerSocketClient::Flush()
{
    if (writer_ == nullptr || unixSocketClient_ == nullptr) {
        return;
    }
    writer_->Flush();
}

void NetworkProfilerSocketClient::DisableHook()
{
    bool expected = false;
    if (g_disableHook.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed)) {
        HILOG_INFO(LOG_CORE, "%s %p", __func__, disableHookCallback_);
        if (disableHookCallback_ != nullptr) {
            disableHookCallback_();
        }
    }
}

bool NetworkProfilerSocketClient::PeerIsConnected()
{
    if (unixSocketClient_ == nullptr) {
        return false;
    }
    return !unixSocketClient_->IsConnected();
}
}