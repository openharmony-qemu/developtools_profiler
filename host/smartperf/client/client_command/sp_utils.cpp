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
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cctype>
#include <climits>
#include <sys/utsname.h>
#include "sys/time.h"
#include "securec.h"
#include "include/sp_utils.h"
#include "include/sp_log.h"
#include "include/common.h"
#include "cpu_collector.h"
#include "collect_result.h"
#include "include/FPS.h"
#include "include/GPU.h"
#include "include/Power.h"
#include "include/DDR.h"
#include "include/profiler_fps.h"
#include "include/FileDescriptor.h"
#include "include/Threads.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
const unsigned int INT_MAX_LEN = 10;
const unsigned int CHAR_NUM_DIFF = 48;
const unsigned int UI_DECIMALISM = 10;
const unsigned int UI_INDEX_2 = 2;
const int BUFFER_SIZE = 1024;
const std::string SMART_PERF_VERSION = "1.0.6";
bool SPUtils::FileAccess(const std::string &fileName)
{
    return (access(fileName.c_str(), F_OK) == 0);
}
bool SPUtils::HasNumber(const std::string &str)
{
    return std::any_of(str.begin(), str.end(), [](char c) { return std::isdigit(c); });
}
bool SPUtils::Cmp(const std::string &a, const std::string &b)
{
    if (HasNumber(a) && HasNumber(b)) {
        std::string stra = a.substr(0, a.find_first_of("0123456789"));
        std::string strb = b.substr(0, b.find_first_of("0123456789"));
        if (stra != strb) {
            return stra < strb;
        }
        int numa = SPUtilesTye::StringToSometype<int>(a.substr(stra.length()));
        int numb = SPUtilesTye::StringToSometype<int>(b.substr(strb.length()));
        return numa < numb;
    }
    return false;
}

bool SPUtils::LoadFile(const std::string &filePath, std::string &content)
{
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(filePath.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    std::ifstream file(realPath);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios::end);
    file.tellg();

    content.clear();
    file.seekg(0, std::ios::beg);
    copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(content));
    // remove '' \n\r
    ReplaceString(content);
    return true;
}

bool SPUtils::LoadCmdWithLinkBreak(const std::string &cmd, bool isClearLinkBreak, std::string &result)
{
    const std::string cmdExc = cmd;
    FILE *fd = popen(cmdExc.c_str(), "r");
    if (fd == nullptr) {
        return false;
    }
    char buf[4096] = {'\0'};
    size_t ret = fread(buf, 1, sizeof(buf) - 1, fd);
    if (ret >= 0) {
        buf[ret] = '\0';
        result = buf;
    }
    if (pclose(fd) == -1) {
        LOGE("Error: Failed to close file");
        return false;
    }

    if (isClearLinkBreak) {
        // remove '' \n\r
        ReplaceString(result);
    }

    return ret >= 0 ? true : false;
}

bool SPUtils::LoadCmd(const std::string &cmd, std::string &result)
{
    return LoadCmdWithLinkBreak(cmd, true, result);
}

std::string SPUtils::IncludePathDelimiter(const std::string &path)
{
    if (!path.empty() && path.back() != '/') {
        return path + "/";
    } else {
        return path;
    }
}

void SPUtils::ForDirFiles(const std::string &path, std::vector<std::string> &files)
{
    std::string pathStringWithDelimiter;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // current dir OR parent dir
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        } else if (ptr->d_type == DT_DIR) {
            pathStringWithDelimiter = IncludePathDelimiter(path) + std::string(ptr->d_name);
            ForDirFiles(pathStringWithDelimiter, files);
        } else {
            files.push_back(IncludePathDelimiter(path) + std::string(ptr->d_name));
        }
    }
    closedir(dir);
}

bool SPUtils::IsSubString(const std::string &str, const std::string &sub)
{
    if (sub.empty() || str.empty()) {
        return false;
    }
    return str.find(sub) != std::string::npos;
}

void SPUtils::StrSplit(const std::string &content, const std::string &sp, std::vector<std::string> &out)
{
    size_t index = 0;
    while (index != std::string::npos) {
        size_t tEnd = content.find_first_of(sp, index);
        std::string tmp = content.substr(index, tEnd - index);
        if (tmp != "" && tmp != " ") {
            out.push_back(tmp);
        }
        if (tEnd == std::string::npos) {
            break;
        }
        index = tEnd + 1;
    }
}

