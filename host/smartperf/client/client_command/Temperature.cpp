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
#include "include/sp_log.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> Temperature::ItemData()
{
    DIR *dp = opendir(thermalBasePath.c_str());
    struct dirent *dirp;
    std::vector<std::string> dirs;
    if (dp == nullptr) {
        std::cout << "Open directory failed!" << std::endl;
    }
    while ((dirp = readdir(dp)) != nullptr) {
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
            dirs.push_back(SPUtils::IncludePathDelimiter(thermalBasePath) + std::string(dirp->d_name));
        }
    }
    closedir(dp);
    std::map<std::string, std::string> result;
    for (auto dir : dirs) {
        std::string dirType = dir + "/type";
        LOGI("dirType====: %s", dirType.c_str());
        std::string dirTemp = dir + "/temp";
        LOGI("dirTemp====: %s", dirTemp.c_str());

        if (SPUtils::FileAccess(dirType)) {
            std::string type;
            std::string temp;
            SPUtils::LoadFile(dirType, type);
            SPUtils::LoadFile(dirTemp, temp);
            for (auto node : collectNodes) {
                if (type.find(node) != std::string::npos) {
                    LOGI("type====: %s", type.c_str());
                    float t = std::stof(temp);
                    LOGI("temp====: %s", temp.c_str());
                    result[type] = std::to_string(t / 1e3);
                }
            }
        }
    }
    return result;
}
}
}
