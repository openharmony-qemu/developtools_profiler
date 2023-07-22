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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
namespace OHOS {
namespace SmartPerf {
class ParseTrace {
public:
    double ParseTraceCold(const std::string &fileNamePath, const std::string &appPid);
    double ParseTraceHot(const std::string &fileNamePath);
    double ParseNohTrace(const std::string &fileNamePath, const std::string &appPid);
    double ParseTraceNoh(const std::string &fileNamePath, const std::string &appPid);
    double ParseCodeTrace(const std::string &fileNamePath, const std::string &appPid);
    double ParseHotTrace(const std::string &fileNamePath);
    static double GetTime(std::string start, std::string end);
    std::string GetStartTime(std::string line, const std::string &startTimeBefore);
    std::string GetWindowTime(std::string line, std::string wt);
    std::string GetFrameId(std::string line, std::string appPid, std::string fid);
private:
    std::ifstream infile;
    std::string flagTime = "0";
    int flagTouch = 0;
    std::string startTime = "0";
    std::string endTime = "0";
    std::string endTimeFlag = "0";
    std::string windowTime = "0";
    std::string frameId = "0";
};
}
}