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
#include "include/GPU.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> GPU::ItemData()
{
    std::map<std::string, std::string> result;
    int freq = GetGpuFreq();
    float load = GetGpuLoad();
    result["gpuFrequency"] = std::to_string(freq);
    result["gpuLoad"] = std::to_string(load);
    return result;
}

int GPU::GetGpuFreq()
{
    std::string gpuFreq;
    for (auto path : gpuCurFreqPaths) {
        if (SPUtils::FileAccess(path)) {
            SPUtils::LoadFile(path, gpuFreq);
        }
    }
    return atoi(gpuFreq.c_str());
}
float GPU::GetGpuLoad()
{
    std::vector<std::string> sps;
    std::string bufferLine;
    for (auto path : gpuCurLoadPaths) {
        if (SPUtils::FileAccess(path)) {
            SPUtils::LoadFile(path, bufferLine);
        }
    }
    SPUtils::StrSplit(bufferLine, "@", sps);
    if (sps.size() > 0) {
        float loadRk = std::stof(sps[0]);
        return loadRk;
    } else {
        float loadWgr = std::stof(bufferLine);
        return loadWgr;
    }
    return -1.0;
}
}
}