std::string SPUtils::ExtractNumber(const std::string &str)
{
    int cntInt = 0;
    const int shift = 10;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            cntInt *= shift;
            cntInt += str[i] - '0';
        }
    }
    return std::to_string(cntInt);
}

void SPUtils::ReplaceString(std::string &res)
{
    std::string flagOne = "\r";
    std::string flagTwo = "\n";
    std::string::size_type ret = res.find(flagOne);
    while (ret != res.npos) {
        res.replace(ret, 1, "");
        ret = res.find(flagOne);
    }
    ret = res.find(flagTwo);
    while (ret != res.npos) {
        res.replace(ret, 1, "");
        ret = res.find(flagTwo);
    }
}

long long SPUtils::GetCurTime()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    long long timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return timestamp;
}

std::string SPUtils::GetTopPkgName()
{
    std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_HEAD);
    std::string curTopPkgStr = "";
    LoadCmd(cmd, curTopPkgStr);
    uint64_t left = curTopPkgStr.find_first_of("[");
    uint64_t right = curTopPkgStr.find_first_of("]");
    std::string topPkg = curTopPkgStr.substr(left + 1, static_cast<int64_t>(right) - static_cast<int64_t>(left) - 1);
    return topPkg;
}

std::string SPUtils::GetRadar()
{
    std::string cmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_APP_START);
    std::string curRadar = "";
    LoadCmd(cmd, curRadar);
    return curRadar;
}
std::string SPUtils::GetScreen()
{
    std::string cmd = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SCREEN);
    std::string screenStr = "";
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return screenStr;
    }
    char buf[4096] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line(buf);
        if (line.find("activeMode") != std::string::npos) {
            screenStr = line;
        }
    }
    pclose(fd);
    return screenStr;
}
std::string SPUtils::GetRadarFrame()
{
    std::string cmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_JANK);
    std::string curRadar = "";
    LoadCmd(cmd, curRadar);
    return curRadar;
}
std::string SPUtils::GetRadarResponse()
{
    std::string cmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_RESPONSE);
    std::string curRadar = "";
    LoadCmd(cmd, curRadar);
    return curRadar;
}
std::string SPUtils::GetRadarComplete()
{
    std::string cmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_COMPLETED);
    std::string curRadar = "";
    LoadCmd(cmd, curRadar);
    return curRadar;
}
static std::string GetSplitOne(std::string cmd)
{
    std::string result;
    SPUtils::LoadCmd(cmd, result);
    return result;
}

std::string SPUtils::GetDeviceInfoMap()
{
    size_t len = 2;
    bool isTcpMessage = false;
    std::map<std::string, std::string> deviceInfoMap;
    std::map<std::string, std::string> cpuInfo = GetCpuInfo(isTcpMessage);
    std::map<std::string, std::string> gpuInfo = GetGpuInfo(isTcpMessage);
    std::map<std::string, std::string> deviceInfo = GetDeviceInfo();
    std::string screenInfos = GetScreen();
    size_t pos = screenInfos.find(": ");
    size_t pos1 = screenInfos.find(",");
    std::string screenSize = screenInfos.substr(pos + len, pos1 - pos - len);
    deviceInfoMap.insert(cpuInfo.begin(), cpuInfo.end());
    deviceInfoMap.insert(gpuInfo.begin(), gpuInfo.end());
    deviceInfoMap.insert(deviceInfo.begin(), deviceInfo.end());
    deviceInfoMap["activeMode"] = screenSize;
    if (deviceInfoMap.empty()) {
        LOGE("Failed to obtain device information");
    }
    for (auto iter = deviceInfoMap.cbegin(); iter != deviceInfoMap.cend(); ++iter) {
        printf("%s: %s\n", iter->first.c_str(), iter->second.c_str());
    }
    std::cout << "" << std::endl;
    return std::string("command exec finished!");
}

