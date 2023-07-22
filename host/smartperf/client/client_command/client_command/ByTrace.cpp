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
void ByTrace::SetTraceConfig(int mSum, int mInterval, long long mThreshold) const
{
    sum = mSum;
    interval = mInterval;
    threshold = mThreshold;
}

void ByTrace::ThreadGetTrace() const
{
    std::string result;
    SPUtils::LoadCmd(std::string("bytrace --trace_begin --overwrite"), result);
    std::cout << "TRACE threadGetTrace >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
}
void ByTrace::ThreadEndTrace() const
{
    std::string result;
    SPUtils::LoadCmd(std::string("bytrace --trace_finish --overwrite"), result);
    std::cout << "TRACE threadGetTrace >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
}
void ByTrace::ThreadFinishTrace(const std::string &pathName) const
{
    std::string result;
    std::stringstream sstream;
    sstream.str("");
    sstream << "bytrace --overwrite sched ace app disk ohos graphic sync workq ability";
    sstream << " > /data/app/el2/100/base/com.ohos.gameperceptio/haps/entry/files/sptrace_";
    sstream << pathName << ".ftrace";
    const std::string &cmdTraceOverwrite = sstream.str();
    SPUtils::LoadCmd(cmdTraceOverwrite, result);
    std::cout << "TRACE threadFinishTrace >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
}

TraceStatus ByTrace::CheckFpsJitters(std::vector<long long> jitters)
{
    long long curTime = SPUtils::GetCurTime();
    if (curNum <= sum && curTriggerFlag < 0) {
        for (size_t i = 0; i < jitters.size(); i++) {
            long long normalJitter = jitters[i] / 1e6;
            if (normalJitter > threshold) {
                TriggerCatch(curTime);
            }
        }
    }
    std::cout << "TRACE CHECK RUNING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "TRACE CHECK lastTriggerTime:" << lastTriggerTime << " curTime:" << curTime << " curTriggerFlag:" <<
        curTriggerFlag << std::endl;
    if ((curTime - lastTriggerTime) > cost && curTriggerFlag > 0) {
        std::string result;
        SPUtils::LoadCmd(std::string("bytrace --trace_finish"), result);
        std::string pathName = std::to_string(curTime);
        std::thread tFinish(&ByTrace::ThreadFinishTrace, this, std::ref(pathName));
        curTriggerFlag = -1;
        std::cout << "TRACE FINISH >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
        tFinish.detach();
    }
    return TraceStatus::TRACE_FINISH;
}
void ByTrace::TriggerCatch(long long curTime) const
{
    if ((curTime - lastTriggerTime) > interval) {
        std::thread tStart(&ByTrace::ThreadGetTrace, this);
        curTriggerFlag = 1;
        lastTriggerTime = curTime;
        curNum++;
        std::cout << "TRACE START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
        tStart.detach();
    }
}
}
}
