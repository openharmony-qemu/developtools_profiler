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
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#include "include/sp_utils.h"
#include "include/FPS.h"
#include "include/startup_delay.h"
#include "include/sp_log.h"
#include "include/common.h"
#include <sys/wait.h>
#include <sys/types.h>
#include "include/GameEvent.h"
#include "transaction/rs_interfaces.h"
#include "include/ByTrace.h"
#include "include/sp_profiler_factory.h"
namespace OHOS {
namespace SmartPerf {
bool HAVE_CATON = false;
std::map<std::string, std::string> FPS::ItemData()
{
    std::map<std::string, std::string> result;
    FpsInfo fpsInfoResult;
    if (surfaceViewName.length() > 0) {
        fpsInfoResult = GetDiffLayersFpsInfo(surfaceViewName);
    } else {
        fpsInfo.currDumpTimeStamps.clear();
        fpsInfoResult = GetFpsInfo();
    }
    prevResultFpsInfo = fpsInfoResult;
    std::string value = FindFpsRefreshrate();
    result["refreshrate"] = value;
    if (processFlag) {
        result["fps"] = "NA";
        result["fpsJitters"] = "NA";
    } else {
        GetFpsAndJitters(fpsInfoResult, result);
    }
    LOGI("FPS:ItemData map size(%u)", result.size());
    return result;
}

std::map<std::string, std::string> FPS::GetFpsAndJitters(FpsInfo &fpsInfoResult,
    std::map<std::string, std::string> &result)
{
    const int fullFrame = 120;
    const int maxFullFrame = 123;
    if (fpsInfoResult.fps > fullFrame && fpsInfoResult.fps < maxFullFrame) {
        fpsInfoResult.fps = fullFrame;
    }
    result["fps"] = std::to_string(fpsInfoResult.fps);
    std::string jitterStr = "";
    std::string split = "";
    for (size_t i = 0; i < fpsInfoResult.jitters.size(); i++) {
        if (i > 0) {
            split = ";;";
        }
        jitterStr += split + std::to_string(fpsInfoResult.jitters[i]);
    }
    if (hapNeedCatonInfo && inGameSceneNum > eleven) {
        CalcFatalCaton(fpsInfoResult.jitters);
    }
    result["fpsJitters"] = jitterStr;
    LOGD("result.fps: %s, result.curTime: %s, result.jitters: %s",
        std::to_string(fpsInfoResult.fps).c_str(),
        std::to_string(fpsInfoResult.curTime).c_str(),
        jitterStr.c_str());
    SetFpsCurrentFpsTime(fpsInfoResult);
    if (isCatchTrace > 0) {
        long long maxJitters = 0;
        if (!fpsInfoResult.jitters.empty()) {
            auto maxElement = std::max_element(fpsInfoResult.jitters.begin(), fpsInfoResult.jitters.end());
            maxJitters = static_cast<long long>(*maxElement / oneSec);
        }
        ByTrace::GetInstance().CheckFpsJitters(maxJitters, fpsInfoResult.fps);
    }
    return result;
}

void FPS::CalcFatalCaton(std::vector<long long>& jitters)
{
    if (!HAVE_CATON && catonNum == 0) {
        for (long long &timestamp : jitters) {
            if ((timestamp / 1e6) > hundred) {
                HAVE_CATON = true;
                catonNum = 1;
                ipcCallback_("haveCaton");
                break;
            }
        }
    }
}

void FPS::GetGameScenStatus()
{
    if (!hapNeedCatonInfo || catonNum > 0) {
        return;
    }
    std::map<std::string, std::string> gameEvent = GameEvent::GetInstance().GetGameEventItemData();
    for (const auto& item : gameEvent) {
        if (item.first == "sceneId") {
            if (item.second == "4") {
                inGameSceneNum++;
            } else {
                inGameSceneNum = 0;
            }
        }
    }
    LOGD("IN GAME SCENE inGameSceneNum (%d)", inGameSceneNum);
}

void FPS::StartExecutionOnce(bool isPause)
{
    if (isPause) {
        return;
    }
    isGameApp = SPUtils::GetIsGameApp(pkgName);
    catonNum = 0;
}

void FPS::SetFpsCurrentFpsTime(FpsInfo fpsInfoResult)
{
    ffTime.fps = fpsInfoResult.fps;
    if (!fpsInfoResult.jitters.empty()) {
        auto maxElement = std::max_element(fpsInfoResult.jitters.begin(), fpsInfoResult.jitters.end());
        ffTime.currentFpsTime = *maxElement;
    }
}

FpsCurrentFpsTime FPS::GetFpsCurrentFpsTime()
{
    return ffTime;
}

void FPS::SetPackageName(const std::string& pName)
{
    pkgName = pName;
}

void FPS::SetProcessId(const std::string &pid)
{
    processId = pid;
}

void FPS::SetLayerName(const std::string& sName)
{
    surfaceViewName = sName;
    isNeedDump = true;
}
FpsInfo FPS::GetDiffLayersFpsInfo(const std::string &sName)
{
    OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
    fpsInfoData = GetSurfaceFrame(sName);
    return fpsInfoData;
}

FpsInfo FPS::GetFpsInfo()
{
    processFlag = false;
    fpsInfoData.fps = 0;
    if (isGameApp) {
        if (gameLayerName.empty()) {
            std::string processIds = "";
            OHOS::SmartPerf::StartUpDelay sp;
            processId = sp.GetPidByPkg(pkgName, &processIds);
            SpProfilerFactory::SetProfilerPidByPkg(processId, processIds);
            gameLayerName = GetGameLayer();
            if (gameLayerName.empty()) {
                fpsInfoData.fps = 0;
                fpsInfoData.jitters.clear();
                return fpsInfoData;
            }
        }
        GetGameScenStatus();
        OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
        fpsInfoData = GetSurfaceFrame(gameLayerName);
        if (fpsInfoData.fps == 0) {
            return GetChangedLayerFps();
        } else {
            return fpsInfoData;
        }
    } else {
        std::string uniteLayer;
        if (ohFlag) {
            uniteLayer = OHOS::SmartPerf::SPUtils::GetSurface();
        } else if (!rkFlag && !isOtherDevice) {
            uniteLayer = "UniRender";
        } else {
            uniteLayer = OHOS::SmartPerf::SPUtils::GetSurface();
            isNeedDump = true;
        }
        if (isSections || pkgName.find("sceneboard") != std::string::npos) {
            OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
            fpsInfoData = GetSurfaceFrame(uniteLayer);
        } else {
            fpsInfoData = GetAppFps(uniteLayer);
        }
    }
    return fpsInfoData;
}

FpsInfo FPS::GetAppFps(std::string &uniteLayer)
{
    bool onTop = OHOS::SmartPerf::SPUtils::IsForeGround(pkgName);
    if (onTop) {
        OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
        fpsInfoData = GetSurfaceFrame(uniteLayer);
    } else {
        LOGD("FPS::app is in the background");
        if (processId.empty()) {
            processFlag = true;
        }
        fpsInfoData.fps = 0;
        fpsInfoData.jitters.clear();
    }
    return fpsInfoData;
}

bool FPS::SetOtherDeviceFlag()
{
    isOtherDevice = true;
    return isOtherDevice;
}

FpsInfo FPS::GetChangedLayerFps()
{
    gameLayerName = GetGameLayer();
    if (gameLayerName.empty()) {
        //start-stop -VIEW -profilerfps ohtest rk3568 historyHap
        if (processId.empty()) {
            processFlag = true;
        }
        fpsInfoData.fps = 0;
        fpsInfoData.jitters.clear();
    } else {
        fpsInfoData = GetSurfaceFrame(gameLayerName);
    }
    return fpsInfoData;
}

FpsInfo FPS::GetSurfaceFrame(const std::string& name)
{
    if (name == "") {
        return FpsInfo();
    }
    FpsInfo tmpFps;
    tmpFps.fps = 0;
    tmpFps.jitters.clear();
    fpsInfo = tmpFps;
    fpsNum = 0;
    prevScreenTimestamp = -1;
    struct timespec time1 = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
    fpsInfo.curTime = static_cast<int>(time1.tv_sec - 1);
    fpsInfo.currTimeDump = (time1.tv_sec - 1) * mod + time1.tv_nsec;
    LOGD("FPS:fpsInfo.curTime: %d, FPS:psInfo.currTimeDump: %lld", fpsInfo.curTime, fpsInfo.currTimeDump);
    if (name == "UniRender" || isNeedDump || isHistoryHap) {
        fpsInfo = GetFpsInfoByDump(name);
    } else {
        fpsInfo = GetFpsInfoByRs(name);
    }
    return fpsInfo;
}

FpsInfo FPS::GetFpsInfoByDump(const std::string& name)
{
    std::string command = "";
    if (isNeedDump && isGameApp) {
        command = "fps -id " + name;
    } else {
        command = "fps " + name;
    }
    const std::vector<const char*> args = { "hidumper", "-s", "10", "-a", command.c_str(), nullptr };
    int pipefd[2];
    pid_t pid;
    if (pipe(pipefd) == -1) {
        LOGE("FPS::Failed to create pipe");
        return fpsInfo;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("FPS::Failed to fork");
        return fpsInfo;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execvp(args[0], const_cast<char *const*>(args.data()));
        LOGE("FPS::Failed to execute hidumper");
        _exit(EXIT_FAILURE);
    }
    close(pipefd[1]);
    ReadDataFromPipe(pipefd[0]);
    waitpid(pid, nullptr, 0);
    return fpsInfo;
}


void FPS::ReadDataFromPipe(int fd)
{
    char tmp[1024];
    bool isBreak = false;
    if (isSections) {
        GetTimeDiff();
    }
    FILE *fp = fdopen(fd, "r");
    if (!fp) {
        LOGE("FPS::Failed to open file descriptor");
        close(fd);
        return;
    }
    std::stringstream sstream;
    while (fgets(tmp, sizeof(tmp), fp) != nullptr) {
        LOGD("FPS::ReadDataFromPipe::dump time: %s", tmp);
        std::string tmpStr(tmp);
        curScreenTimestamp = 0;
        sstream.clear();
        sstream.str(tmpStr);
        sstream >> curScreenTimestamp;
        if (curScreenTimestamp == 0) {
            continue;
        }
        if (CalcFpsAndJitters(isBreak)) {
            break;
        }
    }
    (void)fclose(fp);
    if (!hapLowFpsFlag || !isLowCurFps) {
        CalcJitters();
    }
}

FpsInfo FPS::GetFpsInfoByRs(const std::string& name)
{
    bool isBreak = false;
    uint64_t nodeId;
    std::istringstream s(name);
    s >> nodeId;
    LOGD("FPS::GetFpsInfoByRs nodeId: (%lld)", nodeId);
    std::string fpsInfoResult = OHOS::Rosen::RSInterfaces::GetInstance().GetRefreshInfoToSP(nodeId);
    LOGD("FPS fpsInfoResult: %s", fpsInfoResult.c_str());
    std::stringstream iss;
    iss << fpsInfoResult;
    std::string timeStampLine;
    while (getline(iss, timeStampLine, '\n')) {
        timeStampLine.erase(0, timeStampLine.find_first_not_of(' '));
        curScreenTimestamp = 0;
        if (timeStampLine.empty()) {
            continue;
        }
        std::stringstream ss;
        ss.clear();
        ss << timeStampLine;
        ss >> curScreenTimestamp;
        if (curScreenTimestamp == 0) {
            continue;
        }
        if (CalcFpsAndJitters(isBreak)) {
            break;
        }
    }
    CalcJitters();
    return fpsInfo;
}

bool FPS::CalcFpsAndJitters(bool isBreak)
{
    long long onScreenTime = curScreenTimestamp / mod;
    bool findFpsCurTime = (onScreenTime == fpsInfo.curTime);
    if (findFpsCurTime) {
        isBreak = true;
        fpsNum++;
        fpsInfo.fps = fpsNum;
        fpsInfo.currDumpTimeStamps.push_back(curScreenTimestamp);
    } else {
        findFpsCurTime = false;
    }
    return isBreak && !findFpsCurTime;
}

void FPS::CalcJitters()
{
    bool isOrder = true;
    if (fpsInfo.currDumpTimeStamps.size() > 1) {
        isOrder = fpsInfo.currDumpTimeStamps[1] - fpsInfo.currDumpTimeStamps[0] > 0;
    }
    if (isOrder) {
        for (size_t i = 0; i < fpsInfo.currDumpTimeStamps.size(); i++) {
            curScreenTimestamp = fpsInfo.currDumpTimeStamps[i];
            fpsInfo.currTimeStamps.push_back(curScreenTimestamp);
            long long jitter = CalculateJitter();
            fpsInfo.jitters.push_back(jitter);
            prevlastScreenTimestamp = curScreenTimestamp;
            prevScreenTimestamp = curScreenTimestamp;
        }
    } else {
        for (size_t i = fpsInfo.currDumpTimeStamps.size(); i > 0 ; i--) {
            curScreenTimestamp = fpsInfo.currDumpTimeStamps[i - 1];
            fpsInfo.currTimeStamps.push_back(curScreenTimestamp);
            long long jitter = CalculateJitter();
            fpsInfo.jitters.push_back(jitter);
            prevlastScreenTimestamp = curScreenTimestamp;
            prevScreenTimestamp = curScreenTimestamp;
        }
    }
    if (!fpsInfo.jitters.empty()) {
        fpsInfo.jitters.erase(fpsInfo.jitters.begin());
    }
}
        
long long FPS::CalculateJitter() const
{
    long long jitter;
    if (prevScreenTimestamp == -1) {
        if (prevlastScreenTimestamp != 0 && (curScreenTimestamp - prevlastScreenTimestamp) < mod) {
            jitter = curScreenTimestamp - prevlastScreenTimestamp;
        } else {
            jitter = curScreenTimestamp % mod;
        }
    } else {
        jitter = curScreenTimestamp - prevScreenTimestamp;
    }
    return jitter;
}

void FPS::SetRkFlag()
{
    rkFlag = true;
}

std::string FPS::FindFpsRefreshrate()
{
    std::string value;
    std::string screenInfo;
    SPUtils::LoadFile(screenPath, screenInfo);
    value = GetHardenRefreshrate(screenInfo);
    size_t pos = 0;
    std::string token;
    if (!rkFlag) {
        while ((pos = screenInfo.find(";")) != std::string::npos) {
            token = screenInfo.substr(0, pos);
            screenInfo.erase(0, pos + 1);
            if (token.find("current_fps:") != std::string::npos || token.find("lcd_fps:") != std::string::npos) {
                value = token.substr(token.find(":") + 1);
                break;
            }
        }
    } else {
        std::string screen = OHOS::SmartPerf::SPUtils::GetScreen();
        pos = screen.find("=");
        if (pos != std::string::npos) {
            value = screen.substr(pos + 1);
            if (!value.empty() && value.back() == '\n') {
                value.pop_back();
            }
        }
    }
    return value;
}

std::string FPS::GetHardenRefreshrate(std::string &screenInfo) const
{
    if (screenInfo.empty()) {
        SPUtils::LoadCmd(HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_SCREEN), screenInfo);
    }
    std::string value = "";
    std::string refreshrate = "refreshrate=";
    size_t activeModePos = screenInfo.find("activeMode:");
    if (activeModePos != std::string::npos) {
        size_t refreshRatePos = screenInfo.find(refreshrate, activeModePos);
        if (refreshRatePos != std::string::npos) {
            size_t endPos = screenInfo.find(" ", refreshRatePos);
            if (endPos != std::string::npos) {
                value = screenInfo.substr(refreshRatePos + refreshrate.length(),
                endPos - refreshRatePos - refreshrate.length());
            }
        }
    }
    return value;
}

std::string FPS::GetGameLayer()
{
    std::string gameLayer = "";
    if (processId.empty()) {
        LOGE("FPS::processId is empty");
        return gameLayer;
    }
    const std::string dumperSurface = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_RS_TREE);
    uint64_t nodeId;
    std::string start = "SURFACE_NODE[";
    std::string end = "],";
    char buf[1024] = {'\0'};
    FILE *fd = popen(dumperSurface.c_str(), "r");
    if (fd == nullptr) {
        LOGE("FPS::fd is nullptr");
        return gameLayer;
    }
    while (fgets(buf, sizeof(buf), fd) != nullptr) {
        std::string line = buf;
        size_t startPos = line.find(start);
        size_t endPos = line.find_first_of(end);
        if (startPos != std::string::npos && endPos != std::string::npos) {
            nodeIdStr = line.substr(startPos + start.length(), endPos - startPos - start.length());
        }
        if (line.find(pkgName) != std::string::npos ||
            (line.find("ShellAssistantAnco") != std::string::npos && line.find("Surface") != std::string::npos)) {
            pclose(fd);
            return nodeIdStr;
        }
        const int kShiftAmount = 32;
        if (!nodeIdStr.empty()) {
            std::stringstream ss(nodeIdStr);
            ss >> nodeId;
            if (ss.fail() || !ss.eof()) {
                pclose(fd);
                return gameLayer;
            }
            nodeId = nodeId >> kShiftAmount;
            gameLayer = GetLayerName(gameLayer, nodeId, line, endPos);
            if (!gameLayer.empty()) {
                break;
            }
        }
    }
    if (pclose(fd) == -1) {
        LOGE("FPS Error: Failed to close file");
        return gameLayer;
    }
    LOGD("FPS::gameLayer: (%s)", gameLayer.c_str());
    return gameLayer;
}

