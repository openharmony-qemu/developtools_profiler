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
#include <cstdio>
#include <thread>
#include <cstring>
#include <iterator>
#include "unistd.h"
#include "include/heartbeat.h"
#include "include/sp_utils.h"
#include "include/sp_csv_util.h"
#include "include/sp_profiler_factory.h"
#include "include/sp_thread_socket.h"
#include "include/startup_delay.h"
#include "include/ByTrace.h"
#include "include/smartperf_command.h"
#include "include/sp_log.h"
#include "include/RAM.h"
#include "include/common.h"
#include "include/FPS.h"
#include "include/sp_task.h"

namespace OHOS {
namespace SmartPerf {
SmartPerfCommand::SmartPerfCommand(std::vector<std::string> argv)
{
    LOGD("SmartPerfCommand::SmartPerfCommand size(%u)", argv.size());
    if (argv.size() == oneParam) {
        EnableWriteLogAndDeleteOldLogFiles();
        OHOS::SmartPerf::StartUpDelay sd;
        sd.GetSpTcp();
        std::string pidStr = sd.GetPidByPkg("SP_daemon");
        std::string cmdStr = CMD_COMMAND_MAP.at(CmdCommand::TASKSET);
        std::string result = "";
        SPUtils::LoadCmd(cmdStr + pidStr, result);
        daemon(0, 0);
        CreateSocketThread();
    }
    if (argv.size() == twoParam) {
        auto iterator = COMMAND_HELP_MAP.begin();
        while (iterator != COMMAND_HELP_MAP.end()) {
            if (iterator->second.compare(argv[1]) == 0) {
                HelpCommand(iterator->first, "");
                break;
            }
            if (argv[1].find("-editorServer") != std::string::npos) {
                WLOGI("############################# Found '-editorServer' argument in argv");
                const size_t tokenStartPosition = 14;
                std::string token = argv[1].substr(tokenStartPosition, argv[1].length() - tokenStartPosition);
                HelpCommand(CommandHelp::EDITORSERVER, token);
            }
            ++iterator;
        }
    }
    if (argv.size() >= threeParamMore) {
        for (int i = 1; i <= static_cast<int>(argv.size()) - 1; i++) {
            std::string argStr = argv[i];
            std::string argStr1;
            if (i < static_cast<int>(argv.size()) - 1) {
                argStr1 = argv[i + 1];
            }
            if (COMMAND_MAP.count(argStr) > 0) {
                HandleCommand(argStr, argStr1);
            }
        }
    }
    LOGD("SmartPerfCommand::SmartPerfCommand complete");
}
void SmartPerfCommand::HelpCommand(CommandHelp type, std::string token) const
{
    LOGD("SmartPerfCommand::HelpCommand  type(%d)", type);
    if (type == CommandHelp::HELP) {
        std::cout << smartPerfMsg << std::endl;
    }
    if (type == CommandHelp::VERSION) {
        std::cout << "Version: " << SPUtils::GetVersion() << std::endl;
    }
    if (type == CommandHelp::SCREEN) {
        std::string result = SPUtils::GetScreen();
        std::cout << result << std::endl;
    }
    OHOS::SmartPerf::StartUpDelay sd;
    if (type == CommandHelp::CLEAR) {
        sd.GetSpClear();
    }
    if (type == CommandHelp::SERVER || type == CommandHelp::EDITORSERVER) {
        sd.ClearOldServer();
        std::string pidStr = sd.GetPidByPkg("SP_daemon");
        std::string cmdStr = CMD_COMMAND_MAP.at(CmdCommand::TASKSET);
        std::string result = "";
        SPUtils::LoadCmd(cmdStr + pidStr, result);
        if (type == CommandHelp::SERVER) {
            daemon(0, 0);
        } else {
            EnableWriteLogAndDeleteOldLogFiles();
            if (token.empty()) {
                WLOGE("Error: token is empty when setting TCP token.");
                return;
            } else {
                WLOGI("############################# Setting TCP token...");
                SPTask::GetInstance().SetTcpToken(token);
                WLOGI("############################# EditorServer Socket Create Start, Ready to Start Collector...");
            }
        }
        CreateSocketThread();
    }
}

void SmartPerfCommand::CreateSocketThread() const
{
    InitSomething();
    SpThreadSocket &udpThreadSocket = SpThreadSocket::GetInstance();
    SpThreadSocket &udpExThreadSocket = SpThreadSocket::GetInstance();
    SpThreadSocket &tcpThreadSocket = SpThreadSocket::GetInstance();
    auto tcpSocket = std::thread([&tcpThreadSocket]() { tcpThreadSocket.Process(ProtoType::TCP); });
    sleep(1);
    auto udpSocket = std::thread([&udpThreadSocket]() { udpThreadSocket.Process(ProtoType::UDP); });
    sleep(1);
    auto udpexSocket = std::thread([&udpExThreadSocket]() { udpExThreadSocket.Process(ProtoType::UDPEX); });
    Heartbeat &heartbeat = Heartbeat::GetInstance();
    heartbeat.UpdatestartTime();
    std::thread threadHeartbeat([&heartbeat]() {heartbeat.HeartbeatRule(); });
    threadHeartbeat.detach();
    tcpSocket.join();
    udpSocket.join();
    udpexSocket.join();
}
void SmartPerfCommand::HandleCommand(std::string argStr, const std::string &argStr1)
{
    LOGD("SmartPerfCommand::HandleCommand  argStr(%s) argStr1(%s)", argStr.c_str(), argStr1.c_str());
    switch (COMMAND_MAP.at(argStr)) {
        case CommandType::CT_N:
            num = SPUtilesTye::StringToSometype<int>(argStr1.c_str());
            break;
        case CommandType::CT_PKG:
            pkgName = argStr1;
            if (pkgName.length() > 0) {
                SpProfilerFactory::SetProfilerPkg(pkgName);
                FPS &fps = FPS::GetInstance();
                fps.isGameApp = SPUtils::GetIsGameApp(pkgName);
                fps.firstDump = true;
            }
            break;
        case CommandType::CT_VIEW:
            layerName = argStr1;
            if (layerName.length() > 0) {
                SpProfilerFactory::SetProfilerLayer(layerName);
            }
            break;
        case CommandType::CT_OUT:
            outPathParam = argStr1;
            if (strcmp(outPathParam.c_str(), "") != 0) {
                outPath = outPathParam;
            }
            break;
        case CommandType::CT_C:
        case CommandType::CT_G:
        case CommandType::CT_D:
        case CommandType::CT_F:
        case CommandType::CT_T:
        case CommandType::CT_P:
        case CommandType::CT_R:
        case CommandType::CT_NET:
        case CommandType::CT_TTRACE:
        case CommandType::CT_SNAPSHOT:
        case CommandType::CT_HW:
        case CommandType::CT_GC:
        case CommandType::CT_NAV:
        case CommandType::CT_AS:
            configs.push_back(argStr);
            break;
        default:
            std::cout << "other unknown args:" << argStr << std::endl;
            break;
    }
}

int SmartPerfCommand::GetItemInfo(std::multimap<std::string, std::string, decltype(SPUtils::Cmp) *> &spMap)
{
    int rc = 0;
    std::string errInfo;
    if (!pkgName.empty()) {
        std::string processId = "";
        OHOS::SmartPerf::StartUpDelay sp;
        processId = sp.GetPidByPkg(pkgName);
        LOGD("The cmd pid = %s", processId.c_str());
        SpProfilerFactory::SetProfilerPidByPkg(processId);
    }
    for (size_t j = 0; j < configs.size(); j++) {
        std::string curParam = configs[j];

        if (curParam.find("-gc") != std::string::npos) {
            continue;
        }

        SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(COMMAND_MAP.at(curParam), true);
        if (profiler != nullptr) {
            std::map<std::string, std::string> data = profiler->ItemData();
            spMap.insert(data.cbegin(), data.cend());
        }
    }

    if (!errInfo.empty()) { // GPU Counter init failed
        printf("%s\n", errInfo.c_str());
        LOGE("%s", errInfo.c_str());
        return -1;
    }

    return rc;
}

void SmartPerfCommand::SaveGpuCounter() const
{
    std::string outGpuCounterDataPath = "/data/local/tmp";
    gpuCounter.SaveData(outGpuCounterDataPath);
    gpuCounter.StopCollect();
}
void SmartPerfCommand::StartGpuCounterCollect(std::string config, bool &flag) const
{
    if (config.find("-gc") != std::string::npos) {
        gpuCounter.StartCollect(GpuCounter::GC_START);
        flag = true;
    }
}
std::string SmartPerfCommand::ExecCommand()
{
    RAM &ram = RAM::GetInstance();
    ram.SetFirstFlag();
    int rc = 0;
    int index = 0;
    std::vector<SPData> vmap;
    const long long freq = 1000;
    num = num + 1;
    bool gcFlag = false;
    for (std::string itConfig : configs) {
        if (itConfig.find("-aischedule") != std::string::npos) {
            return std::string("Command not support param aischedule");
        }
        StartGpuCounterCollect(itConfig, gcFlag);
    }
    while (index < num) {
        std::multimap<std::string, std::string, decltype(SPUtils::Cmp) *> spMap(SPUtils::Cmp);
        long long lastTime = SPUtils::GetCurTime();
        spMap.insert(std::pair<std::string, std::string>(std::string("timestamp"), std::to_string(lastTime)));
        rc = GetItemInfo(spMap);
        if (rc == -1) {
            break;
        }
        std::map<std::string, std::string> gpuCounterDataMap;
        gpuCounter.GetGpuRealtimeData(gpuCounterDataMap);
        spMap.insert(gpuCounterDataMap.begin(), gpuCounterDataMap.end());
        std::cout << std::endl;
        PrintMap(spMap, index);
        std::cout << std::endl;
        SPData spdata;
        if (index != 0) {
            spdata.values.insert(spMap.cbegin(), spMap.cend());
            vmap.push_back(spdata);
        }
        long long nextTime = SPUtils::GetCurTime();
        long long costTime = nextTime - lastTime;
        if (costTime < freq) {
            std::this_thread::sleep_for(std::chrono::milliseconds(freq - costTime));
        }
        index++;
    }
    if (gcFlag) {
        SaveGpuCounter();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(freq));
    SpCsvUtil::WriteCsv(std::string(outPath.c_str()), vmap);
    return std::string("command exec finished!");
}
void SmartPerfCommand::PrintfExecCommand(const std::map<std::string, std::string> data) const
{
    int i = 0;
    for (auto a = data.cbegin(); a != data.cend(); ++a) {
        printf("order:%d %s=%s\n", i++, a->first.c_str(), a->second.c_str());
    }
}

void SmartPerfCommand::PrintMap(std::multimap<std::string, std::string,
    decltype(SPUtils::Cmp) *> &spMap, int index) const
{
    int i = 0;
    for (auto iter = spMap.cbegin(); iter != spMap.cend(); ++iter) {
        if (index != 0) {
            printf("order:%d %s=%s\n", i++, iter->first.c_str(), iter->second.c_str());
        }
    }
}

void SmartPerfCommand::InitSomething()
{
    std::string cmdResult;
    std::string stat = CMD_COMMAND_MAP.at(CmdCommand::PROC_STAT);
    if (SPUtils::LoadCmd(stat, cmdResult)) {
        LOGE("SmartPerfCommand::InitSomething Privilege escalation!");
    };
}
}
}
