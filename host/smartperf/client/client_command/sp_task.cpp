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
#include "include/common.h"

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
            interval = SPUtilesTye::StringToSometype<long long>(args[++i]);
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_PKG)) {
            pkg = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_FL)) { // 获取用户fps的值，并赋给snf.   CT_FL
            snf.fps = SPUtilesTye::StringToSometype<int>(args[++i]);
            snf.isEffective = true;
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_FTL)) { // 获取frameTime的值      CT_FTL
            snf.frameTime = SPUtilesTye::StringToSometype<int>(args[++i]);
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
    std::string result = "";
    std::string hiprofiler = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER);
    LOGI("Loading hiprofiler command: %s", hiprofiler.c_str());
    SPUtils::LoadCmd(hiprofiler, result);
    result.clear();
    std::string perf = CMD_COMMAND_MAP.at(CmdCommand::PERF);
    LOGI("Loading perf command: %s", perf.c_str());
    SPUtils::LoadCmd(perf, result);
    std::cout << recvStr.substr(recvStr.find("-SESSIONID")) << std::endl;
    LOGI("Received init task string: %s", recvStr.substr(recvStr.find("-SESSIONID")).c_str());
    ExceptionMsg exMsg = ParseToTask(recvStr, curTaskInfo);
    if (exMsg == ExceptionMsg::NO_ERR) {
        FPS &fps = FPS::GetInstance();
        fps.isGameApp = SPUtils::GetIsGameApp(curTaskInfo.packageName);
        fps.firstDump = true;
        isInit = true;
        LOGI("InitTask success, task initialized.");
        return ErrCode::OK;
    }

    std::string errInfo = EXCEPTION_MSG_MAP.at(exMsg);
    LOGE("InitTask error(%s)", errInfo.c_str());
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
                LOGI("Starting RAM collection in new thread");
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
                LOGI("Starting CPU collection in new thread");
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
                LOGI("Starting FPS collection in new thread");
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
        LOGI("Future is valid, collecting RAM data");
        std::map<std::string, std::string> result = ramResult.get();
        dataMap.insert(result.begin(), result.end());
    } else {
        LOGE("Invalid future for RAM data");
    }
}

void SPTask::CheckFutureCpu(std::future<std::map<std::string, std::string>> &cpuResult,
    std::map<std::string, std::string> &dataMap)
{
    if (cpuResult.valid()) {
        LOGI("Future is valid, collecting CPU data");
        std::map<std::string, std::string> result = cpuResult.get();
        dataMap.insert(result.begin(), result.end());
    } else {
        LOGE("Invalid future for CPU data");
    }
}

void SPTask::CheckFutureFps(std::future<std::map<std::string, std::string>> &fpsResult,
    std::map<std::string, std::string> &dataMap)
{
    if (fpsResult.valid()) {
        LOGI("Future is valid, collecting FPS data");
        std::map<std::string, std::string> result = fpsResult.get();
        dataMap.insert(result.begin(), result.end());
    } else {
        LOGE("Invalid future for FPS data");
    }
}

void SPTask::GetItemData(std::map<std::string, std::string> &dataMap)
{
    std::string foundConfig;
    for (std::string itConfig : curTaskInfo.taskConfig) {
        if (itConfig.find("-snapshot") != std::string::npos && screenshotFlag) {
            LOGI("Snapshot flag is set, capturing screenshot for config: %s", itConfig.c_str());
            Capture::GetInstance().SocketMessage();
            std::map<std::string, std::string> captureMap = Capture::GetInstance().ItemData();
            dataMap.insert(captureMap.begin(), captureMap.end());
        }

        if (itConfig.find("-gc") != std::string::npos ||
            itConfig.find("-o") != std::string::npos ||
            itConfig.find("-lockfreq") != std::string::npos) {
            LOGI("Skipping config: %s", itConfig.c_str());
            continue;
        }

        SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(COMMAND_MAP.at(itConfig), false);
        if (profiler != nullptr) {
            foundConfig += itConfig + " ";
            std::map<std::string, std::string> itemMap = profiler->ItemData();
            dataMap.insert(itemMap.begin(), itemMap.end());
        }
    }
    LOGI("Retrieved profiler for config: %s", foundConfig.c_str());
}