std::string FPS::GetLayerName(std::string &gameLayer, uint64_t &nodeId, const std::string& line, size_t &endPos)
{
    if (isPreset) {
        gameLayer = GetSurfaceId(gameLayer, nodeId, processId, line, endPos);
        LOGD("FPS::GetLayerName::processId: (%s)", processId.c_str());
    } else {
        StartUpDelay startUpDelay;
        if (startUpDelay.GetPidParams().empty()) {
            gameLayer = GetSurfaceId(gameLayer, nodeId, processId, line, endPos);
        }
        for (const auto& pid : startUpDelay.GetPidParams()) {
            gameLayer = GetSurfaceId(gameLayer, nodeId, pid, line, endPos);
            if (!gameLayer.empty()) {
                break;
            }
        }
    }
    return gameLayer;
}

std::string FPS::GetSurfaceId(std::string &gameLayer, uint64_t &nodeId, const std::string &surfaceId,
    const std::string& line, size_t &endPos)
{
    if (std::to_string(nodeId) == surfaceId &&
        line.find("VisibleRegion [Empty], OpaqueRegion [Empty]") != std::string::npos) {
        if (isHistoryHap) {
            size_t startSixPos = 6;
            size_t layerStartPos = line.find("Name [");
            size_t layerEndPos = line.find("], hasConsumer");
            if (layerEndPos - layerStartPos <= 1 && layerEndPos > endPos) {
                return gameLayer;
            }
            layerStartPos += startSixPos;
            gameLayer = line.substr(layerStartPos, layerEndPos - layerStartPos);
        } else {
            gameLayer = nodeIdStr;
        }
    }
    LOGD("FPS::GetSurfaceId::gameLayer: (%s)", gameLayer.c_str());
    return gameLayer;
}