std::map<std::string, std::string> SPUtils::GetDeviceInfo()
{
    std::map<std::string, std::string> resultMap;
    std::string sn = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::SN));
    std::string deviceTypeName = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::DEVICET_NAME));
    std::string brand = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::BRAND));
    std::string version = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::VERSION));
    std::string abilist = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::ABILIST));
    std::string name = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::NAME));
    std::string model = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::MODEL));
    std::string fullname = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::FULL_NAME));
    resultMap["sn"] = sn;
    resultMap["deviceTypeName"] = deviceTypeName;
    resultMap["brand"] = brand;
    resultMap["board"] = "hw";
    resultMap["version"] = version;
    resultMap["abilist"] = abilist;
    resultMap["name"] = name;
    resultMap["model"] = model;
    resultMap["fullname"] = fullname;
    resultMap["daemonPerfVersion"] = SMART_PERF_VERSION;
    return resultMap;
}
std::map<std::string, std::string> SPUtils::GetCpuInfo(bool isTcpMessage)
{
    std::string clusterNames;
    std::vector<std::string> policyFiles;
    std::map<std::string, std::string> resultMap;
    std::string basePath = "/sys/devices/system/cpu/cpufreq/";
    DIR *dir = opendir(basePath.c_str());
    if (dir == nullptr) {
        return resultMap;
    }
    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        }
        std::string clusterName = std::string(ptr->d_name);
        if (!isTcpMessage) {
            clusterNames += clusterName + " ";
            resultMap["cpu_cluster_name"] = clusterNames;
        }
        policyFiles.push_back(IncludePathDelimiter(basePath) + clusterName);
    }
    closedir(dir);
    for (size_t i = 0; i < policyFiles.size(); i++) {
        std::string cpus;
        LoadFile(policyFiles[i] + "/affected_cpus", cpus);
        std::string max;
        LoadFile(policyFiles[i] + "/cpuinfo_max_freq", max);
        std::string min;
        LoadFile(policyFiles[i] + "/cpuinfo_min_freq", min);
        std::string nameBase;
        if (!isTcpMessage) {
            nameBase = "cpu_c" + std::to_string(i + 1) + "_";
        } else {
            nameBase = "cpu-c" + std::to_string(i + 1) + "-";
        }
        resultMap[nameBase + "cluster"] = cpus;
        resultMap[nameBase + "max"] = max;
        resultMap[nameBase + "min"] = min;
    }
    return resultMap;
}
std::map<std::string, std::string> SPUtils::GetGpuInfo(bool isTcpMessage)
{
    const std::vector<std::string> gpuCurFreqPaths = {
        "/sys/class/devfreq/fde60000.gpu/",
        "/sys/class/devfreq/gpufreq/",
    };
    std::map<std::string, std::string> resultMap;
    for (auto path : gpuCurFreqPaths) {
        if (FileAccess(path)) {
            std::string max;
            SPUtils::LoadFile(path + "/max_freq", max);
            std::string min;
            SPUtils::LoadFile(path + "/min_freq", min);
            resultMap["gpu_max_freq"] = max;
            resultMap["gpu_min_freq"] = min;
        }
    }
    return resultMap;
}

void SPUtils::RemoveSpace(std::string &str)
{
    int len = 0;

    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] != ' ') {
            break;
        }

        ++len;
    }

    if (len > 0) {
        str = str.substr(len);
    }

    len = 0;
    for (size_t i = str.length(); i > 0; --i) {
        if (str[i - 1] != ' ') {
            break;
        }

        ++len;
    }

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            str.pop_back();
        }
    }
}


bool SPUtils::IntegerVerification(std::string str, std::string errorInfo)
{
    uint64_t dest = 0;
    bool isValid = false;

    if (str.empty()) {
        errorInfo = "option requires an argument";
        LOGE("sour(%s) error(%s)", str.c_str(), errorInfo.c_str());
        return false;
    }
    if (str.length() > INT_MAX_LEN) {
        errorInfo = "invalid option parameters";
        LOGE("sour(%s) error(%s)", str.c_str(), errorInfo.c_str());
        return false;
    }

    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') {
            errorInfo = "invalid option parameters";
            LOGE("sour(%s) error(%s)", str.c_str(), errorInfo.c_str());
            return false;
        }

        if (!isValid && (str[i] == '0')) {
            continue;
        }

        isValid = true;
        dest *= UI_DECIMALISM;
        dest += (str[i] - CHAR_NUM_DIFF);
    }

    if (dest == 0 || dest > INT_MAX) {
        errorInfo = "option parameter out of range";
        LOGE("sour(%s) dest(%u) error(%s)", str.c_str(), dest, errorInfo.c_str());
        return false;
    }

    return true;
}

