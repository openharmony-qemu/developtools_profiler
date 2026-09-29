/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "unix_socket_client.h"
#include <cstdint>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/un.h>
#include <fdsan.h>
#include "logging.h"
#include "securec.h"
#include "service_base.h"

UnixSocketClient::UnixSocketClient()
{
    serviceBase_ = nullptr;
    socketHandle_ = -1;
}

UnixSocketClient::~UnixSocketClient() {}

bool UnixSocketClient::Connect(const std::string addrname, ServiceBase& serviceBase, void (*callback)())
{
    PROFILER_LOG_INFO(LOG_CORE, "UnixSocketClient connect");
    CHECK_TRUE(socketHandle_ == -1, false, "socketHandle_ != -1 Already Connected");

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_TRUE(sock != -1, false, "Unix Socket Create FAIL");
    fdsan_exchange_owner_tag(sock, 0, PROFILER_SUBSYSTEM);

    struct sockaddr_un addr;
    if (memset_s(&addr, sizeof(struct sockaddr_un), 0, sizeof(struct sockaddr_un)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memset_s error!");
    }
    addr.sun_family = AF_UNIX;
    if (strncpy_s(addr.sun_path, sizeof(addr.sun_path), addrname.c_str(), sizeof(addr.sun_path) - 1) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "strncpy_s error!");
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        fdsan_close_with_tag(sock, PROFILER_SUBSYSTEM);
        PROFILER_LOG_ERROR(LOG_CORE, "Unix Socket Connect FAIL");
        return false;
    }

    serviceBase_ = &serviceBase;
    struct RawPointToService rrs;
    if (strncpy_s(rrs.serviceName_, sizeof(rrs.serviceName_),
        serviceBase_->serviceName_.c_str(), serviceBase_->serviceName_.size()) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "strncpy_s error!");
    }
    rrs.serviceName_[serviceBase_->serviceName_.size()] = 0;
    if (!SendRaw(
        RAW_PROTOCOL_POINTTO_SERVICE, reinterpret_cast<int8_t*>(&rrs), sizeof(struct RawPointToService), sock)) {
        fdsan_close_with_tag(sock, PROFILER_SUBSYSTEM);
        PROFILER_LOG_ERROR(LOG_CORE, "Unix Socket SendRaw FAIL");
        return false;
    }
    socketHandle_ = sock;
    if (!CreateRecvThread(callback)) {
        fdsan_close_with_tag(sock, PROFILER_SUBSYSTEM);
        socketHandle_ = -1;
        PROFILER_LOG_ERROR(LOG_CORE, "Unix Socket Create Recv Thread FAIL");
        return false;
    }
    return true;
}