void FPS::SetTraceCatch()
{
    isCatchTrace = 1;
}

void FPS::GetOhFps(std::vector<std::string>& v)
{
    if (v[number] == "") {
        printf("the args of num must be not-null!\n");
    } else {
        this->num = SPUtilesTye::StringToSometype<int>(v[number].c_str());
        if (this->num < 0) {
            printf("set num:%d not vaild arg\n", this->num);
        }
        printf("set num:%d success\n", this->num);
        ohFlag = true;
        int sectionsNum;
        if (static_cast<int>(v.size()) < four) {
            sectionsNum = 0;
        } else {
            sectionsNum = SPUtilesTye::StringToSometype<int>(v[four].c_str());
        }
        for (int i = 0; i < this->num; i++) {
            GetResultFPS(sectionsNum);
        }
    }
    printf("SP_daemon exec finished!\n");
}

void FPS::GetTimeDiff()
{
    long long clockRealTime = 0;
    long long clockMonotonicRaw = 0;
    const int two = 2;
    std::string strRealTime;
    const std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::TIMESTAMPS);
    FILE *fd = popen(cmd.c_str(), "r");
    if (fd == nullptr) {
        return;
    }
    char buf[1024] = {'\0'};
    while ((fgets(buf, sizeof(buf), fd)) != nullptr) {
        std::string line(buf);
        std::vector<std::string> params;
        SPUtils::StrSplit(line, " ", params);
        if (params[0].find("CLOCK_REALTIME") != std::string::npos && clockRealTime == 0) {
            strRealTime = params[two];
            strRealTime.erase(strRealTime.find('.'), 1);
            clockRealTime = SPUtilesTye::StringToSometype<long long>(strRealTime);
            currRealTime = clockRealTime;
        } else if (params[0].find("CLOCK_MONOTONIC_RAW") != std::string::npos && clockMonotonicRaw == 0) {
            strRealTime = params[two];
            strRealTime.erase(strRealTime.find('.'), 1);
            clockMonotonicRaw = SPUtilesTye::StringToSometype<long long>(strRealTime);
        }
    }
    if (pclose(fd) == -1) {
        LOGE("Error: Failed to close file");
        return;
    }
    fpsInfo.currTimeDiff = clockRealTime - clockMonotonicRaw;
}

