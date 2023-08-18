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

#include "client_connection.h"
#include <cstdint>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "logging.h"

ClientConnection::ClientConnection(int sfd, ServiceEntry& serviceEntry)
{
    serviceBase_ = nullptr;
    socketHandle_ = sfd;
    clientState_ = CLIENT_STAT_WORKING;
    lastProcMS_ = GetTimeMS();
    serviceEntry_ = &serviceEntry;
    HILOG_INFO(LOG_CORE, "new ClientConnection construct");
    CreateRecvThread();
}

ClientConnection::~ClientConnection() {}

int ClientConnection::RawProtocolProc(uint32_t pnum, const int8_t* buf, const uint32_t size)
{
    switch (pnum) {
        case RAW_PROTOCOL_POINTTO_SERVICE: {
            struct RawPointToService* prrs = (struct RawPointToService*)buf;
            protoServiceName_ = prrs->serviceName_;
            serviceBase_ = const_cast<ServiceBase*>(serviceEntry_->FindServiceByName(protoServiceName_));
            HILOG_INFO(LOG_CORE, "RAW_PROTOCOL_POINTTO_SERVICE %s", protoServiceName_.c_str());
        }
            return 1;
        default:
            break;
    }
    return -1;
}
