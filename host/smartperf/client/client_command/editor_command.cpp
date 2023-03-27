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
        if (v[type] == "coldStart") {
            time = SmartPerf::EditorCommand::ColdStart(v);
        } else if (v[type] == "hotStart") {
            time = SmartPerf::EditorCommand::HotStart();
        } else if (v[type] == "responseTime") {
            time = SmartPerf::EditorCommand::ResponseTime();
        } else if (v[type] == "completeTime") {
            time = SmartPerf::EditorCommand::CompleteTime();
        }
        std::cout << "time:" << time << std::endl;
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
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.json", cmdResult);
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    SPUtils::LoadCmd("uitest dumpLayout", cmdResult);
    sleep(1);
    int position = cmdResult.find(":");
    std::string pathJson = cmdResult.substr(position + 1);
    sd.InitXY2(v[type], pathJson);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "coldStart" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("coldStart", traceName);
    std::string cmd = "uinput -T -d " + sd.pointXY + " -u " + sd.pointXY;
    std::cout << "cmd:" << cmd << std::endl;
    sleep(1);
    SPUtils::LoadCmd(cmd, cmdResult);
    thGetTrace.join();
    float time = parseTrace.ParseTraceCold(traceName);
    return time;
}
float EditorCommand::HotStart()
{
    OHOS::SmartPerf::StartUpDelay sd;
    OHOS::SmartPerf::ParseTrace parseTrace;
    std::string cmdResult;
    SPUtils::LoadCmd("rm -rfv /data/local/tmp/*.ftrace", cmdResult);
    std::string traceName = std::string("/data/local/tmp/") + std::string("sp_trace_") + "hotStart" + ".ftrace";
    std::thread thGetTrace = sd.ThreadGetTrace("hotStart", traceName);
    thGetTrace.join();
    float time = parseTrace.ParseTraceHot(traceName);
    return time;
}
}
}
