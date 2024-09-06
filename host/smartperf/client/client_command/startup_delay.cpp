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
#include <thread>
#include <cstdio>
#include <ios>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <sys/wait.h>
#include <sys/types.h>
#include "unistd.h"
#include "include/startup_delay.h"
#include "include/sp_utils.h"
#include "unistd.h"
#include "include/sp_log.h"
#include "include/common.h"

namespace OHOS {
namespace SmartPerf {
StartUpDelay::StartUpDelay() {}
StartUpDelay::~StartUpDelay() {}
void StartUpDelay::GetTrace(const std::string &sessionID, const std::string &traceName) const
{
    std::string result;
    std::string cmdString;
    if (SPUtils::IsHmKernel()) {
        cmdString = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_1024);
    } else {
        cmdString = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_2048);
    }
    SPUtils::LoadCmd(cmdString + traceName, result);
}

std::thread StartUpDelay::ThreadGetTrace(const std::string &sessionID, const std::string &traceName) const
{
    auto thGetTrace = std::thread([this, sessionID, traceName]() { this->GetTrace(sessionID, traceName); });
    return thGetTrace;
}
void StartUpDelay::GetLayout()
{
    std::string result;
    std::string uitest = CMD_COMMAND_MAP.at(CmdCommand::UITEST_DUMPLAYOUT);
    SPUtils::LoadCmd(uitest, result);
}
std::thread StartUpDelay::ThreadGetLayout()
{
    auto thGetLayout = std::thread([this]() { this->GetLayout(); });
    return thGetLayout;
}
void StartUpDelay::InputEvent(const std::string &point)
{
    std::string cmdResult = "";
    int time = 4;
    sleep(time);
    std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_POINT) + point + " -u" + point;
    SPUtils::LoadCmd(cmd, cmdResult);
}
std::thread StartUpDelay::ThreadInputEvent(const std::string &point)
{
    auto thInputEvent = std::thread([this, point]() { this->InputEvent(point); });
    return thInputEvent;
}
void StartUpDelay::GetHisysId() const
{
    int time = 10;
    sleep(time);
    std::string str = "";
    std::string cmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYSEVENT);
    SPUtils::LoadCmd(cmd, str);
    std::stringstream ss(str);
    std::string line = "";
    getline(ss, line);
    std::stringstream ssLine(line);
    std::string word = "";
    std::string secondStr;
    int count = 0;
    int num = 2;
    while (ssLine >> word) {
        count++;
        if (count == num) {
            secondStr = word;
            break;
        }
    }
    std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD);
    SPUtils::LoadCmd(killCmd + secondStr, str);
}

void StartUpDelay::GetHisysIdAndKill() const
{
    int time = 10;
    sleep(time);
    std::string str = "";
    std::string cmd = HISYSEVENT_CMD_MAP.at(HisyseventCmd::HISYS_PID);
    SPUtils::LoadCmd(cmd, str);
    std::stringstream ss(str);
    std::vector<std::string> hisysIdVec;
    std::string singleId;
    while (ss >> singleId) {
        hisysIdVec.push_back(singleId);
    }
    std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD);
    for (size_t i = 0; i < hisysIdVec.size(); i++) {
        SPUtils::LoadCmd(killCmd + hisysIdVec[i], str);
    }
}
bool StartUpDelay::GetSpTcp()
{
    std::string resultPid;
    std::string str;
    std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::PIDOF_SP);
    SPUtils::LoadCmd(cmd, resultPid);
    std::vector<std::string> vec;
    std::string token;
    size_t pos = 0;
    while ((pos = resultPid.find(' ')) != std::string::npos) {
        token = resultPid.substr(0, pos);
        vec.push_back(token);
        resultPid.erase(0, pos + 1);
    }
    if (vec.size() > 0) {
        std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD);
        for (size_t i = 0; i < vec.size(); i++) {
            SPUtils::LoadCmd(killCmd + vec[i], str);
        }
    }
    return false;
}

bool StartUpDelay::GetSpClear()
{
    std::string resultPid;
    std::string str;
    std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::PIDOF_SP);
    SPUtils::LoadCmd("pidof SP_daemon", resultPid);
    std::string token;
    std::string curPid = std::to_string(getpid());
    std::stringstream ss(resultPid);
    std::string killCmd = CMD_COMMAND_MAP.at(CmdCommand::KILL_CMD);
    while (ss >> token) {
        if (token != curPid) {
            SPUtils::LoadCmd("kill " + token, str);
        }
    }
    return false;
}

std::thread StartUpDelay::ThreadGetHisysIds() const
{
    auto thGetHisysIds = std::thread([this]() { this->GetHisysIdAndKill(); });
    return thGetHisysIds;
}

