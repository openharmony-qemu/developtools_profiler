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
#include <future>
#include "include/control_call_cmd.h"
#include "include/startup_delay.h"
#include "include/parse_trace.h"
#include "include/sp_utils.h"
#include "include/parse_click_complete_trace.h"
#include "include/parse_click_response_trace.h"
#include "include/sp_parse_fps.h"
#include "include/parse_page_fps_trace.h"
#include "include/parse_start_frame_trace.h"
#include "include/parse_start_trace_noh.h"
#include "include/parse_radar.h"
#include "include/parse_slide_fps_trace.h"

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
    } else if (v[typeName] == "fps") {
        result = SmartPerf::ControlCallCmd::SlideFps(v);
    } else if (v[typeName] == "pagefps") {
        result = SmartPerf::ControlCallCmd::PageFps();
    } else if (v[typeName] == "startFrame") {
        result = SmartPerf::ControlCallCmd::StartFrameFps(v);
    } else if (v[typeName] == "fpsohtest") {
        SPUtils::LoadCmd("GP_daemon_fps 10", result);
    } else if (v[typeName] == "frameLoss") {
        SmartPerf::ControlCallCmd::GetFrame();
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
    OHOS::SmartPerf::Radar radar;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/sp_trace_delay.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "delay" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("delay", traceName);
    std::thread thGetHisysId = sd.ThreadGetHisysIdResponse();
    std::thread thGetHisysId2 = sd.ThreadGetHisysIdComplete();
    std::future<std::string> futureResult = std::async(std::launch::async, SPUtils::GetRadarResponse);
    std::future<std::string> futureResult2 = std::async(std::launch::async, SPUtils::GetRadarComplete);
    std::string str = futureResult.get();
    std::string str2 = futureResult2.get();
    thGetTrace.join();
    thGetHisysId.join();
    thGetHisysId2.join();
    double strResponseTime = radar.ParseRadarResponse(str);
    stream << strResponseTime;
    double strCompleteTime = radar.ParseRadarComplete(str2);
    std::ostringstream streamComplete;
    streamComplete << strCompleteTime;
    std::string resultTime = "ResponseTime:" + stream.str() + "ms\n" +
     "CompleteTime:" + streamComplete.str() + "ms";
    return resultTime;
}
std::string ControlCallCmd::SlideList()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseSlideFpsTrace slideFpsTrace;
    std::string cmdResult;
    OHOS::SmartPerf::Radar radar;
    std::string resultStream = "";
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/sp_trace_fps.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    if (isOhTest) {
        std::thread thGetTrace = sd.ThreadGetTrace("fps", traceName);
        thGetTrace.join();
        time = pcrt.ParseResponseTrace(traceName);
    } else {
        std::thread thGetTrace = sd.ThreadGetTrace("fps", traceName);
        std::thread thGetHisysId = sd.ThreadGetHisysId();
        std::string str = SPUtils::GetRadarResponse();
        thGetTrace.join();
        thGetHisysId.join();
        double responseTime = radar.ParseRadarResponse(str);
        stream << responseTime;
        std::string responseSlide = "ResponseTime:" + stream.str() + "ms";
        double sFps = slideFpsTrace.ParseSlideFpsTraceNoh(traceName);
        std::ostringstream streamFps;
        streamFps << sFps;
        resultStream = "FPS:" + streamFps.str() + "fps\n" + responseSlide;
    }
    return resultStream;
}
std::string ControlCallCmd::GetFrame()
{
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::Radar radar;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/sp_trace_frame.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "frame" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("frame", traceName);
    std::thread thGetHisysId = sd.ThreadGetHisysId();
    std::string str = SPUtils::GetRadarFrame();
    thGetTrace.join();
    thGetHisysId.join();
    std::string reslut = radar.ParseRadarFrame(str);
    return result;
}
std::string ControlCallCmd::PageFps()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::PageFpsTrace pageFpsTrace;
    std::string cmdResult;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("fps", traceName);
    thGetTrace.join();
    double fps = pageFpsTrace.ParsePageFpsTrace(traceName);
    stream << fps;
    result = "FPS:" + stream.str() + "fps";
    return result;
}
std::string ControlCallCmd::SlideFps(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    ParseFPS parseFPS;
    std::string cmdResult;
    int typePKG = 3;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("fps", traceName);
    thGetTrace.join();
    std::string fps = parseFPS.ParseTraceFile(traceName, v[typePKG]);
    return fps;
}
double ControlCallCmd::ResponseTime()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::Radar radar;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "response" + ".ftrace";
    if (isOhTest) {
        std::thread thGetTrace = sd.ThreadGetTrace("response", traceName);
        thGetTrace.join();
        time = pcrt.ParseResponseTrace(traceName);
    } else {
        std::thread thGetTrace = sd.ThreadGetTrace("response", traceName);
        std::thread thGetHisysId = sd.ThreadGetHisysId();
        std::string str = SPUtils::GetRadarResponse();
        thGetTrace.join();
        thGetHisysId.join();
        time = radar.ParseRadarResponse(str);
    }
    return time;
}
double ControlCallCmd::ColdStartHM(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    std::string cmdResult;
    int typePKG = 3;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("coldStart", traceName);
    thGetTrace.join();
    std::string pid = sd.GetPidByPkg(v[typePKG]);
    return parseTrace.ParseTraceCold(traceName, pid);
}
double ControlCallCmd::CompleteTime()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseClickCompleteTrace pcct;
    std::string cmdResult;
    OHOS::SmartPerf::Radar radar;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "complete" + ".ftrace";
    if (isOhTest) {
        std::thread thGetTrace = sd.ThreadGetTrace("complete", traceName);
        thGetTrace.join();
        time = pcct.ParseCompleteTrace(traceName);
    } else {
        std::thread thGetTrace = sd.ThreadGetTrace("complete", traceName);
        std::thread thGetHisysId = sd.ThreadGetHisysId();
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
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    size_t position2 = cmdResult.find("json");
    std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("coldStart", traceName);
    std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
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
    OHOS::SmartPerf::Radar radar;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    size_t position2 = cmdResult.find("json");
    std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
    std::string deviceType = sd.GetDeviceType();
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "startResponse" + ".ftrace";
    if (sd.pointXY == "0 0") {
        return noNameType;
    } else {
        std::thread thGetTrace = sd.ThreadGetTrace("startResponse", traceName);
        std::thread thInputEvent = sd.ThreadInputEvent(sd.pointXY);
        std::thread thGetHisysId = sd.ThreadGetHisysId();
        std::string str = SPUtils::GetRadar();
        thGetTrace.join();
        thInputEvent.join();
        thGetHisysId.join();
        time = radar.ParseRadarStartResponse(str);
        return time;
    }
}
std::string ControlCallCmd::GetAppStartTime()
{
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    OHOS::SmartPerf::Radar radar;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/sp_trace_start.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "start" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("start", traceName);
    std::thread thGetHisysId = sd.ThreadGetHisysId();
    std::string str = SPUtils::GetRadar();
    thGetTrace.join();
    thGetHisysId.join();
    std::string resultStream = radar.ParseRadarAppStrart(str);
    return resultStream;
}
double ControlCallCmd::ColdStart(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    OHOS::SmartPerf::Radar radar;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    size_t position2 = cmdResult.find("json");
    std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
    std::string deviceType = sd.GetDeviceType();
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    if (sd.pointXY == "0 0") {
        return noNameType;
    } else {
        if (isOhTest) {
            std::thread thGetTrace = sd.ThreadGetTrace("coldStart", traceName);
            std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
            sleep(1);
            SPUtils::LoadCmd(cmd, cmdResult);
            sleep(1);
            std::string pid = sd.GetPidByPkg(v[typePKG]);
            thGetTrace.join();
            time = parseTrace.ParseTraceCold(traceName, pid);
        } else {
            std::thread thGetTrace = sd.ThreadGetTrace("coldStart", traceName);
            std::thread thInputEvent = sd.ThreadInputEvent(sd.pointXY);
            std::thread thGetHisysId = sd.ThreadGetHisysId();
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
    OHOS::SmartPerf::Radar radar;
    std::string cmdResult;
    std::string deviceType = sd.GetDeviceType();
    if (isOhTest) {
        SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
        std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "hotStart" + ".ftrace";
        std::thread thGetTrace = sd.ThreadGetTrace("hotStart", traceName);
        thGetTrace.join();
        return parseTrace.ParseTraceHot(traceName);
    } else {
        int type = 4;
        int typePKG = 3;
        SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
        SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
        SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
        sleep(1);
        size_t position = cmdResult.find(":");
        size_t position2 = cmdResult.find("json");
        std::string pathJson = cmdResult.substr(position + 1, position2 - position + typePKG);
        sd.InitXY2(v[type], pathJson, v[typePKG]);
        if (sd.pointXY == "0 0") {
            return noNameType;
        } else {
            std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
            SPUtils::LoadCmd(cmd, cmdResult);
            sleep(1);
            sd.ChangeToBackground();
            sleep(1);
            std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "hotStart" + ".ftrace";
            std::thread thGetTrace = sd.ThreadGetTrace("hotStart", traceName);
            std::thread thInputEvent = sd.ThreadInputEvent(sd.pointXY);
            std::thread thGetHisysId = sd.ThreadGetHisysId();
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
