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
#include "include/sp_log.h"
#include "include/common.h"
namespace OHOS {
namespace SmartPerf {
void Dubai::DumpDubaiBegin()
{
    std::string result;
    std::string dumpBubaiB = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_DUBAI_B);
    SPUtils::LoadCmd(dumpBubaiB, result);
    LOGI("Dubai::DumpDubaiBegin");
}
void Dubai::DumpDubaiFinish()
{
    std::string result;
    std::string dumpBubaiF = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_DUBAI_F);
    SPUtils::LoadCmd(dumpBubaiF, result);
    LOGI("Dubai::DumpDubaiFinish");
}

void Dubai::MoveDubaiDb()
{
    std::string result;
    const std::string dubaiXpower = "/data/service/el2/100/xpower";
    const std::string XpowerDb = "/dubai.db";
    const std::string Database = "/data/app/el2/100/database/";
    const std::string PkgEntry = "/entry/rdb";
    const std::string cpDubai = "cp " + dubaiXpower + XpowerDb + " " + Database + dubaiPkgName + PkgEntry;
    const std::string dubaiPathChmod = "chmod 777 " + Database + dubaiPkgName + PkgEntry + XpowerDb;
    LOGI("cpDubai: (%s)", cpDubai.c_str());
    LOGI("dubaiPathChmod: (%s)", dubaiPathChmod.c_str());
    SPUtils::LoadCmd(cpDubai, result);
    SPUtils::LoadCmd(dubaiPathChmod, result);
}

void Dubai::CallBeginAndFinish()
{
    Dubai::DumpDubaiBegin();
    Dubai::DumpDubaiFinish();
    LOGI("Dubai::CallBeginAndFinish");
}

std::string Dubai::CallMoveDubaiDbFinished()
{
    std::string dubaiMoveFinish;
    if (Dubai::isDumpDubaiFinish) {
        Dubai::MoveDubaiDb();
        LOGI("Dubai::MoveDubaiDbFinished");
    }
    dubaiMoveFinish = "get_dubai_db";
    return dubaiMoveFinish;
}
}
}
