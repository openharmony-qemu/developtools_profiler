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
#include "include/sp_utils.h"
#include "include/Dubai.h"
namespace OHOS {
namespace SmartPerf {

void Dubai::DumpDubaiBegin()
{
    std::string result;
    SPUtils::LoadCmd("hidumper -s 1213 -a '-b'", result);
}
void Dubai::DumpDubaiFinish()
{
    std::string result;
    SPUtils::LoadCmd("hidumper -s 1213 -a '-f'", result);
}

void Dubai::MoveDubaiDb()
{
    std::string result;
    const std::string dubaiName = "dubai.db";
    const std::string dubaiPath = "/data/system_ce/0/dubai/" + dubaiName;
    const std::string devicePath = "/data/app/el2/100/database/com.ohos.smartperf/entry/rdb";
    SPUtils::LoadCmd("rm -rf " + devicePath + "/dubai.*", result);
    SPUtils::LoadCmd("cp " + dubaiPath + " " + devicePath, result);
    SPUtils::LoadCmd("chmod 777 " + devicePath + "/" + dubaiName, result);
}
}
}
