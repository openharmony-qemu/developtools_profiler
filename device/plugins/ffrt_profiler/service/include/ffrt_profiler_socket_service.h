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

#ifndef FFRT_PROFILER_SERVICE_H
#define FFRT_PROFILER_SERVICE_H

#include <memory>
#include <string>
#include <vector>

#include "service_entry.h"
#include "ffrt_profiler_common.h"

namespace OHOS::Developtools::Profiler {
class FfrtProfilerManager;

class FfrtProfilerSocketService : public ServiceBase {
public:
    FfrtProfilerSocketService(std::shared_ptr<FfrtProfilerManager> ffrtMgr);
    ~FfrtProfilerSocketService();
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    void SetConfig(int32_t shmSize, int32_t flushCount, bool block, int32_t clockType);
private:
    bool StartService(const std::string& unixSocketName);

private:
    std::shared_ptr<ServiceEntry> serviceEntry_{nullptr};
    std::shared_ptr<FfrtProfilerManager> ffrtMgr_{nullptr};
    std::mutex mtx_;
    FfrtConfig config_;
};
} // namespace OHOS::Developtools::Profiler
#endif // FFRT_PROFILER_SERVICE_H