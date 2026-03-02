/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include "preload_socket_client.h"
#include "common.h"
#include "unix_socket_client.h"
#include "share_memory_allocator.h"
#include "logging.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

const static char DEFAULT_UNIX_SOCKET_PRELOAD_FULL_PATH[] = "/dev/unix/socket/hook_unix_socket";

using namespace OHOS::Developtools::NativeDaemon;

PreloadSocketClient::PreloadSocketClient(std::string appName)
    : appName_(appName)
{
    unixSocketClient_ = nullptr;
    serviceName_ = "HookService";
    connectState_ = Connect(DEFAULT_UNIX_SOCKET_PRELOAD_FULL_PATH);
}

bool PreloadSocketClient::GetConnectState()
{
    return connectState_.load() && (unixSocketClient_->GetClientState() != CLIENT_STAT_THREAD_EXITED);
}

std::string PreloadSocketClient::GetDiagPath()
{
    return path_;
}

bool PreloadSocketClient::Wait()
{
    std::unique_lock<std::mutex> lock(recvMtx_);
    if (recvCond_.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout) {
        return false;
    }

    return true;
}

PreloadSocketClient::~PreloadSocketClient()
{
    unixSocketClient_ = nullptr;
}

bool PreloadSocketClient::Connect(const std::string addrname)
{
    if (unixSocketClient_ != nullptr) {
        return false;
    }
    unixSocketClient_ = std::make_shared<UnixSocketClient>();
    if (!unixSocketClient_->Connect(addrname, *this, nullptr)) {
        unixSocketClient_ = nullptr;
        return false;
    }

    std::string msg = PRELOAD_APP_IDENTIFY + appName_;

    if (!unixSocketClient_->SendHookConfig(reinterpret_cast<const uint8_t *>(msg.c_str()), msg.size())) {
        return false;
    }
    return true;
}

bool PreloadSocketClient::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (buf == nullptr || size == 0) {
        return false;
    }

    std::string path(reinterpret_cast<const char *>(buf), size);
    PROFILER_LOG_INFO(LOG_CORE, "PreloadSocketClient::ProtocolProc get arkts diag path %s", path.c_str());

    path_ = path;
    recvCond_.notify_all();

    return true;
}
