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
#include <future>
#include <thread>
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
#include <dlfcn.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
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
#include "include/FileDescriptor.h"
#include "include/Threads.h"
#include "parameters.h"
#include "hitrace_converter.h"
#include <memory>

#ifdef ARKTEST_ENABLE
#include "test_server_client.h"
#endif

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
const unsigned int INT_MAX_LEN = 10;
const unsigned int CHAR_NUM_DIFF = 48;
const unsigned int UI_DECIMALISM = 10;
const unsigned int UI_INDEX_2 = 2;
const int EXEC_FAILURE_EXIT_CODE = 127;
const int EXEC_BUF_SIZE = 4096;
const unsigned int PID_MAX_LEN = 10;
const unsigned int WINID_MAX_LEN = 22;
const std::string SMART_PERF_VERSION = "1.1.0";
const std::string LOWERCASE_H(1, static_cast<char>(104));
const std::string LOWERCASE_W(1, static_cast<char>(119));
const std::string PRODUCT_NAME = LOWERCASE_H + LOWERCASE_W;
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

bool SPUtils::SafeLoadExec(const std::vector<std::string> &args, std::string &result)
{
    if (args.empty()) {
        return false;
    }
    int pipefd[UI_INDEX_2];
    if (pipe(pipefd) == -1) {
        return false;
    }
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }
    if (pid == 0) {
        close(pipefd[0]);
        int maxFd = static_cast<int>(sysconf(_SC_OPEN_MAX));
        for (int fd = STDERR_FILENO + 1; fd < maxFd; ++fd) {
            if (fd != pipefd[1]) {
                close(fd);
            }
        }
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(EXEC_FAILURE_EXIT_CODE);
        }
        if (dup2(pipefd[1], STDERR_FILENO) == -1) {
            _exit(EXEC_FAILURE_EXIT_CODE);
        }
        close(pipefd[1]);
        std::vector<char *> argv;
        for (const auto &arg : args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(EXEC_FAILURE_EXIT_CODE);
    }
    close(pipefd[1]);
    char buf[EXEC_BUF_SIZE];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        result.append(buf, n);
    }
    close(pipefd[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status);
}

bool SPUtils::IsValidPid(const std::string &pid)
{
    if (pid.empty() || pid.size() > PID_MAX_LEN) {
        return false;
    }
    return std::all_of(pid.begin(), pid.end(), ::isdigit);
}

bool SPUtils::IsValidWinId(const std::string &winId)
{
    if (winId.empty() || winId.size() > WINID_MAX_LEN) {
        return false;
    }
    return std::all_of(winId.begin(), winId.end(), ::isdigit);
}

bool SPUtils::IsValidPath(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    //拒绝所有控制字符
    for (char c : str) {
        if (static_cast<unsigned char>(c) <= 0x1F) {
            return false;
        }
    }
    const char* specialChars = "\\\"$`;&|><(){}[]^~*?#!%@";
    if (str.find_first_of(specialChars) != std::string::npos) {
        return false;
    }
    return true;
}

