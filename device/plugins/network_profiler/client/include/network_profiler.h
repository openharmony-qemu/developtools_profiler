/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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


#ifndef NETWORK_PROFILER_H
#define NETWORK_PROFILER_H

#include <atomic>
#include <string>
#include <mutex>
#include <thread>
#include "param/sys_param.h"
#include "network_profiler_common.h"
#include "network_profiler_socker_client.h"

namespace OHOS::Developtools::Profiler {

class EXPORT_API NetworkProfiler {
public:
    struct CachedData {
        uint8_t cachedType;
        const char* cachedData;
        size_t cachedDataSize;
    };
    static void Disable();
    static void ServiceCloseCallback();
    static NetworkProfiler* GetInstance();
    virtual ~NetworkProfiler();

    bool IsProfilerEnable();
    void NetworkProfiling(const uint8_t type, const char* data, size_t dataSize);

    void SetClockId(int32_t type)
    {
        clockType_ = type;
    }
    void SetWaitingFlag(bool flag);
    void SetEnableFlag(bool flag);
    void SendCachedData();

private:
    NetworkProfiler();
    void CheckNetworkProfilerParam();
    void Enable();
    void NetworkProfilerSendData(const void* src, size_t size, const char* payload = nullptr, size_t payloadSize = 0);
    void GetThreadName(const char* src);

private:
    static NetworkProfiler* instance_;
    int32_t clockType_{0};
};
} // OHOS::Developtools::Profiler
#define NetworkProfilerIns OHOS::Developtools::Profiler::NetworkProfiler::GetInstance()

#endif // NETWORK_PROFILER_H