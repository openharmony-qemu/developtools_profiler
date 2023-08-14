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

#include "common.h"
#include "hook_common.h"
#include "unix_socket_client.h"
#include "logging.h"

namespace {
const int MOVE_BIT_8 = 8;
const int MOVE_BIT_32 = 32;
const int MOVE_BIT_48 = 48;
const int MOVE_BIT_56 = 56;
constexpr int FLUSH_FLAG = 20;
std::atomic<uint64_t> g_flushCount = 0;
} // namespace

HookSocketClient::HookSocketClient(int pid, ClientConfig *config) : pid_(pid), config_(config)
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
        HILOG_INFO(LOG_CORE, "~HookSocketClient Flush()");
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
    HILOG_INFO(LOG_CORE, "recv hook client config:%s\n", configStr.c_str());

    smbFd_ = context.ReceiveFileDiscriptor();
    eventFd_ = context.ReceiveFileDiscriptor();
    stackWriter_ = std::make_shared<StackWriter>("hooknativesmb", config_->shareMemroySize,
        smbFd_, eventFd_, config_->isBlocked);

    COMMON::PrintMallinfoLog("stackWriter init(byte) => ");
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
    if (stackWriter_ == nullptr || unixSocketClient_ == nullptr) {
        return false;
    }

    stackWriter_->WriteWithPayloadTimeout(data, size, payload, payloadSize);
    g_flushCount++;
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