void SPTask::ConfigDataThread()
{
    for (std::string itConfig : curTaskInfo.taskConfig) {
        if (!sdkData) {
            ConfigureSdkData(itConfig);
        }

        if (itConfig.find("-gc") != std::string::npos) {
            LOGI("Starting GpuCounter collection in new thread");
            gpuCounter.StartCollect(GpuCounter::GC_START);
        }

        if (itConfig.find("-lockfreq") != std::string::npos) {
            lockFreq.SetIsCollecting(true);
            lockFreqThread = std::thread([this]() {
                LOGI("Starting lock frequency locking thread");
                this->lockFreq.LockingThread();
            });
        }
    }
}

void SPTask::ConfigureSdkData(std::string itConfig)
{
    if (itConfig.find("-o") != std::string::npos) {
        sdkData = true;
        OHOS::system::SetParameter("debug.smartperf.sdkdataenable", "1");
        SdkDataRecv &sdkDataRecv = SdkDataRecv::GetInstance();
        sdkDataRecv.SetRunningState(true);
        LOGI("Starting sdkdata collection in new thread");
        sdk = std::thread([&sdkDataRecv, this]() { this->RunSdkServer(sdkDataRecv); });
    }
}

void SPTask::RunSdkServer(SdkDataRecv &sdkDataRecv)
{
    sdkDataRecv.ServerThread(sdkvec);
}

void SPTask::ResetSdkParam()
{
    LOGI("Disabling SDK data collection and resetting parameters");
    OHOS::system::SetParameter("debug.smartperf.sdkdataenable", "0");
    sdkData = false;
    SdkDataRecv &sdkDataRecv = SdkDataRecv::GetInstance();
    sdkDataRecv.SetRunningState(false);
    int listenFd = sdkDataRecv.GetListenFd();
    if (listenFd != -1) {
        LOGI("Closing listenFd: %d", listenFd);
        close(listenFd);
        sdkDataRecv.SetListenFd(-1);
    }
    if (sdk.joinable()) {
        LOGI("Joining SDK server thread");
        sdk.join();
    }
}

void SPTask::StopSdkRecv()
{
    if (!sdkData || sdkvec.size() <= 0) {
        LOGI("SDK data is not enabled or sdkvec is empty, returning early");
        return;
    }

    std::string outSdkDataDir = baseOutPath + "/" + curTaskInfo.sessionId;
    char outSdkDataDirChar[PATH_MAX] = {0x00};
    if (realpath(outSdkDataDir.c_str(), outSdkDataDirChar) == nullptr) {
        LOGE("data dir %s is nullptr", outSdkDataDir.c_str());
        return;
    }
    std::string outSdkDataPath = std::string(outSdkDataDirChar) + "/sdk_data.csv";
    sdkDataMtx.lock();
    std::ofstream outFile;
    outFile.open(outSdkDataPath.c_str(), std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        LOGE("data %s open failed", outSdkDataPath.c_str());
        sdkDataMtx.unlock();
        return;
    }
    std::string title = "source,timestamp,eventName,enable,value\r";
    outFile << title << std::endl;
    for (const auto &item : sdkvec) {
        outFile << item << std::endl;
    }
    outFile.close();
    sdkDataMtx.unlock();
    LOGI("SDK data written successfully to %s", outSdkDataPath.c_str());
    return;
}