bool SPUtils::IsTraceFileName(const std::string &name)
{
    size_t atPos = name.find('@');
    if (atPos == std::string::npos) {
        return false;
    }
    if (name.find("trace_") == 0) {
        for (size_t i = 6; i < atPos; ++i) {
            if (!isdigit(static_cast<unsigned char>(name[i]))) {
                return false;
            }
        }
    } else if (name.find("Other_trace_") == 0) {
        for (size_t i = 13; i < atPos; ++i) {
            if (!isdigit(static_cast<unsigned char>(name[i]))) {
                return false;
            }
        }
    } else {
        return false;
    }
    size_t dotPos = name.rfind('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    std::string pidTid = name.substr(atPos + 1, dotPos - atPos - 1);
    bool hasDash = false;
    for (char c : pidTid) {
        if (c == '-') {
            if (hasDash) {
                return false;
            }
            hasDash = true;
        } else if (!isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    if (!hasDash) {
        return false;
    }
    std::string ext = name.substr(dotPos);
    if (ext != ".sys" && ext != ".ftrace") {
        return false;
    }
    return true;
}

bool SPUtils::IsPathTraversal(const std::string& str)
{
    if (str.find("..") != std::string::npos) {
        return true;
    }
    return false;
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
    std::string screenStr = "";
    std::string cmdResult;
    if (!SafeLoadExec({"hidumper", "-s", "10", "-a", "screen"}, cmdResult)) {
        return screenStr;
    }
    std::istringstream iss(cmdResult);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("activeMode") != std::string::npos) {
            screenStr = line;
        }
    }
    return screenStr;
}

std::string SPUtils::GetRadarFrame()
{
    std::vector<std::string> params;
    std::string slideCmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_SCROLL_ANIMATION);
    std::string otherCmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_JANK);
    std::string curRadar = "";
    const std::string splitStr = "{";
    std::promise<std::string> slidePromise;
    std::promise<std::string> otherPromise;
    std::future<std::string> slideResult = slidePromise.get_future();
    std::future<std::string> otherResult = otherPromise.get_future();
    std::thread slideThread([&slidePromise, &slideCmd]() {
        std::string result = "";
        LoadCmd(slideCmd, result);
        slidePromise.set_value(result);
    });
    std::thread otherThread([&otherPromise, &otherCmd]() {
        std::string result = "";
        LoadCmd(otherCmd, result);
        otherPromise.set_value(result);
    });
    slideThread.join();
    otherThread.join();
    std::string slideValue = slideResult.get();
    std::string otherValue = otherResult.get();
    if (!otherValue.empty()) {
        curRadar = otherValue;
    } else {
        curRadar = slideValue;
    }
    StrSplit(curRadar, splitStr, params);
    for (auto param : params) {
        if (param.find("LAUNCHER_APP_LAUNCH") != std::string::npos) {
            curRadar = param;
            break;
        }
    }
    LOGD("GetRadarFrame::curRadar: %s", curRadar.c_str());
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

std::string SPUtils::GetDeviceInfoMap()
{
    std::map<std::string, std::string> deviceInfoMap;
    deviceInfoMap.merge(GetCpuInfo(false));
    deviceInfoMap.merge(GetGpuInfo(false));
    deviceInfoMap.merge(GetDeviceInfo());

    std::string screenInfos = GetScreen();
    size_t pos = screenInfos.find(": ");
    size_t pos1 = screenInfos.find(",");
    size_t len = 2;
    std::string screenSize = screenInfos.substr(pos + len, pos1 - pos - len);
    deviceInfoMap["activeMode"] = screenSize;
    for (auto iter = deviceInfoMap.cbegin(); iter != deviceInfoMap.cend(); ++iter) {
        printf("%s: %s\n", iter->first.c_str(), iter->second.c_str());
    }
    std::cout << std::endl;
    return std::string("command exec finished!");
}

