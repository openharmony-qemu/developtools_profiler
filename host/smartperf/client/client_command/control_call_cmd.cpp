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
#include "unistd.h"
#include <thread>
#include <cstdio>
#include <cstring>
#include <map>
#include <sstream>
#include <iomanip>
#include <future>
#include "include/control_call_cmd.h"
#include "include/startup_delay.h"
#include "include/parse_trace.h"
#include "include/sp_utils.h"
#include "include/parse_click_complete_trace.h"
#include "include/parse_click_response_trace.h"
#include "include/parse_page_fps_trace.h"
#include "include/parse_start_frame_trace.h"
#include "include/parse_radar.h"
#include "include/parse_slide_fps_trace.h"
#include "include/sp_log.h"
#include "include/stalling_rate_trace.h"
#include "common.h"

namespace OHOS {
namespace SmartPerf {
std::string ControlCallCmd::GetResult(std::vector<std::string> v)
{
    if (v[ohType] == "ohtest") {
        isOhTest = true;
    }
    if (v[typeName] == "coldStart") {
        time = SmartPerf::ControlCallCmd::ColdStart(v);
    } else if (v[typeName] == "hotStart") {
        time = SmartPerf::ControlCallCmd::HotStart(v);
    } else if (v[typeName] == "responseTime") {
        time = SmartPerf::ControlCallCmd::ResponseTime();
    } else if (v[typeName] == "completeTime") {
        time = SmartPerf::ControlCallCmd::CompleteTime();
    } else if (v[typeName] == "startResponse") {
        time = SmartPerf::ControlCallCmd::StartResponse(v);
    } else if (v[typeName] == "coldStartHM") {
        time = SmartPerf::ControlCallCmd::ColdStartHM(v);
    } else if (v[typeName] == "pagefps") {
        result = SmartPerf::ControlCallCmd::PageFps();
    } else if (v[typeName] == "startFrame") {
        result = SmartPerf::ControlCallCmd::StartFrameFps(v);
    } else if (v[typeName] == "fpsohtest") {
        std::string ohTestFps = CMD_COMMAND_MAP.at(CmdCommand::OHTESTFPS);
        SPUtils::LoadCmd(ohTestFps, result);
    } else if (v[typeName] == "frameLoss") {
        result = SmartPerf::ControlCallCmd::GetFrame();
    } else if (v[typeName] == "appStartTime") {
        result = ControlCallCmd::GetAppStartTime();
    } else if (v[typeName] == "slideList") {
        result = ControlCallCmd::SlideList();
    } else if (v[typeName] == "timeDelay") {
        result = ControlCallCmd::TimeDelay();
    }
    if (time == noNameType) {
        std::cout << "Startup error, unknown application or application not responding" << std::endl;
    } else {
        if (time != 0) {
            stream << time;
            result = "time:" + stream.str() + "ms";
        }
        std::cout << result << std::endl;
    }
    return result;
}
std::string ControlCallCmd::TimeDelay()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::ParseRadar radar;
    OHOS::SmartPerf::StallingRateTrace srt;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + std::string("sp_trace_") + "delay" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "delay" + ".ftrace";
    std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("delay", traceName); });
    std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysIdAndKill(); });
    std::promise<std::string> promResponse;
    std::promise<std::string> promComplete;
    std::promise<std::string> promRadarFrame;
    std::promise<std::string> promResponseMoved = std::move(promResponse);
    std::promise<std::string> promCompleteMoved = std::move(promComplete);
    std::promise<std::string> promRadarFrameMoved = std::move(promRadarFrame);
    std::future<std::string> futureResponse = promResponseMoved.get_future();
    std::thread([promiseResponse = std::move(promResponseMoved)]() mutable {
        promiseResponse.set_value(SPUtils::GetRadarResponse());
    }).detach();
    std::future<std::string> futureComplete = promCompleteMoved.get_future();
    std::thread([promiseComplete = std::move(promCompleteMoved)]() mutable {
        promiseComplete.set_value(SPUtils::GetRadarComplete());
    }).detach();
    std::future<std::string> futureRadarFrame = promRadarFrameMoved.get_future();
    std::thread([promiseRadarFrame = std::move(promRadarFrameMoved)]() mutable {
        promiseRadarFrame.set_value(SPUtils::GetRadarFrame());
    }).detach();
    std::string responseStr = futureResponse.get();
    std::string completeStr = futureComplete.get();
    std::string radarFrameStr = futureRadarFrame.get();
    thGetTrace.join();
    thGetHisysId.join();
    double strResponseTime = radar.ParseRadarResponse(responseStr);
    stream << strResponseTime;
    double strCompleteTime = radar.ParseRadarComplete(completeStr);
    std::ostringstream streamComplete;
    streamComplete << strCompleteTime;
    std::string maxFrame = radar.ParseRadarMaxFrame(radarFrameStr);
    std::string resultTime = "ResponseTime:" + stream.str() + "ms\n" + "CompleteTime:" + streamComplete.str() + "ms\n";
    double rateResult = srt.StallingRateResult(traceName);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(two) << rateResult;
    std::string ssResult = ss.str();
    std::string hitchTimeRate = "HitchTimeRate:" + ssResult + "ms/s \n";
    resultTime = resultTime + hitchTimeRate + maxFrame;
    return resultTime;
}
std::string ControlCallCmd::SlideList()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseSlideFpsTrace slideFpsTrace;
    std::string cmdResult;
    OHOS::SmartPerf::ParseRadar radar;
    OHOS::SmartPerf::StallingRateTrace srt;
    std::string resultStream = "";
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + std::string("sp_trace_") + "fps" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    if (isOhTest) {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("fps", traceName); });
        thGetTrace.join();
        time = pcrt.ParseResponseTrace(traceName);
    } else {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("fps", traceName); });
        std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysIdAndKill(); });
        std::promise<std::string> promResponse;
        std::promise<std::string> promRadarFrame;
        std::promise<std::string> promResponseMoved = std::move(promResponse);
        std::promise<std::string> promRadarFrameMoved = std::move(promRadarFrame);
        std::future<std::string> futureResponse = promResponseMoved.get_future();
        std::thread([promiseResponse = std::move(promResponseMoved)]() mutable {
            promiseResponse.set_value(SPUtils::GetRadarResponse());
        }).detach();
        std::future<std::string> futureRadarFrame = promRadarFrameMoved.get_future();
        std::thread([promiseRadarFrame = std::move(promRadarFrameMoved)]() mutable {
            promiseRadarFrame.set_value(SPUtils::GetRadarFrame());
        }).detach();
        std::string responseStr = futureResponse.get();
        std::string radarFrameStr = futureRadarFrame.get();
        thGetTrace.join();
        thGetHisysId.join();
        double responseTime = radar.ParseRadarResponse(responseStr);
        stream << responseTime;
        std::string maxFrame = radar.ParseRadarMaxFrame(radarFrameStr);
        std::string responseSlide = "ResponseTime:" + stream.str() + "ms\n";
        double sFps = slideFpsTrace.ParseSlideFpsTraceNoh(traceName);
        std::ostringstream streamFps;
        streamFps << sFps;
        double stallingRateResult = srt.StallingRateResult(traceName);
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(two) << stallingRateResult;
        std::string ssResult = ss.str();
        std::string hitchTimeRate = "HitchTimeRate:" + ssResult + "ms/s \n";
        resultStream = "FPS:" + streamFps.str() + "fps\n" + responseSlide + hitchTimeRate + maxFrame;
    }
    return resultStream;
}
std::string ControlCallCmd::GetFrame()
{
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::ParseRadar radar;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + std::string("sp_trace_") + "frame" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "frame" + ".ftrace";
    std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("frame", traceName); });
    std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysIdAndKill(); });
    std::string str = SPUtils::GetRadarFrame();
    thGetTrace.join();
    thGetHisysId.join();
    std::string reslut = radar.ParseRadarFrame(str);
    return result;
}
std::string ControlCallCmd::PageFps()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParsePageFpsTrace pageFpsTrace;
    std::string cmdResult;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + "*" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("fps", traceName); });
    thGetTrace.join();
    double fps = pageFpsTrace.PageFpsTrace(traceName);
    stream << fps;
    result = "FPS:" + stream.str() + "fps";
    return result;
}
double ControlCallCmd::ResponseTime()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::ParseRadar radar;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + "*" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "response" + ".ftrace";
    if (isOhTest) {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("response", traceName); });
        thGetTrace.join();
        time = pcrt.ParseResponseTrace(traceName);
    } else {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("response", traceName); });
        std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysId(); });
        std::string str = SPUtils::GetRadarResponse();
        thGetTrace.join();
        thGetHisysId.join();
        time = radar.ParseRadarResponse(str);
    }
    return time;
}
double ControlCallCmd::ColdStartHM(std::vector<std::string> v) const
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    std::string cmdResult;
    int typePKG = 3;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + "*" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("coldStart", traceName); });
    thGetTrace.join();
    std::string pid = sd.GetPidByPkg(v[typePKG]);
    return parseTrace.ParseTraceCold(traceName, pid);
}
double ControlCallCmd::CompleteTime()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseClickCompleteTrace pcct;
    std::string cmdResult;
    OHOS::SmartPerf::ParseRadar radar;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + "*" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "complete" + ".ftrace";
    if (isOhTest) {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("complete", traceName); });
        thGetTrace.join();
        time = pcct.ParseCompleteTrace(traceName);
    } else {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("complete", traceName); });
        std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysId(); });
        std::string str = SPUtils::GetRadarComplete();
        thGetTrace.join();
        thGetHisysId.join();
        time = radar.ParseRadarComplete(str);
    }
    return time;
}
std::string ControlCallCmd::StartFrameFps(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    OHOS::SmartPerf::StartFrameTraceNoh startFrameTraceNoh;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE);
    std::string uitest = CMD_COMMAND_MAP.at(CmdCommand::UITEST_DUMPLAYOUT);
    SPUtils::LoadCmd(rmTrace + "*" + ".json", cmdResult);
    SPUtils::LoadCmd(rmTrace + "*" + ".ftrace", cmdResult);
    SPUtils::LoadCmd(uitest, cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    size_t position2 = cmdResult.find("json");
    std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("coldStart", traceName); });
    std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_POINT) + sd.pointXY + " -u " + sd.pointXY;
    sleep(1);
    SPUtils::LoadCmd(cmd, cmdResult);
    sleep(1);
    thGetTrace.join();
    double fps = startFrameTraceNoh.ParseStartFrameTraceNoh(traceName);
    stream << fps;
    result = "FPS:" + stream.str() + "fps";
    return result;
}
double ControlCallCmd::StartResponse(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    OHOS::SmartPerf::ParseRadar radar;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE);
    std::string uitest = CMD_COMMAND_MAP.at(CmdCommand::UITEST_DUMPLAYOUT);
    SPUtils::LoadCmd(rmTrace + "*" + ".json", cmdResult);
    SPUtils::LoadCmd(rmTrace + "*" + ".ftrace", cmdResult);
    SPUtils::LoadCmd(uitest, cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    size_t position2 = cmdResult.find("json");
    std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "startResponse" + ".ftrace";
    if (sd.pointXY == "0 0") {
        return noNameType;
    } else {
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("startResponse", traceName); });
        std::thread thInputEvent = std::thread([&sd]() { sd.InputEvent(sd.pointXY); });
        std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysId(); });
        std::string str = SPUtils::GetRadar();
        thGetTrace.join();
        thInputEvent.join();
        thGetHisysId.join();
        time = radar.ParseRadarStartResponse(str);
        return time;
    }
}
std::string ControlCallCmd::GetAppStartTime() const
{
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::ParseRadar radar;
    OHOS::SmartPerf::StallingRateTrace srt;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + std::string("sp_trace_") + "start" + ".ftrace";
    SPUtils::LoadCmd(rmTrace, cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "start" + ".ftrace";
    std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("start", traceName); });
    std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysIdAndKill(); });

    std::promise<std::string> promRadar;
    std::promise<std::string> promRadarFrame;
    std::promise<std::string> promRadarMoved = std::move(promRadar);
    std::promise<std::string> promRadarFrameMoved = std::move(promRadarFrame);
    std::future<std::string> futureRadar = promRadarMoved.get_future();
    std::thread([promiseRadar = std::move(promRadarMoved)]() mutable {
        promiseRadar.set_value(SPUtils::GetRadar());
    }).detach();
    std::future<std::string> futureRadarFrame = promRadarFrameMoved.get_future();
    std::thread([promiseRadarFrame = std::move(promRadarFrameMoved)]() mutable {
        promiseRadarFrame.set_value(SPUtils::GetRadarFrame());
    }).detach();
    std::string radarStr = futureRadar.get();
    std::string radarFrameStr = futureRadarFrame.get();
    thGetTrace.join();
    thGetHisysId.join();
    std::string resultStream = radar.ParseRadarAppStrart(radarStr);
    std::string resultStream2 = radar.ParseRadarMaxFrame(radarFrameStr);
    double stallingRateResult2 = srt.StallingRateResult(traceName);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(two) << stallingRateResult2;
    std::string ssResult = ss.str();
    std::string hitchTimeRate = "HitchTimeRate:" + ssResult + "ms/s \n";
    resultStream = resultStream + hitchTimeRate + resultStream2;
    return resultStream;
}
double ControlCallCmd::ColdStart(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    OHOS::SmartPerf::ParseRadar radar;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE);
    std::string uitest = CMD_COMMAND_MAP.at(CmdCommand::UITEST_DUMPLAYOUT);
    SPUtils::LoadCmd(rmTrace + "*" + ".json", cmdResult);
    SPUtils::LoadCmd(rmTrace + "*" + ".ftrace", cmdResult);
    SPUtils::LoadCmd(uitest, cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    size_t position2 = cmdResult.find("json");
    std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    if (sd.pointXY == "0 0") {
        return noNameType;
    } else {
        if (isOhTest) {
            std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("coldStart", traceName); });
            std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_POINT) + sd.pointXY + " -u " + sd.pointXY;
            sleep(1);
            SPUtils::LoadCmd(cmd, cmdResult);
            sleep(1);
            std::string pid = sd.GetPidByPkg(v[typePKG]);
            thGetTrace.join();
            time = parseTrace.ParseTraceCold(traceName, pid);
        } else {
            std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("coldStart", traceName); });
            std::thread thInputEvent = std::thread([&sd]() { sd.InputEvent(sd.pointXY); });
            std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysId(); });
            sleep(1);
            std::string str = SPUtils::GetRadar();
            thGetTrace.join();
            thInputEvent.join();
            thGetHisysId.join();
            time = radar.ParseRadarStart(str);
        }
        return time;
    }
}
double ControlCallCmd::HotStart(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    OHOS::SmartPerf::ParseRadar radar;
    std::string cmdResult;
    if (isOhTest) {
        std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE) + "*" + ".ftrace";
        SPUtils::LoadCmd(rmTrace, cmdResult);
        std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "hotStart" + ".ftrace";
        std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("hotStart", traceName); });
        thGetTrace.join();
        return parseTrace.ParseTraceHot(traceName);
    } else {
        int type = 4;
        int typePKG = 3;
        std::string rmTrace = CMD_COMMAND_MAP.at(CmdCommand::RM_FILE);
        std::string uitest = CMD_COMMAND_MAP.at(CmdCommand::UITEST_DUMPLAYOUT);
        SPUtils::LoadCmd(rmTrace + "*" + ".json", cmdResult);
        SPUtils::LoadCmd(rmTrace + "*" + ".ftrace", cmdResult);
        SPUtils::LoadCmd(uitest, cmdResult);
        sleep(1);
        size_t position = cmdResult.find(":");
        size_t position2 = cmdResult.find("json");
        std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
        sd.InitXY2(v[type], pathJson, v[typePKG]);
        if (sd.pointXY == "0 0") {
            return noNameType;
        } else {
            std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::UINPUT_POINT) + sd.pointXY + " -u " + sd.pointXY;
            SPUtils::LoadCmd(cmd, cmdResult);
            sleep(1);
            sd.ChangeToBackground();
            sleep(1);
            std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "hotStart" + ".ftrace";
            std::thread thGetTrace = std::thread([&sd, traceName]() { sd.GetTrace("hotStart", traceName); });
            std::thread thInputEvent = std::thread([&sd]() { sd.InputEvent(sd.pointXY); });
            std::thread thGetHisysId = std::thread([&sd]() { sd.GetHisysId(); });
            sleep(1);
            std::string str = SPUtils::GetRadar();
            thGetTrace.join();
            thInputEvent.join();
            thGetHisysId.join();
            time = radar.ParseRadarStart(str);
            return time;
        }
    }
}
}
}
