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

#include "ffrt_profiler_socker_client.h"

#include "ffrt_profiler_common.h"
#include "ffrt_profiler.h"

namespace {
std::atomic<uint64_t> g_flushCount = 0;
std::atomic<bool> g_disableHook = false;
constexpr uint32_t FILTER_SIZE = (1 << 10);
} // namespace

namespace OHOS::Developtools::Profiler {
FfrtProfilerSocketClient::FfrtProfilerSocketClient(int pid, FfrtProfiler* profiler, void (*disableHookCallback)())
    : pid_(pid), disableHookCallback_(disableHookCallback), profiler_(profiler)
{
    smbFd_ = 0;
    eventFd_ = 0;
    unixSocketClient_ = nullptr;
    serviceName_ = "FfrtProfilerService";
    Connect(FFRT_PROFILER_UNIX_SOCKET_FULL_PATH);
}

FfrtProfilerSocketClient::~FfrtProfilerSocketClient()
{
    if (writer_) {
        writer_->Flush();
    }
    unixSocketClient_ = nullptr;
    writer_ = nullptr;
}

bool FfrtProfilerSocketClient::Connect(const std::string addrname)
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

bool FfrtProfilerSocketClient::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf,
    const uint32_t size)
{
    CHECK_TRUE(size == sizeof(FfrtConfig), true, "FfrtProfilerSocketClient config size not match = %u\n", size);
    FfrtConfig* config = reinterpret_cast<FfrtConfig *>(const_cast<int8_t*>(buf));
    smbFd_ = context.ReceiveFileDiscriptor();
    eventFd_ = context.ReceiveFileDiscriptor();
    PROFILER_LOG_INFO(LOG_CORE, "ffrt profiler client: smbFd: %d, eventFd: %d, shmSize: %d",
        smbFd_, eventFd_, config->shmSize);

    flushInterval_ = static_cast<uint32_t>(config->flushCount);
    block_ = config->block;
    profiler_->SetClockId(config->clock);
    PROFILER_LOG_INFO(LOG_CORE, "ffrt profiler client: flushInterval: %d, block: %d, clock: %d",
        flushInterval_, block_, config->clock);

    std::string smbName = "ffrtProfilerSmb_" + std::to_string(pid_);
    writer_ = std::make_shared<FfrtProfilerWriter>(smbName, config->shmSize, smbFd_, eventFd_, config->block);
    profiler_->SetEnableFlag(true);
    return true;
}

bool FfrtProfilerSocketClient::SendFfrtProfilerData(const void* data, size_t size, const void* payload,
    size_t payloadSize)
{
    if (writer_ == nullptr || unixSocketClient_ == nullptr || g_disableHook) {
        return false;
    } else if (unixSocketClient_->GetClientState() == CLIENT_STAT_THREAD_EXITED) {
        DisableHook();
        return false;
    }

    if (payloadSize > FILTER_SIZE) {
        PROFILER_LOG_ERROR(LOG_CORE, "payloadSize exceeds the maximum of %d bytes", FILTER_SIZE);
        return false;
    }
    bool ret = writer_->WriteWithPayloadTimeout(
        data,
        size,
        payload,
        payloadSize,
        std::bind(&FfrtProfilerSocketClient::PeerIsConnected, this));
    if (!ret && block_) {
        DisableHook();
        return false;
    }
    if (++g_flushCount % flushInterval_ == 0) {
        writer_->Flush();
    }
    return true;
}

void FfrtProfilerSocketClient::Flush()
{
    if (writer_ == nullptr || unixSocketClient_ == nullptr) {
        return;
    }
    writer_->Flush();
}

void FfrtProfilerSocketClient::DisableHook()
{
    bool expected = false;
    if (g_disableHook.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed)) {
        PROFILER_LOG_INFO(LOG_CORE, "%s", __func__);
        if (disableHookCallback_ != nullptr) {
            disableHookCallback_();
        }
    }
}

bool FfrtProfilerSocketClient::PeerIsConnected()
{
    if (unixSocketClient_ == nullptr) {
        return false;
    }
    return !unixSocketClient_->IsConnected();
}
}