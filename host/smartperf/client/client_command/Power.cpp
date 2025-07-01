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
#include "include/Power.h"
#include "include/sp_log.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> Power::ItemData()
{
    std::map<std::string, std::string> result;
    if (!rkFlag) {
        std::string currentNow;
        SPUtils::LoadFile(currentNowPath, currentNow);
        std::string voltageNow;
        SPUtils::LoadFile(voltageNowPath, voltageNow);
        if (currentNow.empty()) {
            currentNow = "NA";
        }
        if (voltageNow.empty()) {
            voltageNow = "NA";
        }

        result["currentNow"] = currentNow;
        result["voltageNow"] = voltageNow;
    } else {
        result["failed"] = "RK does not support power acquisition";
        LOGE("failed:RK does not support power acquisition");
    }
    LOGI("Power:ItemData map size(%u)", result.size());
    return result;
}
void Power::SetRkFlag()
{
    rkFlag = true;
}
}
}