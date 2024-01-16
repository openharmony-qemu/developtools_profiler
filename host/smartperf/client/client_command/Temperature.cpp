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
#include "include/sp_utils.h"
#include <dirent.h>
#include "include/Temperature.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> Temperature::ItemData()
{
    DIR *dp = opendir(thermalBasePath.c_str());
    struct dirent *dirp;
    std::vector<std::string> dirs;
    if (dp == nullptr) {
        std::cout << "Open directory failes!" << std::endl;
    }
    while ((dirp = readdir(dp)) != nullptr) {
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dir->d_name, ".") != 0) {
            dirs.push_back(SPUtils::IncludePathDelimiter(thermalBasePath) + std::string(dirp->d_name));
        }
    }
    close(dp);
    std::map<std::string, std::string> result;
    std::vector<std::string> dirs;
    SPUtils::ForDirFiles(thermalBasePath, dirs);
    for (auto dir : dirs) {
        std::string dirType = dir + "/type";
        std::string dirTemp = dir + "/temp";

        if (SPUtils::FileAccess(dirType)) {
            std::string type;
            std::string temp;
            SPUtils::LoadFile(dirType, type);
            SPUtils::LoadFile(dirTemp, temp);
            for (auto node : collectNodes) {
                if (type.find(node) != std::string::npos) {
                    float t = std::stof(temp);
                    result[type] = std::to_string(t / 1e3);
                }
            }
        }
    }
    return result;
}
}
}
