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

#include <thread>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include "include/heartbeat.h"
#include "include/sp_utils.h"
#include "sp_log.h"

namespace OHOS {
namespace SmartPerf {
void Heartbeat::HeartbeatRule()
{
    int checkTimeJitter = 300;
    while (isrunning){
        auto end = std::chrono::steady_clock::now();
        LOGI("endTime: %lld", std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch().count());
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - updateStart).count();
        LOGI("ListeningTimeJitter: %lld",duration);
        if (duration > timeout){
            LOGI("editor connect timeout have Disconnected successfully!");
            exit(0);  
        }
        sleep(checkTimeJitter); 
    } 
}

void Heartbeat::UpdatestartTime()
{
    updateStart = std::chrono::steady_clock::now();
    LOGI("Listening to messages from edtior or device");
}
}
}