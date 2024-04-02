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
const static std::string PARAM_KAY = "hiviewdfx.hiprofiler.networkprofiler.target";
const static char NETWORK_PROFILER_UNIX_SOCKET_PATH[] = "network_profiler_unix_socket";
const static char NETWORK_PROFILER_UNIX_SOCKET_FULL_PATH[] = "/dev/unix/socket/network_profiler_unix_socket";
static constexpr uint8_t MAX_COMM_LEN = 16;

struct NetworkConfig {
    int32_t shmSize = 0;
    int32_t flushCount = 0;
    int32_t clock = 0;
    bool block = 0;
};

enum class EventType : int32_t {
    INVALID,
    TRAFFIC,
    HTTP,
};

struct alignas(8) NetworkResultBase { //8 is 8 bit
    int32_t type = static_cast<int32_t>(EventType::INVALID);
    int32_t tid = 0;
    struct timespec ts;
    uint8_t threadName[MAX_COMM_LEN] = {0};
};

struct alignas(8) NetworkData : public NetworkResultBase { //8 is 8 bit
    char* data;
};
}

#endif // NETWORK_PROFILER_COMMON_H