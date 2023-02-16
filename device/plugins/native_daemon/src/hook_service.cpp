/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "logging.h"
#include "parameter.h"
#include "socket_context.h"

namespace {
const int BUF_MAX_LEN = 10;
}  // namespace

HookService::HookService(int smbFd,
                         int eventFd,
                         int pid,
                         std::string processName,
                         uint64_t config)
    : smbFd_(smbFd), eventFd_(eventFd), hookConfig_(config), pid_(pid), processName_(processName)
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
    if (size != sizeof(uint64_t)) {
        HILOG_ERROR(LOG_CORE, "ProtocolProc hook config error");
    }
    uint64_t peerConfig = *const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(buf));
    if (peerConfig == -1u) {
        return true;
    }
    if (pid_ == 0) {
        printf("Process %" PRIu64 " hook started.\n", peerConfig);
        pid_ = static_cast<int>(peerConfig);
    } else if (peerConfig != (uint64_t)pid_) {
        HILOG_ERROR(LOG_CORE, "ProtocolProc receive peerConfig:%" PRIu64 " not expected", peerConfig);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "ProtocolProc, receive message from hook client, and send hook config to process %d", pid_);
    context.SendHookConfig(hookConfig_);
    context.SendFileDescriptor(smbFd_);
    context.SendFileDescriptor(eventFd_);
    return true;
}
