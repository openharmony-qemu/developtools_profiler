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
#include "include/GpuCounter.h"
#include <iostream>
#include <fstream>
#include <climits>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <chrono>
#include "include/sp_utils.h"
#include "include/sp_log.h"

namespace OHOS {
namespace SmartPerf {
const long long WAIT_EXIT_TIME = 500;
const long long WAIT_RUN_TIME = 200;
const int ERROR_CODE_NEGATIVE_TWO = -2;
const int ERROR_CODE_NEGATIVE_THREE = -3;
const int ERROR_CODE_NEGATIVE_FOUR = -4;

GpuCounter::GpuCounter()
{
    originalEP = GetPerm();
    LOGI("original execute permissions(%d)", static_cast<int>(originalEP));
}

std::map<std::string, std::string> GpuCounter::ItemData()
{
    std::map<std::string, std::string> result;

    if (initCheckPath == "/data/local/tmp/" && (gcStatus == GC_START || gcStatus == GC_INIT) && (!initMap.empty())) {
        result.insert(initMap.begin(), initMap.end());
        initMap.clear();
        return result;
    }

    gcStatus = GC_STOP;
    KillCounter();

    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_EXIT_TIME));

    // After 'kill -9', a CSV file will also be generated
    if (SPUtils::FileAccess(constOutSourCVSFile)) {
        std::string outStr;
        std::string newFileName = constOutDestCVSPrefix + "_" + std::to_string(SPUtils::GetCurTime()) + ".csv";
        std::string mvCmd = constMvFile + constOutSourCVSFile + "  " + sandBoxPath + newFileName;
        LOGI("ItemData new file name(%s)", newFileName.c_str());
        if (sandBoxPath != "/data/local/tmp/" && (!isSandBoxWrite)) {
            SetPerm(EP_PERMISSIVE);
            SPUtils::LoadCmd(mvCmd, outStr);
            SetPerm(EP_ENFORCING);
        } else {
            SPUtils::LoadCmd(mvCmd, outStr);
        }
        fileList.push_back(newFileName);
    }

    LOGI("GpuCounter ItemData file size(%u)", fileList.size());
    result["gpu_counter"] = "true";

    std::string content = "";
    for (auto it = fileList.begin(); it != fileList.end(); ++it) {
        content += *it;
        content += ',';
    }
    if (!content.empty()) {
        content.pop_back();
    }

    result["info"] = content;

    LOGI("GpuCounter ItemData map siez=%u", result.size());
    return result;
}

int GpuCounter::CheckResources(const std::string &packageName, std::string &errorInfo)
{
    std::string result;

    SPUtils::LoadCmd(constWhoami, result);
    if (result.empty() || result.find(constUserInfo) == std::string::npos) {
        errorInfo = "Non root users";
        return ERROR_CODE_NEGATIVE_THREE;
    }

    result.clear();
    SPUtils::LoadCmd(constCheckProductInfo, result);
    if (result.empty() || result.find(constProductInfo) == std::string::npos) {
        errorInfo = "Non Hisilicon chips";
        return ERROR_CODE_NEGATIVE_FOUR;
    }
    if ((!SPUtils::FileAccess(constV2File)) || (!SPUtils::FileAccess(constExecFile)) ||
        (!SPUtils::FileAccess(constConfigFile)) || (!SPUtils::FileAccess(constLibFile))) {
        errorInfo = "Missing dependency files such as counters_collector";
        return -1;
    }
    if (packageName.empty()) {
        errorInfo = "package name is empty";
        return ERROR_CODE_NEGATIVE_TWO;
    }

    if (packageName == "/data/local/tmp/") {
        sandBoxPath = packageName;
    } else {
        sandBoxPath = constSandBoxPath + packageName + constSandBoxFile;
    }

    return 0;
}

int GpuCounter::Init(const std::string &packageName, std::map<std::string, std::string> &retMap)
{
    int ret = 0;
    std::string result;
    std::string errorInfo;
    Rest();
    ret = CheckResources(packageName, errorInfo);
    if (ret != 0) {
        retMap["gpu_counter"] = "false";
        retMap["error"] = errorInfo;
        initMap.insert(retMap.begin(), retMap.end());
        LOGE("%s", errorInfo.c_str());
        return ret;
    }

    initCheckPath = packageName;
    if (access(sandBoxPath.c_str(), W_OK) == 0) {
        isSandBoxWrite = true;
    }

    SPUtils::LoadCmd(constAddPermissionsCounter, result);
    ret = Start();
    if (ret == 0) {
        retMap["gpu_counter"] = "true";
    } else {
        errorInfo = "counters_collector run failed";
        retMap["gpu_counter"] = "false";
        retMap["error"] = errorInfo;
        LOGE("%s", errorInfo.c_str());
    }
    initMap.insert(retMap.begin(), retMap.end());
    return ret;
}

void GpuCounter::Rest()
{
    std::string result;

    gcStatus = GC_INIT;
    fileList.clear();
    startCaptureTime = 0;
    KillCounter(); // counters_collector if runing kill
    if (SPUtils::FileAccess(constOutSourCVSFile)) {
        SPUtils::LoadCmd(constRmCsv, result); // clear history files
    }
    sandBoxPath = "";
    initCheckPath = "";
}

