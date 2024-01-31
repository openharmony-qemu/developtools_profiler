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
        void GetLastCompleteLine();
        std::string GetLineTime(std::string lineStr) const;
        std::string CutString(std::string lineStr, std::string start, std::string end, size_t offset) const;
    private:
        std::ifstream infile;
        std::string responseLine = "";
        std::string newCompleteLine = "";
        std::string lastCompleteLine = "";
        std::string completeLine = "";
        bool needUpdateResponseLine = false;
        int frameNum = 0;
        int frameNow = 0;
        int count = 0;
        int four = 4;
        bool updateCount = false;
        double frameRate = 0;
};
}
}
#endif // SMARTPERF_COMMAND_H