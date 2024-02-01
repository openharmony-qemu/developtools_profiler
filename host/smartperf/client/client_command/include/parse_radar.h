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
#ifndef PARSE_RADAR_H
#define PARSE_RADAR_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
namespace OHOS {
    namespace SmartPerf {
        class Radar {
            public:
                double ParseRadarStart(std::string string);
                double ParseRadarStartResponse(std::string string);
                double ParseRadarResponse(std::string string);
                std::string  ParseRadarAppStrart(std::string string);
                double ParseRadarComplete(std::string string);
                std::string extract_string(const std::string& str, const std::string& target);
                std::string ParseRadarFrame(std::string string);
        };
    }
}
#endif // PARSE_RADAR_H