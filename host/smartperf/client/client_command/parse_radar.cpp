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
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <regex>
#include "include/parse_radar.h"
#include "include/sp_log.h"
namespace OHOS {
namespace SmartPerf {
double ParseRadar::ParseRadarStart(std::string string)
{
    double time = -1;
    std::string target = "\"E2E_LATENCY\":";
    time = std::stod(ExtractString(string, target));
    return time;
}
double ParseRadar::ParseRadarStartResponse(std::string string)
{
    double time = -1;
    std::string target = "\"RESPONSE_LATENCY\":";
    time = std::stod(ExtractString(string, target));
    return time;
}
std::string ParseRadar::ParseRadarAppStrart(std::string string)
{
    std::string animationCompleteTime = ExtractString(string, "\"ANIMATION_LATENCY\":");
    std::string completeTime = ExtractString(string, "\"E2E_LATENCY\":");
    std::string responseTime = ExtractString(string, "\"RESPONSE_LATENCY\":");
    std::string firstFrameDrawnTime = ExtractString(string, "\"FIRST_FRAEM_DRAWN_LATENCY\":");
    std::string result = "ResponseTime:" + responseTime +
        "ms\n"
        "FirstFrameDrawnTime:" +
        firstFrameDrawnTime +
        "ms\n"
        "AnimationCompleteTime:" +
        animationCompleteTime +
        "ms\n"
        "CompleteTime:" +
        completeTime + "ms\n";
    LOGI("Radar.result====: %s", result.c_str());
    return result;
}
double ParseRadar::ParseRadarResponse(std::string string)
{
    double time = -1;
    std::string target = "\"RESPONSE_LATENCY\":";
    time = std::stod(ExtractString(string, target));
    return time;
}
double ParseRadar::ParseRadarComplete(std::string string)
{
    double time = -1;
    std::string target = "\"E2E_LATENCY\":";
    time = std::stod(ExtractString(string, target));
    return time;
}
std::string ParseRadar::ParseRadarMaxFrame(std::string string)
{
    std::string maxRenderSeqMissedFrames = ExtractString(string, "\"MAX_RENDER_SEQ_MISSED_FRAMES\":");
    std::string result = "MAX_RENDER_SEQ_MISSED_FRAMES:" + maxRenderSeqMissedFrames;
    return result;
}
std::string ParseRadar::ParseRadarFrame(std::string string)
{
    std::string budleName = ExtractString(string, "\"BUNDLE_NAME_EX\":");
    std::cout << "BUNDLE_NAME:" << budleName << std::endl;
    std::string sceneId = ExtractString(string, "\"SCENE_ID\":");
    std::cout << "SCENE_ID:" << sceneId << std::endl;
    std::string totalAppFrames = ExtractString(string, "\"TOTAL_APP_FRAMES\":");
    std::cout << "TOTAL_APP_FRAMES:" << totalAppFrames << std::endl;
    std::string totalAppMissedFrames = ExtractString(string, "\"TOTAL_APP_MISSED_FRAMES\":");
    std::cout << "TOTAL_APP_MISSED_FRAMES:" << totalAppMissedFrames << std::endl;
    std::string maxAppFramsestime = ExtractString(string, "\"MAX_APP_FRAMETIME\":");
    std::cout << "MAX_APP_FRAMETIME:" << maxAppFramsestime << "ms" << std::endl;
    std::string maxAppSeqMissedFrames = ExtractString(string, "\"MAX_APP_SEQ_MISSED_FRAMES\":");
    std::cout << "MAX_APP_SEQ_MISSED_FRAMES:" << maxAppSeqMissedFrames << std::endl;
    std::string totalRenderFrames = ExtractString(string, "\"TOTAL_RENDER_FRAMES\":");
    std::cout << "TOTAL_RENDER_FRAMES:" << totalRenderFrames << std::endl;
    std::string totalRenderMissedFrames = ExtractString(string, "\"TOTAL_RENDER_MISSED_FRAMES\":");
    std::cout << "TOTAL_RENDER_MISSED_FRAMES:" << totalRenderMissedFrames << std::endl;
    std::string maxRenderFrametime = ExtractString(string, "\"MAX_RENDER_FRAMETIME\":");
    std::cout << "MAX_RENDER_FRAMETIME:" << maxRenderFrametime << "ms" << std::endl;
    std::string averageRenderFrametime = ExtractString(string, "\"AVERAGE_RENDER_FRAMETIME\":");
    std::cout << "AVERAGE_RENDER_FRAMETIME:" << averageRenderFrametime << "ms" << std::endl;
    std::string maxRenderSeqMissedFrames = ExtractString(string, "\"MAX_RENDER_SEQ_MISSED_FRAMES\":");
    std::cout << "MAX_RENDER_SEQ_MISSED_FRAMES:" << maxRenderSeqMissedFrames << std::endl;
    std::string result = "";
    return result;
}
std::string ParseRadar::ExtractString(const std::string &str, const std::string &target) const
{
    LOGI("Radar.str====: %s", str.c_str());
    size_t pos = str.find(target);
    if (pos != std::string::npos) {
        pos += target.length();
        size_t commaPos = str.find(",", pos);
        if (commaPos != std::string::npos) {
            std::string result = str.substr(pos, commaPos - pos);
            return result;
        }
    }

    return "-1";
}
}
}