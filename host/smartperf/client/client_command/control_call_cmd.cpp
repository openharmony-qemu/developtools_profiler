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

namespace OHOS {
namespace SmartPerf {
std::string ControlCallCmd::GetResult(int argc, std::vector<std::string> v)
{
    std::ostringstream stream;
    if (v[ohType] == "ohtest") {
        isOhTest = true;
    }
    if (v[typeName] == "coldStart") {
        time = SmartPerf::ControlCallCmd::ColdStart(v);
        stream << time;
        result = "time:" + stream.str();
    } else if (v[typeName] == "hotStart") {
        time = SmartPerf::ControlCallCmd::HotStart(v);
        stream << time;
        result = "time:" + stream.str();
    } else if (v[typeName] == "responseTime") {
        time = SmartPerf::ControlCallCmd::ResponseTime();
        stream << time;
        result = "time:" + stream.str();
    } else if (v[typeName] == "completeTime") {
        time = SmartPerf::ControlCallCmd::CompleteTime();
        stream << time;
        result = "time:" + stream.str();
    } else if (v[typeName] == "coldStartHM") {
        time = SmartPerf::ControlCallCmd::ColdStartHM(v);
        stream << time;
        result = "time:" + stream.str();
    } else if (v[typeName] == "fps") {
        result = SmartPerf::ControlCallCmd::SlideFps(v);
    }  else if (v[typeName] == "pagefps") {
        double fps = SmartPerf::ControlCallCmd::PageFps();
        stream << fps;
        result = "FPS:" + stream.str() + "fps";
    } else if (v[typeName] == "FPS") {
        result = SmartPerf::ControlCallCmd::SlideFPS(v);
    } else if (v[typeName] == "startFrame") {
        double fps = SmartPerf::ControlCallCmd::StartFrameFps(v);
        stream << fps;
        result = "FPS:" + stream.str() + "fps";
    } else if (v[typeName] == "fpsohtest") {
        SPUtils::LoadCmd("GP_daemon_fps 10", result);
    }
    if (time == noNameType) {
        std::cout << "Startup error, unknown application or application not responding"<< std::endl;
    } else {
        std::cout << result << std::endl;
    }
    return result;
}
double ControlCallCmd::PageFps()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::PageFpsTrace pageFpsTrace;
    std::string cmdResult;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("fps", traceName);
    thGetTrace.join();
    double fps = pageFpsTrace.ParsePageFpsTrace(traceName);
    return fps;
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
std::string ControlCallCmd::SlideFPS(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    ParseFPS parseFPS;
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
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "fps" + ".ftrace";
    std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
    sleep(1);
    SPUtils::LoadCmd(cmd, cmdResult);
    sleep(1);
    std::thread thGetTrace = sd.ThreadGetTrace("fps", traceName);
    cmd = "uinput -T -m 650 1500 650 500 30";
    SPUtils::LoadCmd(cmd, cmdResult);
    thGetTrace.join();
    std::string fps = parseFPS.ParseTraceFile(traceName, v[typePKG]);
    return fps;
}
double ControlCallCmd::ResponseTime()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "response" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("response", traceName);
    thGetTrace.join();
    time = pcrt.ParseResponseTrace(traceName);
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
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "complete" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("complete", traceName);
    thGetTrace.join();
    time = pcct.ParseCompleteTrace(traceName);
    return time;
}
double ControlCallCmd::StartFrameFps(std::vector<std::string> v)
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
    return fps;
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
