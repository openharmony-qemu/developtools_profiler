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
#include <sstream>
#include <thread>
#include "include/sp_utils.h"
#include "include/Capture.h"
namespace OHOS {
namespace SmartPerf {
void Capture::ThreadGetCatch(const std::string &curTime) const
{
    std::string result;
    std::string cmdCapture = "snapshot_display -f /data/local/tmp/capture/" + curTime +".png";
    SPUtils::LoadCmd(cmdCapture, result);
    std::cout << "Screen Capture Thread >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
}
void Capture::TriggerGetCatch(long long curTime) const
{
    std::string curTimeStr = std::to_string(curTime);
    std::thread tStart(&Capture::ThreadGetCatch, this, curTimeStr);
    tStart.detach();
}
}
}
