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
#include "include/sp_task.h"
#include "include/sp_profiler_factory.h"
#include "include/sp_utils.h"
namespace OHOS {
namespace SmartPerf {

// init::-SESSIONID 12345678 -INTERVAL 1000 -PKG ohos.samples.ecg -c -g -t -p -f -r
static ExceptionMsg ParseToTask(std::string command, TaskInfo &taskInfo)
{
    std::vector<std::string> args;
    size_t pos = 0;
    while ((pos = command.find(" ")) != std::string::npos) {
        args.push_back(command.substr(0, pos));
        command.erase(0, pos + 1);
    }
    args.push_back(command);

    for (std::string arg:args) {
        std::cout << "arg:" << arg << std::endl;
    }

    std::string sessionId;
    long long interval = 1000;
    std::string pkg;
    std::vector<std::string> configs;
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_SESSIONID)) {
            sessionId = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_INTERVAL)) {
            interval = std::stoll(args[++i]);
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_PKG)) {
            pkg = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_NET)) {
            std::cout << "arg:net:" << std::endl;
        } else {
            configs.push_back(args[i]);
        }
    }
    if (sessionId.empty()) {
        return ExceptionMsg::SESSION_ID_NULL;
    } else if (pkg.empty()) {
        return ExceptionMsg::PACKAGE_NULL;
    } else if (configs.size() == 0) {
        return ExceptionMsg::TASK_CONFIG_NULL;
    }
    taskInfo = { sessionId, pkg, configs, interval };
    return  ExceptionMsg::NO_ERR;
}

static std::string MapToString(std::map<std::string, std::string> myMap)
{
    // 将Map转换为字符串
    std::string str = "{ ";
    for (auto it = myMap.begin(); it != myMap.end(); ++it) {
        str += "\"" + it->first + "\": " + it->second + ", ";
    }
    const int subLen = 2;
    str.erase(str.end() - subLen, str.end());
    str += " }";
    return str;
}

ErrCode SPTask::InitTask(std::string recvStr)
{
    ExceptionMsg exMsg = ParseToTask(recvStr, curTaskInfo);
    if (exMsg == ExceptionMsg::NO_ERR) {
        isInit = true;
        return ErrCode::OK;
    }
    if (recvStr.find("-net") != std::string::npos) {
        isInitNet = true;
    }
    std::cout << "ExceptionMsg:" << ExceptionMsgMap.at(exMsg) << std::endl;
    return ErrCode::FAILED;
}
ErrCode SPTask::StartTask(std::function<void(std::string data)> msgTask)
{
    if (!isInit) {
        return ErrCode::FAILED;
    }
    isRunning = true;
    startTime = SPUtils::GetCurTime();
    SpProfilerFactory::SetProfilerPkg(curTaskInfo.packageName);
    std::string pidCmd = "pidof " + curTaskInfo.packageName;
    std::string pidResult;
    if (SPUtils::LoadCmd(pidCmd, pidResult)) {
        SpProfilerFactory::SetProfilerPid(pidResult);
    }
    thread = std::thread([this, msgTask]() {
        std::cout << "Task " << curTaskInfo.sessionId << ": collecting data loop..." << std::endl;
        while (isRunning) {
            // 执行采集任务
            long long lastTime = SPUtils::GetCurTime();
            std::lock_guard<std::mutex> lock(mtx);
            std::map<std::string, std::string> dataMap;
            for (std::string itConfig:curTaskInfo.taskConfig) {
                SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(commandMap.at(itConfig));
                std::map<std::string, std::string> itemMap = profiler->ItemData();
                dataMap.insert(itemMap.begin(), itemMap.end());
            }
            if (isInitNet) {
                std::map<std::string, std::string> itemMap = SPUtils::GetNetwork();
                dataMap.insert(itemMap.begin(), itemMap.end());
            }
            SPData spdata;
            spdata.values = dataMap;
            vmap.push_back(spdata);
            long long nextTime = SPUtils::GetCurTime();
            long long costTime = nextTime - lastTime;
            if (costTime < curTaskInfo.freq) {
                std::this_thread::sleep_for(std::chrono::milliseconds(curTaskInfo.freq - costTime));
            }
            if (isRunning) {
                msgTask(MapToString(dataMap));
            }
        }
    });
    return ErrCode::OK;
}
void SPTask::StopTask()
{
    if (isInit) {
        std::string thisBasePath = baseOutPath + "/" + curTaskInfo.sessionId;
        if (!SPUtils::FileAccess(thisBasePath)) {
            std::string cmdResult;
            SPUtils::LoadCmd("mkdir -p " + thisBasePath, cmdResult);
        }
        std::string outGeneralPath = thisBasePath + "/t_general_info.csv";
        std::string outIndexpath = thisBasePath + "/t_index_info.csv";
        long long endTime  = SPUtils::GetCurTime();
        long long testDuration = (endTime - startTime) / 1000;
        std::map<std::string, std::string> taskInfoMap = {
            {"sessionId", curTaskInfo.sessionId},
            {"taskId", curTaskInfo.sessionId},
            {"appName", curTaskInfo.packageName},
            {"packageName", curTaskInfo.packageName},
            {"startTime", std::to_string(startTime)},
            {"endTime", std::to_string(endTime)},
            {"testDuration", std::to_string(testDuration)},
            {"taskName", "testtask"},
            {"board", "hw"},
        };
        std::map<std::string, std::string> deviceInfo = SPUtils::GetDeviceInfo();
        std::map<std::string, std::string> cpuInfo = SPUtils::GetCpuInfo();
        std::map<std::string, std::string> gpuInfo = SPUtils::GetGpuInfo();
        std::map<std::string, std::string> destMap;
        destMap.insert(taskInfoMap.begin(), taskInfoMap.end());
        destMap.insert(deviceInfo.begin(), deviceInfo.end());
        destMap.insert(cpuInfo.begin(), cpuInfo.end());
        destMap.insert(gpuInfo.begin(), gpuInfo.end());
        OHOS::SmartPerf::SpCsvUtil::WriteCsvH(outGeneralPath, destMap);
        OHOS::SmartPerf::SpCsvUtil::WriteCsv(outIndexpath, vmap);
    }
    isRunning = false;
    isInit = false;
    isInitNet = false;
    vmap.clear();
    if (thread.joinable()) {
        thread.join();
    }
}
}
}