void SPTask::InitDataFile()
{
    vmap.clear();
    sdkvec.clear();
    gpuCounter.GetGpuCounterSaveReportData().clear();
    SdkDataRecv::GetInstance().SetStartRecordTime();
    startTime = SPUtils::GetCurTime();
    std::vector<std::string> files = {
        "sdk_data.csv",
        "gpu_counter.csv",
        "t_general_info.csv",
        "t_index_info.csv",
    };
    std::string fileDir = baseOutPath + "/" + curTaskInfo.sessionId;
    LOGI("Initializing data files in directory: %s", fileDir.c_str());

    for (const auto &file : files) {
        std::string filePath = fileDir + "/" + file;
        char filePathChar[PATH_MAX] = {0x00};
        if (realpath(filePath.c_str(), filePathChar) == nullptr) {
            LOGE("File %s does not exist, skipping deletion.", filePath.c_str());
            continue;
        }

        if (std::remove(filePathChar) == 0) {
            LOGI("Successfully removed file: %s", filePath.c_str());
        } else {
            LOGE("Failed to remove file: %s", filePath.c_str());
        }
    }

    LOGI("Data file initialization completed.");
    return;
}

void SPTask::SetProfilerPid()
{
    SpProfilerFactory::editorFlag = true;
    std::string processId = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(curTaskInfo.packageName);
    SpProfilerFactory::SetProfilerPidByPkg(processId);
    LOGI("Set process ID for package: %s, process ID: %s",
          curTaskInfo.packageName.c_str(), processId.c_str());
}

void SPTask::AsyncGetDataMap(std::function<void(std::string data)> msgTask)
{
    long long lastTime = SPUtils::GetCurTime();
    LOGD("Start AsyncGetDataMap processing");
    asyncDataMtx.lock();
    std::map<std::string, std::string> dataMap;
    if (!curTaskInfo.packageName.empty()) {
        SetProfilerPid();
    }
    dataMap.insert(std::pair<std::string, std::string>(std::string("timestamp"), std::to_string(lastTime)));
    LOGD("Timestamp added to dataMap: %lld", lastTime);
    std::future<std::map<std::string, std::string>> fpsResult = AsyncCollectFps();
    LOGD("FPS data collection initiated.");
    std::future<std::map<std::string, std::string>> cpuResult = AsyncCollectCpu();
    LOGD("CPU data collection initiated.");
    GetItemData(dataMap);
    std::future<std::map<std::string, std::string>> ramResult = AsyncCollectRam();
    LOGD("RAM data collection initiated.");
    CheckFutureFps(fpsResult, dataMap);
    CheckFutureCpu(cpuResult, dataMap);
    CheckFutureRam(ramResult, dataMap);
    if (curTaskInfo.stuckInfo.isEffective) {
        std::map<std::string, std::string> timeUsedMap = DetectionAndGrab();
        if (!timeUsedMap.empty()) {
            dataMap.insert(timeUsedMap.begin(), timeUsedMap.end());
            LOGD("Stuck detection data added to dataMap.");
        }
    }
    SPData spdata;
    spdata.values.insert(dataMap.begin(), dataMap.end());
    if (GetRecordState()) {
        vmap.push_back(spdata);
        LOGD("Data added to vmap for recording.");
    }
    gpuCounter.GetGpuRealtimeData(dataMap);
    LOGD("GPU real-time data added to dataMap.");
    SdkDataRecv::GetInstance().GetSdkDataRealtimeData(dataMap);
    LOGD("SDK real-time data added to dataMap.");
    msgTask(MapToString(dataMap));
    LOGD("Data map converted to string and passed to msgTask.");
    nextTime = SPUtils::GetCurTime();
    long long costTime = nextTime - lastTime;
    long long pTime = 998;
    LOGD("Time spent on data collection: %lld ms", costTime);
    if (costTime < curTaskInfo.freq) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pTime - costTime));
        LOGD("Sleeping for %lld ms to maintain frequency.", pTime - costTime);
    }
    asyncDataMtx.unlock();
    LOGD("Data collection process completed a second loop.");
}

