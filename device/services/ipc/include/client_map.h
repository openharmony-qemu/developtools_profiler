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

#ifndef CLIENT_MAP_H
#define CLIENT_MAP_H

#include <map>
#include "client_connection.h"

class ServiceEntry;

class ClientMap {
public:
    static ClientMap& GetInstance();
    int PutClientSocket(int socketFileDescriptor, ServiceEntry& p);
    int AutoRelease();
    int ClearClientSocket();

private:
    ClientMap();
    ~ClientMap();

    std::map<int, std::shared_ptr<ClientConnection>> socketClients_;
};

#endif