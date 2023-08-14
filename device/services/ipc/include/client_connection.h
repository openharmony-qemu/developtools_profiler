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

#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <cstdint>
#include "service_entry.h"
#include "socket_context.h"

class ClientConnection final : public SocketContext {
public:
    ClientConnection(int sfd, ServiceEntry& serviceEntry);
    ~ClientConnection();

    int RawProtocolProc(uint32_t pnum, const int8_t* buf, const uint32_t size) override;

    ServiceEntry* serviceEntry_;
    std::string protoServiceName_;
};

#endif