bool SPUtils::VeriyParameter(std::set<std::string> &keys, std::string param, std::string &errorInfo)
{
    std::string keyParam;
    std::string valueParm;
    std::vector<std::string> out;
    std::vector<std::string> subOut;
    std::map<std::string, std::string> mapInfo;

    if (param.empty()) {
        errorInfo = "The parameter cannot be empty";
        return false;
    }
    if (param.find("-PKG") != std::string::npos &&
        param.find("-PID") != std::string::npos) {
        LOGE("-PKG and -PID cannot be used together with");
        return false;
    }
    SPUtils::StrSplit(param, "-", out);

    for (auto it = out.begin(); it != out.end(); ++it) { // Parsing keys and values
        subOut.clear();
        SPUtils::StrSplit(*it, " ", subOut);
        if (mapInfo.end() != mapInfo.find(subOut[0])) {
            errorInfo = "duplicate parameters -- '" + subOut[0] + "'";
            return false;
        }

        if (subOut.size() >= UI_INDEX_2) {
            keyParam = subOut[0];
            valueParm = subOut[1];
            SPUtils::RemoveSpace(keyParam);
            SPUtils::RemoveSpace(valueParm);
            mapInfo[keyParam] = valueParm;
        } else if (subOut.size() >= 1) {
            keyParam = subOut[0];
            SPUtils::RemoveSpace(keyParam);
            mapInfo[keyParam] = "";
        }
    }

    if (!VeriyKey(keys, mapInfo, errorInfo)) {
        LOGE("%s", errorInfo.c_str());
        return false;
    }

    if (!VerifyValueStr(mapInfo, errorInfo)) {
        LOGE("%s", errorInfo.c_str());
        return false;
    }

    if (!IntegerValueVerification(keys, mapInfo, errorInfo)) {
        LOGE("%s", errorInfo.c_str());
        return false;
    }
    return true;
}

bool SPUtils::VeriyKey(std::set<std::string> &keys, std::map<std::string, std::string> &mapInfo,
    std::string &errorInfo)
{
    for (auto it = mapInfo.begin(); it != mapInfo.end(); ++it) {
        if (keys.end() == keys.find(it->first)) {
            errorInfo = "invalid parameter -- '" + it->first + "'";
            return false;
        }
    }

    return true;
}

bool SPUtils::VerifyValueStr(std::map<std::string, std::string> &mapInfo, std::string &errorInfo)
{
    auto a = mapInfo.find("VIEW");
    if (mapInfo.end() != a && a->second.empty()) { // Cannot be null
        errorInfo += "option requires an argument -- '" + a->first + "'";
        return false;
    }
    a = mapInfo.find("PKG");
    if (mapInfo.end() != a && a->second.empty()) { // Cannot be null
        errorInfo += "option requires an argument -- '" + a->first + "'";
        return false;
    }
    a = mapInfo.find("PID");
    if (mapInfo.end() != a && a->second.empty()) { // Cannot be null
        errorInfo += "option requires an argument -- '" + a->first + "'";
        return false;
    }
    a = mapInfo.find("OUT");
    if (mapInfo.end() != a) {
        if (a->second.empty()) {
            errorInfo += "option requires an argument -- '" + a->first + "'";
            return false;
        }
        // The total length of file path and name cannot exceed PATH_MAX
        if (a->second.length() >= PATH_MAX) {
            errorInfo +=
                "invalid parameter, file path cannot exceed " + std::to_string(PATH_MAX) + " -- '" + a->first + "'";
            return false;
        }
        size_t pos = a->second.rfind('/');
        if (pos == a->second.length()) { // not file name
            errorInfo += "invalid parameter,not file name -- '" + a->first + "'";
            return false;
        }
        if (std::string::npos != pos &&
            (!SPUtils::FileAccess(a->second.substr(0, pos)))) { // determine if the directory exists
            errorInfo += "invalid parameter,file path not found -- '" + a->first + "'";
            return false;
        }
        std::string outStr = a->second;
        std::vector<std::string> outList;
        SPUtils::StrSplit(outStr, "/", outList);
        for (auto it = outList.begin(); outList.end() != it; ++it) {
            if ((*it).length() >= NAME_MAX) {
                errorInfo += "invalid parameter, file directory or name cannot exceed 255 -- '" + a->first + "'";
                return false;
            }
        }
    }
    return true;
}

