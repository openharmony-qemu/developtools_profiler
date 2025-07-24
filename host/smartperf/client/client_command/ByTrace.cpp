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
#include <filesystem>
#include <thread>
#include "unistd.h"
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/sp_log.h"
#include "include/common.h"

namespace {
constexpr long long RM_10000 = 10000;
}
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> ByTrace::ItemData()
{
    return std::map<std::string, std::string>();
}

void ByTrace::SetTraceConfig(long long mThreshold, int mLowfps) const
{
    threshold = mThreshold;
    lowfps = mLowfps;
    LOGD("ByTrace::SetTraceConfig threshold(%lld), lowfps(%d)", threshold, lowfps);
}

void ByTrace::SetByTrace() const
{
    std::vector<std::string> values;
    std::string delimiter = "||";
    std::string delim = "=";
    SPUtils::StrSplit(jittersAndLowFps, delimiter, values);
    long long mThreshold = 0;
    int lowFps = 0;
    for (std::string& vItem : values) {
        std::vector<std::string> vItems;
        SPUtils::StrSplit(vItem, delim, vItems);
        if (vItems[0] == "fpsJitterTime") {
            mThreshold = SPUtilesTye::StringToSometype<int>(vItems[1]);
        }
        if (vItems[0] == "lowFps") {
            lowFps = SPUtilesTye::StringToSometype<int>(vItems[1]);
        }
    }
    SetTraceConfig(mThreshold, lowFps);
}

void ByTrace::ClearTraceFiles() const
{
    if (!std::filesystem::exists(traceCpPath_)) {
        SPUtils::CreateDir(traceCpPath_);
    } else {
        RemoveTraceFiles();
    }
}

void ByTrace::RemoveTraceFiles() const
{
    if (std::filesystem::is_directory(traceCpPath_)) {
        for (const auto& entry : std::filesystem::directory_iterator(traceCpPath_)) {
            if (!entry.is_directory()) {
                std::filesystem::remove(entry.path());
            }
        }
    }
}

void ByTrace::CpTraceFile() const
{
    std::string result;
    std::string cpResult;
    const std::string cpHiviewTracePath = hiviewTracePath_ + hiviewTrace;
    const std::string cpTrace = "cp -r " + cpHiviewTracePath + " " + traceCpPath_;
    SPUtils::LoadCmd(cpTrace, result);
    if (!result.empty()) {
        const std::string tracePathChmod = "chmod 777 " + traceCpPath_;
        SPUtils::LoadCmd(tracePathChmod, cpResult);
    }
}

void ByTrace::CheckFpsJitters(long long& jitters, int cfps) const
{
    times++;
    int two = 2;
    if (times > two) {
        nowTime = SPUtils::GetCurTime();
        if (lastEnableTime > 0) {
            long long diff =
                lastEnableTime > nowTime ? (LLONG_MAX - lastEnableTime + nowTime) : (nowTime - lastEnableTime);
            LOGD("ByTrace::Time difference: %lld ms", diff);
            if (diff > RM_10000) {
            LOGW("ByTrace::Time difference exceeded threshold, resetting start capture time.");
            lastEnableTime = 0;
        }
        }
    }
    if (cfps < lowfps || jitters > threshold) {
        if (lastEnableTime == 0) {
            lastEnableTime = nowTime;
            ipcCallback_("surfaceCaton");
        }
    }
}
}
}
