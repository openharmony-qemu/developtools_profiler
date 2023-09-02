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