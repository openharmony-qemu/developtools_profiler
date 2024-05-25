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
namespace OHOS {
namespace SmartPerf {
void ByTrace::SetTraceConfig(int mSum, int mInterval, long long mThreshold, int mLowfps, int mCurNum) const
{
    sum = mSum;
    interval = mInterval;
    threshold = mThreshold;
    lowfps = mLowfps;
    curNum = mCurNum;
    LOGI("ByTrace::SetTraceConfig mSum(%d) mInterval(%d) mThreshold(%ll) mLowfps(%d) mCurNum(%d)", mSum, mInterval,
        mThreshold, mLowfps, mCurNum);
}
void ByTrace::ThreadGetTrace() const
{
    isTaskCompleted = true;
    std::string result;
    std::string cmdString;
    if (access("/sys/kernel/tracing/hongmeng", F_OK) != -1) {
        cmdString = "hitrace -t 10 -b 102400 --overwrite idle ace app ohos ability graphic nweb ";
    } else {
        cmdString = "hitrace -t 10 -b 20480 --overwrite idle ace app ohos ability graphic nweb ";
    }
    std::string cmdStringEnd { "sched freq sync workq multimodalinput > " };
    std::string time = std::to_string(SPUtils::GetCurTime());
    std::string traceFile = "/data/local/tmp/sptrace_" + time + ".ftrace";
    std::string traceCmdExe = cmdString + cmdStringEnd + traceFile;
    SPUtils::LoadCmd(traceCmdExe, result);
    std::cout << "TRACE threadGetTrace >> CMD >>" << traceCmdExe << std::endl;
    LOGI("TRACE threadGetTrace  CMD(%s)", traceCmdExe.c_str());
    isTaskCompleted = false;
}
TraceStatus ByTrace::CheckFpsJitters(std::vector<long long> jitters, int cfps) const
{
    times++;
    int two = 2;
    long long curTime = SPUtils::GetCurTime();
    if (curNum <= sum && currentTrigger < 0 && times > two) {
        times = 0;
        for (size_t i = 1; i < jitters.size(); i++) {
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
    int oneHundred = 100;
    if ((curTime - lastTriggerTime) / 1e3 > interval) {
        if (isTaskCompleted) {
            while (isTaskCompleted) {
                std::this_thread::sleep_for(std::chrono::milliseconds(oneHundred));
            }
        }
        std::thread tStart(&ByTrace::ThreadGetTrace, this);
        currentTrigger = 1;
        lastTriggerTime = curTime;
        curNum++;
        tStart.detach();
    }
}
}
}
