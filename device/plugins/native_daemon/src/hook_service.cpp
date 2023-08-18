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

#include <cinttypes>
#include <unistd.h>

#include "common.h"
#include "logging.h"
#include "parameter.h"
#include "socket_context.h"

namespace OHOS::Developtools::NativeDaemon {
HookService::HookService(int smbFd,
                         int eventFd,
                         int pid,
                         const std::string& processName,
                         const ClientConfig& clientConfig)
    : smbFd_(smbFd), eventFd_(eventFd), pid_(pid), processName_(processName), clientConfig_(clientConfig)
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
        HILOG_DEBUG(LOG_CORE, "Start IPC Service FAIL");
        return false;
    }
    serviceEntry_->RegisterService(*this);
    return true;
}

bool HookService::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (size != sizeof(int)) {
        HILOG_ERROR(LOG_CORE, "ProtocolProc hook config error, pid_ = %d, size = %d", pid_, size);
    }
    int peerConfig = *const_cast<int *>(reinterpret_cast<const int *>(buf));
    if (peerConfig == -1) {
        return true;
    }
    if (pid_ == 0) {
        printf("Process %d hook started.\n", peerConfig);
        pid_ = peerConfig;
    } else if (peerConfig != pid_) {
        HILOG_ERROR(LOG_CORE, "ProtocolProc receive peerConfig:%d not expected", peerConfig);
        return false;
    }
    CHECK_TRUE(getuid() == 0 || COMMON::CheckApplicationPermission(pid_, ""), false,
               "Application debug permisson denied!");
    HILOG_DEBUG(LOG_CORE, "ProtocolProc, receive message from hook client, and send hook config to process %d", pid_);
    context.SendHookConfig(reinterpret_cast<uint8_t *>(&clientConfig_), sizeof(clientConfig_));
    context.SendFileDescriptor(smbFd_);
    context.SendFileDescriptor(eventFd_);
    return true;
}
}