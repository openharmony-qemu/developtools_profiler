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
#include "include/editor_command.h"
#include "include/startup_delay.h"
#include "include/parse_trace.h"
#include "include/sp_utils.h"
#include "include/parse_click_complete_trace.h"
#include "include/parse_click_response_trace.h"

namespace OHOS {
namespace SmartPerf {
EditorCommand::EditorCommand(int argc, std::vector<std::string> v)
{
    if (argc >= threeParamMore) {
        int type = 2;
        float time = 0.0;
        int typeName = 4;
        float noNameType = 5.0;
        if (v[type] == "coldStart") {
            time = SmartPerf::EditorCommand::ColdStart(v);
        } else if (v[type] == "hotStart") {
            time = SmartPerf::EditorCommand::HotStart(v);
        } else if (v[type] == "responseTime") {
            time = SmartPerf::EditorCommand::ResponseTime();
        } else if (v[type] == "completeTime") {
            time = SmartPerf::EditorCommand::CompleteTime();
        }
        if (time == noNameType) {
            std::cout << v[typeName] << " Duplicate Application Name" << std::endl;
        } else {
            std::cout << "time:" << time << std::endl;
        }
    }
}
float EditorCommand::ResponseTime()
{
    OHOS::SmartPerf::ParseClickResponseTrace pcrt;
    OHOS::SmartPerf::StartUpDelay sd;
    std::string cmdResult;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "response" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("response", traceName);
    thGetTrace.join();
    float time = pcrt.ParseResponseTrace(traceName);
    return time;
}
float EditorCommand::CompleteTime()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseClickCompleteTrace pcct;
    std::string cmdResult;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "complete" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("complete", traceName);
    thGetTrace.join();
    float time = pcct.ParseCompleteTrace(traceName);
    return time;
}
float EditorCommand::ColdStart(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    float noNameType = 5.0;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    std::string pathJson = cmdResult.substr(position + 1);
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    if (sd.pointXY == "0 0") {
        return noNameType;
    } else {
        std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
        std::thread thGetTrace = sd.ThreadGetTrace("coldStart", traceName);
        std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
        sleep(1);
        SPUtils::LoadCmd(cmd, cmdResult);
        std::string pid = sd.GetPidByPkg(v[typePKG]);
        thGetTrace.join();
        std::string deviceType = sd.GetDeviceType();
        float time = 0.0;
        if (deviceType == " rk3568") {
            time = parseTrace.ParseTraceCold(traceName, pid);
        } else {
            time = parseTrace.ParseTraceNoah(traceName, pid);
        }
        return time;
    }
}
float EditorCommand::HotStart(std::vector<std::string> v)
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    std::string cmdResult;
    int type = 4;
    int typePKG = 3;
    float noNameType = 5.0;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
    sleep(1);
    size_t position = cmdResult.find(":");
    std::string pathJson = cmdResult.substr(position + 1);
    sd.InitXY2(v[type], pathJson, v[typePKG]);
    if (sd.pointXY == "0 0") {
        return noNameType;
    } else {
        std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
        sleep(1);
        SPUtils::LoadCmd(cmd, cmdResult);
        sleep(1);
        SPUtils::LoadCmd("uinput -T -m 600 2760 600 1300 200", cmdResult);
        sleep(1);
        std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "hotStart" + ".ftrace";
        std::thread thGetTrace = sd.ThreadGetTrace("hotStart", traceName);
        SPUtils::LoadCmd(cmd, cmdResult);
        std::string pid = sd.GetPidByPkg(v[typePKG]);
        thGetTrace.join();
        std::string deviceType = sd.GetDeviceType();
        float time = 0.0;
        if (deviceType == " rk3568") {
            time = parseTrace.ParseTraceCold(traceName, pid);
        } else {
            time = parseTrace.ParseTraceNoah(traceName, pid);
        }
        return time;
    }
}
}
}
