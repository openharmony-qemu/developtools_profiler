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
#ifndef SP_THREAD_SOCKET_H
#define SP_THREAD_SOCKET_H
#include <functional>
#include "sp_profiler_factory.h"
#include "sp_server_socket.h"
#include "sp_utils.h"
#include "sp_task.h"
#include "control_call_cmd.h"
#include "startup_delay.h"
#include "sp_log.h"
#include "common.h"
#include "heartbeat.h"
namespace OHOS {
namespace SmartPerf {
class SpThreadSocket {
public:
    static bool flagRunning;
    static std::string resultFPS;
    std::string MapToString(std::map<std::string, std::string> dataMap) const
    {
        std::string result;
        int i = 0;
        std::string splitStr = "";
        for (auto iter = dataMap.cbegin(); iter != dataMap.cend(); ++iter) {
            printf("%s = %s\n", iter->first.c_str(), iter->second.c_str());
            if (i > 0) {
                splitStr = "$$";
            }
            result += splitStr + iter->first.c_str() + "||" + iter->second.c_str();
            i++;
        }
        return result;
    }
    std::string SplitMsg(const std::string &recvBuf) const
    {
        std::vector<std::string> sps;
        SPUtils::StrSplit(recvBuf, "::", sps);
        return sps[1];
    }

    void Process(ProtoType type) const
    {
        std::cout << "Socket Process called!" << std::endl;
        LOGI("Socket Process called!");
        SpServerSocket spSocket;
        spSocket.Init(type);
        if (type == ProtoType::TCP) {
            std::cout << "Socket TCP Init called!" << std::endl;
            LOGI("Socket TCP Init called!");
            TypeTcp(spSocket);
        }
        if (type == ProtoType::UDP || type == ProtoType::UDPEX) {
            LOGI("Socket UDP Init called! type(%d)", static_cast<int>(type));
            SocketHeartbeat();
            while (1) {
                spSocket.Recvfrom();
                HandleMsg(spSocket);
            }
        }
        std::cout << "Socket Process finished!" << std::endl;
        LOGI("Socket Process finished!");
        spSocket.Close();
    }
    void TypeTcp(SpServerSocket &spSocket) const
    {
        SocketHeartbeat();
        while (1) {
            int procFd = spSocket.Accept();
            std::cout << "Socket TCP procFd: " << procFd << std::endl;
            while (procFd > 0) {
                int reFd = spSocket.Recv();
                if (reFd < 0) {
                    break;
                }
                std::string recvStr = spSocket.RecvBuf();
                std::cout << "Socket TCP Recv: " << recvStr << std::endl;
                // 解析消息 分发处理
                DealMsg(recvStr, spSocket);
            }
        }
    }
    // TCP
    void InitRecv(std::string recvStr, SpServerSocket &spSocket) const
    {
        std::string errorInfo;
        std::string checkStr = recvStr.substr(std:string("init::").length());
        if (!SPTask::GetInstance().CheckTcpParam(checkStr, errorInfo)) {
            LOGE("init error(%s) recvStr(%s)", errorInfo.c_str(), recvStr.c_str());
            spSocket.Send("init::False,\"error\":" + errorInfo);
            return;
        }
        ErrCode code = SPTask::GetInstance().InitTask(SplitMsg(recvStr));
        LOGI("init::%s", (code == ErrCode::OK) ? "True" : "False");
        spSocket.Send(std::string("init::") + ((code == ErrCode::OK) ? "True" : "False"));
    }
    void StartRecv(SpServerSocket &spSocket) const
    {
        if (flagRunning) {
            LOGI("SP_daemon is running");
            spSocket.Send("SP_daemon is running");
            return;
        }
        auto lambdaTask = [](const std::string &data) {
            std::cout << data << std::endl;
        };
        ErrCode code = SPTask::GetInstance().StartTask(lambdaTask);
        LOGI("start:::%s", (code == ErrCode::OK) ? "True" : "False");
        if (code == ErrCode::OK) {
            spSocket.Send("start::True");
            flagRunning = true;
        } else if (code == ErrCode::FAILED) {
            spSocket.Send("start::False");
        }
    }
    void StartRecvRealtime(SpServerSocket &spSocket) const
    {
        auto lambdaTask = [&spSocket](const std::string &data) { spSocket.Send(data); };
        ErrCode code = SPTask::GetInstance().StartTask(lambdaTask);
        LOGI("start::%s", (code == ErrCode::OK) ? "True" : "False");
        if (code == ErrCode::OK) {
            spSocket.Send("start::True");
        } else if (code == ErrCode::FAILED) {
            spSocket.Send("start::False");
        }
    }
    void StopRecvRealtime(SpServerSocket &spSocket) const
    {
        SPTask::GetInstance().StopTask();
        LOGI("stop::True");
        spSocket.Send("stop::True");
        flagRunning = false;
        spSocket.Close();
    }
    void StartRecvRecord(SpServerSocket &spSocket) const
    {
        LOGI("startRecord::True");
        SPTask::GetInstance().StartRecord();

        spSocket.Send("startRecord::True");
    }
    void StopRecvRecord(SpServerSocket &spSocket) const
    {
        LOGI("stopRecord::True");
        SPTask::GetInstance().StopRecord();

        spSocket.Send("stopRecord::True");
    }
    void DealMsg(std::string recvStr, SpServerSocket &spSocket) const
    {
        SocketHeartbeat();
        if (recvStr.find("init::") != std::string::npos) {
            InitRecv(recvStr, spSocket);
        } else if (recvStr.find("start:::") != std::string::npos) {
            StartRecv(spSocket);
        } else if (recvStr.find("start::") != std::string::npos) {
            StartRecvRealtime(spSocket);
        } else if (recvStr.find("stop::") != std::string::npos) {
            StopRecvRealtime(spSocket);
        } else if (recvStr.find("startRecord::") != std::string::npos) {
            StartRecvRecord(spSocket);
        } else if (recvStr.find("stopRecord::") != std::string::npos) {
            StopRecvRecord(spSocket);
        } else if (recvStr.find("SP_daemon -editor") != std::string::npos) {
            EditorRecv(recvStr, spSocket);
        }
    }
    void EditorRecv(std::string recvStr, const SpServerSocket &spSocket) const
    {
        std::vector<std::string> vec;
        size_t size = recvStr.size();
        size_t j = 0;
        for (size_t i = 0; i < size; i++) {
            if (recvStr[i] == ' ') {
                vec.push_back(recvStr.substr(j, i - j));
                j = i + 1;
            }
        }
        vec.push_back(recvStr.substr(j, size - j));
        const int type = 2;
        if (vec[type] == "findAppPage") {
            BackDesktop();
        }
        OHOS::SmartPerf::ControlCallCmd controlCallCmd;
        std::string result = controlCallCmd.GetResult(vec);
        spSocket.Send(result);
    }

