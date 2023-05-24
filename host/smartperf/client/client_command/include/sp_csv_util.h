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
    static void WriteCsv(std::string path, std::vector<SPData> vmap)
    {
        std::ofstream outFile;
        char realPath[PATH_MAX] = {0x00};
        if (realpath(path.c_str(), realPath) == nullptr) {
            std::cout << "" << std::endl;
        }
        outFile.open(path.c_str(), std::ios::out);
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
    static void WriteCsvH(std::string path, std::map<std::string, std::string> vmap)
    {
        std::ofstream outFile;
        char realPath[PATH_MAX] = {0x00};
        if (realpath(path.c_str(), realPath) == nullptr) {
            std::cout << "" << std::endl;
        }
        outFile.open(path.c_str(), std::ios::out);
        for (auto iter = vmap.cbegin(); iter != vmap.cend(); ++iter) {
            outFile << iter->first + "," + iter->second << std::endl;
        }
        outFile.close();
    }
};
}
}

#endif // SP_CSV_UTILS_H
