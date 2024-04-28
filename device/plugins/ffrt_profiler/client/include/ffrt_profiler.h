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


#ifndef FFRT_PROFILER_H
#define FFRT_PROFILER_H

#include <atomic>
#include <string>
#include <mutex>
#include <thread>
#include "param/sys_param.h"
#include "ffrt_profiler_common.h"
#include "ffrt_profiler_socker_client.h"

namespace OHOS::Developtools::Profiler {
class EXPORT_API FfrtProfiler {
public:
    static FfrtProfiler* GetInstance();
    virtual ~FfrtProfiler();

    bool IsProfilerEnabled();
    void FfrtProfilerTrace(const uint8_t traceType, const std::string& lable = "", uint64_t cookie = 0);
    void FfrtProfiling(const EventType type, const char* payload, size_t payloadSize);

    void SetClockId(int32_t type)
    {
        clockType_ = type;
    }

    void SetEnableFlag(bool flag)
    {
        enable_ = flag;
    }

private:
    FfrtProfiler();
    void CheckFfrtProfilerParam();
    void Enable();
    void Disable();
    void FfrtProfilerSendData(const void* src, size_t size, const char* payload = nullptr, size_t payloadSize = 0);
    void GetThreadName(const void* src);

private:
    static FfrtProfiler* instance_;
    std::atomic_bool enable_{false};
    std::string processName_;
    int32_t pid_{0};
    std::shared_ptr<FfrtProfilerSocketClient> socketClent_{nullptr};
    int32_t clockType_{0};
};
} // OHOS::Developtools::Profiler
#define FfrtProfilerIns OHOS::Developtools::Profiler::FfrtProfiler::GetInstance()

#endif // FFRT_PROFILER_H