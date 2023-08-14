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

#ifndef UNIX_SOCKET_SERVER_H
#define UNIX_SOCKET_SERVER_H

#include <ostream>
#include <thread>

#include "socket_context.h"

#include "client_connection.h"

class ServiceEntry;

class UnixSocketServer {
public:
    UnixSocketServer();
    ~UnixSocketServer();

    int socketHandle_;
    bool StartServer(const std::string& addrname, ServiceEntry& p);

    std::string sAddrName_;
    // private:
    ServiceEntry* serviceEntry_;

private:
    void UnixSocketAccept();
    std::map<int, std::shared_ptr<ClientConnection>> socketClients_;
    std::thread acceptThread_;
};

#endif