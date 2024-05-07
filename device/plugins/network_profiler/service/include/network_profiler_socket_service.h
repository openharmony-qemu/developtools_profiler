/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#ifndef NETWORK_PROFILER_SERVICE_H
#define NETWORK_PROFILER_SERVICE_H

#include <memory>
#include <string>
#include <vector>

#include "service_entry.h"
#include "network_profiler_common.h"

namespace OHOS::Developtools::Profiler {
class NetworkProfilerManager;

class NetworkProfilerSocketService : public ServiceBase {
public:
    NetworkProfilerSocketService(std::shared_ptr<NetworkProfilerManager> networkMgr);
    ~NetworkProfilerSocketService();
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    void SetConfig(int32_t shmSize, int32_t flushCount, bool block, int32_t clockType);
private:
    bool StartService(const std::string& unixSocketName);

private:
    std::shared_ptr<ServiceEntry> serviceEntry_{nullptr};
    std::shared_ptr<NetworkProfilerManager> networkMgr_{nullptr};
    std::mutex mtx_;
    NetworkConfig config_;
};
} // namespace OHOS::Developtools::Profiler
#endif // NETWORK_PROFILER_SERVICE_H