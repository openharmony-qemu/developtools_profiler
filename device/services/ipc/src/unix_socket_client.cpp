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
#include "logging.h"
#include "securec.h"
#include "service_base.h"

UnixSocketClient::UnixSocketClient()
{
    serviceBase_ = nullptr;
    socketHandle_ = -1;
}

UnixSocketClient::~UnixSocketClient() {}

bool UnixSocketClient::Connect(const std::string addrname, ServiceBase& serviceBase)
{
    HILOG_ERROR(LOG_CORE, "UnixSocketClient connect");
    CHECK_TRUE(socketHandle_ == -1, false, "socketHandle_ != -1 Already Connected");

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_TRUE(sock != -1, false, "Unix Socket Create FAIL");

    struct sockaddr_un addr;
    if (memset_s(&addr, sizeof(struct sockaddr_un), 0, sizeof(struct sockaddr_un)) != EOK) {
        HILOG_ERROR(LOG_CORE, "memset_s error!");
    }
    addr.sun_family = AF_UNIX;
    if (strncpy_s(addr.sun_path, sizeof(addr.sun_path), addrname.c_str(), sizeof(addr.sun_path) - 1) != EOK) {
        HILOG_ERROR(LOG_CORE, "strncpy_s error!");
    }

    CHECK_TRUE(connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) != -1, close(sock) != 0,
        "Unix Socket Connect FAIL");

    serviceBase_ = &serviceBase;
    struct RawPointToService rrs;
    if (strncpy_s(rrs.serviceName_, sizeof(rrs.serviceName_),
        serviceBase_->serviceName_.c_str(), serviceBase_->serviceName_.size()) != EOK) {
        HILOG_ERROR(LOG_CORE, "strncpy_s error!");
    }
    rrs.serviceName_[serviceBase_->serviceName_.size()] = 0;
    CHECK_TRUE(
        SendRaw(RAW_PROTOCOL_POINTTO_SERVICE, reinterpret_cast<int8_t*>(&rrs), sizeof(struct RawPointToService), sock),
        close(sock) != 0, "Unix Socket SendRaw FAIL");
    socketHandle_ = sock;
    CHECK_TRUE(CreateRecvThread(), close(sock) != 0, "Unix Socket Create Recv Thread FAIL");

    return true;
}
