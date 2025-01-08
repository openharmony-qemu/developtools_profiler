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
#include "profiler_fps.h"
#include "sp_log.h"
#include "common.h"
#include "heartbeat.h"
#include "Dubai.h"
#include "Network.h"
namespace OHOS {
namespace SmartPerf {
class SpThreadSocket {
public:
    static bool flagRunning;
    enum SocketConnectType {
        CMD_SOCKET,
        EDITOR_SOCKET,
    };

    enum SocketErrorType {
        OK,
        TOKEN_CHECK_FAILED,
        INIT_FAILED,
        START_FAILED,
        STOP_FAILED,
        START_RECORD_FAILED,
        STOP_RECORD_FAILED,
    };
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
        if (recvBuf.empty()) {
            LOGE("SplitMsg recvBuf is null");
            return recvBuf;
        }
        size_t pos = recvBuf.find("::");
        if (pos != std::string::npos) {
            std::vector<std::string> sps;
            SPUtils::StrSplit(recvBuf, "::", sps);
            if (sps.size() > 1) {
                return sps[1];
            } else {
                LOGE("SplitMsg sps size is zreo");
                return recvBuf;
            }
        } else {
            return recvBuf;
        }
    }

    void Process(ProtoType type) const
    {
        std::cout << "Socket Process called!" << std::endl;
        SpServerSocket spSocket;
        spSocket.Init(type);
        if (type == ProtoType::TCP) {
            std::cout << "Socket TCP Init called!" << std::endl;
            TypeTcp(spSocket);
        }
        if (type == ProtoType::UDP || type == ProtoType::UDPEX) {
            SocketHeartbeat();
            while (1) {
                spSocket.Recvfrom();
                HandleMsg(spSocket);
            }
        }
        std::cout << "Socket Process finished!" << std::endl;
        spSocket.Close();
    }
    SocketErrorType CheckToken(std::string recvStr, SpServerSocket &spSocket) const
    {
        if (recvStr.find_last_of(":") == std::string::npos) {
            if (recvStr.find("SP_daemon -editor") != std::string::npos) {
                return OK;
            } else {
                return TOKEN_CHECK_FAILED;
            }
        }
        std::string token = recvStr.substr(recvStr.find_last_of(":") + 1);
        token = token.substr(0, token.find(' '));
        std::string tcpToken = SPTask::GetInstance().GetTcpToken();
        if (tcpToken == "" && token == "-SESSIONID") {
            return OK;
        }
        if (token != tcpToken) {
            return TOKEN_CHECK_FAILED;
        }

        return OK;
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
                LOGD("TCP recv : %s", recvStr.c_str());
                // 解析消息 分发处理
                const SocketErrorType tokenStatus = CheckToken(recvStr, spSocket);
                DealMsg(recvStr, spSocket, tokenStatus);
            }
        }
    }
    // TCP
    void InitRecv(std::string recvStr, SpServerSocket &spSocket, SocketConnectType type) const
    {
        std::string errorInfo;
        std::string checkStr = recvStr.substr(std::string("init::").length());
        if (!SPTask::GetInstance().CheckTcpParam(checkStr, errorInfo) &&
            checkStr.find(SPTask::GetInstance().GetTcpToken()) == std::string::npos) {
            LOGE("init error(%s) recvStr(%s)", errorInfo.c_str(), recvStr.c_str());
            if (type == CMD_SOCKET) {
                spSocket.Send("init::False,\"error\":" + errorInfo);
            } else {
                spSocket.Send(std::string("init::") + std::to_string(INIT_FAILED));
            }
            return;
        }
        if (recvStr.find("-lockfreq") != std::string::npos &&
            SPTask::GetInstance().GetTcpToken() == "") {
            LOGE("lockfreq must have token");
            return;
        }
        ErrCode code = SPTask::GetInstance().InitTask(SplitMsg(recvStr));
        LOGD("init::%s", (code == ErrCode::OK) ? "True" : "False");
        if (type == CMD_SOCKET) {
            spSocket.Send(std::string("init::") + ((code == ErrCode::OK) ? "True" : "False"));
            return;
        }
        if (code == ErrCode::OK) {
            spSocket.Send("init::True");
        } else {
            spSocket.Send(std::string("init::") + std::to_string(INIT_FAILED));
        }
    }
    void StartRecv(SpServerSocket &spSocket) const
    {
        if (flagRunning) {
            spSocket.Send("SP_daemon is running");
            return;
        }
        auto lambdaTask = [](const std::string &data) {
            std::cout << data << std::endl;
        };
        ErrCode code = SPTask::GetInstance().StartTask(lambdaTask);
        SPTask::GetInstance().StartRecord();
        LOGD("start:::%s", (code == ErrCode::OK) ? "True" : "False");
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
        LOGD("start::%s", (code == ErrCode::OK) ? "True" : "False");
        if (code == ErrCode::OK) {
            spSocket.Send("start::True");
        } else if (code == ErrCode::FAILED) {
            spSocket.Send(std::string("start::") + std::to_string(START_FAILED));
        }
    }
    void StopRecvRealtime(SpServerSocket &spSocket) const
    {
        ErrCode code = SPTask::GetInstance().StopTask();
        if (code == ErrCode::OK) {
            spSocket.Send("stop::True");
            flagRunning = false;
            spSocket.Close();
        } else if (code == ErrCode::FAILED) {
            spSocket.Send(std::string("stop::") + std::to_string(STOP_FAILED));
        }
    }
    void StartRecvRecord(SpServerSocket &spSocket) const
    {
        LOGD("startRecord::True");
        ErrCode code = SPTask::GetInstance().StartRecord();
        if (code == ErrCode::OK) {
            spSocket.Send("startRecord::True");
        } else {
            spSocket.Send(std::string("startRecord::") + std::to_string(START_RECORD_FAILED));
        }
        spSocket.Send("startRecord::True");
    }
    void StopRecvRecord(SpServerSocket &spSocket) const
    {
        ErrCode code = SPTask::GetInstance().StopRecord();
        if (code == ErrCode::OK) {
            spSocket.Send("stopRecord::True");
        } else {
            spSocket.Send(std::string("stopRecord::") + std::to_string(STOP_RECORD_FAILED));
        }
    }
    void SendTokenFailedMessage(SpServerSocket &socket, std::string &message) const
    {
        if (message.find("init:::") != std::string::npos ||
            message.find("start:::") != std::string::npos) {
            return;
        }
        const std::vector<std::string> messageType = {
            "init::",
            "start::",
            "stop::",
            "startRecord::",
            "stopRecord::",
        };
        for (auto it : messageType) {
            if (message.find(it) != std::string::npos) {
                LOGD((it + std::to_string(TOKEN_CHECK_FAILED)).c_str());
                socket.Send(it + std::to_string(TOKEN_CHECK_FAILED));
                return;
            }
        }
    }
    void DealMsg(std::string recvStr, SpServerSocket &spSocket, SocketErrorType tokenStatus) const
    {
        SocketHeartbeat();
        if (tokenStatus == TOKEN_CHECK_FAILED) {
            SendTokenFailedMessage(spSocket, recvStr);
            return;
        }
        if (recvStr.find("init:::") != std::string::npos) {
            InitRecv(recvStr, spSocket, CMD_SOCKET);
        } else if (recvStr.find("start:::") != std::string::npos) {
            StartRecv(spSocket);
        } else if (recvStr.find("init::") != std::string::npos) {
            InitRecv(recvStr, spSocket, EDITOR_SOCKET);
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
            LOGD("UDP recv : %s", recvBuf.c_str());
            SpProfiler *profiler = SpProfilerFactory::GetProfilerItem(iterator->first);
            if (profiler == nullptr) {
                HandleNullMsg(spSocket, profiler, retCode, recvBuf, iterator);
            } else {
                std::map<std::string, std::string> data;
                if (iterator->first == MessageType::CATCH_NETWORK_TRAFFIC) {
                    Network::GetInstance().IsFindHap();
                    profiler->ItemData(); // record the collection point for the first time,no need to return
                    data["network_traffic"] = "true";
                } else if (iterator->first == MessageType::GET_NETWORK_TRAFFIC) {
                    Network::GetInstance().IsStopFindHap();
                    data = profiler->ItemData();
                    data["network_traffic"] = "true";
                } else {
                    data = profiler->ItemData();
                }
                HandleUDPMsg(spSocket, data, retCode, iterator);
            }
            LOGD("sendData key(%d) content(%s)", iterator->first, retCode.c_str());
            break;
        }
    }
    void HandleUDPMsg(SpServerSocket &spSocket, std::map<std::string, std::string> data, std::string retCode,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const
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
        } else {
            retCode = MapToString(data);
            spSocket.Sendto(retCode);
            LOGD("UDP send retCode = %s", retCode.c_str());
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
        LOGD("UDP send resultCpu = %s", resultCpu.c_str());
    }
    void HandleNullMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode, std::string recvBuf,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const
    {
        if (iterator->first == MessageType::SET_PKG_NAME) {
            if (recvBuf.find("smartperf") != std::string::npos) {
                retCode = SplitMsg(recvBuf);
                if (retCode.find("smartperf") != std::string::npos) {
                    Dubai::dubaiPkgName = retCode;
                    LOGD("UDP send dubaiPkgName: (%s)", Dubai::dubaiPkgName.c_str());
                }
            } else {
                retCode = SplitMsg(recvBuf);
                if (!SpProfilerFactory::editorFlag) {
                    std::string processId = "";
                    OHOS::SmartPerf::StartUpDelay sp;
                    processId = sp.GetPidByPkg(retCode);
                    SpProfilerFactory::SetProfilerPidByPkg(processId);
                    SpProfilerFactory::SetProfilerPkg(retCode);
                }
            }
            spSocket.Sendto(retCode);
            LOGD("UDP send PkgName = %s", retCode.c_str());
        } else if (profiler == nullptr && (iterator->first == MessageType::GET_APP_TYPE)) {
            retCode = SplitMsg(recvBuf);
            ProfilerFPS &pfps = ProfilerFPS::GetInstance();
            pfps.isGameApp = SPUtils::GetIsGameApp(retCode);
            pfps.firstDump = true;
        } else if (profiler == nullptr && (iterator->first == MessageType::SET_GAME_VIEW)) {
            retCode = SplitMsg(recvBuf);
            SpProfilerFactory::SetProfilerGameLayer(retCode);
        } else if (iterator->first == MessageType::CATCH_TRACE_CONFIG ||
            iterator->first == MessageType::CATCH_TRACE_CMD) {
            SpProfilerFactory::SetByTrace(SplitMsg(recvBuf));
        } else if (iterator->first == MessageType::GET_CPU_NUM) {
            retCode = SPUtils::GetCpuNum();
            spSocket.Sendto(retCode);
            LOGD("UDP send cpuNum = %s", retCode.c_str());
        } else if (iterator->first == MessageType::BACK_TO_DESKTOP) {
            BackDesktop();
        } else {
            HandleNullAddMsg(spSocket, profiler, retCode, recvBuf, iterator);
        }
    }
    void HandleNullAddMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode, std::string recvBuf,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const
    {
        if (iterator->first == MessageType::START_DUBAI_DB) {
            if (recvBuf.find("smartperf") != std::string::npos) {
                retCode = SplitMsg(recvBuf);
                if (retCode.find("smartperf") != std::string::npos) {
                    Dubai::dubaiPkgName = retCode;
                    LOGD("UDP send dubaiPkgName: (%s)", Dubai::dubaiPkgName.c_str());
                }
            }
            Dubai::CallBeginAndFinish();
        } else if (iterator->first == MessageType::SET_DUBAI_DB) {
            Dubai::CallBeginAndFinish();
            Dubai::isDumpDubaiFinish = true;
            ProfilerFPS::isLowCurFps = false;
            retCode = Dubai::CallMoveDubaiDbFinished();
            spSocket.Sendto(retCode);
            LOGD("UDP send DuBai get finish");
        } else if (iterator->first == MessageType::CHECK_UDP_STATUS) {
            retCode = "UDP status is normal";
            spSocket.Sendto(retCode);
            LOGD("UDP status is normal");
        } else {
            retCode = iterator->second;
            spSocket.Sendto(retCode);
            LOGD("UDP sendData: (%s)", retCode.c_str());
        }
    }
};
bool SpThreadSocket::flagRunning = false;
}
}
#endif