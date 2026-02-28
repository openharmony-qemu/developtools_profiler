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

#ifndef PRELOAD_SOCKET_CLIENT
#define PRELOAD_SOCKET_CLIENT

#include "hook_common.h"
#include "service_base.h"
#include <mutex>
#include <condition_variable>

class UnixSocketClient;

class PreloadSocketClient : public ServiceBase {
public:
    PreloadSocketClient(std::string appName);
    ~PreloadSocketClient();
    bool Connect(const std::string addrname);
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    bool GetConnectState();
    std::string GetDiagPath();
    bool Wait();

private:
    std::shared_ptr<UnixSocketClient> unixSocketClient_;
    std::string appName_;
    std::string path_;
    std::mutex recvMtx_;
    std::condition_variable recvCond_;
    std::atomic<bool> connectState_ = false;
};

#endif // PRELOAD_SOCKET_CLIENT