void FPS::GetResultFPS(int sectionsNum)
{
    isNeedDump = true;
    isSections = true;
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, nullptr);
    FpsInfo fpsInfoResult = GetFpsInfo();
    unsigned long runTime;
    if (fpsInfoResult.fps == 0) {
        if (lastCurrTime == 0) {
            long long currTime = (fpsInfoResult.currTimeDump / msClear) * msClear + fpsInfoResult.currTimeDiff;
            lastCurrTime = currTime / oneSec;
            printf("fps:%d|%lld\n", fpsInfoResult.fps, currTime / oneSec);
        } else {
            printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime + oneThousand);
            lastCurrTime = lastCurrTime + oneThousand;
        }
    } else {
        long long currTime = (fpsInfoResult.currTimeStamps[0] / msClear) * msClear + fpsInfoResult.currTimeDiff;
        lastCurrTime = currTime / oneSec;
        printf("fps:%d|%lld\n", fpsInfoResult.fps, lastCurrTime);
    }
    prevResultFpsInfo = fpsInfoResult;
    if (sectionsNum != 0 && fpsInfoResult.fps != 0) {
        GetSectionsFps(fpsInfoResult, sectionsNum);
    }
    time_t now = time(nullptr);
    if (now == -1) {
        LOGE("Failed to get current time.");
        return;
    }
    char *dt = ctime(&now);
    LOGD("printf time is: %s", dt);
    fflush(stdout);
    gettimeofday(&end, nullptr);
    runTime = end.tv_sec * 1e6 - start.tv_sec * 1e6 + end.tv_usec - start.tv_usec;
    LOGD("printf time is runTime: %s", std::to_string(runTime).c_str());
    if (runTime < sleepTime) {
        usleep(sleepTime - runTime);
    }
    OHOS::SmartPerf::SPUtils::GetCurrentTime(prevResultFpsInfo.curTime);
}

