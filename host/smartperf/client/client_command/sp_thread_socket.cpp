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
#include "include/GetLog.h"
#include "include/RAM.h"
namespace OHOS {
namespace SmartPerf {
std::string g_pkgName = "";
bool g_preset = false;
std::string g_pkgAndPid = "";
std::string SpThreadSocket::MapToString(std::map<std::string, std::string> dataMap) const
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
std::string SpThreadSocket::SplitMsg(const std::string &recvBuf) const
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

void SpThreadSocket::Process(ProtoType type)
{
    std::cout << "Socket Process called!" << std::endl;
    SpServerSocket spSocket;
    spSocket.Init(type);
    if (type == ProtoType::TCP) {
        std::cout << "Socket TCP Init called!" << std::endl;
        WLOGI("Socket TCP Init called!");
        TypeTcp(spSocket);
    }
    if (type == ProtoType::UDP || type == ProtoType::UDPEX) {
        SocketHeartbeat();
        while (socketConnect == true) {
            spSocket.Recvfrom();
            HandleMsg(spSocket);
        }
    }
    std::cout << "Socket Process finished!" << std::endl;
    spSocket.Close();
}
SocketErrorType SpThreadSocket::CheckToken(std::string recvStr,
    SpServerSocket &spSocket, std::string recvStrNoToken) const
{
    if (recvStr.find_last_of(":") == std::string::npos) {
        if (recvStr.find("SP_daemon -editor") != std::string::npos) {
            LOGI("Received string contains 'SP_daemon -editor', token check passed.");
            return SocketErrorType::OK;
        } else {
            LOGE("Token check failed: %s", recvStrNoToken.c_str());
            return SocketErrorType::TOKEN_CHECK_FAILED;
        }
    }
    std::string token = recvStr.substr(recvStr.find_last_of(":") + 1);
    token = token.substr(0, token.find(' '));
    std::string tcpToken = SPTask::GetInstance().GetTcpToken();
    LOGD("Comparing token with TCP token...");
    if (tcpToken == "" && token == "-SESSIONID") {
        LOGI("Token is empty but received token is '-SESSIONID', token check passed.");
        return SocketErrorType::OK;
    }
    if (token != tcpToken) {
        LOGE("Token mismatch.");
        return SocketErrorType::TOKEN_CHECK_FAILED;
    }
    LOGD("Token match");
    return SocketErrorType::OK;
}
void SpThreadSocket::TypeTcp(SpServerSocket &spSocket)
{
    SocketHeartbeat();
    WLOGI("Socket TCP Init Finished, Wait Client Socket Connect...");
    while (socketConnect == true) {
        int procFd = spSocket.Accept();
        std::cout << "Socket TCP procFd: " << procFd << std::endl;
        while (procFd > 0) {
            int reFd = spSocket.Recv();
            if (reFd < 0) {
                WLOGE("Error receiving data, reFd: %d", reFd);
                break;
            }
            std::string recvStr = spSocket.RecvBuf();
            std::string recvStrNoToken = recvStr.substr(0, recvStr.find("::"));
            LOGD("TCP recv data:%s", recvStr.c_str());
            WLOGD("Received data: %s", recvStrNoToken.c_str());
            // 解析消息 分发处理
            const SocketErrorType tokenStatus = CheckToken(recvStr, spSocket, recvStrNoToken);
            WLOGD("Token check status: %d", tokenStatus);
            DealMsg(recvStr, spSocket, tokenStatus);
        }
    }
}
// TCP
void SpThreadSocket::InitRecv(std::string recvStr, SpServerSocket &spSocket, SocketConnectType type) const
{
    std::string errorInfo;
    std::string checkStr = recvStr.substr(std::string("init::").length());
    if (!SPTask::GetInstance().CheckTcpParam(checkStr, errorInfo) &&
        checkStr.find(SPTask::GetInstance().GetTcpToken()) == std::string::npos) {
        WLOGE("Init error(%s)", errorInfo.c_str());
        if (type == SocketConnectType::CMD_SOCKET) {
            spSocket.Send("init::False,\"error\":" + errorInfo);
        } else {
            spSocket.Send(std::string("init::") + SocketErrorTypeToString(SocketErrorType::INIT_FAILED));
        }
        return;
    }
    if (recvStr.find("-lockfreq") != std::string::npos &&
        SPTask::GetInstance().GetTcpToken() == "") {
        WLOGE("'-lockfreq' must have a valid token.");
        return;
    }
    ErrCode code = SPTask::GetInstance().InitTask(SplitMsg(recvStr));
    if (type == SocketConnectType::CMD_SOCKET) {
        spSocket.Send(std::string("init::") + ((code == ErrCode::OK) ? "True" : "False"));
        WLOGI("Sent init::"  + ((code == ErrCode::OK) ? "True" : "False"));
        return;
    }
    if (code == ErrCode::OK) {
        spSocket.Send("init::True");
        WLOGI("Sent init::True response");
    } else {
        spSocket.Send(std::string("init::") + SocketErrorTypeToString(SocketErrorType::INIT_FAILED));
        WLOGE("Sent init::%d for failure", SocketErrorType::INIT_FAILED);
    }
}
void SpThreadSocket::StartRecv(SpServerSocket &spSocket)
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
    if (code == ErrCode::OK) {
        spSocket.Send("start::True");
        flagRunning = true;
        WLOGI("Sent start::True message to socket.");
    } else if (code == ErrCode::FAILED) {
        spSocket.Send("start::False");
        WLOGE("Sent start::False message to socket.");
    }
}
void SpThreadSocket::StartRecvRealtime(SpServerSocket &spSocket) const
{
    auto lambdaTask = [&spSocket](const std::string &data) { spSocket.Send(data); };
    ErrCode code = SPTask::GetInstance().StartTask(lambdaTask);
    if (code == ErrCode::OK) {
        spSocket.Send("start::True");
        WLOGI("Sent start::True message to socket.");
    } else if (code == ErrCode::FAILED) {
        spSocket.Send(std::string("start::") + SocketErrorTypeToString(SocketErrorType::START_FAILED));
        WLOGE("Sent start::" + SocketErrorTypeToString(SocketErrorType::START_FAILED) + " message to socket.");
    }
}
void SpThreadSocket::StopRecvRealtime(SpServerSocket &spSocket)
{
    ErrCode code = SPTask::GetInstance().StopTask();
    if (code == ErrCode::OK) {
        spSocket.Send("stop::True");
        WLOGI("Sent stop::True message to socket.");
        flagRunning = false;
        spSocket.Close();
    } else if (code == ErrCode::FAILED) {
        spSocket.Send(std::string("stop::") + SocketErrorTypeToString(SocketErrorType::STOP_FAILED));
        WLOGE("Sent stop::" + SocketErrorTypeToString(SocketErrorType::STOP_FAILED) + " message to socket.");
    }
}
void SpThreadSocket::StartRecvRecord(SpServerSocket &spSocket) const
{
    ErrCode code = SPTask::GetInstance().StartRecord();
    if (code == ErrCode::OK) {
        spSocket.Send("startRecord::True");
        WLOGI("Sent startRecord::True message to socket.");
    } else {
        spSocket.Send(std::string("startRecord::") + SocketErrorTypeToString(SocketErrorType::START_RECORD_FAILED));
        WLOGE("Sent startRecord::" + SocketErrorTypeToString(SocketErrorType::START_RECORD_FAILED) +
        " message to socket.");
    }
}
void SpThreadSocket::StopRecvRecord(SpServerSocket &spSocket) const
{
    ErrCode code = SPTask::GetInstance().StopRecord();
    if (code == ErrCode::OK) {
        spSocket.Send("stopRecord::True");
        WLOGI("Sent stopRecord::True message to socket.");
    } else {
        spSocket.Send(std::string("stopRecord::") + SocketErrorTypeToString(SocketErrorType::STOP_RECORD_FAILED));
        WLOGE("Sent stopRecord::" + SocketErrorTypeToString(SocketErrorType::STOP_RECORD_FAILED) +
        " message to socket.");
    }
}
void SpThreadSocket::SendTokenFailedMessage(SpServerSocket &socket, std::string &message) const
{
    if (message.find("init:::") != std::string::npos ||
        message.find("start:::") != std::string::npos) {
        WLOGI("Skipping token check failure for init::: or start::: command.");
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
            WLOGE("Sending token check failed message for command: %s", it.c_str());
            socket.Send(it + SocketErrorTypeToString(SocketErrorType::TOKEN_CHECK_FAILED));
            return;
        }
    }
    WLOGW("No matching command found for token check failure in message: %s", message.c_str());
}
void SpThreadSocket::DealMsg(std::string recvStr, SpServerSocket &spSocket, SocketErrorType tokenStatus)
{
    SocketHeartbeat();
    if (tokenStatus == SocketErrorType::TOKEN_CHECK_FAILED) {
        SendTokenFailedMessage(spSocket, recvStr);
        return;
    }
    if (recvStr.find("init:::") != std::string::npos) {
        WLOGI("Processing 'init:::' command.");
        InitRecv(recvStr, spSocket, SocketConnectType::CMD_SOCKET);
    } else if (recvStr.find("start:::") != std::string::npos) {
        WLOGI("Processing 'start:::' command.");
        StartRecv(spSocket);
    } else if (recvStr.find("init::") != std::string::npos) {
        WLOGI("Processing 'init::' command.");
        InitRecv(recvStr, spSocket, SocketConnectType::EDITOR_SOCKET);
    } else if (recvStr.find("start::") != std::string::npos) {
        WLOGI("Processing 'start::' command.");
        StartRecvRealtime(spSocket);
    } else if (recvStr.find("stop::") != std::string::npos) {
        WLOGI("Processing 'stop::' command.");
        StopRecvRealtime(spSocket);
    } else if (recvStr.find("startRecord::") != std::string::npos) {
        WLOGI("Processing 'startRecord::' command.");
        StartRecvRecord(spSocket);
    } else if (recvStr.find("stopRecord::") != std::string::npos) {
        WLOGI("Processing 'stopRecord::' command.");
        StopRecvRecord(spSocket);
    } else if (recvStr.find("SP_daemon -editor") != std::string::npos) {
        EditorRecv(recvStr, spSocket);
    } else {
        WLOGW("Received unknown command: %s", recvStr.c_str());
    }
}
void SpThreadSocket::EditorRecv(std::string recvStr, const SpServerSocket &spSocket) const
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
void SpThreadSocket::BackDesktop() const
{
    std::string cmdResult;
    std::string uinput = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_BACK);
    SPUtils::LoadCmd(uinput, cmdResult);
}
// UDP
void SpThreadSocket::HandleMsg(SpServerSocket &spSocket) const
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
            } else if (iterator->first == MessageType::GET_LOG) {
                    data = GetLogProcess(profiler, recvBuf);
            } else {
                GetProcessIdByPkgName(iterator);
                data = profiler->ItemData();
            }
            HandleUDPMsg(spSocket, data, retCode, iterator);
        }
        LOGD("sendData key(%d) content(%s)", iterator->first, retCode.c_str());
        break;
    }
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
void SpThreadSocket::SocketHeartbeat() const
{
    Heartbeat &heartbeat = Heartbeat::GetInstance();
    heartbeat.UpdatestartTime();
}
void SpThreadSocket::FetchCpuStats(SpServerSocket &spSocket, std::map<std::string, std::string> data) const
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
void SpThreadSocket::HandleNullMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode,
    std::string recvBuf, std::unordered_map<MessageType, std::string>::const_iterator iterator) const
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
            if (recvBuf.find("$") != std::string::npos) {
                g_pkgAndPid = SplitMsg(recvBuf);
                g_pkgName = SpGetPkg(g_pkgAndPid);
            } else {
                g_pkgName = SplitMsg(recvBuf);
            }
            LOGD("UDP recv g_pkgName (%s)", g_pkgName.c_str());
        }
        spSocket.Sendto(retCode);
        LOGD("UDP send PkgName = %s", retCode.c_str());
    } else if (profiler == nullptr && (iterator->first == MessageType::GET_APP_TYPE)) {
        retCode = SplitMsg(recvBuf);
        std::thread rStart([this, retCode]() { this->ResetValue(retCode); });
        rStart.detach();
    } else if (profiler == nullptr && (iterator->first == MessageType::GET_DAEMON_VERSION)) {
        retCode = "Version: " + SPUtils::GetVersion();
        spSocket.Sendto(retCode);
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

void SpThreadSocket::GetProcessIdByPkgName(std::unordered_map<MessageType, std::string>::const_iterator iterator) const
{
    if (iterator->first == MessageType::GET_FPS_AND_JITTERS || iterator->first == MessageType::GET_CUR_FPS ||
        iterator->first == MessageType::GET_RAM_INFO) {
        if (!SpProfilerFactory::editorFlag) {
            LOGD("SpProfilerFactory::g_pkgName(%s)", g_pkgName.c_str());
            std::string processId = "";
            std::string processIds = "";
            g_preset = IsPreset(g_pkgAndPid);
            if (g_preset) {
                processId = SpGetPid(g_pkgAndPid);
            } else {
                OHOS::SmartPerf::StartUpDelay sp;
                processId = sp.GetPidByPkg(g_pkgName, &processIds);
            }
            SpProfilerFactory::SetProfilerPidByPkg(processId, processIds);
            SpProfilerFactory::SetProfilerPkg(g_pkgName);
        }
    }
}

void SpThreadSocket::ResetValue(std::string retCode) const
{
    ProfilerFPS &pfps = ProfilerFPS::GetInstance();
    pfps.isGameApp = SPUtils::GetIsGameApp(retCode);
    pfps.firstDump = true;
    RAM &ram = RAM::GetInstance();
    ram.SetHapFirstFlag();
}
void SpThreadSocket::HandleNullAddMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode,
    std::string recvBuf, std::unordered_map<MessageType, std::string>::const_iterator iterator) const
{
    Dubai &db = Dubai::GetInstance();
    if (iterator->first == MessageType::START_DUBAI_DB) {
        std::thread dStart([&db]() { db.CallBeginAndFinish(); });
        dStart.detach();
    } else if (iterator->first == MessageType::SET_DUBAI_DB) {
        db.CallBeginAndFinish();
        db.isDumpDubaiFinish = true;
        ProfilerFPS::isLowCurFps = false;
        retCode = db.CallMoveDubaiDbFinished();
        LOGD("UDP send GetDubaiDb Message: (%s)", retCode.c_str());
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

std::string SpThreadSocket::SocketErrorTypeToString(SocketErrorType errorType) const
{
    switch (errorType) {
        case SocketErrorType::OK:
            return "OK";
        case SocketErrorType::TOKEN_CHECK_FAILED:
            return "TOKEN_CHECK_FAILED";
        case SocketErrorType::INIT_FAILED:
            return "INIT_FAILED";
        case SocketErrorType::START_FAILED:
            return "START_FAILED";
        case SocketErrorType::STOP_FAILED:
            return "STOP_FAILED";
        case SocketErrorType::START_RECORD_FAILED:
            return "START_RECORD_FAILED";
        case SocketErrorType::STOP_RECORD_FAILED:
            return "STOP_RECORD_FAILED";
        default:
            return "UNKNOWN";
    }
}
std::map<std::string, std::string> SpThreadSocket::GetLogProcess(SpProfiler *profilerItem, std::string buffer) const
{
    if (buffer.find("::") != std::string::npos) {
        int port = SPUtilesTye::StringToSometype<int>(buffer.substr(buffer.find("::") + 2));
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
std::string SpThreadSocket::SpGetPkg(const std::string &spMsg) const
{
    if (spMsg.empty()) {
        LOGE("spMsg is null");
        return spMsg;
    }
    size_t pos = spMsg.find("$");
    if (pos != std::string::npos) {
        std::vector<std::string> sps;
        SPUtils::StrSplit(spMsg, "$", sps);
        if (sps.size() > 1) {
            return sps[0];
        } else {
            LOGE("SpGetPkg sps size is zreo");
            return spMsg;
        }
    } else {
        return spMsg;
    }
}

std::string SpThreadSocket::SpGetPid(const std::string &spMsg) const
{
    if (spMsg.empty()) {
        LOGE("spMsg is null");
        return spMsg;
    }
    size_t pos = spMsg.find("$");
    if (pos != std::string::npos) {
        std::vector<std::string> sps;
        SPUtils::StrSplit(spMsg, "$", sps);
        if (sps.size() > 1) {
            return sps[1];
        } else {
            LOGE("SpGetPid sps size is zreo");
            return "";
        }
    } else {
        return "";
    }
}

bool SpThreadSocket::IsPreset(const std::string &spMsg) const
{
    if (spMsg.find("$") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}
}
}