std::thread StartUpDelay::ThreadGetHisysId() const
{
    auto thGetHisysId = std::thread([this]() { this->GetHisysId(); });
    return thGetHisysId;
}
void StartUpDelay::ChangeToBackground()
{
    std::string result;
    sleep(1);
    std::string uinput = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_BACK);
    SPUtils::LoadCmd(uinput, result);
    sleep(1);
}
std::string StartUpDelay::GetPidByPkg(const std::string &curPkgName)
{
    std::string resultPid = "";
    std::string resultProcId = "";
    const char* args[] = { "pidof", curPkgName.c_str(), nullptr };
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        LOGE("startup_delay::Failed to create pipe");
        return resultPid;
    }
    pid_t pid = fork();
    if (pid == -1) {
        LOGE("startup_delay::Failed to fork");
        return resultPid;
    } else if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        if (execvp(args[0], const_cast<char *const*>(args)) == -1) {
            LOGE("startup_delay::Failed to execute pid");
            return resultPid;
        }
    } else {
        close(pipefd[1]);
        char buf[1024];
        ssize_t nread = read(pipefd[0], buf, sizeof(buf));
        if (nread == -1) {
            return resultPid;
        }
        resultPid = std::string(buf, nread);
        resultProcId = resultPid.substr(0, resultPid.size() - 1);
        close(pipefd[0]);
        waitpid(pid, nullptr, 0);
    }
    return resultProcId;
}
void StartUpDelay::InitXY2(const std::string &curAppName, const std::string &fileName, const std::string &appPkgName)
{
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(fileName.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    std::ifstream file(realPath, std::ios::in);
    std::string strLine = "";
    std::regex pattern("\\d+");
    size_t findIndex = std::string::npos;
    while (getline(file, strLine)) {
        size_t appPkgIndex = strLine.find("AppName_text_" + appPkgName);
        size_t appIndex = strLine.find(curAppName);
        if (appIndex != std::string::npos) {
            findIndex = appIndex;
        } else {
            findIndex = appPkgIndex;
        }
        if (findIndex == std::string::npos) {
            break;
        }
        size_t bounds = strLine.rfind("bounds", findIndex);
        if (bounds > 0) {
            std::string boundStr = strLine.substr(bounds, 30);
            std::smatch result;
            std::string::const_iterator iterStart = boundStr.begin();
            std::string::const_iterator iterEnd = boundStr.end();
            std::vector<std::string> pointVector;
            while (std::regex_search(iterStart, iterEnd, result, pattern)) {
                std::string startX = result[0];
                iterStart = result[0].second;
                pointVector.push_back(startX);
            }
            size_t num = 3;
            if (pointVector.size() > num) {
                int x = (std::atoi(pointVector[2].c_str()) + std::atoi(pointVector[0].c_str())) / 2;
                int y = (std::atoi(pointVector[3].c_str()) + std::atoi(pointVector[1].c_str())) / 2;
                pointXY = std::to_string(x) + " " + std::to_string(y);
            } else {
                size_t leftStart = boundStr.find_first_of("[");
                size_t leftEnd = boundStr.find_first_of("]");
                int pointXYlength = static_cast<int>(leftEnd - leftStart);
                pointXY = boundStr.substr(leftStart + 1, pointXYlength - 1);
                pointXY = pointXY.replace(pointXY.find(","), 1, " ");
            }
            break;
        }
    }
}
void StartUpDelay::InitXY(const std::string &curAppName, const std::string &fileName)
{
    char realPath[PATH_MAX] = {0x00};
    if ((realpath(fileName.c_str(), realPath) == nullptr)) {
        std::cout << "" << std::endl;
    }
    std::ifstream file(realPath, std::ios::in);
    std::string strLine = "";
    std::regex pattern("\\d+");
    size_t appIndex = -1;
    while (getline(file, strLine)) {
        appIndex = strLine.find(curAppName);
        if (appIndex <= 0) {
            break;
        }
        size_t bounds = strLine.rfind("bounds", appIndex);
        if (bounds > 0) {
            std::string boundStr = strLine.substr(bounds, 30);
            std::smatch result;
            std::string::const_iterator iterStart = boundStr.begin();
            std::string::const_iterator iterEnd = boundStr.end();
            std::vector<std::string> pointVector;
            while (std::regex_search(iterStart, iterEnd, result, pattern)) {
                std::string startX = result[0];
                iterStart = result[0].second;
                pointVector.push_back(startX);
            }
            size_t num = 3;
            size_t pointNum = pointVector.size();
            if (pointNum > num) {
                int x = (std::atoi(pointVector[2].c_str()) + std::atoi(pointVector[0].c_str())) / 2;
                int y = (std::atoi(pointVector[3].c_str()) + std::atoi(pointVector[1].c_str())) / 2;
                pointXY = std::to_string(x) + " " + std::to_string(y);
            } else {
                size_t leftStart = boundStr.find_first_of("[");
                size_t leftEnd = boundStr.find_first_of("]");
                int pointXYlength = static_cast<int>(leftEnd - leftStart);
                pointXY = boundStr.substr(leftStart + 1, pointXYlength - 1);
                pointXY = pointXY.replace(pointXY.find(","), 1, " ");
            }
            break;
        }
    }
}
}
}
