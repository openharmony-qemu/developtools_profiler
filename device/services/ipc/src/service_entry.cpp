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

#include "service_entry.h"
#include "logging.h"
#include "service_base.h"
#include "unix_socket_server.h"

ServiceEntry::ServiceEntry()
{
    unixSocketServer_ = nullptr;
}
ServiceEntry::~ServiceEntry() {}
bool ServiceEntry::StartServer(const std::string& addrname)
{
    CHECK_TRUE(unixSocketServer_ == nullptr, false, "Servier Already Started");

    auto server = std::make_shared<UnixSocketServer>();

    CHECK_TRUE(server->StartServer(addrname, *this), false, "StartServer FAIL");

    unixSocketServer_ = server;
    return true;
}

bool ServiceEntry::RegisterService(ServiceBase& psb)
{
    CHECK_TRUE(serviceMap_.find(psb.serviceName_) == serviceMap_.end(), false, "RegisterService FAIL");
    serviceMap_[psb.serviceName_] = &psb;
    return true;
}
const ServiceBase* ServiceEntry::FindServiceByName(const std::string& service_name)
{
    CHECK_TRUE(serviceMap_.find(service_name) != serviceMap_.end(), nullptr, "FindServiceByName FAIL %s",
               service_name.c_str());

    return serviceMap_[service_name];
}

namespace {
const int MS_PER_S = 1000;
const int US_PER_S = 1000000;
const int NS_PER_US = 1000;
const int NS_PER_S = 1000000000;
const int NS_PER_MS = 1000000;
} // namespace

uint64_t GetTimeMS()
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * MS_PER_S + ts.tv_nsec / NS_PER_MS;
}

uint64_t GetTimeUS()
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * US_PER_S + ts.tv_nsec / NS_PER_US;
}

uint64_t GetTimeNS()
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * NS_PER_S + ts.tv_nsec;
}