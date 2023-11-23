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
#include "include/ByTrace.h"
namespace OHOS {
namespace SmartPerf {
void ByTrace::SetTraceConfig(int mSum, int mInterval, long long mThreshold, int mLowfps, int mCurNum) const
{
    sum = mSum;
    interval = mInterval;
    threshold = mThreshold;
    lowfps = mLowfps;
    curNum = mCurNum;
}
void ByTrace::SetTraceCmd(std::string mTraceCmd) const
{
    traceCmd = mTraceCmd;
}
void ByTrace::ThreadGetTrace() const
{
    std::string result;
    std::stringstream sstream;
    std::string time = std::to_string(SPUtils::GetCurTime());
    sstream.str("");
    sstream << traceCmd;
    sstream << " > /data/app/el2/100/base/com.ohos.smartperf/haps/entry/files/sptrace_";
    sstream << time << ".ftrace";
    std::string traceCmdExe = sstream.str();
    SPUtils::LoadCmd(traceCmdExe, result);
    std::cout << "TRACE threadGetTrace >> CMD >>" << traceCmdExe << std::endl;
}
TraceStatus ByTrace::CheckFpsJitters(std::vector<long long> jitters, int cfps)
{
    times++;
    int two = 2;
    long long curTime = SPUtils::GetCurTime();
    if (curNum <= sum && currentTrigger < 0 && times > two) {
        for (size_t i = 0; i < jitters.size(); i++) {
            long long normalJitter = jitters[i] / 1e6;
            if (normalJitter > threshold || cfps < lowfps) {
                TriggerCatch(curTime);
            }
        }
    }
    std::cout << "TRACE CHECK RUNING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "TRACE CHECK lastTriggerTime:" << lastTriggerTime << " curTime:" << curTime << " currentTrigger:" <<
        currentTrigger << std::endl;
    if ((curTime - lastTriggerTime) / 1e3 > interval && currentTrigger == 1) {
        currentTrigger = -1;
    }
    return TraceStatus::TRACE_FINISH;
}
void ByTrace::TriggerCatch(long long curTime) const
{
    if ((curTime - lastTriggerTime) / 1e3 > interval) {
        std::thread tStart(&ByTrace::ThreadGetTrace, this);
        currentTrigger = 1;
        lastTriggerTime = curTime;
        curNum++;
        std::cout << "TRACE START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
        tStart.detach();
    }
}
}
}