void FPS::GetSectionsFps(FpsInfo &fpsInfoResult, int nums) const
{
    int msCount = 0;
    long long msJiange = 0;
    if (nums != 0) {
        msJiange = msClear / nums;
    }
    long long msStartTime = (fpsInfoResult.currTimeStamps[0] / msClear) * msClear + msJiange;
    long long currLastTime = lastCurrTime;
    long long harTime = msJiange / 1000000;
    int printCount = 0;
    long long currTimeStart = 0;
    long long currTimeLast = 0;
    for (size_t i = 0; i < fpsInfoResult.currTimeStamps.size(); i++) {
        long long currTime = fpsInfoResult.currTimeStamps[i];
        if (currTime <= msStartTime) {
            if (msCount == 0) {
                currTimeStart = currTime;
            }
            currTimeLast = currTime;
            msCount++;
        } else {
            while (currTime > msStartTime) {
                PrintSections(msCount, currTimeLast, currTimeStart, currLastTime);
                printCount++;
                msCount = 1;
                msStartTime += msJiange;
                currLastTime += harTime;
                currTimeLast = currTime;
                currTimeStart = currTime;
            }
        }
        if (i == (static_cast<size_t>(fpsInfoResult.currTimeStamps.size()) - 1)) {
            PrintSections(msCount, currTimeLast, currTimeStart, currLastTime);
            currTimeLast = currTime;
            printCount++;
            GetSectionsPrint(printCount, currLastTime, nums, harTime);
        }
    }
}