ErrCode SPTask::StartTask(std::function<void(std::string data)> msgTask)
{
    LOGI("Task starting...");
    RAM &ram = RAM::GetInstance();
    ram.SetFirstFlag();
    LOGI("RAM first flag set.");
    if (!isInit) {
        LOGE("Initialization failed.");
        return ErrCode::FAILED;
    }
    isRunning = true;
    realTimeStart = SPUtils::GetCurTime();
    LOGI("Task initialized, realTimeStart = %lld", realTimeStart);
    if (!curTaskInfo.packageName.empty()) {
        SpProfilerFactory::SetProfilerPkg(curTaskInfo.packageName);
        LOGI("Profiler package name set to: %s", curTaskInfo.packageName.c_str());
    } else {
        LOGW("No package name provided for profiler.");
    }
    InitDataFile();
    LOGI("Data files initialized.");
    ConfigDataThread();
    LOGI("Data collection thread configuration completed.");
    thread = std::thread([this, msgTask]() {
        LOGI("Starting data collection thread.");
        while (isRunning) {
            AsyncGetDataMap(msgTask);
        }
        LOGI("Data collection thread exiting.");
    });
    return ErrCode::OK;
}

void SPTask::CreatPath(std::string path)
{
    if (!SPUtils::FileAccess(path)) {
        LOGW("Path does not exist, attempting to create: %s", Path.c_str());
        std::string cmdResult;
        std::string creatPath = CMD_COMMAND_MAP.at(CmdCommand::CREAT_DIR) + path;
        bool cmdSuccess = SPUtils::LoadCmd(creatPath, cmdResult);
        if (cmdSuccess) {
            LOGI("Path created successfully: %s", Path.c_str());
        } else {
            LOGE("Failed to create path: %s. Command result: %s", Path.c_str(), cmdResult.c_str());
        }
    } else {
        LOGI("Path already exists: %s", Path.c_str());
    }
}

std::map<std::string, std::string> SPTask::SetTaskInfo()
{
    long long endTime = SPUtils::GetCurTime();
    long long testDuration = (endTime - startTime) / 1000;
    std::string refreshrate;
    LOGI("Test duration: %lld seconds", testDuration);
    const std::string gpuDataVersion = "1.1";
    std::string screenStr = SPUtils::GetScreen();
    size_t pos3 = screenStr.find("=");
    if (pos3 != std::string::npos) {
        refreshrate = screenStr.substr(pos3 + 1);
        LOGI("Screen refresh rate: %s", refreshrate.c_str());
    } else {
        LOGW("Failed to extract refresh rate from screen string: %s", screenStr.c_str());
    }

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
        { "gpuDataVersion", gpuDataVersion },
        { "battery_change", std::to_string(battaryStart - battaryEnd) },
    };
    return taskInfoMap;
}

