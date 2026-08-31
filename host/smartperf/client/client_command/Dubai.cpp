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
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
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
    LOGD("Dubai::DumpDubaiBegin");
}
void Dubai::DumpDubaiFinish()
{
    std::string result;
    std::string dumpBubaiF = HIDUMPER_CMD_MAP.at(HidumperCmd::DUMPER_DUBAI_F);
    SPUtils::LoadCmd(dumpBubaiF, result);
    LOGD("Dubai::DumpDubaiFinish");
}

void Dubai::MoveDubaiDb()
{
    std::string result;
    const std::string dubaiXpower = "/data/service/el2/100/xpower/dubai.db";
    const std::string Database = "/data/app/el2/100/database/";
    const std::string PkgEntry = "/entry/rdb";
    const std::string dubaiDbPath = Database + dubaiPkgName + PkgEntry + "/dubai.db";
    LOGD("cpDubai: (%s) to (%s)",
        dubaiXpower.c_str(), dubaiDbPath.c_str());
    if (!IsFileAccessible(dubaiXpower)) {
        sleep(1);
    }
    if (!SPUtils::CopyFile(dubaiXpower, dubaiDbPath)) {
        result = "";
        LOGE("Dubai::Copy dubai.db failed");
    } else {
        SPUtils::Chmod(dubaiDbPath, FILE_PERMISSION_FULL);
    }
}

void Dubai::MoveDubaiDb(const std::string &path)
{
    std::string result;
    const std::string dubaiLocalPath = "/data/local/tmp/dubai/";
    SPUtils::CreateDir(dubaiLocalPath);
    const std::string dubaiDbLocalPath = dubaiLocalPath + "dubai.db";
    LOGD("cpDubai: (%s) to (%s)",
        path.c_str(), dubaiDbLocalPath.c_str());
    if (!IsFileAccessible(path)) {
        sleep(1);
    }
    if (!SPUtils::CopyFile(path, dubaiDbLocalPath)) {
        result = "";
        LOGE("Dubai::Copy dubai.db failed");
    } else {
        SPUtils::Chmod(dubaiDbLocalPath, FILE_PERMISSION_FULL);
    }
}

void Dubai::CallBeginAndFinish()
{
    DumpDubaiBegin();
    DumpDubaiFinish();
}

std::string Dubai::CallMoveDubaiDbFinished()
{
    std::string dubaiMoveFinish;
    if (isDumpDubaiFinish) {
        MoveDubaiDb();
    }
    dubaiMoveFinish = "get_dubai_db";
    return dubaiMoveFinish;
}

bool Dubai::IsFileAccessible(const std::string &filename)
{
    char duBaiRealPath[PATH_MAX] = {0x00};
    if (realpath(filename.c_str(), duBaiRealPath) == nullptr) {
        std::cout << "" << std::endl;
    }
    std::ifstream file(duBaiRealPath);
    return file.good();
}
}
}