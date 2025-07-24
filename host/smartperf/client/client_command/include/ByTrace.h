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
#ifndef BY_TRACE_H
#define BY_TRACE_H
#include "common.h"
#include <fstream>
#include "sp_profiler.h"
namespace OHOS {
namespace SmartPerf {
class ByTrace : public SpProfiler {
public:
    static ByTrace &GetInstance()
    {
        static ByTrace instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
    // trace配置
    void SetTraceConfig(long long mThreshold, int mLowfps) const;
    // 校验fps-jitters
    void CheckFpsJitters(long long& jitters, int cfps) const;
    // 触发trace
    void SetByTrace() const;
    void ClearTraceFiles() const;
    void RemoveTraceFiles() const;
    void CpTraceFile() const;
    static inline std::string jittersAndLowFps = "";
    static inline std::string hiviewTrace = "";
public:
    // 抓trace触发条件:默认 某一帧的某个jitter>100 ms触发
    mutable long long threshold = 100;
    mutable long long lastEnableTime = 0;
    // 低帧触发
    mutable int lowfps = -1;
    // 前2秒采的不准
    mutable int times = 0;
    mutable std::string traceCpPath_ {"/data/local/tmp/hitrace"};
    mutable long long comperTime = 1000;
    mutable long long nowTime = 0;
private:
    ByTrace() {};
    ByTrace(const ByTrace &);
    ByTrace &operator = (const ByTrace &);

    mutable std::string hiviewTracePath_ {"/data/log/hiview/unified_collection/trace/special/"};
};
}
}
#endif