void FPS::PrintSections(int msCount, long long currTimeLast, long long currTimeStart, long long currLastTime) const
{
    int conversionFps = 1000000;
    int conversionTime = 1000;
    long long times = 120;
    int fpsNums = 0;
    if (msCount == 0) {
        fpsNums = 0;
    } else {
        fpsNums = msCount - 1;
    }
    double timeN = (currTimeLast - currTimeStart) * 1.0 / conversionTime;
    if (timeN == 0) {
        printf("sectionsFps:%d|%lld\n", 0, currLastTime);
        return;
    }
    double fpsSections = (fpsNums * conversionFps) / timeN;
    int fpsSectionsInt = round(fpsSections);
    if (fpsSectionsInt > static_cast<int>(times)) {
        fpsSectionsInt = static_cast<int>(times);
    }
    printf("sectionsFps:%d|%lld\n", fpsSectionsInt, currLastTime);
}

void FPS::GetSectionsPrint(int printCount, long long msStartTime, int numb, long long harTime) const
{
    if (printCount < numb) {
        for (int i = 0; i < numb - printCount; i++) {
            msStartTime += harTime;
            printf("sectionsFps:%d|%lld\n", 0, msStartTime);
        }
    }
}

void FPS::GetFPS(std::vector<std::string>& v)
{
    if (v[number] == "") {
        printf("the args of num must be not-null!\n");
    } else {
        this->num = SPUtilesTye::StringToSometype<int>(v[number].c_str());
        if (this->num < 0) {
            printf("set num:%d not valid arg\n", this->num);
        }
        printf("set num:%d success\n", this->num);
        int sectionsNum = (static_cast<int>(v.size()) >= four) ?
                            SPUtilesTye::StringToSometype<int>(v[four].c_str()) : 0;
        if (sectionsNum > ten) {
            printf("set sectionsNum:%d not valid arg \n", sectionsNum);
        } else {
            for (int i = 0; i < this->num; i++) {
                GetResultFPS(sectionsNum);
            }
        }
    }
    printf("SP_daemon exec finished!\n");
}
}
}

