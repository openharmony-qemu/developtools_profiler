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
#include <thread>
#include <string>
#include <climits>
#include "include/sp_task.h"
#include "include/sp_profiler_factory.h"
#include "include/sp_utils.h"
#include "include/FPS.h"
#include "include/startup_delay.h"
#include "include/sp_log.h"
#include "ByTrace.h"
#include <cstdio>
#include <ios>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include "unistd.h"

namespace OHOS {
namespace SmartPerf {
const long long RM_0 = 0;
const long long RM_5000 = 5000;
const long long RM_1000000 = 1000000;
// init::-SESSIONID 12345678 -INTERVAL 1000 -PKG ohos.samples.ecg -c -g -t -p -f -r -fl 30
static ExceptionMsg ParseToTask(std::string command, TaskInfo &taskInfo)
{
    std::vector<std::string> args;
    size_t pos = 0;
    while ((pos = command.find(" ")) != std::string::npos) {
        args.push_back(command.substr(0, pos));
        command.erase(0, pos + 1);
    }
    args.push_back(command);
    StuckNotification snf;
    std::string sessionId;
    long long interval = 1000;
    std::string pkg;
    bool isFPS = false;
    std::vector<std::string> configs;
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_SESSIONID)) {
            sessionId = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_INTERVAL)) {
            interval = std::stoll(args[++i]);
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_PKG)) {
            pkg = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_FL)) { // 获取用户fps的值，并赋给snf.   CT_FL
            snf.fps = std::stoi(args[++i]);
            snf.isEffective = true;
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_FTL)) { // 获取frameTime的值      CT_FTL
            snf.frameTime = std::stoi(args[++i]);
            snf.isEffective = true;
        } else {
            if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_F)) { // 判断用户设置是否有-f
                isFPS = true;
            }
            if (commandMap.end() != commandMap.find(args[i])) {
                configs.push_back(args[i]);
            }
        }
    }
    if (snf.isEffective && (!isFPS)) {
        return ExceptionMsg::TASK_CONFIG_NULL;
    }
    if (sessionId.empty()) {
        LOGE("ExceptionMsg ParseToTask sessoin id is null");
        return ExceptionMsg::SESSION_ID_NULL;
    } else if (configs.size() == 0) {
        LOGE("ExceptionMsg ParseToTask configs size is 0");
        return ExceptionMsg::TASK_CONFIG_NULL;
    }
    taskInfo = { sessionId, pkg, configs, interval, snf };
    return ExceptionMsg::NO_ERR;
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
    LOGI("SPTask::InitTask start param(%s)", recvStr.c_str());
    std::string result = "";
    if (!SPUtils::FileAccess("/data/local/tmp/hitrace")) {
        SPUtils::LoadCmd("mkdir /data/local/tmp/hitrace", result);
    } else {
        SPUtils::LoadCmd("rm -rf /data/local/tmp/hitrace/*.htrace", result);
    }
    result.clear();
    if (!SPUtils::FileAccess("/data/local/tmp/hitrace/perf")) {
        SPUtils::LoadCmd("mkdir /data/local/tmp/hitrace/perf", result);
    } else {
        SPUtils::LoadCmd("rm -rf /data/local/tmp/hitrace/perf/*.data", result);
    }
    std::cout << recvStr << std::endl;
    ExceptionMsg exMsg = ParseToTask(recvStr, curTaskInfo);
    if (exMsg == ExceptionMsg::NO_ERR) {
        isInit = true;
        LOGI("SPTask::InitTask Ok");
        return ErrCode::OK;
    }

    std::string errInfo = ExceptionMsgMap.at(exMsg);
    LOGI("SPTask::InitTask error(%s)", errInfo.c_str());
    std::cout << "ExceptionMsg:" << errInfo << std::endl;
    return ErrCode::FAILED;
}
ErrCode SPTask::StartTask(std::function<void(std::string data)> msgTask)
{
    LOGI("SPTask::StartTask start ");
    if (!isInit) {
        LOGW("SPTask::StartTask initialization failed");
        return ErrCode::FAILED;
    }
    isRunning = true;
    startTime = SPUtils::GetCurTime();
    if (!curTaskInfo.packageName.empty()) {
        SpProfilerFactory::SetProfilerPkg(curTaskInfo.packageName);
    }
    thread = std::thread([this, msgTask]() {
        std::cout << "Task " << curTaskInfo.sessionId << ": collecting data loop..." << std::endl;
        while (isRunning) {
            // 执行采集任务
            long long lastTime = SPUtils::GetCurTime();
            std::lock_guard<std::mutex> lock(mtx);
            std::map<std::string, std::string> dataMap;
            dataMap.insert(std::pair<std::string, std::string>(std::string("timestamp"), std::to_string(lastTime)));
            for (std::string itConfig : curTaskInfo.taskConfig) {
                SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(commandMap.at(itConfig));
                std::map<std::string, std::string> itemMap = profiler->ItemData();
                dataMap.insert(itemMap.begin(), itemMap.end());
            }
            if (curTaskInfo.stuckInfo.isEffective) {
                std::map<std::string, std::string> timeUsedMap = DetectionAndGrab();
                if (!timeUsedMap.empty()) {
                    dataMap.insert(timeUsedMap.begin(), timeUsedMap.end());
                }
            }
            SPData spdata;
            spdata.values.insert(dataMap.begin(), dataMap.end());
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

    LOGI("SPTask::StartTask complete");
    return ErrCode::OK;
}
void SPTask::StopTask()
{
    LOGI("SPTask::StopTask start");
    if (isInit) {
        KillHiperfCmd();

        std::string thisBasePath = baseOutPath + "/" + curTaskInfo.sessionId;
        if (!SPUtils::FileAccess(thisBasePath)) {
            std::string cmdResult;
            SPUtils::LoadCmd("mkdir -p " + thisBasePath, cmdResult);
        }
        std::string outGeneralPath = thisBasePath + "/t_general_info.csv";
        std::string outIndexpath = thisBasePath + "/t_index_info.csv";
        long long endTime = SPUtils::GetCurTime();
        long long testDuration = (endTime - startTime) / 1000;
        std::string screenStr = SPUtils::GetScreen();
        int pos3 = screenStr.find("=");
        std::string refreshrate = screenStr.substr(pos3 + 1);
        std::map<std::string, std::string> taskInfoMap = {
            { "sessionId", curTaskInfo.sessionId },
            { "taskId", curTaskInfo.sessionId },
            { "appName", curTaskInfo.packageName },
            { "packageName", curTaskInfo.packageName },
            { "startTime", std::to_string(startTime) },
            { "endTime", std::to_string(endTime) },
            { "testDuration", std::to_string(testDuration) },
            { "taskName", "testtask" },
            { "board", "hw" },
            { "target_fps", refreshrate },
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
    vmap.clear();
    if (thread.joinable()) {
        thread.join();
    }

    LOGI("SPTask::StopTask complete");
}

std::map<std::string, std::string> SPTask::DetectionAndGrab()
{
    std::map<std::string, std::string> templateMap;
    if (!curTaskInfo.stuckInfo.isEffective) {
        return templateMap;
    }

    FpsCurrentFpsTime fcf = FPS::GetInstance().GetFpsCurrentFpsTime();
    long long curframeTime = fcf.currentFpsTime / RM_1000000; // Convert to milliseconds
    std::cout << "start::" << startCaptuerTime << std::endl;
    if (curTaskInfo.stuckInfo.fps > fcf.fps || curTaskInfo.stuckInfo.frameTime < curframeTime) {
        if (!isCaptureTrace) {
            isCaptureTrace = true;
            startCaptuerTime = SPUtils::GetCurTime();
            std::cout << "ThreadGetHiperf::" << startCaptuerTime << std::endl;
            ThreadGetHiperf(startCaptuerTime);
        } else {
            long long diff = RM_0;
            long long nowTime = SPUtils::GetCurTime();
            diff = startCaptuerTime > nowTime ? (LLONG_MAX - startCaptuerTime + nowTime) : (nowTime - startCaptuerTime);
            std::cout << "else::" << startCaptuerTime << std::endl;
            if (diff > RM_5000 && (!CheckCounterId())) {
                isCaptureTrace = false;
                startCaptuerTime = RM_0;
            }
            std::cout << "end::" << startCaptuerTime << std::endl;
        }
    }
    templateMap["fpsWarn"] = std::to_string(fcf.fps);
    templateMap["FrameTimeWarn"] = std::to_string(fcf.currentFpsTime);
    templateMap["TraceTime"] = std::to_string(startCaptuerTime);
    return templateMap;
}

bool SPTask::CheckCounterId()
{
    std::string result;

    SPUtils::LoadCmd("ps -ef |grep hiprofiler_cmd |grep -v grep", result);
    if (result.empty()) {
        return false;
    }

    if (result.find("-k") != std::string::npos) {
        return true;
    }

    return false;
}
std::thread SPTask::ThreadGetHiperf(long long ts)
{
    std::thread thGetTrace(&SPTask::GetHiperf, this, std::to_string(ts));
    thGetTrace.detach();
    return thGetTrace;
}

void SPTask::GetHiperf(std::string traceName)
{
    std::string result;
    std::string tmp = SetHiperf(traceName);
    std::cout << tmp << std::endl;
    result.clear();
    SPUtils::LoadCmd(tmp, result);
}


bool SPTask::CheckTcpParam(std::string str, std::string &errorInfo)
{
    std::set<std::string> keys;
    for (auto a : commandMap) {
        keys.insert(a.first.substr(1)); // 不需要前面的'-'
    }

    return SPUtils::VeriyParameter(keys, str, errorInfo);
}

void SPTask::KillHiperfCmd()
{
    std::string killCmd = "kill -9 ";
    std::string result;
    std::vector<std::string> out;

    std::cout << "pidof hiprofiler_cmd=" << killCmd << std::endl;
    SPUtils::LoadCmd("pidof hiprofiler_cmd", result);
    SPUtils::StrSplit(result, " ", out);

    for (auto it = out.begin(); out.end() != it; ++it) {
        result.clear();
        SPUtils::LoadCmd(killCmd + (*it), result);
    }

    return;
}

std::string SPTask::SetHiperf(std::string traceName)
{
    std::string hiPrefix = "hiprofiler_";
    std::string dataPrefix = "perf_";
    requestId++;
    std::string trtmp = strOne + hiPrefix + traceName + strTwo + "\n" + strThree + std::to_string(requestId) + "\n" +
        strFour + "\n" + strFive + hiPrefix + traceName + strSix + "\n" + strNine + strEleven + "\n" + strSeven +
        dataPrefix + traceName + strEight + strTen + "\n" + conFig;
    return trtmp;
}
}
}
