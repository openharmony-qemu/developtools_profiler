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
#include "include/FileDescriptor.h"
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
#include "include/sp_thread_socket.h"
#include "effective.h"
namespace OHOS {
namespace SmartPerf {
const long long RM_1000 = 1000;
const long long END_WAITING_TIME = 8; // End waiting time,unit seconds
std::vector<std::string> ParseCommandArgs(std::string &command)
{
    std::vector<std::string> args;
    size_t pos = 0;
    while ((pos = command.find(" ")) != std::string::npos) {
        args.push_back(command.substr(0, pos));
        command.erase(0, pos + 1);
    }
    args.push_back(command);
    return args;
}

// init::-SESSIONID 12345678 -INTERVAL 1000 -PKG ohos.samples.ecg -c -g -t -p -f -r -fl 30
static ExceptionMsg ParseToTask(std::string command, TaskInfo &taskInfo)
{
    StuckNotification snf;
    snf.isEffective = false;
    std::string sessionId;
    long long interval = 1000;
    std::string pkg;
    std::string pid;
    bool isFPS = false;
    bool isPrint = false;
    std::vector<std::string> configs;
    std::vector<std::string> args = ParseCommandArgs(command);
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_SESSIONID)) {
            sessionId = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_INTERVAL)) {
            interval = SPUtilesTye::StringToSometype<long long>(args[++i]);
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_PKG)) {
            pkg = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_PID)) {
            pid = args[++i];
        } else if (args[i] == COMMAND_MAP_REVERSE.at(CommandType::CT_PRINT)) {
            isPrint = true;
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
    taskInfo = { sessionId, pkg, pid, configs, interval, snf, isPrint };
    return ExceptionMsg::NO_ERR;
}

ErrCode SPTask::InitTask(const std::string &recvStr)
{
    std::string result = "";
    const std::string hiprofiler = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER);
    SPUtils::LoadCmd(hiprofiler, result);
    result.clear();
    const std::string perf = CMD_COMMAND_MAP.at(CmdCommand::PERF);
    SPUtils::LoadCmd(perf, result);
    std::cout << recvStr.substr(recvStr.find("-SESSIONID")) << std::endl;
    WLOGI("Received init task string: %s", recvStr.substr(recvStr.find("-SESSIONID")).c_str());
    ExceptionMsg exMsg = ParseToTask(recvStr, curTaskInfo);
    if (exMsg == ExceptionMsg::NO_ERR) {
        taskMgr_ = std::make_shared<TaskManager>();
        taskMgr_->AddTask(recvStr);
        if (curTaskInfo.stuckInfo.isEffective) {
            if (curTaskInfo.stuckInfo.fps != 0) {
                Effective::GetInstance().fps_ = curTaskInfo.stuckInfo.fps;
            } else {
                Effective::GetInstance().frameTime_ = curTaskInfo.stuckInfo.frameTime;
            }
        }
        SetAppInitFlag();
        LOGD("InitTask success, task initialized.");
        return ErrCode::OK;
    }

    std::string errInfo = EXCEPTION_MSG_MAP.at(exMsg);
    LOGE("InitTask error(%s)", errInfo.c_str());
    return ErrCode::FAILED;
}

void SPTask::ConfigDataThread()
{
    for (std::string& itConfig : curTaskInfo.taskConfig) {
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
    std::string processIds = "";
    OHOS::SmartPerf::StartUpDelay sp;
    processId = sp.GetPidByPkg(curTaskInfo.packageName, &processIds);
    SpProfilerFactory::SetProfilerPidByPkg(processId, processIds);
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
    InitDataFile();
    LOGD("Data files initialized.");
    ConfigDataThread();
    LOGD("Data collection thread configuration completed.");
    gameEvent.RegisterGameEvent();
    thread = std::thread([=]() {
        if (taskMgr_ == nullptr) {
            return;
        }
        WLOGI("Starting data collection thread.");
        CreatPath(baseOutPath);
        std::string thisBasePath = baseOutPath + "/" + curTaskInfo.sessionId;
        CreatPath(thisBasePath);
        std::string outIndexpath = thisBasePath + "/t_index_info.csv";
        GpuCounter::GetInstance().SetSavePathDirectory(thisBasePath);

        taskMgr_->SetFilePath(outIndexpath);
        taskMgr_->EnableIPCCallback(msgTask);
        taskMgr_->SetNextTime(nextTime);
        taskMgr_->Start();
        taskMgr_->Wait();
    });
    EnablePrint();
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
        { "pid", curTaskInfo.pid },
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

    WLOGI("Write CSV header done.");
}

ErrCode SPTask::StopTask()
{
    if (taskMgr_ != nullptr) {
        taskMgr_->Stop();
        taskMgr_->WriteToCSV();
    }
    if (GetRecordState()) {
        WLOGI("Record state is valid. Stopping task and cleaning up.");
        StopGetInfo();
        StopSdkRecv();

        sdkvec.clear();
        recordState = false;
    } else {
        WLOGW("Record state is invalid. Skipping task stop operations.");
    }

    ResetSdkParam();
    lockFreq.SetIsCollecting(false);
    if (lockFreqThread.joinable()) {
        LOGD("Joining lockFreqThread.");
        lockFreqThread.join();
    }
    gameEvent.UnregisterGameEvent();

    WLOGI("Stopping task. isRunning: %d, isInit: %d", isRunning, isInit);
    isRunning = false;
    isInit = false;
    realTimeStart = 0;
    if (stdOutLog >= 0) {
        close(stdOutLog);
        stdOutLog = -1;
    }
    if (thread.joinable()) {
        LOGD("Joining thread.");
        thread.join();
    }
    LOGD("Killing Hiperf command.");
    SpProfilerFactory::editorFlag = false;
    WLOGI("Task successfully stopped.");
    return ErrCode::OK;
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

    for (auto& a : COMMAND_MAP) {
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
    const std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD) + "-9 ";
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
        const std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
        SPUtils::LoadCmd(hiprofilerPid, result);
        SPUtils::StrSplit(result, " ", out);
        if (out.empty()) {
            break;
        }

        sleep(1);
    } while (END_WAITING_TIME - runTime++ > 0);

    out.clear();
    const std::string hiprofilerPid = CMD_COMMAND_MAP.at(CmdCommand::HIPROFILER_PID);
    SPUtils::LoadCmd(hiprofilerPid, result);
    SPUtils::StrSplit(result, " ", out);
    LOGD("pidof hiprofiker_cmd size(%d)", out.size());
    for (auto it = out.begin(); out.end() != it; ++it) {
        result.clear();
        SPUtils::LoadCmd(killCmd + (*it), result);
    }

    return;
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
    SetStartFlag();
    if (taskMgr_ != nullptr) {
        taskMgr_->SetRecordState(true);
    }
    return ErrCode::OK;
}

