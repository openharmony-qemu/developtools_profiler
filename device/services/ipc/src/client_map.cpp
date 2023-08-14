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

#include "client_map.h"
#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "logging.h"

ClientMap& ClientMap::GetInstance()
{
    static ClientMap instance;
    return instance;
}

ClientMap::ClientMap() {}

ClientMap::~ClientMap() {}

int ClientMap::PutClientSocket(int socketFileDescriptor, ServiceEntry& p)
{
    if (socketClients_.find(socketFileDescriptor) == socketClients_.end()) {
        socketClients_[socketFileDescriptor] = std::make_shared<ClientConnection>(socketFileDescriptor, p);
        return 1;
    }
    return -1;
}

int ClientMap::ClearClientSocket()
{
    socketClients_.clear();
    return 0;
}

int ClientMap::AutoRelease()
{
    for (auto iter = socketClients_.begin(); iter != socketClients_.end(); ++iter) {
        auto p = iter->second;
        switch (p->GetClientState()) {
            case CLIENT_STAT_WORKING:
                break;
            case CLIENT_STAT_WAIT_THREAD_EXIT:
                break;
            case CLIENT_STAT_THREAD_EXITED:
                HILOG_INFO(LOG_CORE, "AutoRelease release %d", iter->first);
                socketClients_.erase(iter->first);
                return 1;
                break;
            default:
                break;
        }
    }
    return 1;
}
