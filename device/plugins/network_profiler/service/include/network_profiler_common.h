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

#ifndef NETWORK_PROFILER_COMMON_H
#define NETWORK_PROFILER_COMMON_H

#include <string.h>
#include <fstream>
#include <vector>

namespace OHOS::Developtools::Profiler {
static constexpr uint8_t MAX_TNAME_LEN = 16;
static constexpr int32_t ALIGNAS_LEN = 8;

void SplitParamValue(const std::string& str, const std::string &sep, std::vector<std::string>& ret);
std::string GetProcessNameByPid(int32_t pid);

struct NetworkConfig {
    int32_t shmSize = 0;
    int32_t flushCount = 0;
    int32_t clock = 0;
    bool block = 0;
};

enum class NetworkEventType : int32_t {
    INVALID = -1,
    TRAFFIC,
    HTTP,
};

struct alignas(ALIGNAS_LEN) NetworkEvent { //8 is 8 bit
    int32_t type = static_cast<int32_t>(NetworkEventType::INVALID);
    int32_t tid = 0;
    struct timespec ts;
    char threadName[MAX_TNAME_LEN] = {0};
};
}

#endif // NETWORK_PROFILER_COMMON_H