void SPTask::StopGetInfo()
{
    bool isTcpMessage = true;
    LOGI("Creating base output path: %s", baseOutPath.c_str());
    CreatPath(baseOutPath);
    std::string thisBasePath = baseOutPath + "/" + curTaskInfo.sessionId;
    LOGI("Creating session path: %s", thisBasePath.c_str());
    CreatPath(thisBasePath);
    std::string outIndexpath = thisBasePath + "/t_index_info.csv";

    std::map<std::string, std::string> taskInfoMap = SetTaskInfo();
    std::map<std::string, std::string> deviceInfo = SPUtils::GetDeviceInfo();
    if (deviceInfo.empty()) {
        LOGW("Failed to get device info.");
    }
    std::map<std::string, std::string> cpuInfo = SPUtils::GetCpuInfo(isTcpMessage);
    if (cpuInfo.empty()) {
        LOGW("Failed to get CPU info.");
    }
    std::map<std::string, std::string> gpuInfo = SPUtils::GetGpuInfo(isTcpMessage);
    if (gpuInfo.empty()) {
        LOGW("Failed to get GPU info.");
    }
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

    LOGI("Write CSV header done.");
}
void SPTask::StopGpuCounterRecv()
{
    std::string outGpuCounterDataPath = baseOutPath + "/" + curTaskInfo.sessionId;
    LOGI("GPU counter data will be saved to: %s", outGpuCounterDataPath.c_str());

    if (GetRecordState()) {
        LOGI("Record state is valid. Saving GPU counter data.");
        gpuCounter.GetInstance().SaveData(outGpuCounterDataPath);
    } else {
        LOGW("Record state is invalid. GPU counter data will not be saved.");
    }
}
ErrCode SPTask::StopTask()
{
    LOGI("Attempting to stop task. Current record state: %d", GetRecordState());
    if (GetRecordState()) {
        LOGI("Record state is valid. Stopping task and cleaning up.");
        StopGetInfo();
        StopSdkRecv();
        StopGpuCounterRecv();

        vmap.clear();
        sdkvec.clear();
        gpuCounter.GetGpuCounterData().clear();
        recordState = false;
        screenshotFlag = false;
    } else {
        LOGW("Record state is invalid. Skipping task stop operations.");
    }

    LOGI("Resetting SDK parameters and stopping GPU collection.");
    ResetSdkParam();
    gpuCounter.StopCollect();
    lockFreq.SetIsCollecting(false);
    if (lockFreqThread.joinable()) {
        LOGI("Joining lockFreqThread.");
        lockFreqThread.join();
    } else {
        LOGW("lockFreqThread is not joinable.");
    }

    LOGI("Stopping task. isRunning: %d, isInit: %d", isRunning, isInit);
    isRunning = false;
    isInit = false;
    realTimeStart = 0;

    if (thread.joinable()) {
        LOGI("Joining main thread.");
        thread.join();
    } else {
        LOGW("Main thread is not joinable.");
    }
    LOGI("Killing Hiperf command.");
    KillHiperfCmd();
    SpProfilerFactory::editorFlag = false;
    LOGI("Task successfully stopped.");
    return ErrCode::OK;
}

std::map<std::string, std::string> SPTask::DetectionAndGrab()
{
    std::map<std::string, std::string> templateMap;
    if (!curTaskInfo.stuckInfo.isEffective) {
        LOGW("StuckInfo is not effective. Returning empty map.");
        return templateMap;
    }

    FpsCurrentFpsTime fcf = FPS::GetInstance().GetFpsCurrentFpsTime();
    long long nowTime = SPUtils::GetCurTime();
    long long curframeTime = fcf.currentFpsTime / RM_1000000; // Convert to milliseconds
    std::cout << "start::" << startCaptuerTime << std::endl;
    LOGI("Start capture time: %lld", startCaptuerTime);

    if (startCaptuerTime > 0) {
        long long diff =
            startCaptuerTime > nowTime ? (LLONG_MAX - startCaptuerTime + nowTime) : (nowTime - startCaptuerTime);
        LOGI("Time difference: %lld ms", diff);

        if (diff > RM_5000 && (!CheckCounterId())) {
            LOGW("Time difference exceeded threshold, resetting start capture time.");
            startCaptuerTime = RM_0;
        }
    }

    if (curTaskInfo.stuckInfo.fps > fcf.fps || curTaskInfo.stuckInfo.frameTime < curframeTime) {
        if (startCaptuerTime == 0) {
            LOGI("FPS or frame time threshold exceeded, starting capture thread.");
            startCaptuerTime = nowTime;
            std::cout << "ThreadGetHiperf::" << startCaptuerTime << std::endl;
            LOGI("ThreadGetHiperf::%ld", startCaptuerTime);
            ThreadGetHiperf(startCaptuerTime);
        }
    }
    templateMap["fpsWarn"] = std::to_string(fcf.fps);
    templateMap["FrameTimeWarn"] = std::to_string(fcf.currentFpsTime);
    templateMap["TraceTime"] = std::to_string(startCaptuerTime);
    LOGI("Returning templateMap: fpsWarn = %s, FrameTimeWarn = %s, TraceTime = %s",
          templateMap["fpsWarn"].c_str(), templateMap["FrameTimeWarn"].c_str(), templateMap["TraceTime"].c_str());

    return templateMap;
}

