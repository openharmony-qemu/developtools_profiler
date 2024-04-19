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
const int ERROR_CODE_NEGATIVE_TWO = -2;
const int ERROR_CODE_NEGATIVE_THREE = -3;
const int ERROR_CODE_NEGATIVE_FOUR = -4;
std::map<std::string, std::string> GpuCounter::ItemData()
{
    std::map<std::string, std::string> result;

    isStop = true;
    KillCounter();

    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_EXIT_TIME));

    // After 'kill -9', a CSV file will also be generated
    if (SPUtils::FileAccess(constOutSourCVSFile)) {
        std::string outStr;
        std::string newFileName = constOutDestCVSPrefix + "_" + std::to_string(SPUtils::GetCurTime()) + ".csv";
        std::string mvCmd = constMvFile + constOutSourCVSFile + "  " + sandBoxPath + newFileName;
        std::cout << "FileAccess mvCmd" << mvCmd << std::endl;
        SPUtils::LoadCmd(mvCmd, outStr);
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
        content = content.substr(0, content.length() - 1); // Cancel the last ',' sign
    }

    result["info"] = content;

    LOGI("GpuCounter ItemData map siez=%u", result.size());
    return result;
}


int GpuCounter::Init(std::string packageName, std::map<std::string, std::string> &retMap)
{
    int ret = 0;
    std::string result;
    std::string errorInfo;
    Rest();
    do {
        SPUtils::LoadCmd(constCheckProductInfo, result);
        if (result.empty() || result.find(constProductInfo) == std::string::npos) {
            errorInfo = "Non Hisilicon chips";
            ret = ERROR_CODE_NEGATIVE_FOUR;
            break;
        }
        if ((!SPUtils::FileAccess(constV2File)) || (!SPUtils::FileAccess(constExecFile)) ||
            (!SPUtils::FileAccess(constConfigFile)) || (!SPUtils::FileAccess(constLibFile))) {
            errorInfo = "Missing dependency files such as counters_collector";
            ret = -1;
            break;
        }
        if (packageName.empty()) {
            errorInfo = "package name is empty";
            ret = ERROR_CODE_NEGATIVE_TWO;
            break;
        }
        sandBoxPath = constSandBoxPath + packageName + constSandBoxFile;
        if (access(sandBoxPath.c_str(), W_OK) == -1) { // Determine whether SandBox can be written
            errorInfo = "unable to access sandbox path(" + sandBoxPath + ")";
            ret = ERROR_CODE_NEGATIVE_THREE;
            break;
        }
    } while (false);
    if (ret != 0) {
        retMap["gpu_counter"] = "false";
        retMap["error"] = errorInfo;
        LOGE("%s", errorInfo.c_str());
        return ret;
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
    return ret;
}

void GpuCounter::Rest()
{
    std::string result;
    isStop = false;
    fileList.clear();
    startCaptureTime = 0;
    KillCounter(); // counters_collector if runing kill
    if (SPUtils::FileAccess(constOutSourCVSFile)) {
        SPUtils::LoadCmd(constRmCsv, result); // clear history files
    }
}

int GpuCounter::Start()
{
    int ret = -1;

    captureDuration = GetCounterDuration();
    std::cout << "captureDuration" << captureDuration << std::endl;
    if (captureDuration <= 0) {
        captureDuration = constDefaultCaptureDuration;
        LOGW("read config duration failed,load default(%ll)", captureDuration);
    }

    ret = Capture();
    if (ret) {
        LOGW("counters_collector run failed  ret(%d)", ret);
        return -1;
    }

    startCaptureTime = SPUtils::GetCurTime();

    LOGI("GpuCounter Started ret(%d)", ret);
    return ret;
}

void GpuCounter::Check()
{
    if (isStop) {
        return;
    }

    long long diff = 0;
    long long nowTime = SPUtils::GetCurTime();

    diff = startCaptureTime > nowTime ? (LLONG_MAX - startCaptureTime + nowTime) : (nowTime - startCaptureTime);
    if (diff < captureDuration) {
        return;
    }

    std::cout << " GpuCounter::Check diff > captureDuration diffvalue" << diff << std::endl;
    std::vector<std::string> pidList;
    GetCounterId(pidList);
    if (!pidList.empty()) { // GPU process did not exit
        return;
    }

    std::string result;
    std::string newFileName = constOutDestCVSPrefix + "_" + std::to_string(nowTime) + ".csv";
    std::string mvCmd = constMvFile + constOutSourCVSFile + "  " + sandBoxPath + newFileName;
    SPUtils::LoadCmd(mvCmd, result);
    fileList.push_back(newFileName);
    std::cout << " new file name=" << newFileName << std::endl;

    Start();

    return;
}


void GpuCounter::GetCounterId(std::vector<std::string> &pidList)
{
    std::string result;

    std::cout << "GetCounterId constGetCounterId=" << constGetCounterId << std::endl;
    pidList.clear();
    SPUtils::LoadCmd(constGetCounterId, result);
    SPUtils::StrSplit(result, " ", pidList);
    std::cout << "GetCounterId end result=" << result << std::endl;

    return;
}

void GpuCounter::KillCounter()
{
    std::vector<std::string> pidList;
    std::string result;

    std::cout << "KillCounter start" << std::endl;
    GetCounterId(pidList);

    for (auto it = pidList.begin(); pidList.end() != it; ++it) {
        std::string killStr = constKillProcess + *it;
        std::cout << "KillCounter killStr" << killStr << std::endl;
        result.clear();
        SPUtils::LoadCmd(killStr, result);
    }
    std::cout << "KillCounter end" << std::endl;
    return;
}


long long GpuCounter::GetCounterDuration()
{
    bool startFlag = false;
    long long ret = -1;
    std::ifstream file(constConfigFile);
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

            std::string value = out[1];
            if (value.empty()) {
                return -1;
            }

            SPUtils::RemoveSpace(value);
            try {
                ret = std::stoll(value);
                break;
            } catch (std::exception &e) {
                return -1;
            }
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
}
}
