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
        void Radar::ParseRadarFrame(std::string string)
        {
            std::string budle_name = extract_string(string, "\"BUNDLE_NAME\":");
            std::cout << "BUNDLE_NAME:" << budle_name << std::endl;
            std::string scene_id = extract_string(string, "\"SCENE_ID\":");
            std::cout << "SCENE_ID:" << scene_id << std::endl;
            std::string total_app_frames = extract_string(string, "\"TOTAL_APP_FRAMES\":");
            std::cout << "TOTAL_APP_FRAMES:" << total_app_frames << std::endl;
            std::string total_app_missed_frames = extract_string(string, "\"TOTAL_APP_MISSED_FRAMES\":");
            std::cout << "TOTAL_APP_MISSED_FRAMES:" << total_app_missed_frames << std::endl;
            std::string max_app_framsestime = extract_string(string, "\"MAX_APP_FRAMETIME\":");
            std::cout << "MAX_APP_FRAMETIME:" << max_app_framsestime << "ms" << std::endl;
            std::string max_app_seq_missed_frames = extract_string(string, "\"MAX_APP_SEQ_MISSED_FRAMES\":");
            std::cout << "MAX_APP_SEQ_MISSED_FRAMES:" << max_app_seq_missed_frames << std::endl;
            std::string total_render_frames = extract_string(string, "\"TOTAL_RENDER_FRAMES\":");
            std::cout << "TOTAL_RENDER_FRAMES:" << total_render_frames << std::endl;
            std::string total_render_missed_frames = extract_string(string, "\"TOTAL_RENDER_MISSED_FRAMES\":");
            std::cout << "TOTAL_RENDER_MISSED_FRAMES:" << total_render_missed_frames << std::endl;
            std::string max_render_frametime = extract_string(string, "\"MAX_RENDER_FRAMETIME\":");
            std::cout << "MAX_RENDER_FRAMETIME:" << max_render_frametime << "ms" << std::endl;
            std::string average_render_frametime = extract_string(string, "\"AVERAGE_RENDER_FRAMETIME\":");
            std::cout << "AVERAGE_RENDER_FRAMETIME:" << average_render_frametime << "ms" << std::endl;
            std::string max_render_seq_missed_frames = extract_string(string, "\"MAX_RENDER_SEQ_MISSED_FRAMES\":");
            std::cout << "MAX_RENDER_SEQ_MISSED_FRAMES:" << max_render_seq_missed_frames << "ms" << std::endl;
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