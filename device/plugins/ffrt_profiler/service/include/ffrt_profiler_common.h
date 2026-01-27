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

#ifndef FFRT_PROFILER_COMMON_H
#define FFRT_PROFILER_COMMON_H

#include <string.h>
#include <fstream>

namespace OHOS::Developtools::Profiler {
inline const std::string PARAM_KAY = "hiviewdfx.hiprofiler.ffrtprofiler.target";
static constexpr uint8_t MAX_COMM_LEN = 16;
static constexpr int32_t TRACE_DATA = -1;
static constexpr int32_t ALIGNAS_NUM = 8;

void SplitString(const std::string& str, const std::string &sep, std::vector<std::string>& ret);
std::string GetProcessName(int32_t pid);

struct FfrtConfig {
    int32_t shmSize = 0;
    int32_t flushCount = 0;
    int32_t clock = 0;
    bool block = 0;
};

enum class EventType : int32_t {
    MIN_TYPE = 0,
    INVALID = 0,
};

struct alignas(ALIGNAS_NUM) FfrtResultBase {
    int32_t type = static_cast<int32_t>(EventType::INVALID);
    int32_t tid = 0;
    struct timespec ts;
    char threadName[MAX_COMM_LEN] = {0};
};

struct alignas(ALIGNAS_NUM) FfrtTraceEvent : public FfrtResultBase {
    uint64_t cookie = 0;
    uint8_t traceType;
    uint8_t cpu = 0;
};
}

#endif // FFRT_PROFILER_COMMON_H