/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hidebug_app_thread_cpu.h"

#include <unistd.h>

#include "hilog/log.h"


namespace OHOS {
namespace HiviewDFX {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_App_Thread_Cpu"

HidebugAppThreadCpu::HidebugAppThreadCpu()
{
    InitThreadCpuCollector();
}

void HidebugAppThreadCpu::InitThreadCpuCollector()
{
    int32_t pid = getprocpid();
    threadStatInfoCollector_ = UCollectUtil::ThreadCpuCollector::Create(pid);
    if (!threadStatInfoCollector_) {
        HILOG_ERROR(LOG_CORE, "InitThreadCpuCollector fail, CreateThreadStatInfoCollector fail!");
    }
}

CollectResult<std::vector<ThreadCpuStatInfo>> HidebugAppThreadCpu::CollectThreadStatInfos() const
{
    if (!threadStatInfoCollector_) {
        CollectResult<std::vector<ThreadCpuStatInfo>> threadCollectResult;
        return threadCollectResult;
    }
    return threadStatInfoCollector_->CollectThreadStatInfos(true);
}

}
}