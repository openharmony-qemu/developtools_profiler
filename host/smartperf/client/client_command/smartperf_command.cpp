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
#include "unistd.h"
#include "include/sp_utils.h"
#include "include/sp_csv_util.h"
#include "include/sp_profiler_factory.h"
#include "include/sp_thread_socket.h"
#include "include/ByTrace.h"
#include "include/smartperf_command.h"
namespace OHOS {
namespace SmartPerf {
SmartPerfCommand::SmartPerfCommand(int argc, char *argv[])
{
    if (argc == oneParam) {
        daemon(0, 0);
        InitSomething();
        SpThreadSocket udpThreadSocket;
        SpThreadSocket tcpThreadSocket;
        std::thread tSocket(&SpThreadSocket::Process, tcpThreadSocket, ProtoType::TCP);
        std::thread tSocket1(&SpThreadSocket::Process, udpThreadSocket, ProtoType::UDP);
        tSocket.join();
        tSocket1.join();
    }
    if (argc == twoParam) {
        auto iterator = commandHelpMap.begin();
        while (iterator != commandHelpMap.end()) {
            if (strcmp(argv[1], iterator->second.c_str()) == 0) {
                HelpCommand(iterator->first);
                break;
            }
            ++iterator;
        }
    }
    if (argc >= threeParamMore) {
        for (int i = 1; i <= argc - 1; i++) {
            std::string argStr = argv[i];
            std::string argStr1;
            if (i < argc - 1) {
                argStr1 = argv[i + 1];
            }
            if (commandMap.count(argStr) > 0) {
                HandleCommand(argStr, argStr1);
            }
        }
    }
}
void SmartPerfCommand::HelpCommand(CommandHelp type) const
{
    if (type == CommandHelp::HELP) {
        std::cout << smartPerfMsg << std::endl;
    }
    if (type == CommandHelp::VERSION) {
        std::cout << smartPerfVersion << std::endl;
    }
}
void SmartPerfCommand::HandleCommand(std::string argStr, std::string argStr1)
{
    switch (commandMap.at(argStr)) {
        case CommandType::CT_N:
            num = atoi(argStr1.c_str());
            break;
        case CommandType::CT_PKG:
            pkgName = argStr1;
            break;
        case CommandType::CT_PID:
            pid = argStr1;
            break;
        case CommandType::CT_OUT:
            outPathParam = argStr1;
            if (strcmp(outPathParam.c_str(), "") != 0) {
                outPath = outPathParam + std::string(".csv");
            }
            break;
        case CommandType::CT_C:
        case CommandType::CT_G:
        case CommandType::CT_D:
        case CommandType::CT_F:
        case CommandType::CT_T:
        case CommandType::CT_P:
        case CommandType::CT_R:
        case CommandType::CT_TTRACE:
        case CommandType::CT_SNAPSHOT:
        case CommandType::CT_HW:
            configs.push_back(argStr);
            break;
        default:
            std::cout << "other unknown args:" << argStr << std::endl;
            break;
    }
    SpProfilerFactory::SetProfilerPid(pid);
    SpProfilerFactory::SetProfilerPkg(pkgName);
}

std::string SmartPerfCommand::ExecCommand()
{
    int index = 0;
    std::vector<SPData> vmap;
    while (index < num) {
        std::map<std::string, std::string> spMap;
        long long timestamp = SPUtils::GetCurTime();
        spMap.insert(std::pair<std::string, std::string>(std::string("timestamp"), std::to_string(timestamp)));

        for (size_t j = 0; j < configs.size(); j++) {
            std::string curParam = configs[j];
            SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(commandMap.at(curParam));
            if (profiler != nullptr) {
                std::map<std::string, std::string> data = profiler->ItemData();
                std::map<std::string, std::string> tempData = spMap;
                tempData.insert(data.cbegin(), data.cend());
                spMap = tempData;
            }
        }

        std::cout << std::endl;
        int i = 0;
        for (auto iter = spMap.cbegin(); iter != spMap.cend(); ++iter) {
            printf("order:%d %s=%s\n", i, iter->first.c_str(), iter->second.c_str());
            i++;
        }
        std::cout << std::endl;

        SPData spdata;
        spdata.values = spMap;
        vmap.push_back(spdata);
        sleep(1);
        index++;
    }
    SpCsvUtil::WriteCsv(std::string(outPath.c_str()), vmap);
    return std::string("command exec finished!");
}
void SmartPerfCommand::InitSomething()
{
    std::string cmdResult;
    if (SPUtils::LoadCmd("chmod o+r /proc/stat", cmdResult)) {
        printf("Privilege escalation! \n");
    };
    if (!SPUtils::FileAccess("/data/local/tmp/capture")) {
        SPUtils::LoadCmd("mkdir /data/local/tmp/capture", cmdResult);
        printf("/data/local/tmp/capture created! \n");
    };
}
}
}