void SPTask::SetStartFlag()
{
    InitDataFile();
    recordState = true;
}

ErrCode SPTask::StopRecord()
{
    battaryEnd = GetCurrentBattary();
    long long stopRecordTime = SPUtils::GetCurTime();
    WLOGI("StopRecord initiated: Battery %d, Stop time %lld.", battaryEnd, stopRecordTime);
    while (stopRecordTime > nextTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ClearStopFlag();
    if (taskMgr_ != nullptr) {
        taskMgr_->SetRecordState(false);
    }
    return ErrCode::OK;
}

void SPTask::ClearStopFlag()
{
    recordState = false;
    std::string outGpuCounterDataPath = baseOutPath + "/" + curTaskInfo.sessionId;

    if (isInit) {
        StopGetInfo();
        if (sdkData) {
            StopSdkRecv();
        }
        gpuCounter.GetInstance().SaveData(outGpuCounterDataPath);
    }

    sdkvec.clear();
    gpuCounter.GetGpuCounterData().clear();
    Capture::GetInstance().SetCollectionNum();
    KillHiperfCmd();
}

time_t SPTask::GetRealStartTime() const
{
    return realTimeStart;
}

std::string SPTask::GetCsvTitle()
{
    return csvTitle;
}
void SPTask::SetRecordState(bool status)
{
    recordState = status;
}
void SPTask::EnablePrint()
{
    if (curTaskInfo.isPrint) {
        stdOutLog = SPUtils::GetTtyDeviceFd();
        if (dup2(stdOutLog, STDERR_FILENO) < 0) {
            LOGE("Dup2 fail");
        } else {
            close(stdOutLog);
            stdOutLog = -1;
        }
    } else {
        int &fd = SPUtils::GetTtyDeviceFd();
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }
}

void SPTask::ConfigureSdkDataHap()
{
    if (isSdkTask) {
        sdkData = true;
        OHOS::system::SetParameter("debug.smartperf.sdkdataenable", "1");
        SdkDataRecv &sdkDataRecv = SdkDataRecv::GetInstance();
        sdkDataRecv.SetRunningState(true);
        WLOGI("Starting sdkdata collection in new thread");
        sdk = std::thread([&sdkDataRecv, this]() { this->RunSdkServer(sdkDataRecv); });
    }
}

void SPTask::SetAppCmd(std::string &recvBuf)
{
    std::stringstream ssLine(recvBuf);
    std::string word = "";
    int count = 0;
    int counter = 3;
    while (ssLine >> word) {
        count++;
        if (count == counter) {
            curTaskInfo.packageName = word;
        } else if (count > counter) {
            if  (word.find(":::") != std::string::npos) {
                size_t pos = word.find(":::");
                word = word.substr(0, pos);
            }
            curTaskInfo.taskConfig.push_back(word);
        }
    }
}

TaskInfo SPTask::GetCurTaskInfo()
{
    return curTaskInfo;
}

void SPTask::SetAppReceiveDataOffFlag()
{
    appReceiveDataFlag = true;
}

void SPTask::SetAppReceiveDataOnFlag()
{
    appReceiveDataFlag = false;
}

void SPTask::SetAppInitFlag()
{
    isInit = true;
    if (!GetCurTaskInfo().packageName.empty()) {
        SetProfilerPid();
        SpProfilerFactory::SetProfilerPkg(curTaskInfo.packageName);
    }
}

std::string SPTask::ProcessingCollectionsMap(SPData spdata)
{
    std::string appCollectMap = "t_index_info$$";
    std::vector<std::string> keysToFind = {"fps", "refreshrate", "currentNow", "ddrFrequency", "cpu0Frequency",
        "cpu1Frequency", "cpu2Frequency", "cpu3Frequency", "cpu4Frequency", "cpu5Frequency", "cpu6Frequency",
        "cpu7Frequency", "cpu8Frequency", "cpu9Frequency", "cpu10Frequency", "cpu11Frequency", "gpuFrequency",
        "pss", "shell_frame", "shell_back", "soc_thermal", "cpu0Usage", "cpu1Usage", "cpu2Usage", "cpu3Usage",
        "cpu4Usage", "cpu5Usage", "cpu6Usage", "cpu7Usage", "cpu8Usage", "cpu9Usage", "cpu10Usage", "cpu11Usage",
        "gpuLoad"};
    for (const auto& key : keysToFind) {
        if (spdata.values.find(key) != spdata.values.end()) {
            appCollectMap += key + ":" + spdata.values[key] + ",";
        }
    }
    return appCollectMap;
}
}
}