int GpuCounter::Start()
{
    int ret = 0;

    gcStatus = GC_START;
    captureDuration = GetCounterDuration();
    LOGI("Start captureDuration(%lld)", captureDuration);
    if (captureDuration <= 0) {
        captureDuration = constDefaultCaptureDuration;
        LOGW("read config duration failed,load default(%lld)", captureDuration);
    }

    ThreadCapture();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_RUN_TIME));
    std::vector<std::string> pidList;
    GetCounterId(pidList);
    if (pidList.empty()) {
        LOGE("counters_collector run failed  ret(%d)", ret);
        return -1;
    }
    startCaptureTime = SPUtils::GetCurTime();
    LOGI("GpuCounter Started ret(%d)", ret);
    return ret;
}

void GpuCounter::Check()
{
    if (gcStatus != GC_START) {
        return;
    }

    long long diff = 0;
    long long nowTime = SPUtils::GetCurTime();
    diff = startCaptureTime > nowTime ? (LLONG_MAX - startCaptureTime + nowTime) : (nowTime - startCaptureTime);
    if (diff < captureDuration) {
        return;
    }

    LOGI("GpuCounter::Check diff(%lld)", diff);
    std::vector<std::string> pidList;
    GetCounterId(pidList);
    if (!pidList.empty()) { // GPU process did not exit
        return;
    }

    if (SPUtils::FileAccess(constOutSourCVSFile)) {
        std::string result;
        std::string newFileName = constOutDestCVSPrefix + "_" + std::to_string(nowTime) + ".csv";
        std::string mvCmd = constMvFile + constOutSourCVSFile + "  " + sandBoxPath + newFileName;

        if (sandBoxPath != "/data/local/tmp/" && (!isSandBoxWrite)) {
            SetPerm(EP_PERMISSIVE);
            SPUtils::LoadCmd(mvCmd, result);
            SetPerm(EP_ENFORCING);
        } else {
            SPUtils::LoadCmd(mvCmd, result);
        }
        fileList.push_back(newFileName);
        LOGI("new file name(%s)", newFileName.c_str());
    }

    Start();

    return;
}


void GpuCounter::GetCounterId(std::vector<std::string> &pidList)
{
    std::string result;

    pidList.clear();
    SPUtils::LoadCmd(constGetCounterId, result);
    SPUtils::StrSplit(result, " ", pidList);

    return;
}

void GpuCounter::KillCounter()
{
    std::vector<std::string> pidList;
    std::string result;

    GetCounterId(pidList);

    for (auto it = pidList.begin(); pidList.end() != it; ++it) {
        std::string killStr = constKillProcess + *it;
        result.clear();
        SPUtils::LoadCmd(killStr, result);
    }
    return;
}

bool GpuCounter::IsNum(const std::string value)
{
    bool isNum = false;
    for (size_t i = 0; i < value.length(); i++) {
        if (value[i] == ' ') {
            continue;
        } else if (value[i] >= '0' && value[i] <= '9') {
            isNum = true;
            break;
        } else {
            break;
        }
    }
    return isNum;
}

long long GpuCounter::GetCounterDuration()
{
    bool startFlag = false;
    long long ret = -1;
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(constConfigFile.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    std::ifstream file(realPath);
    std::string line;
    if (!file.is_open()) {
        return -1;
    }
    while (std::getline(file, line)) {
        if (!startFlag) {
            if (line.find("collector {") != std::string::npos) {
                startFlag = true;
            }
            continue;
        }
        if (line.find("duration_ms:") != std::string::npos) {
            std::vector<std::string> out;
            SPUtils::StrSplit(line, ":", out);
            if (out.size() <= 1) {
                break;
            }
            std::string value = out[1];
            if (IsNum(value)) {
                ret = std::stoll(value);
            }
            break;
        } else if (line.find("}") != std::string::npos) {
            startFlag = false;
        }
    }
    file.close();
    return ret;
}

int GpuCounter::Capture()
{
    return system(constCmd.c_str());
}

std::thread GpuCounter::ThreadCapture()
{
    auto th = std::thread([this]() { this->Capture(); });
    th.detach();
    return th;
}

GpuCounter::ExecutePermissions GpuCounter::GetPerm()
{
    ExecutePermissions ep = EP_INVALID;
    std::string result;
    SPUtils::LoadCmd("getenforce", result);
    if (result == "Permissive") {
        ep = EP_PERMISSIVE;
    } else if (result == "Enforcing") {
        ep = EP_ENFORCING;
    }

    return ep;
}

void GpuCounter::SetPerm(ExecutePermissions code)
{
    if (!(code == EP_ENFORCING || code == EP_PERMISSIVE)) {
        return;
    }

    if (originalEP == EP_PERMISSIVE) {
        return;
    }

    std::string result;
    std::string cmd = "setenforce " + std::to_string(int(code));
    SPUtils::LoadCmd(cmd, result);
    return;
}
}
}