bool SPUtils::IntegerValueVerification(std::set<std::string> &keys, std::map<std::string, std::string> &mapInfo,
    std::string &errorInfo)
{
    std::vector<std::string> integerCheck; // Number of integers to be detected

    if (keys.end() != keys.find("N")) {
        integerCheck.push_back("N");
    }
    if (keys.end() != keys.find("fl")) {
        integerCheck.push_back("fl");
    }
    if (keys.end() != keys.find("ftl")) {
        integerCheck.push_back("ftl");
    }
    if (keys.end() != keys.find("PID")) {
        integerCheck.push_back("PID");
    }

    for (auto it = integerCheck.begin(); it != integerCheck.end(); ++it) {
        auto a = mapInfo.find(*it);
        if (mapInfo.end() != a) {
            if (a->second.empty()) {
                errorInfo += "option requires an argument -- '" + a->first + "'";
                return false;
            }
            if (!SPUtils::IntegerVerification(a->second, errorInfo)) {
                errorInfo += "option parameter out of range -- '" + a->first + "'";
                return false;
            }
        }
    }

    return true;
}

bool SPUtils::IsHmKernel()
{
    bool isHM = false;
    utsname unameBuf;
    if ((uname(&unameBuf)) == 0) {
        std::string osRelease = unameBuf.release;
        isHM = osRelease.find("HongMeng") != std::string::npos;
    }
    return isHM;
}

std::string SPUtils::GetCpuNum()
{
    std::string cpuCores = "cpuCores||";
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<std::vector<CpuFreq>> result = collector->CollectCpuFrequency();
    std::vector<CpuFreq> &cpufreq = result.data;
    size_t cpuNum = cpufreq.size();
    cpuCores += std::to_string(cpuNum);
    if (cpuNum == 0) {
        std::cout << "CPU frequency collection failed." << std::endl;
        LOGE("CPU frequency collection failed.");
    }
    return cpuCores;
}
void SPUtils::GetCurrentTime(int prevTime)
{
    LOGD("SPUtils::prevTime (%d)", prevTime);
    unsigned long sleepNowTime = 10000;
    bool shouldContinue = true;
    while (shouldContinue) {
        struct timespec time1 = { 0 };
        clock_gettime(CLOCK_MONOTONIC, &time1);
        int curTimeNow = static_cast<int>(time1.tv_sec - 1);
        if (curTimeNow == prevTime) {
            usleep(sleepNowTime);
        } else {
            shouldContinue = false;
        }
    }
}

bool SPUtils::IsForeGround(const std::string &pkg)
{
    bool isFoundAppName = false;
    bool isForeground = false;
    std::string result;
    const std::string cmd = "hidumper -s AbilityManagerService -a -l";
    if (cmd.empty()) {
        LOGI("cmd is null");
        return false;
    }

    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return false;
    }
    size_t bytesRead;
    std::array<char, BUFFER_SIZE> buf;
    while ((bytesRead = fread(buf.data(), 1, buf.size(), fd)) > 0) {
        result.append(buf.data(), bytesRead);
    }
    if (pclose(fd) == -1) {
        LOGE("Error: failed to close file");
        return false;
    }

    std::istringstream iss(result);
    std::string line;
    while (std::getline(iss, line)) {
        if (!isFoundAppName && line.find("mission name #[#" + pkg) != std::string::npos) {
            isFoundAppName = true;
        }
        if (isFoundAppName) {
            if (line.find("app state") != std::string::npos) {
                isForeground = IsFindForeGround(line);
                break;
            }
        }
    }
    return isForeground;
}

