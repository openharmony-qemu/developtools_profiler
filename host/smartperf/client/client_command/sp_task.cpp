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
#include "include/sp_csv_util.h"
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
    SPUtils::LoadCmd(hiprofiler, result);
    result.clear();
    std::string perf = CMD_COMMAND_MAP.at(CmdCommand::PERF);
    SPUtils::LoadCmd(perf, result);
    std::cout << recvStr.substr(recvStr.find("-SESSIONID")) << std::endl;
    WLOGI("Received init task string: %s", recvStr.substr(recvStr.find("-SESSIONID")).c_str());
    ExceptionMsg exMsg = ParseToTask(recvStr, curTaskInfo);
    if (exMsg == ExceptionMsg::NO_ERR) {
        FPS &fps = FPS::GetInstance();
        fps.isGameApp = SPUtils::GetIsGameApp(curTaskInfo.packageName);
        fps.firstDump = true;
        isInit = true;
        LOGD("InitTask success, task initialized.");
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
                LOGD("Starting RAM collection in new thread");
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
                LOGD("Starting CPU collection in new thread");
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
                LOGD("Starting FPS collection in new thread");
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
        if (itConfig.find("-snapshot") != std::string::npos && screenshotFlag) {
            Capture::GetInstance().SocketMessage();
            std::map<std::string, std::string> captureMap = Capture::GetInstance().ItemData();
            dataMap.insert(captureMap.begin(), captureMap.end());
        }

        if (itConfig.find("-gc") != std::string::npos ||
            itConfig.find("-o") != std::string::npos ||
            itConfig.find("-lockfreq") != std::string::npos) {
            continue;
        }

        SpProfiler *profiler = SpProfilerFactory::GetCmdProfilerItem(COMMAND_MAP.at(itConfig), false);
        if (profiler != nullptr) {
            std::map<std::string, std::string> itemMap = profiler->ItemData();
            dataMap.insert(itemMap.begin(), itemMap.end());
        }
    }
}