bool SPTask::CheckCounterId()
{
    std::string result;
    std::string hiprofilerCmd = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_CMD);
    LOGI("Loading hiprofiler command: %s", hiprofilerCmd.c_str());
    SPUtils::LoadCmd(hiprofilerCmd, result);
    if (result.empty()) {
        LOGW("Failed to load hiprofiler command or received empty result.");
        return false;
    }

    if (result.find("-k") != std::string::npos) {
        LOGI("Command result contains '-k'. Returning true.");
        return true;
    }

    LOGI("Command result does not contain '-k'. Returning false.");
    return false;
}
std::thread SPTask::ThreadGetHiperf(long long timeStamp)
{
    LOGI("Starting new thread with timestamp: %lld", timeStamp);
    auto thGetTrace = [this, timeStamp]() { this->GetHiperf(std::to_string(timeStamp)); };
    std::thread spThread(thGetTrace);
    LOGI("Detaching thread for timestamp: %lld", timeStamp);
    spThread.detach();
    return spThread;
}

void SPTask::GetHiperf(const std::string &traceName)
{
    std::string result;
    std::string tmp = SetHiperf(traceName);
    LOGI("Constructed hiprofiler command: %s", tmp.c_str());
    std::cout << tmp << std::endl;
    SPUtils::LoadCmd(tmp, result);
    LOGD("hiprofiler exec (%s), hiprofiler exec trace name(%s), hiprofiler exec end (%s)",
        tmp.c_str(), traceName.c_str(), result.c_str());
}


bool SPTask::CheckTcpParam(std::string str, std::string &errorInfo)
{
    std::set<std::string> keys;
    std::string params;
    if (str.find("-SESSIONID") != std::string::npos) {
        params = str.substr(str.find("-SESSIONID"));
    } else {
        LOGE("Init parameter error not contain '-SESSIONID'");
        return false;
    }
    LOGI("Start validating Init parameter: %s", params.c_str());

    for (auto a : COMMAND_MAP) {
        keys.insert(a.first.substr(1)); // 不需要前面的'-'
    }
    bool isValid = SPUtils::VeriyParameter(keys, params, errorInfo);
    if (isValid) {
        LOGI("Init parameter validation successful");
    } else {
        LOGE("Init parameter validation failed, Error: %s", errorInfo.c_str());
    }
    return isValid;
}

void SPTask::KillHiperfCmd()
{
    long long now = 0;
    long long runTime = 0;
    std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD) + "-9 ";
    std::string result;
    std::vector<std::string> out;

    if (startCaptuerTime <= 0) {
        LOGW("startCaptuerTime is invalid: %lld, returning from KillHiperfCmd.", startCaptuerTime);
        return;
    }

    now = SPUtils::GetCurTime();
    runTime = now > startCaptuerTime ? now - startCaptuerTime : LLONG_MAX - startCaptuerTime + now;
    runTime = runTime / RM_1000; // Convert to seconds

    LOGD("Preparing to exit run time(%lld)", runTime);
    do {
        out.clear();
        std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
        SPUtils::LoadCmd(hiprofilerPid, result);
        SPUtils::StrSplit(result, " ", out);
        if (out.empty()) {
            LOGW("No hiprofiler PID found, exiting loop.");
            break;
        } else {
            LOGI("Found hiprofiler PID(s): %s", result.c_str());
        }

        sleep(1);
    } while (END_WAITING_TIME - runTime++ > 0);

    out.clear();
    std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
    SPUtils::LoadCmd(hiprofilerPid, result);
    SPUtils::StrSplit(result, " ", out);

    if (!out.empty()) {
        LOGI("Killing hiprofiler process with PIDs: %s", result.c_str());
    } else {
        LOGW("No hiprofiler PID found after waiting, nothing to kill.");
    }

    LOGD("pidof hiprofiker_cmd size(%d)", out.size());
    for (auto it = out.begin(); out.end() != it; ++it) {
        result.clear();
        SPUtils::LoadCmd(killCmd + (*it), result);
        LOGI("Executed kill command: %s", (killCmd + (*it)).c_str());
    }

    return;
}