bool SPUtils::IsFindForeGround(const std::string &line)
{
    return line.find("FOREGROUND") != std::string::npos;
}

bool SPUtils::IsFindAbilist()
{
    std::string abilist = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::ABILIST));
    std::string brand = GetSplitOne(DEVICE_CMD_MAP.at(DeviceCmd::BRAND));
    if (abilist.find("arm") != std::string::npos && brand.find("HUA") != std::string::npos) {
        return true;
    } else if (abilist.find("arm") != std::string::npos && brand.find("HUA") == std::string::npos) {
        return OHOS::SmartPerf::FPS::GetInstance().SetOtherDeviceFlag();
    } else {
        return false;
    }
}
void SPUtils::SetRkFlag()
{
    bool findAbilistResult = IsFindAbilist();
    if (!findAbilistResult) {
        OHOS::SmartPerf::FPS::GetInstance().SetRkFlag();
        OHOS::SmartPerf::Power::GetInstance().SetRkFlag();
        OHOS::SmartPerf::GPU::GetInstance().SetRkFlag();
        OHOS::SmartPerf::ProfilerFPS::GetInstance().SetRkFlag();
        OHOS::SmartPerf::DDR::GetInstance().SetRkFlag();
    }
}
bool SPUtils::GetPathPermissions(const std::string &path)
{
    const std::string dataCsv = "/data/local/tmp/data.csv";
    const std::string indexInfoCsv = "/data/local/tmp/smartperf/1/t_index_info.csv";
    int isDataCsv = strcmp(path.c_str(), dataCsv.c_str());
    int isIndexInfoCsv = strcmp(path.c_str(), indexInfoCsv.c_str());
    if (!path.empty()) {
        std::string cmdResult;
        std::string cmd;
        if (isDataCsv == 0 || isIndexInfoCsv == 0) {
            cmd = "ls -l " + path;
            LoadCmd(cmd, cmdResult);
            std::string result = cmdResult.substr(0, 10);
            return result == "-rw-r--r--";
        } else {
            LOGE("the path is false");
            return false;
        }
    } else {
        LOGE("THE path is empty");
        return false;
    }
}

bool SPUtils::GetIsGameApp(std::string pkg)
{
    bool isGame = false;
    const std::string cmd = "hidumper -s 66006 -a '-t " + pkg + "'";
    LOGD("SPUtils::GetIsGameApp cmd (%s)", cmd.c_str());
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        LOGD("ProfilerFPS::fd is empty");
        return false;
    }
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line(buf);
        if (line.find("---") != std::string::npos || line.length() <= 1) {
            continue;
        }
        if (line.find("bundleName unknown") != std::string::npos) {
            isGame = false;
            break;
        } else {
            std::vector<std::string> params;
            SPUtils::StrSplit(line, " ", params);
            if (params[0] == "1" && params[1] == pkg) {
                isGame = true;
                LOGD("SPUtils::GetIsGameApp isGame", isGame);
                break;
            }
        }
    }
    if (pclose(fd) == -1) {
        LOGE("SPUtils::IsGameApp Error Failed to close file");
    }
    return isGame;
}
std::string SPUtils::GetVersion()
{
    return SMART_PERF_VERSION;
}

int& SPUtils::GetTtyDeviceFd()
{
    static int fd = dup(STDERR_FILENO);
    return fd;
}

void SPUtils::GetBundleName(const std::string &pid, std::string &bundleName)
{
    std::string filePath = "/proc/";
    filePath.append(pid).append("/cmdline");
    LOGD("GetBundleName filePath:%s", filePath.c_str());
    if (!LoadFile(filePath, bundleName)) {
        LOGD("Get process name by pid failed! %s", filePath.c_str());
        return;
    }
    bundleName.resize(strlen(bundleName.c_str()));
    if (bundleName.size() >= 2 && bundleName.substr(0, 2) == "./") { // 2: size
        bundleName = bundleName.substr(2); // 2: point
    }
    size_t found = bundleName.rfind("/");
    std::string procName;
    if (found != std::string::npos) {
        procName = bundleName.substr(found + 1);
    } else {
        procName = bundleName;
    }
    bundleName = std::move(procName);
    LOGD("GetBundleName bundleName:%s", bundleName.c_str());
}
}
}
