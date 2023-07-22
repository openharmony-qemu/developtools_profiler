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
#include "include/sp_utils.h"
#include "include/DDR.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> DDR::ItemData()
{
    std::map<std::string, std::string> result;
    result["ddrFrequency"] = std::to_string(GetDdrFreq());
    return result;
}
long long DDR::GetDdrFreq()
{
    long long curFreq = -1;
    std::string ddrfreq;
    SPUtils::LoadFile(ddrCurFreqPath, ddrfreq);
    curFreq = std::atoll(ddrfreq.c_str());
    return curFreq;
}
}
}