std::string SPTask::SetHiperf(const std::string &traceName)
{
    std::string hiPrefix = "hiprofiler_";
    std::string dataPrefix = "perf_";
    requestId++;
    LOGD("Setting hiprofiler trace with traceName: %s, current requestId: %lld", traceName.c_str(), requestId);

    std::string trtmp = strOne + hiPrefix + traceName + strTwo + "\n" + strThree + std::to_string(requestId) + "\n" +
        strFour + "\n" + strFive + hiPrefix + traceName + strSix + "\n" + strNine + strEleven + "\n" + strSeven +
        dataPrefix + traceName + strEight + strTen + "\n" + conFig;
    LOGD("Constructed hiprofiler command: %s", trtmp.c_str());
    return trtmp;
}

bool SPTask::GetRecordState()
{
    return recordState;
}
int SPTask::GetCurrentBattary()
{
    std::string content;
    const std::string  cmd = "hidumper -s 3302 -a -i | grep capacity";
    SPUtils::LoadCmd(cmd, content);

    if (content.empty()) {
        LOGE("Failed to retrieve battery capacity: Command output is empty.");
        return 0;
    }

    content = content.substr(content.find(':') + 1);
    if (content == "") {
        LOGE("Battery capacity parsing failed: No valid data after colon.");
        return 0;
    }
    return SPUtilesTye::StringToSometype<int>(content);
}

ErrCode SPTask::StartRecord()
{
    LOGD("SPTask StartRecord");
    battaryStart = GetCurrentBattary();
    if (battaryStart == 0) {
        LOGE("StartRecord failed: Battery level is 0. Cannot start recording.");
        return ErrCode::FAILED;
    }

    startTime = SPUtils::GetCurTime();
    LOGI("StartRecord initiated: Battery level %d, Start time %lld.", battaryStart, startTime);

    while (startTime > nextTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    InitDataFile();
    screenshotFlag = true;
    recordState = true;
    LOGI("Recording started: Screenshot flag set to %d, Record state set to %d.", screenshotFlag, recordState);
    return ErrCode::OK;
}

ErrCode SPTask::StopRecord()
{
    LOGD("SPTask StopRecord");
    battaryEnd = GetCurrentBattary();
    if (battaryEnd == 0) {
        LOGE("StopRecord failed: Battery level is 0. Cannot stop recording.");
        return ErrCode::FAILED;
    }

    long long stopRecordTime = SPUtils::GetCurTime();
    LOGI("StopRecord initiated: Battery end level %d, Stop time %lld.", battaryEnd, stopRecordTime);
    while (stopRecordTime > nextTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    screenshotFlag = false;
    recordState = false;
    LOGI("Recording stopped: Screenshot flag set to %d, Record state set to %d.", screenshotFlag, recordState);
    std::string outGpuCounterDataPath = baseOutPath + "/" + curTaskInfo.sessionId;
    LOGI("Saving GPU counter data to path: %s", outGpuCounterDataPath.c_str());

    if (isInit) {
        StopGetInfo();
        if (sdkData) {
            StopSdkRecv();
        }
        gpuCounter.GetInstance().SaveData(outGpuCounterDataPath);
    }

    vmap.clear();
    sdkvec.clear();
    gpuCounter.GetGpuCounterData().clear();
    Capture::GetInstance().SetCollectionNum();

    return ErrCode::OK;
}
time_t SPTask::GetRealStartTime() const
{
    return realTimeStart;
}
void SPTask::SetTcpToken(std::string token)
{
    tcpToken = token;
}
std::string SPTask::GetTcpToken()
{
    return tcpToken;
}
}
}