void SPTask::ConfigDataThread()
{
    for (std::string itConfig : curTaskInfo.taskConfig) {
        if (!sdkData) {
            ConfigureSdkData(itConfig);
        }

        if (itConfig.find("-gc") != std::string::npos) {
            WLOGI("Starting GpuCounter collection in new thread");
            gpuCounter.StartCollect(GpuCounter::GC_START);
        }

        if (itConfig.find("-lockfreq") != std::string::npos) {
            lockFreq.SetIsCollecting(true);
            lockFreqThread = std::thread([this]() {
                WLOGI("Starting lock frequency locking thread");
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
        WLOGI("Starting sdkdata collection in new thread");
        sdk = std::thread([&sdkDataRecv, this]() { this->RunSdkServer(sdkDataRecv); });
    }
}

void SPTask::RunSdkServer(SdkDataRecv &sdkDataRecv)
{
    sdkDataRecv.ServerThread(sdkvec);
}

void SPTask::ResetSdkParam()
{
    WLOGI("Disabling sdk data collection and resetting parameters");
    OHOS::system::SetParameter("debug.smartperf.sdkdataenable", "0");
    sdkData = false;
    SdkDataRecv &sdkDataRecv = SdkDataRecv::GetInstance();
    sdkDataRecv.SetRunningState(false);
    int listenFd = sdkDataRecv.GetListenFd();
    if (listenFd != -1) {
        LOGD("Closing sdk data listenFd: %d", listenFd);
        close(listenFd);
        sdkDataRecv.SetListenFd(-1);
    }
    if (sdk.joinable()) {
        LOGD("Joining sdk data server thread");
        sdk.join();
    }
}

void SPTask::StopSdkRecv()
{
    if (!sdkData || sdkvec.size() <= 0) {
        WLOGI("SDK data is not enabled or sdkvec is empty");
        return;
    }
    LOGD("SDK data stop");
    std::string outSdkDataDir = baseOutPath + "/" + curTaskInfo.sessionId;
    char outSdkDataDirChar[PATH_MAX] = {0x00};
    if (realpath(outSdkDataDir.c_str(), outSdkDataDirChar) == nullptr) {
        WLOGE("data dir %s is nullptr", outSdkDataDir.c_str());
        return;
    }
    std::string outSdkDataPath = std::string(outSdkDataDirChar) + "/sdk_data.csv";
    sdkDataMtx.lock();
    std::ofstream outFile;
    outFile.open(outSdkDataPath.c_str(), std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        WLOGE("data %s open failed", outSdkDataPath.c_str());
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
    WLOGI("SDK data written successfully to %s", outSdkDataPath.c_str());
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

    for (const auto &file: files) {
        std::string filePath = fileDir + "/" + file;
        char filePathChar[PATH_MAX] = {0x00};
        if (realpath(filePath.c_str(), filePathChar) == nullptr) {
            LOGE("%s is not exist, init finish.", filePath.c_str());
            continue;
        }
        std::remove(filePathChar);
    }

    LOGD("Data file initialization completed.");
    return;
}

void SPTask::SetProfilerPid()
{
    SpProfilerFactory::editorFlag = true;
    std::string processId = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(curTaskInfo.packageName);
    SpProfilerFactory::SetProfilerPidByPkg(processId);
}

void SPTask::AsyncGetDataMap(std::function<void(std::string data)> msgTask)
{
    long long lastTime = SPUtils::GetCurTime();
    asyncDataMtx.lock();
    std::map<std::string, std::string> dataMap;
    if (!curTaskInfo.packageName.empty()) {
        SetProfilerPid();
    }
    dataMap.insert(std::pair<std::string, std::string>(std::string("timestamp"), std::to_string(lastTime)));
    std::future<std::map<std::string, std::string>> fpsResult = AsyncCollectFps();
    std::future<std::map<std::string, std::string>> cpuResult = AsyncCollectCpu();
    std::future<std::map<std::string, std::string>> ramResult = AsyncCollectRam();
    GetItemData(dataMap);
    CheckFutureFps(fpsResult, dataMap);
    CheckFutureCpu(cpuResult, dataMap);
    CheckFutureRam(ramResult, dataMap);
    if (curTaskInfo.stuckInfo.isEffective && recordTrace) {
        std::map<std::string, std::string> timeUsedMap = DetectionAndGrab();
        if (!timeUsedMap.empty()) {
            dataMap.insert(timeUsedMap.begin(), timeUsedMap.end());
        }
    }
    SPData spdata;
    spdata.values.insert(dataMap.begin(), dataMap.end());
    if (GetRecordState()) {
        ProcessErrorData(spdata);
    }
    gpuCounter.GetGpuRealtimeData(dataMap);
    SdkDataRecv::GetInstance().GetSdkDataRealtimeData(dataMap);
    msgTask(MapToString(dataMap));
    nextTime = SPUtils::GetCurTime();
    long long costTime = nextTime - lastTime;
    long long pTime = 998;
    if (costTime < curTaskInfo.freq) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pTime - costTime));
    }
    asyncDataMtx.unlock();
}

ErrCode SPTask::StartTask(std::function<void(std::string data)> msgTask)
{
    LOGD("Task starting...");
    RAM &ram = RAM::GetInstance();
    ram.SetFirstFlag();
    LOGD("RAM first flag set.");
    if (!isInit) {
        WLOGE("Initialization failed.");
        return ErrCode::FAILED;
    }
    isRunning = true;
    realTimeStart = SPUtils::GetCurTime();
    LOGD("Task initialized, realTimeStart = %lld", realTimeStart);
    if (!curTaskInfo.packageName.empty()) {
        SpProfilerFactory::SetProfilerPkg(curTaskInfo.packageName);
        LOGD("Profiler package name set to: %s", curTaskInfo.packageName.c_str());
    } else {
        LOGW("No package name provided for profiler.");
    }
    InitDataFile();
    LOGD("Data files initialized.");
    ConfigDataThread();
    LOGD("Data collection thread configuration completed.");
    thread = std::thread([this, msgTask]() {
        WLOGI("Starting data collection thread.");
        while (isRunning) {
            AsyncGetDataMap(msgTask);
        }
        WLOGI("Data collection thread exiting.");
    });
    return ErrCode::OK;
}

void SPTask::CreatPath(std::string path)
{
    if (!SPUtils::FileAccess(path)) {
        LOGD("CreatPath does not exist, attempting to create: %s", path.c_str());
        std::string cmdResult;
        std::string creatPath = CMD_COMMAND_MAP.at(CmdCommand::CREAT_DIR) + path;
        bool cmdSuccess = SPUtils::LoadCmd(creatPath, cmdResult);
        if (cmdSuccess) {
            LOGD("CreatPath created successfully: %s", path.c_str());
        } else {
            LOGE("Failed to create path: %s. Command result: %s", path.c_str(), cmdResult.c_str());
        }
    } else {
        LOGD("CreatPath already exists: %s", path.c_str());
    }
}

std::map<std::string, std::string> SPTask::SetTaskInfo()
{
    long long endTime = SPUtils::GetCurTime();
    long long testDuration = (endTime - startTime) / 1000;
    std::string refreshrate;
    LOGD("Test duration: %lld seconds", testDuration);
    const std::string gpuDataVersion = "1.1";
    std::string screenStr = SPUtils::GetScreen();
    size_t pos3 = screenStr.find("=");
    if (pos3 != std::string::npos) {
        refreshrate = screenStr.substr(pos3 + 1);
        LOGD("Screen refresh rate: %s", refreshrate.c_str());
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
        { "battery_change", std::to_string(battaryEnd - battaryStart) },
    };
    return taskInfoMap;
}

void SPTask::StopGetInfo()
{
    bool isTcpMessage = true;
    CreatPath(baseOutPath);
    std::string thisBasePath = baseOutPath + "/" + curTaskInfo.sessionId;
    CreatPath(thisBasePath);
    std::string outIndexpath = thisBasePath + "/t_index_info.csv";

    std::map<std::string, std::string> taskInfoMap = SetTaskInfo();
    std::map<std::string, std::string> deviceInfo = SPUtils::GetDeviceInfo();
    if (deviceInfo.empty()) {
        LOGW("Failed to get device info when stop.");
    }
    std::map<std::string, std::string> cpuInfo = SPUtils::GetCpuInfo(isTcpMessage);
    if (cpuInfo.empty()) {
        LOGW("Failed to get CPU info when stop.");
    }
    std::map<std::string, std::string> gpuInfo = SPUtils::GetGpuInfo(isTcpMessage);
    if (gpuInfo.empty()) {
        LOGW("Failed to get GPU info when stop.");
    }
    std::map<std::string, std::string> destMap;
    destMap.insert(taskInfoMap.begin(), taskInfoMap.end());
    destMap.insert(deviceInfo.begin(), deviceInfo.end());
    destMap.insert(cpuInfo.begin(), cpuInfo.end());
    destMap.insert(gpuInfo.begin(), gpuInfo.end());
    OHOS::SmartPerf::SpCsvUtil::WriteCsvH(destMap);
    OHOS::SmartPerf::SpCsvUtil::WriteCsv(outIndexpath, vmap);
    WLOGI("Write CSV header done.");
}
void SPTask::StopGpuCounterRecv()
{
    std::string outGpuCounterDataPath = baseOutPath + "/" + curTaskInfo.sessionId;

    if (GetRecordState()) {
        WLOGI("Record state is valid. Saving GPU counter data.");
        gpuCounter.GetInstance().SaveData(outGpuCounterDataPath);
    } else {
        WLOGW("Record state is invalid. GPU counter data will not be saved.");
    }
}
ErrCode SPTask::StopTask()
{
    if (GetRecordState()) {
        WLOGI("Record state is valid. Stopping task and cleaning up.");
        StopGetInfo();
        StopSdkRecv();
        StopGpuCounterRecv();

        vmap.clear();
        sdkvec.clear();
        gpuCounter.GetGpuCounterData().clear();
        recordState = false;
        screenshotFlag = false;
    } else {
        WLOGW("Record state is invalid. Skipping task stop operations.");
    }

    ResetSdkParam();
    gpuCounter.StopCollect();
    lockFreq.SetIsCollecting(false);
    if (lockFreqThread.joinable()) {
        LOGD("Joining lockFreqThread.");
        lockFreqThread.join();
    }

    WLOGI("Stopping task. isRunning: %d, isInit: %d", isRunning, isInit);
    isRunning = false;
    isInit = false;
    realTimeStart = 0;

    if (thread.joinable()) {
        LOGD("Joining thread.");
        thread.join();
    }
    LOGD("Killing Hiperf command.");
    SpProfilerFactory::editorFlag = false;
    WLOGI("Task successfully stopped.");
    return ErrCode::OK;
}

std::map<std::string, std::string> SPTask::DetectionAndGrab()
{
    std::map<std::string, std::string> templateMap;
    if (!curTaskInfo.stuckInfo.isEffective) {
        LOGE("StuckInfo is not effective. Returning empty map.");
        return templateMap;
    }

    FpsCurrentFpsTime fcf = FPS::GetInstance().GetFpsCurrentFpsTime();
    long long nowTime = SPUtils::GetCurTime();
    long long curframeTime = fcf.currentFpsTime / RM_1000000; // Convert to milliseconds
    LOGD("Start capture time: %lld", startCaptuerTime);

    if (startCaptuerTime > 0) {
        long long diff =
            startCaptuerTime > nowTime ? (LLONG_MAX - startCaptuerTime + nowTime) : (nowTime - startCaptuerTime);
        LOGD("Time difference: %lld ms", diff);

        if (diff > RM_5000 && (!CheckCounterId())) {
            LOGW("Time difference exceeded threshold, resetting start capture time.");
            startCaptuerTime = RM_0;
        }
    }

    if (curTaskInfo.stuckInfo.fps > fcf.fps || curTaskInfo.stuckInfo.frameTime < curframeTime) {
        if (startCaptuerTime == 0) {
            startCaptuerTime = nowTime;
            LOGD("ThreadGetHiperf::%ld", startCaptuerTime);
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
    LOGD("Loading hiprofiler command");
    SPUtils::LoadCmd(hiprofilerCmd, result);
    if (result.empty()) {
        LOGW("Failed to load hiprofiler command or received empty result.");
        return false;
    }

    if (result.find("-k") != std::string::npos) {
        LOGD("Command contains '-k'.");
        return true;
    }

    LOGD("Command does not contain '-k'.");
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
    LOGD("Start validating Init parameter: %s", params.c_str());

    for (auto a : COMMAND_MAP) {
        keys.insert(a.first.substr(1)); // 不需要前面的'-'
    }
    bool isValid = SPUtils::VeriyParameter(keys, params, errorInfo);
    if (isValid) {
        LOGD("Init parameter validation successful");
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
            break;
        }

        sleep(1);
    } while (END_WAITING_TIME - runTime++ > 0);

    out.clear();
    std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
    SPUtils::LoadCmd(hiprofilerPid, result);
    SPUtils::StrSplit(result, " ", out);
    LOGD("pidof hiprofiker_cmd size(%d)", out.size());
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

bool SPTask::GetRecordState()
{
    return recordState;
}
int SPTask::GetCurrentBattary()
{
    std::string content;
    const std::string cmd = "hidumper -s 3302 -a -i | grep capacity";
    SPUtils::LoadCmd(cmd, content);
    content = content.substr(content.find(':') + 1);
    if (content == "") {
        WLOGE("Battery capacity is empty.");
        return 0;
    }
    return SPUtilesTye::StringToSometype<int>(content);
}

ErrCode SPTask::StartRecord()
{
    battaryStart = GetCurrentBattary();
    startTime = SPUtils::GetCurTime();
    WLOGI("StartRecord initiated: Battery %d, Start time %lld.", battaryStart, startTime);
    while (startTime > nextTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    InitDataFile();
    screenshotFlag = true;
    recordState = true;
    recordTrace = true;
    return ErrCode::OK;
}

ErrCode SPTask::StopRecord()
{
    battaryEnd = GetCurrentBattary();
    long long stopRecordTime = SPUtils::GetCurTime();
    WLOGI("StopRecord initiated: Battery %d, Stop time %lld.", battaryEnd, stopRecordTime);
    while (stopRecordTime > nextTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    screenshotFlag = false;
    recordState = false;
    recordTrace = false;
    std::string outGpuCounterDataPath = baseOutPath + "/" + curTaskInfo.sessionId;

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
    KillHiperfCmd();

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
void SPTask::ProcessErrorData(SPData &spdata)
{
    unsigned int newDataSize = spdata.values.size();
    if (newDataSize > dataSize) {
        for (auto iter = spdata.values.cbegin(); iter != spdata.values.cend(); ++iter) {
            csvTitle += iter->first + ",";
        }
        if (!vmap.empty()) {
            vmap.pop_back();
        }
    }
    dataSize = newDataSize;
    vmap.push_back(spdata);
}
std::string SPTask::GetCsvTitle()
{
    return csvTitle;
}
void SPTask::SetRecordState(bool status)
{
    recordState = status;
}
}
}