std::map<std::string, std::string> SPUtils::GetDeviceInfo()
{
    std::map<std::string, std::string> resultMap;
    resultMap["sn"] = OHOS::system::GetParameter((DEVICE_CMD_MAP.at(DeviceCmd::SN)), "Unknown");
    resultMap["deviceTypeName"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::DEVICET_NAME), "Unknown");
    resultMap["brand"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::BRAND), "Unknown");
    resultMap["board"] = GetProductName();
    resultMap["version"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::VERSION), "Unknown");
    resultMap["abilist"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::ABILIST), "Unknown");
    resultMap["name"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::NAME), "Unknown");
    resultMap["model"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::MODEL), "Unknown");
    resultMap["fullname"] = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::FULL_NAME), "Unknown");
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
    for (auto& path : gpuCurFreqPaths) {
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


bool SPUtils::IntegerVerification(const std::string& str, std::string& errorInfo)
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

bool SPUtils::VeriyParameter(std::set<std::string> &keys, const std::string& param, std::string &errorInfo)
{
    std::vector<std::string> out;
    std::map<std::string, std::string> mapInfo;

    if (!IsInvalidInputfromComParam(param, errorInfo)) {
        LOGE("%s", errorInfo.c_str());
        return false;
    }

    SPUtils::StrSplit(param, "-", out);
    if (!RemSpaceAndTraPara(out, mapInfo, errorInfo)) {
        LOGE("%s", errorInfo.c_str());
        return false;
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

bool SPUtils::IsInvalidInputfromComParam(const std::string& param, std::string &errorInfo)
{
    if (param.empty()) {
        errorInfo = "The parameter cannot be empty";
        return false;
    }
    if (param.find("-PKG") != std::string::npos &&
        param.find("-PID") != std::string::npos) {
        LOGE("-PKG and -PID cannot be used together with");
        return false;
    }
    const size_t paramLength = 1;
    if (param.length() == paramLength && param[0] == '-') {
        errorInfo = "invalid parameter -- '" + param + "'";
        return false;
    }
    std::string commandShell = "";
    for (const auto& a : COMMAND_SHELL_MAP) {
        commandShell = a.first.substr(1);
    }
    if (commandShell.empty()) {
        LOGE("commandShell is NULL!");
        return false;
    }
    if (param.find("--") != std::string::npos && param.find(commandShell)) {
        errorInfo = "invalid parameter -- '" + param + "'";
        return false;
    }
    return true;
}

bool SPUtils::RemSpaceAndTraPara(std::vector<std::string>& outParam, std::map<std::string, std::string>& mapInfo,
    std::string &errorInfo)
{
    std::string keyParam;
    std::string valueParm;
    std::vector<std::string> subOut;

    for (auto it = outParam.begin(); it != outParam.end(); ++it) { // Parsing keys and values
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
    if (mapInfo.end() != a && !IsValidPkgName(a->second)) {
        errorInfo += "invalid parameter, invalid package name -- '" + a->second + "'";
        return false;
    }
    a = mapInfo.find("PID");
    if (mapInfo.end() != a && !IsValidPid(a->second)) {
        errorInfo += "invalid parameter, invalid pid -- '" + a->second + "'";
        return false;
    }
    a = mapInfo.find("OUT");
    if (mapInfo.end() != a) {
        if (!IsValidPath(a->second)) {
            errorInfo += "invalid parameter, invalid path -- '" + a->second + "'";
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
        if (pos != std::string::npos && (!SPUtils::FileAccess(a->second.substr(0, pos)))) {
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
    if (!SafeLoadExec({"hidumper", "-s", "AbilityManagerService", "-a", "-l"}, result)) {
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
    std::string abilist = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::ABILIST), "Unknown");
    std::string brand = OHOS::system::GetParameter(DEVICE_CMD_MAP.at(DeviceCmd::BRAND), "Unknown");
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

bool SPUtils::GetIsGameApp(const std::string& pkg)
{
    if (!IsValidPkgName(pkg)) {
        LOGE("SPUtils::GetIsGameApp invalid pkg name");
        return false;
    }
    bool isGame = false;
    std::string cmdResult;
    std::string arg = "-t " + pkg;
    if (!SafeLoadExec({"hidumper", "-s", "66006", "-a", arg}, cmdResult)) {
        LOGD("FPS::SafeLoadExec failed");
        return false;
    }
    std::istringstream iss(cmdResult);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("---") != std::string::npos || line.length() <= 1) {
            continue;
        }
        if (line.find("bundleName unknown") != std::string::npos) {
            isGame = IsFindDHGame(pkg);
            break;
        } else {
            std::vector<std::string> params;
            SPUtils::StrSplit(line, " ", params);
            if (params.size() >= UI_INDEX_2 && params[0] == "1" && params[1] == pkg) {
                isGame = true;
                LOGD("SPUtils::GetIsGameApp isGame %d", isGame);
                break;
            }
        }
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

void SPUtils::GetTestsaPlugin(int command)
{
    #ifdef ARKTEST_ENABLE
    //Call the function
    std::string stopJsonString = "{\"command\": \"stopCollect\"}";
    OHOS::testserver::TestServerClient::GetInstance().SpDaemonProcess(command, stopJsonString);
    #endif
}

void SPUtils::KillStartDaemon()
{
    std::string pidCmd = "ps -ef | grep SP_daemon";
    std::string cmdResult = "";
    OHOS::SmartPerf::SPUtils::LoadCmd(pidCmd, cmdResult);
    if (cmdResult.find("testserver") != std::string::npos) {
        int command = 2;
        GetTestsaPlugin(command);
    }
}

bool SPUtils::Chmod(const std::string& sourceFilePath, mode_t mode)
{
    mode_t oldMask = umask(0);
    int retCode = chmod(sourceFilePath.c_str(), mode);
    umask(oldMask);
    if (retCode != 0) {
        LOGE("Failed to set %s permission, error code %d", sourceFilePath.c_str(), retCode);
        return false;
    }
    return true;
}

void SPUtils::CreateDir(const std::string &dirPath)
{
    if (SPUtils::FileAccess(dirPath)) {
        Chmod(dirPath.c_str(), FILE_PERMISSION_FULL);
        return;
    }
    std::string path = dirPath;
    std::vector<std::string> dirsToCreate;
    while (!path.empty() && !SPUtils::FileAccess(path)) {
        dirsToCreate.push_back(path);
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos) {
            break;
        }
        path = path.substr(0, pos);
    }
    mode_t oldMask = umask(0);
    for (auto it = dirsToCreate.rbegin(); it != dirsToCreate.rend(); ++it) {
        if (mkdir(it->c_str(), FILE_PERMISSION_FULL) != 0 && errno != EEXIST) {
            umask(oldMask);
            LOGE("%s not be created, errno %d!", it->c_str(), errno);
            return;
        }
        LOGD("%s created successfully!", it->c_str());
    }
    umask(oldMask);
}

void SPUtils::RemoveDirOrFile(const std::string &dirPath)
{
    char pathChar[PATH_MAX] = {0x00};
    if ((realpath(dirPath.c_str(), pathChar) == nullptr)) {
        LOGI("%s is not exist.", dirPath.c_str());
        return;
    }
    LOGD("%s is exist, remove...", dirPath.c_str());
    struct stat pathStat;
    if (stat(pathChar, &pathStat) != 0) {
        LOGE("Failed to stat %s", pathChar);
        return;
    }
    if (!S_ISDIR(pathStat.st_mode)) {
        if (remove(pathChar) != 0) {
            LOGE("Failed to remove file %s", pathChar);
        } else {
            LOGI("File %s removed successfully!", pathChar);
        }
        return;
    }
    DIR *dir = opendir(pathChar);
    if (dir == nullptr) {
        LOGE("Failed to open dir %s", pathChar);
        return;
    }
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        std::string childPath = std::string(pathChar) + "/" + entry->d_name;
        RemoveDirOrFile(childPath);
    }
    closedir(dir);
    if (rmdir(pathChar) != 0) {
        LOGE("Failed to remove dir %s", pathChar);
    }
}

bool SPUtils::CopyFile(const std::string &src, const std::string &dst)
{
    char srcReal[PATH_MAX] = {0x00};
    if (realpath(src.c_str(), srcReal) == nullptr) {
        LOGE("CopyFile src path invalid: %s", src.c_str());
        return false;
    }
    std::ifstream srcFile(srcReal, std::ios::binary);
    if (!srcFile.is_open()) {
        LOGE("CopyFile failed to open src: %s", srcReal);
        return false;
    }
    size_t lastSlash = dst.find_last_of('/');
    std::string fileName = (lastSlash != std::string::npos) ? dst.substr(lastSlash + 1) :
        dst;
    std::string safePath;
    if (lastSlash != std::string::npos) {
        std::string dirPath = dst.substr(0, lastSlash);
        char dstReal[PATH_MAX] = {0x00};
        if (realpath(dirPath.c_str(), dstReal) == nullptr) {
            LOGE("data dir %s is nullptr", dirPath.c_str());
            return false;
        }
        safePath = std::string(dstReal) + "/" + fileName;
    } else {
        safePath = fileName;
    }
    std::ofstream dstFile(safePath, std::ios::binary);
    if (!dstFile.is_open()) {
        LOGE("CopyFile failed to open dst: %s", safePath.c_str());
        return false;
    }
    dstFile << srcFile.rdbuf();
    if (!dstFile.good()) {
        LOGE("CopyFile write failed: %s", safePath.c_str());
        return false;
    }
    return true;
}

void SPUtils::TarFiles(const std::string& tarStr)
{
    std::string tarCommand = CMD_COMMAND_MAP.at(CmdCommand::TAR) + tarStr;
    std::string cmdResult;
    if (!SPUtils::LoadCmd(tarCommand, cmdResult)) {
        LOGE("Failed to tar log files");
    }
}

size_t SPUtils::GetFileSize(std::string filePath)
{
    struct stat statbuf;
    if (stat(filePath.c_str(), &statbuf) == -1) {
        LOGE("Failed to get file size for %s", filePath.c_str());
        return 0;
    }
    return static_cast<size_t>(statbuf.st_size);
}

bool SPUtils::IsFindDHGame(const std::string &pkg)
{
    if (!IsValidPkgName(pkg)) {
        LOGE("SPUtils::IsFindDHGame invalid pkg name");
        return false;
    }
    bool isdhGame = false;
    const std::string dumperSurface = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SURFACE);
    char buf[1024] = {'\0'};
    FILE *fd = popen(dumperSurface.c_str(), "r");
    if (fd == nullptr) {
        return isdhGame;
    }
    while (fgets(buf, sizeof(buf), fd) != nullptr) {
        std::string line = buf;
        if (line.find(pkg) != std::string::npos || line.find("ShellAssistantAnco") != std::string::npos) {
            isdhGame = true;
            break;
        }
    }
    pclose(fd);
    return isdhGame;
}

std::string SPUtils::GetSurface()
{
    std::string cmdResult;
    std::string dumperSurface = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SURFACE);
    LoadCmd(dumperSurface, cmdResult);
    size_t positionLeft = cmdResult.find("[");
    size_t positionRight = cmdResult.find("]");
    size_t positionNum = 1;
    return cmdResult.substr(positionLeft + positionNum, positionRight - positionLeft - positionNum);
}

std::string SPUtils::GetProductName()
{
    return PRODUCT_NAME;
}

std::string SPUtils::EscapeShellArgs(const std::string& args)
{
    std::string escaped;
    const size_t expansion = 2;
    escaped.reserve(args.size() * expansion);
    const char* specicalChars = R"(\\"$`;&|><(){}[]^~*?#!%@)";
    for (char c : args) {
        if (std::strchr(specicalChars, c)) {
            escaped += '\\';
        }
        escaped += c;
    }
    LOGD("SPUtils::EscapeShellArgs escaped = (%s)", escaped.c_str());
    return escaped;
}

bool SPUtils::IsValidPkgName(const std::string& pkgName)
{
    if (pkgName.empty() || !isalpha(pkgName[0])) {
        return false;
    }
    return std::all_of(pkgName.begin(), pkgName.end(), [](char c) {
        return isalnum(c) || c == '.' || c == '_';
    });
}

std::string SPUtils::GetWinId()
{
    std::string winId = "";
    std::string cmdResult;
    if (!SafeLoadExec({"hidumper", "-s", "WindowManagerService", "-a", "-a"}, cmdResult)) {
        LOGE("hidumper is failed");
        return "";
    }
    std::istringstream iss(cmdResult);
    std::string line;
    while (std::getline(iss, line)) {
        GetWinIdNum(line, winId);
    }
    if (!SPUtils::IsValidPid(winId)) {
        LOGE("winId is Abnormal characters");
        return "";
    }
    return winId;
}

void SPUtils::GetWinIdNum(std::string &hidumperStr, std::string &winId)
{
    if (hidumperStr.find("hw0") != std::string::npos) {
        int inDex = 3;
        std::istringstream iss(hidumperStr);
        std::string token;
        int count = 0;
        while (iss >> token) {
            if (count == inDex) {
                winId = token;
                break;
            }
            count++;
        }
    }
}

void SPUtils::EnDumpFun(const std::string &winId)
{
    const std::string dumpCmd = "hidumper -s WindowManagerService -a '-w " + winId + " -touchmonitor -b'";
    std::string cmdResult = "";
    LoadCmd(dumpCmd, cmdResult);
}

void SPUtils::OverDumpFun()
{
    std::string winId = GetWinId();
    std::string cmdResult = "";
    const std::string dumpCmd = "hidumper -s WindowManagerService -a '-w " + winId + " -touchmonitor -e'";
    LoadCmd(dumpCmd, cmdResult);
}

bool SPUtils::ConvertTraceByTime(const std::string& inputPath, const std::string& outputPath,
    uint64_t durationSec, std::string& errorMsg)
{
    auto parser = std::make_shared<TraceFileParser>(inputPath);
    if (!parser->Parse()) {
        errorMsg = parser->GetErrorMsg();
        return false;
    }

    uint64_t endTime = parser->GetTraceEndTime();
    uint64_t startTime = (endTime > durationSec * 1000000000)
                         ? (endTime - durationSec * 1000000000)
                         : parser->GetTraceStartTime();

    TraceClipper clipper(parser);
    if (!clipper.ClipByTimeRange(startTime, endTime)) {
        errorMsg = clipper.GetErrorMsg();
        return false;
    }

    if (!clipper.SaveToFile(outputPath)) {
        errorMsg = clipper.GetErrorMsg();
        return false;
    }

    return true;
}
}
}
