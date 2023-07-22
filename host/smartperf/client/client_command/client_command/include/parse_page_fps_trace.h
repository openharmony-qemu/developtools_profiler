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
        class PageFpsTrace {
            public:
                double ParsePageFpsTrace(std::string file);
                double CalculateTime();
                std::string getLineTime(std::string line);
                std::string CutString(std::string line, std::string start, std::string end, size_t offset);
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
                double frameEndTime = 0;
                double frameStartInterval = 0;
        };
    }
}