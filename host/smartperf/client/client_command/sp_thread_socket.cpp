/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <future>
#include <map>
#include <mutex>
#include <climits>
#include "parameters.h"
#include "include/sp_csv_util.h"
#include "include/sdk_data_recv.h"
#include "include/GpuCounter.h"
#include "include/lock_frequency.h"
#include "include/sp_thread_socket.h"
#include "include/sp_profiler_factory.h"
#include "include/sp_log.h"
#include "include/sp_task.h"
#include "include/profiler_fps.h"
#include "include/heartbeat.h"
#include "include/control_call_cmd.h"
#include "include/sp_profiler_factory.h"
#include "include/Network.h"
#include "include/startup_delay.h"
#include "include/Dubai.h"
#include "include/sp_server_socket.h"
#include "include/sp_utils.h"
#include "include/common.h"
#include "include/GetLog.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> SpThreadSocket::GetLogProcess(SpProfiler *profilerItem, std::string buffer) const
{
    if (buffer.find("::") != std::string::npos) {
        int port = std::stoi(buffer.substr(buffer.find("::") + 2));
        WLOGI("Get File log UDP message received, port is %d", port);
        // Init log file socket and file process
        GetLog::GetInstance().SetLogFileSocket(-1);
        GetLog::GetInstance().SetLogFileSocketPort(port);
        return profilerItem->ItemData();
    } else {
        WLOGE("Get File log UDP message received, but port is not found");
        GetLog::GetInstance().SetLogFileSocketPort(-1);
    }

    return std::map<std::string, std::string>();
}

void SpThreadSocket::HandleUDPMsg(SpServerSocket &spSocket, std::map<std::string, std::string> data,
    std::string retCode, std::unordered_map<MessageType, std::string>::const_iterator iterator) const
{
    std::cout << "iterator->first: " << static_cast<int>(iterator->first) << std::endl;
    if (iterator->first == MessageType::GET_CUR_FPS) {
        ProfilerFPS::isLowCurFps = true;
        std::string resultfps = "vfps||";
        for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
            if (iter->first != "fpsJitters") {
                std::string temp = iter->second + "@@";
                resultfps += std::string(temp.c_str());
            }
        }
        spSocket.Sendto(resultfps);
        LOGD("UDP send Cur_resultfps = %s", resultfps.c_str());
    } else if (iterator->first == MessageType::GET_CPU_FREQ_LOAD) {
        FetchCpuStats(spSocket, data);
    } else if (iterator->first == MessageType::GET_LOG) {
        if (GetLog::GetInstance().GetLogFileSocketPort() == -1) {
            return;
        }
        int logSocket = -1;
        int connectCount = 0;
        const int maxTryCount = 2;

        while (logSocket < 0) {
            WLOGI("Connect file log socket, try times: %d", connectCount + 1);
            if (connectCount > maxTryCount) {
                WLOGE("Connect file log socket failed");
                return;
            }
            connectCount++;
            logSocket = GetLog::GetInstance().LogFileSocketConnect();
        }

        int ret = GetLog::GetInstance().SendLogFile();
        if (ret < 0) {
            return;
        }
    } else {
        retCode = MapToString(data);
        spSocket.Sendto(retCode);
        LOGD("UDP send retCode = %s", retCode.c_str());
    }
}
}
}