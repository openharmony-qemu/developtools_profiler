/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "profiler_process_manager.h"
#include <thread>
#include <chrono>
#include "parameters.h"

ProfilerProcessManager& ProfilerProcessManager::GetInstance()
{
    static ProfilerProcessManager instance;
    return instance;
}

bool ProfilerProcessManager::StartDependentProcess()
{
    constexpr int waitProcMills = 300;
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "0");
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "0");

    OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "1");
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    return true;
}

void ProfilerProcessManager::KillDependentProcess()
{
    constexpr int waitProcMills = 300;
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "0");
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "0");
}