    void BackDesktop() const
    {
        std::string cmdResult;
        std::string uinput = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_BACK);
        SPUtils::LoadCmd(uinput, cmdResult);
    }

    // UDP
    void HandleMsg(SpServerSocket &spSocket) const
    {
        std::string retCode = "";
        auto iterator = MESSAGE_MAP.begin();
        while (iterator != MESSAGE_MAP.end()) {
            std::string recvBuf = spSocket.RecvBuf();
            if (recvBuf.size() != 0) {
                Heartbeat &heartbeat = Heartbeat::GetInstance();
                heartbeat.UpdatestartTime();
            }
            if (!SPUtils::IsSubString(recvBuf, iterator->second)) {
                ++iterator;
                continue;
            }
            SpProfiler *profiler = SpProfilerFactory::GetProfilerItem(iterator->first);
            if (profiler == nullptr) {
                HandleNullMsg(spSocket, profiler, retCode, recvBuf, iterator);
            } else {
                std::map<std::string, std::string> data;
                if (iterator->first == MessageType::CATCH_NETWORK_TRAFFIC) {
                    profiler->ItemData(); // record the collection point for the first time,no need to return
                    data["network_traffic"] = "true";
                } else if (iterator->first == MessageType::GET_NETWORK_TRAFFIC) {
                    data = profiler->ItemData();
                    data["network_traffic"] = "true";
                } else {
                    data = profiler->ItemData();
                }
                HandleUDPMsg(spSocket, data, retCode, iterator);
            }
            LOGI("sendData key(%d) content(%s)", iterator->first, retCode.c_str());
            break;
        }
    }
    void HandleUDPMsg(SpServerSocket &spSocket, std::map<std::string, std::string> data, std::string retCode,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const
    {
        std::cout << "iterator->first: " << static_cast<int>(iterator->first) << std::endl;
        if (iterator->first == MessageType::GET_LOW_POWER_FPS) {
            for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
                if (iter->first != "fpsJitters") {
                    std::string temp = iter->second + "@@";
                    resultFPS += std::string(temp.c_str());
                }
            }
        } else if (iterator->first == MessageType::GET_CUR_FPS) {
            std::string resultfps = "vfps||";
            for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
                if (iter->first != "fpsJitters") {
                    std::string temp = iter->second + "@@";
                    resultFPS += std::string(temp.c_str());
                    resultfps += std::string(temp.c_str());
                }
            }
            spSocket.Sendto(resultfps);
        } else if (iterator->first == MessageType::GET_CPU_FREQ_LOAD) {
            FetchCpuStats(spSocket, data);
        } else {
            retCode = MapToString(data);
            spSocket.Sendto(retCode);
        }
    }
    void SocketHeartbeat() const
    {
        Heartbeat &heartbeat = Heartbeat::GetInstance();
        heartbeat.UpdatestartTime();
    }
    void FetchCpuStats(SpServerSocket &spSocket, std::map<std::string, std::string> data) const
    {
        std::string resultCpuFrequency = "";
        std::string resultCpuUsage = "";
        std::string resultCpu = "";
        int cpuFrequencyNum = 0;
        int cpuUsageNum = 0;
        int cpuFlag = 1;
        while (cpuFlag) {
            resultCpuFrequency = "cpu" + std::to_string(cpuFrequencyNum) + "Frequency";
            resultCpuUsage = "cpu" + std::to_string(cpuUsageNum) + "Usage";
            auto iterCpuFrequency = data.find(resultCpuFrequency);
            auto iterCpuUsage = data.find(resultCpuUsage);
            if (iterCpuFrequency != data.end()) {
                resultCpuFrequency += "||" + iterCpuFrequency->second;
                resultCpu += "$$" + resultCpuFrequency;
                cpuFrequencyNum++;
            } else {
                cpuFlag = 0;
            }
            if (iterCpuUsage != data.end()) {
                resultCpuUsage += "||" + iterCpuUsage->second;
                resultCpu += "$$" + resultCpuUsage;
                cpuUsageNum++;
            } else {
                cpuFlag = 0;
            }
        }
        spSocket.Sendto(resultCpu);
    }
    void HandleNullMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode, std::string recvBuf,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const
    {
        if (iterator->first == MessageType::SET_PKG_NAME) {
            retCode = SplitMsg(recvBuf);
            std::string processId = "";
            OHOS::SmartPerf::StartUpDelay sp;
            processId = sp.GetPidByPkg(retCode);
            SpProfilerFactory::SetProfilerPidByPkg(processId);
            SpProfilerFactory::SetProfilerPkg(retCode);
            spSocket.Sendto(retCode);
        } else if (iterator->first == MessageType::FPS_STOP) {
            spSocket.Sendto(resultFPS);
            resultFPS = "FPS||";
        } else if (profiler == nullptr && (iterator->first == MessageType::SET_GAME_VIEW)) {
            retCode = SplitMsg(recvBuf);
            SpProfilerFactory::SetProfilerGameLayer(retCode);
        } else if (iterator->first == MessageType::CATCH_TRACE_CONFIG ||
            iterator->first == MessageType::CATCH_TRACE_CMD) {
            SpProfilerFactory::SetByTrace(SplitMsg(recvBuf));
        } else if (iterator->first == MessageType::GET_CPU_NUM) {
            retCode = SPUtils::GetCpuNum();
            spSocket.Sendto(retCode);
        } else if (iterator->first == MessageType::BACK_TO_DESKTOP) {
            BackDesktop();
        } else {
            retCode = iterator->second;
            spSocket.Sendto(retCode);
        }
    }
};
bool SpThreadSocket::flagRunning = false;
std::string SpThreadSocket::resultFPS = "FPS||";
}
}
#endif