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
#include "unistd.h"
#include "include/sp_utils.h"
#include "include/ByTrace.h"
#include "include/sp_log.h"
#include "include/common.h"
namespace OHOS {
namespace SmartPerf {
void ByTrace::SetTraceConfig(int mSum, int mInterval, long long mThreshold, int mLowfps, int mCurNum) const
{
    sum = mSum;
    interval = mInterval;
    threshold = mThreshold;
    lowfps = mLowfps;
    curNum = mCurNum;
    LOGD("ByTrace::SetTraceConfig mSum(%d) mInterval(%d) mThreshold(%lld) mLowfps(%d) mCurNum(%d)",
        mSum, mInterval, mThreshold, mLowfps, mCurNum);
}
void ByTrace::ThreadGetTrace() const
{
    std::string result;
    std::string cmdString;
    if (SPUtils::IsHmKernel()) {
        cmdString = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_1024);
    } else {
        cmdString = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_2048);
    }
    std::string time = std::to_string(SPUtils::GetCurTime());
    std::string traceFile = "/data/local/tmp/sptrace_" + time + ".ftrace";
    std::string traceCmdExe = cmdString + traceFile;
    SPUtils::LoadCmd(traceCmdExe, result);
    LOGD("TRACE threadGetTrace  CMD(%s)", traceCmdExe.c_str());
}
TraceStatus ByTrace::CheckFpsJitters(std::vector<long long> jitters, int cfps) const
{
    times++;
    int two = 2;
    long long curTime = SPUtils::GetCurTime();
    LOGD("Bytrace get curTime : %lld", curTime);
    if (curNum <= sum && currentTrigger < 0 && times > two) {
        for (size_t i = 0; i < jitters.size(); i++) {
            long long normalJitter = jitters[i] / 1e6;
            if (normalJitter > threshold || cfps < lowfps) {
                TriggerCatch(curTime);
            }
        }
    }
    if ((curTime - lastTriggerTime) / 1e3 > interval && currentTrigger == 1) {
        currentTrigger = -1;
    }
    return TraceStatus::TRACE_FINISH;
}
void ByTrace::TriggerCatch(long long curTime) const
{
    if ((curTime - lastTriggerTime) / 1e3 > interval && !CheckHitraceId()) {
        std::thread tStart([this] { this->ThreadGetTrace(); });
        currentTrigger = 1;
        lastTriggerTime = curTime;
        curNum++;
        tStart.detach();
    }
}

bool ByTrace::CheckHitraceId() const
{
    std::string result;
    std::string hitrace = CMD_COMMAND_MAP.at(CmdCommand::HITRACE_CMD);
    SPUtils::LoadCmd(hitrace, result);
    if (result.empty()) {
        return false;
    }
    if (result.find("-t") != std::string::npos) {
        return true;
    }
    return false;
}
}
}
