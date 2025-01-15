/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef AI_SCHEDULE_H
#define AI_SCHEDULE_H
#include "sp_profiler.h"
#include <string>

namespace OHOS {
namespace SmartPerf {
class AISchedule : public SpProfiler {
    public:
    std::map<std::string, std::string> ItemData() override;
    static AISchedule &GetInstance()
    {
        static AISchedule instance;
        return instance;
    }

    private:
    AISchedule() {};
    AISchedule(const AISchedule &) = delete;
    AISchedule &operator=(const AISchedule &) = delete;
    std::map<std::string, std::string> aiScheduleParams;
    const std::string AI_SCHEDULE_PARAM_PID = "pid";
    const std::string AI_SCHEDULE_PARAM_TYPE = "functionType";
    const std::string CREATE_PLUGIN = "onCreatePlugin";
    std::string gamePid;
};
} // namespace SmartPerf
} // namespace OHOS
#endif // AI_SCHEDULE_H