/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include "hook_service.h"

#include "hook_manager.h"
#include <cinttypes>
#include <unistd.h>

#include "common.h"
#include "logging.h"
#include "parameter.h"
#include "socket_context.h"

namespace OHOS::Developtools::NativeDaemon {
HookService::HookService(const ClientConfig& clientConfig, std::shared_ptr<HookManager> hook, bool multipleProcesses)
    : clientConfig_(clientConfig), hookMgr_(hook), multipleProcesses_(multipleProcesses)
{
    serviceName_ = "HookService";
    if (getuid() == 0) {
        StartService(DEFAULT_UNIX_SOCKET_HOOK_FULL_PATH);
    } else {
        StartService(DEFAULT_UNIX_SOCKET_HOOK_PATH);
    }
}

HookService::~HookService()
{
    serviceEntry_ = nullptr;
}

bool HookService::StartService(const std::string& unixSocketName)
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

bool HookService::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (size != sizeof(int)) {
        return false;
    }
    int peerConfig = *const_cast<int *>(reinterpret_cast<const int *>(buf));
    if (peerConfig == -1) {
        return false;
    }
    hookMgr_->SetPid(peerConfig);
    std::string filePath = "/proc/" + std::to_string(peerConfig) + "/cmdline";
    std::string bundleName;
    if (!LoadStringFromFile(filePath, bundleName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Get process name by pid failed!, pid: %d", peerConfig);
        return false;
    }
    hookMgr_->SetPid(peerConfig);
    bundleName.resize(strlen(bundleName.c_str()));

    if (bundleName.substr(0, 2) == "./") { // 2: size
        bundleName = bundleName.substr(2); // 2: point
    }
    size_t found = bundleName.rfind("/");
    std::string procName;
    if (found != std::string::npos) {
        procName = bundleName.substr(found + 1);
    } else {
        procName = bundleName;
    }

    std::lock_guard<std::mutex> lock(mtx_);
    if ((!firstProcess_) && (!multipleProcesses_)) {
        return false;
    }
    firstProcess_ = false;
    auto [eventFd, smbFd] = hookMgr_->GetFds(peerConfig, procName);
    if (eventFd == smbFd) {
        PROFILER_LOG_ERROR(LOG_CORE, "Get eventFd and smbFd failed!, name: %s, pid: %d", procName.c_str(), peerConfig);
        return false;
    }

    PROFILER_LOG_DEBUG(LOG_CORE, "ProtocolProc, receive message from hook client, and send hook config to process %d",
                       peerConfig);
    context.SendHookConfig(reinterpret_cast<uint8_t *>(&clientConfig_), sizeof(clientConfig_));
    context.SendFileDescriptor(smbFd);
    context.SendFileDescriptor(eventFd);
    hookMgr_->ResetStartupParam();
    return true;
}
}