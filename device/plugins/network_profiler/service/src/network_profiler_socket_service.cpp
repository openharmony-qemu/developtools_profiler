/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "network_profiler_socket_service.h"

#include <cinttypes>
#include <unistd.h>

#include "socket_context.h"
#include "logging.h"
#include "network_profiler_manager.h"

namespace OHOS::Developtools::Profiler {
NetworkProfilerSocketService::NetworkProfilerSocketService(std::shared_ptr<NetworkProfilerManager> networkMgr)
    : networkMgr_(networkMgr)
{
    serviceName_ = "NetworkProfilerService";
    StartService(NETWORK_PROFILER_UNIX_SOCKET_PATH);
}

NetworkProfilerSocketService::~NetworkProfilerSocketService()
{
    serviceEntry_ = nullptr;
}

bool NetworkProfilerSocketService::StartService(const std::string& unixSocketName)
{
    serviceEntry_ = std::make_shared<ServiceEntry>();
    if (!serviceEntry_->StartServer(unixSocketName)) {
        serviceEntry_ = nullptr;
        PROFILER_LOG_DEBUG(LOG_CORE, "Start IPC Service FAIL");
        return false;
    }
    serviceEntry_->RegisterService(*this);
    return true;
}

bool NetworkProfilerSocketService::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf,
    const uint32_t size)
{
    if (size != sizeof(int)) {
        return false;
    }
    int pid = -1;
    if (memcpy_s(&pid, sizeof(int), buf, size) != EOK) {
        return false;
    }
    if (pid <= 0) {
        return false;
    }
    std::string bundleName = GetProcessNameByPid(pid);
    std::lock_guard<std::mutex> lock(mtx_);
    auto [eventFd, smbFd] = networkMgr_->GetNetworkProfilerCtx(pid, bundleName);
    if (eventFd == 0 && smbFd == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "Wait for process %s to restart", bundleName.c_str());
        return true;
    }
    if (eventFd == smbFd) {
        PROFILER_LOG_ERROR(LOG_CORE, "Get eventFd and smbFd failed!, name: %s, pid: %d", bundleName.c_str(), pid);
        return false;
    }

    PROFILER_LOG_INFO(LOG_CORE, "ProtocolProc, send eventfd: %d, smbFd: %d, peerPid: %d", eventFd, smbFd, pid);
    context.SendHookConfig(reinterpret_cast<const uint8_t*>(&config_), sizeof(config_));
    context.SendFileDescriptor(smbFd);
    context.SendFileDescriptor(eventFd);
    return true;
}

void NetworkProfilerSocketService::SetConfig(int32_t shmSize, int32_t flushCount, bool block, int32_t clockType)
{
    config_.shmSize = shmSize;
    config_.flushCount = flushCount;
    config_.block = block;
    config_.clock = clockType;
}
} // namespace OHOS::Developtools::Profiler