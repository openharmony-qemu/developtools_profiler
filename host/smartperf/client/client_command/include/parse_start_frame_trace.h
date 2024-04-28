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
#ifndef PARSE_START_FRAME_TRACE_H
#define PARSE_START_FRAME_TRACE_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
namespace OHOS {
namespace SmartPerf {
class StartFrameTraceNoh {
public:
    double ParseStartFrameTraceNoh(std::string file);
    double CalculateTime();
    double GetFps();
    std::string GetLineTime(std::string line);
    std::string CutString(std::string line, const std::string &start, const std::string &end, size_t offset);
    std::vector<std::string> Split(std::string str, const std::string &pattern);

private:
    std::ifstream infile;
    int twelve = 12;
    int four = 4;
    std::string startLine = "";
    std::string fullScreenLine = "";
    int frameNum = 0;
    int count = 0;
    std::string pid = "";
    bool isNeedEnd = false;
    double frameStartTime = 0;
    double frameEndTime = 0;
    struct FrameTimeSet {
        int frameId = 0;
        double times = 0.0;
    };
    FrameTimeSet startTimes;
    FrameTimeSet endTimes;
    std::vector<FrameTimeSet> startTimeArr;
    std::vector<FrameTimeSet> endTimeArr;
    std::string::size_type maxnpos = std::string::npos;
};
}
}
#endif