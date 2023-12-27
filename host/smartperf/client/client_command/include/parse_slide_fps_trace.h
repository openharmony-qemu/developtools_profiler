/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PARSESLIDEFPSTRACE_H
#define PARSESLIDEFPSTRACE_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace OHOS {
namespace SmartPerf {
class ParseSlideFpsTrace {
    public:
        double ParseSlideFpsTraceNoh(std::string file);
        double CalculateTime();
        std::string GetLineTime(std::string lineStr);
        std::string CutString(std::string lineStr, std::string start, std::string end, size_t off);
    private:
        std::ifstream infile;
        std::string startLine = "";
        std::string responseLine = "";
        std::string completeLine = "";
        bool needUpdateResponseLine = false;
        int frameNum = 0;
        std::string pid = "";
        int count = 0;
        bool updateCount = false;
        double frameStartTime = 0;
        bool animatorFlag = false;
        int animatorSecond = 0;
        std::string::size_type maxnpos = std::string::npos;
        std::string line;
        int numA = 2;
};
}
}
#endif // SMARTPERF_COMMAND_H