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
#include "include/RAM.h"
#include "include/CPU.h"
#include "include/Capture.h"
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
#include <future>
#include "common.h"

namespace OHOS {
namespace SmartPerf {
const long long RM_0 = 0;
const long long RM_5000 = 5000;
const long long RM_1000 = 1000;
const long long RM_1000000 = 1000000;
const long long END_WAITING_TIME = 8; // End waiting time,unit seconds
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
    snf.isEffective = false;
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
            if (COMMAND_MAP.end() != COMMAND_MAP.find(args[i])) {
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

ErrCode SPTask::InitTask(const std::string &recvStr)
{
    LOGI("SPTask::InitTask start param(%s)", recvStr.c_str());
    std::string result = "";
    std::string hiprofiler = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER);
    SPUtils::LoadCmd(hiprofiler, result);
    result.clear();
    std::string perf = CMD_COMMAND_MAP.at(CmdCommand::PERF);
    SPUtils::LoadCmd(perf, result);
    std::cout << recvStr << std::endl;
    ExceptionMsg exMsg = ParseToTask(recvStr, curTaskInfo);
    if (exMsg == ExceptionMsg::NO_ERR) {
        isInit = true;
        LOGI("SPTask::InitTask Ok");
        return ErrCode::OK;
    }

    std::string errInfo = EXCEPTION_MSG_MAP.at(exMsg);
    LOGI("SPTask::InitTask error(%s)", errInfo.c_str());
    std::cout << "ExceptionMsg:" << errInfo << std::endl;
    return ErrCode::FAILED;
}

std::future<std::map<std::string, std::string>> SPTask::AsyncCollectRam()
{
    std::promise<std::map<std::string, std::string>> p;
    std::future<std::map<std::string, std::string>> futureResult;
    for (std::string ramConfig : curTaskInfo.taskConfig) {
        if (ramConfig.find("-r") != std::string::npos) {
            futureResult = p.get_future();
            std::thread([p = std::move(p)]() mutable {
                p.set_value(RAM::GetInstance().ItemData());
            }).detach();
        }
    }
    return futureResult;
}

std::future<std::map<std::string, std::string>> SPTask::AsyncCollectCpu()
{
    std::promise<std::map<std::string, std::string>> p;
    std::future<std::map<std::string, std::string>> futureResult;
    for (std::string cpuConfig : curTaskInfo.taskConfig) {
        if (cpuConfig.find("-c") != std::string::npos) {
            futureResult = p.get_future();
            std::thread([p = std::move(p)]() mutable {
                p.set_value(CPU::GetInstance().ItemData());
            }).detach();
        }
    }
    return futureResult;
}

std::future<std::map<std::string, std::string>> SPTask::AsyncCollectFps()
{
    std::promise<std::map<std::string, std::string>> p;
    std::future<std::map<std::string, std::string>> futureResult;
    for (std::string fpsConfig : curTaskInfo.taskConfig) {
        if (fpsConfig.find("-f") != std::string::npos) {
            futureResult = p.get_future();
            std::thread([p = std::move(p)]() mutable {
                p.set_value(FPS::GetInstance().ItemData());
            }).detach();
        }
    }
    return futureResult;
}

void SPTask::CheckFutureRam(std::future<std::map<std::string, std::string>> &ramResult,
    std::map<std::string, std::string> &dataMap)
{
    if (ramResult.valid()) {
        std::map<std::string, std::string> result = ramResult.get();
        dataMap.insert(result.begin(), result.end());
    }
}

void SPTask::CheckFutureCpu(std::future<std::map<std::string, std::string>> &cpuResult,
    std::map<std::string, std::string> &dataMap)
{
    if (cpuResult.valid()) {
        std::map<std::string, std::string> result = cpuResult.get();
        dataMap.insert(result.begin(), result.end());
    }
}

void SPTask::CheckFutureFps(std::future<std::map<std::string, std::string>> &fpsResult,
    std::map<std::string, std::string> &dataMap)
{
    if (fpsResult.valid()) {
        std::map<std::string, std::string> result = fpsResult.get();
        dataMap.insert(result.begin(), result.end());
    }
}

void SPTask::GetItemData(std::map<std::string, std::string> &dataMap)
{
    for (std::string itConfig : curTaskInfo.taskConfig) {
        if (itConfig.find("-snapshot") != std::string::npos) {
            Capture::GetInstance().SocketMessage();
            std::map<std::string, std::string> captureMap = Capture::GetInstance().ItemData();
            dataMap.insert(captureMap.begin(), captureMap.end());
        }
        SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(COMMAND_MAP.at(itConfig), false);
        if (profiler != nullptr) {
            std::map<std::string, std::string> itemMap = profiler->ItemData();
            dataMap.insert(itemMap.begin(), itemMap.end());
        }
    }
}

ErrCode SPTask::StartTask(std::function<void(std::string data)> msgTask)
{
    LOGI("SPTask::StartTask start ");
    RAM &ram = RAM::GetInstance();
    ram.SetFirstFlag();
    if (!isInit) {
        LOGW("SPTask::StartTask initialization failed");
        return ErrCode::FAILED;
    }
    isRunning = true;
    startTime = SPUtils::GetCurTime();
    if (!curTaskInfo.packageName.empty()) {
        SpProfilerFactory::SetProfilerPkg(curTaskInfo.packageName);
    }
    vmap.clear();
    thread = std::thread([this, msgTask]() {
        while (isRunning) {
            long long lastTime = SPUtils::GetCurTime();
            std::lock_guard<std::mutex> lock(mtx);
            std::map<std::string, std::string> dataMap;
            dataMap.insert(std::pair<std::string, std::string>(std::string("timestamp"), std::to_string(lastTime)));
            std::future<std::map<std::string, std::string>> fpsResult = AsyncCollectFps();
            std::future<std::map<std::string, std::string>> cpuResult = AsyncCollectCpu();
            GetItemData(dataMap);
            std::future<std::map<std::string, std::string>> ramResult = AsyncCollectRam();
            CheckFutureFps(fpsResult, dataMap);
            CheckFutureCpu(cpuResult, dataMap);
            CheckFutureRam(ramResult, dataMap);
            if (curTaskInfo.stuckInfo.isEffective) {
                std::map<std::string, std::string> timeUsedMap = DetectionAndGrab();
                if (!timeUsedMap.empty()) {
                    dataMap.insert(timeUsedMap.begin(), timeUsedMap.end());
                }
            }
            SPData spdata;
            spdata.values.insert(dataMap.begin(), dataMap.end());
            vmap.push_back(spdata);

            msgTask(MapToString(dataMap));
            long long nextTime = SPUtils::GetCurTime();
            long long costTime = nextTime - lastTime;
            long long pTime = 998;
            if (costTime < curTaskInfo.freq) {
                std::this_thread::sleep_for(std::chrono::milliseconds(pTime - costTime));
            }
        }
    });
    LOGI("SPTask::StartTask complete");
    return ErrCode::OK;
}
void SPTask::WritePath(std::string thisBasePath)
{
    if (!SPUtils::FileAccess(thisBasePath)) {
        std::string cmdResult;
        std::string writePath = CMD_COMMAND_MAP.at(CmdCommand::WRITE_PATH) + curTaskInfo.sessionId;
        SPUtils::LoadCmd(writePath, cmdResult);
    }
}

void SPTask::StopTask()
{
    bool isTcpMessage = true;
    LOGI("SPTask::StopTask start");

    if (isInit) {
        std::string thisBasePath = baseOutPath + "/" + curTaskInfo.sessionId;
        WritePath(thisBasePath);
        std::string outIndexpath = thisBasePath + "/t_index_info.csv";
        long long endTime = SPUtils::GetCurTime();
        long long testDuration = (endTime - startTime) / 1000;
        std::string screenStr = SPUtils::GetScreen();
        size_t pos3 = screenStr.find("=");
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
        std::map<std::string, std::string> cpuInfo = SPUtils::GetCpuInfo(isTcpMessage);
        std::map<std::string, std::string> gpuInfo = SPUtils::GetGpuInfo(isTcpMessage);
        std::map<std::string, std::string> destMap;
        destMap.insert(taskInfoMap.begin(), taskInfoMap.end());
        destMap.insert(deviceInfo.begin(), deviceInfo.end());
        destMap.insert(cpuInfo.begin(), cpuInfo.end());
        destMap.insert(gpuInfo.begin(), gpuInfo.end());
        OHOS::SmartPerf::SpCsvUtil::WriteCsvH(destMap);
        if (!vmap.empty()) {
            vmap.erase(vmap.begin());
        }
        OHOS::SmartPerf::SpCsvUtil::WriteCsv(outIndexpath, vmap);
    }
    isRunning = false;
    isInit = false;
    vmap.clear();
    if (thread.joinable()) {
        thread.join();
    }

    KillHiperfCmd();

    LOGI("SPTask::StopTask complete");
}

std::map<std::string, std::string> SPTask::DetectionAndGrab()
{
    std::map<std::string, std::string> templateMap;
    if (!curTaskInfo.stuckInfo.isEffective) {
        return templateMap;
    }

    FpsCurrentFpsTime fcf = FPS::GetInstance().GetFpsCurrentFpsTime();
    long long nowTime = SPUtils::GetCurTime();
    long long curframeTime = fcf.currentFpsTime / RM_1000000; // Convert to milliseconds
    std::cout << "start::" << startCaptuerTime << std::endl;

    if (startCaptuerTime > 0) {
        long long diff =
            startCaptuerTime > nowTime ? (LLONG_MAX - startCaptuerTime + nowTime) : (nowTime - startCaptuerTime);
        if (diff > RM_5000 && (!CheckCounterId())) {
            startCaptuerTime = RM_0;
        }
    }

    if (curTaskInfo.stuckInfo.fps > fcf.fps || curTaskInfo.stuckInfo.frameTime < curframeTime) {
        if (startCaptuerTime == 0) {
            startCaptuerTime = nowTime;
            std::cout << "ThreadGetHiperf::" << startCaptuerTime << std::endl;
            ThreadGetHiperf(startCaptuerTime);
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
    std::string hiprofilerCmd = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_CMD);
    SPUtils::LoadCmd(hiprofilerCmd, result);
    if (result.empty()) {
        return false;
    }

    if (result.find("-k") != std::string::npos) {
        return true;
    }

    return false;
}
std::thread SPTask::ThreadGetHiperf(long long timeStamp)
{
    auto thGetTrace = [this, timeStamp]() { this->GetHiperf(std::to_string(timeStamp)); };
    std::thread spThread(thGetTrace);
    spThread.detach();
    return spThread;
}

void SPTask::GetHiperf(const std::string &traceName)
{
    std::string result;
    std::string tmp = SetHiperf(traceName);
    std::cout << tmp << std::endl;
    LOGD("hiprofiler exec (%s)", tmp.c_str());
    LOGI("hiprofiler exec trace name(%s)", traceName.c_str());
    SPUtils::LoadCmd(tmp, result);
    LOGI("hiprofiler exec end (%s)", result.c_str());
}


bool SPTask::CheckTcpParam(std::string str, std::string &errorInfo)
{
    std::set<std::string> keys;
    for (auto a : COMMAND_MAP) {
        keys.insert(a.first.substr(1)); // 不需要前面的'-'
    }

    auto itr = keys.find("gc"); // editor tcp does not support gc
    if (keys.end() != itr) {
        keys.erase(itr);
    }

    return SPUtils::VeriyParameter(keys, str, errorInfo);
}

void SPTask::KillHiperfCmd()
{
    long long now = 0;
    long long runTime = 0;
    std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD) + "-9 ";
    std::string result;
    std::vector<std::string> out;

    if (startCaptuerTime <= 0) {
        return;
    }

    now = SPUtils::GetCurTime();
    runTime = now > startCaptuerTime ? now - startCaptuerTime : LLONG_MAX - startCaptuerTime + now;
    runTime = runTime / RM_1000; // Convert to seconds

    LOGI("Preparing to exit run time(%lld)", runTime);
    do {
        out.clear();
        std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
        SPUtils::LoadCmd(hiprofilerPid, result);
        SPUtils::StrSplit(result, " ", out);
        if (out.empty()) {
            break;
        }

        sleep(1);
    } while (END_WAITING_TIME - runTime++ > 0);

    out.clear();
    std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
    SPUtils::LoadCmd(hiprofilerPid, result);
    SPUtils::StrSplit(result, " ", out);
    LOGI("pidof hiprofiler_cmd size(%u)", out.size());
    for (auto it = out.begin(); out.end() != it; ++it) {
        result.clear();
        SPUtils::LoadCmd(killCmd + (*it), result);
    }

    return;
}

std::string SPTask::SetHiperf(const std::string &traceName)
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
