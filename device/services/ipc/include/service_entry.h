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

#ifndef SERVICE_ENTRY_H
#define SERVICE_ENTRY_H

#include <cstdint>
#include <map>
#include "service_base.h"

static const uint32_t PROTOCOL_TYPE_FILTER = 0xF0000000;

enum ProtocolType {
    PROTOCOL_TYPE_RAW = 0,
    PROTOCOL_TYPE_PROTOBUF = 0x10000000,
};

struct ProtocolHead {
    uint32_t protoType;
    uint32_t protoSize;
    int8_t datas[0];
};

class UnixSocketServer;

class ServiceEntry {
public:
    ServiceEntry();
    ~ServiceEntry();

    bool StartServer(const std::string& addrname);

    bool RegisterService(ServiceBase& psb);

    const ServiceBase* FindServiceByName(const std::string& service_name);

private:
    std::shared_ptr<UnixSocketServer> unixSocketServer_;

    std::map<std::string, ServiceBase*> serviceMap_;
};

uint64_t GetTimeMS();
uint64_t GetTimeUS();
uint64_t GetTimeNS();

#endif