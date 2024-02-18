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
namespace OHOS {
    namespace SmartPerf {
        double Radar::ParseRadarStart(std::string str)
        {
            double time = -1;
            std::string target = "\"E2E_LATENCY\":";
            time = std::stod(extract_string(str, target));
            return time;
        }
        double Radar::ParseRadarStartResponse(std::string string)
        {
            double time = -1;
            std::string target = "\"RESPONSE_LATENCY\":";
            time = std::stod(extract_string(string, target));
            return time;
        }
        std::string Radar::ParseRadarAppStrart(std::string string)
        {
            std::string animationCompleteTime = extract_string(string, "\"ANIMATION_LATENCY\":");
            std::string completeTime = extract_string(string, "\"E2E_LATENCY\":");
            std::string responseTime = extract_string(string, "\"RESPONSE_LATENCY\":");
            std::string firstFrameDrawnTime = extract_string(string, "\"FIRST_FRAEM_DRAWN_LATENCY\":");
            std::string result = "ResponseTime:" + responseTime + "ms\n"
            "FirstFrameDrawnTime:" + firstFrameDrawnTime + "ms\n"
            "AnimationCompleteTime:" + animationCompleteTime + "ms\n"
            "CompleteTime:" + completeTime + "ms\n";
            return result;
        }
        double Radar::ParseRadarResponse(std::string string)
        {
            double time = -1;
            std::string target = "\"RESPONSE_LATENCY\":";
            time = std::stod(extract_string(string, target));
            return time;
        }
        double Radar::ParseRadarComplete(std::string string)
        {
            double time = -1;
            std::string target = "\"E2E_LATENCY\":";
            time = std::stod(extract_string(string, target));
            return time;
        }
        std::string Radar::ParseRadarFrame(std::string string)
        {
            std::string budleName = extract_string(string, "\"BUNDLE_NAME_EX\":");
            std::cout << "BUNDLE_NAME:" << budleName << std::endl;
            std::string sceneId = extract_string(string, "\"SCENE_ID\":");
            std::cout << "SCENE_ID:" << sceneId << std::endl;
            std::string totalAppFrames = extract_string(string, "\"TOTAL_APP_FRAMES\":");
            std::cout << "TOTAL_APP_FRAMES:" << totalAppFrames << std::endl;
            std::string totalAppMissedFrames = extract_string(string, "\"TOTAL_APP_MISSED_FRAMES\":");
            std::cout << "TOTAL_APP_MISSED_FRAMES:" << totalAppMissedFrames << std::endl;
            std::string maxAppFramsestime = extract_string(string, "\"MAX_APP_FRAMETIME\":");
            std::cout << "MAX_APP_FRAMETIME:" << maxAppFramsestime << "ms" << std::endl;
            std::string maxAppSeqMissedFrames = extract_string(string, "\"MAX_APP_SEQ_MISSED_FRAMES\":");
            std::cout << "MAX_APP_SEQ_MISSED_FRAMES:" << maxAppSeqMissedFrames << std::endl;
            std::string totalRenderFrames = extract_string(string, "\"TOTAL_RENDER_FRAMES\":");
            std::cout << "TOTAL_RENDER_FRAMES:" << totalRenderFrames << std::endl;
            std::string totalRenderMissedFrames = extract_string(string, "\"TOTAL_RENDER_MISSED_FRAMES\":");
            std::cout << "TOTAL_RENDER_MISSED_FRAMES:" << totalRenderMissedFrames << std::endl;
            std::string maxRenderFrametime = extract_string(string, "\"MAX_RENDER_FRAMETIME\":");
            std::cout << "MAX_RENDER_FRAMETIME:" << maxRenderFrametime << "ms" << std::endl;
            std::string averageRenderFrametime = extract_string(string, "\"AVERAGE_RENDER_FRAMETIME\":");
            std::cout << "AVERAGE_RENDER_FRAMETIME:" << averageRenderFrametime << "ms" << std::endl;
            std::string maxRenderSeqMissedFrames = extract_string(string, "\"MAX_RENDER_SEQ_MISSED_FRAMES\":");
            std::cout << "MAX_RENDER_SEQ_MISSED_FRAMES:" << maxRenderSeqMissedFrames << std::endl;
            std::string result = "";
            return result;
        }
        std::string Radar::extract_string(const std::string& str, const std::string& target)
        {
            size_t pos = str.find(target);
            if (pos != std::string::npos) {
                pos += target.length();
                size_t comma_pos = str.find(",", pos);
                if (comma_pos != std::string::npos) {
                    std::string result = str.substr(pos,comma_pos - pos);
                    return result;
                }
            }
            
            return "-1";
        }
    }
}