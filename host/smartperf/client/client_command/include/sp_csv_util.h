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
#ifndef SP_CSV_UTIL_H
#define SP_CSV_UTIL_H
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <map>
#include "common.h"
#include "sp_data.h"
namespace OHOS {
namespace SmartPerf {
class SpCsvUtil {
public:
    static void WriteCsv(const std::string &path, std::vector<SPData> vmap)
    {
        std::ofstream outFile;
        outFile.open(path.c_str(), std::ios::out | std::ios::trunc);
        int i = 0;
        std::string title = "";
        for (SPData spdata : vmap) {
            std::string lineContent = "";
            for (auto iter = spdata.values.cbegin(); iter != spdata.values.cend(); ++iter) {
                if (i == 0) {
                    title += iter->first + ",";
                }
                lineContent += iter->second + ",";
            }
            if (i == 0) {
                title.pop_back();
                outFile << title << std::endl;
            }
            lineContent.pop_back();
            outFile << lineContent << std::endl;
            ++i;
        }
        outFile.close();
    }
    static void WriteCsvH(std::map<std::string, std::string> vmap)
    {
        const std::string outGeneralPath = "/data/local/tmp/smartperf/1/t_general_info.csv";
        std::ofstream outFile(outGeneralPath, std::ios::out | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cout << "Error opening file!" << std::endl;
            return;
        }
        for (const auto& [key, value] : vmap) {
            outFile << key << "," << value << std::endl;
        }
        outFile.close();
    }
};
}
}

#endif // SP_CSV_UTILS_H
