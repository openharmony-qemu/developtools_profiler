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

#include "hidebug/hidebug.h"

#include <memory>
#include <vector>
#include <unistd.h>

#include "hidebug/hidebug_type.h"
#include "hidebug_native_interface.h"

double OH_HiDebug_GetAppCpuUsage()
{
    double cpuUsage = OHOS::HiviewDFX::HidebugNativeInterface::CreateInstance()->GetCpuUsage();
    return cpuUsage;
}

HiDebug_ThreadCpuUsagePtr OH_HiDebug_GetAppThreadCpuUsage()
{
    auto nativeInterface = OHOS::HiviewDFX::HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        return nullptr;
    }
    std::map<uint32_t, double> threadMap = nativeInterface->GetAppThreadCpuUsage();
    HiDebug_ThreadCpuUsagePtr head = nullptr;
    HiDebug_ThreadCpuUsagePtr prev = nullptr;
    for (const auto[threadId, cpuUsage] : threadMap) {
        HiDebug_ThreadCpuUsagePtr node = (HiDebug_ThreadCpuUsagePtr) malloc(sizeof(HiDebug_ThreadCpuUsage));
        if (node == nullptr) {
            continue;
        }
        node->threadId = threadId;
        node->cpuUsage = cpuUsage;
        node->next = nullptr;
        if (prev == nullptr) {
            head = node;
        } else {
            prev->next = node;
        }
        prev = node;
    }
    return head;
}

void OH_HiDebug_FreeThreadCpuUsage(HiDebug_ThreadCpuUsagePtr *threadCpuUsage)
{
    if (threadCpuUsage == nullptr || *threadCpuUsage == nullptr) {
        return;
    }
    HiDebug_ThreadCpuUsagePtr node = *threadCpuUsage;
    while (node != nullptr) {
        HiDebug_ThreadCpuUsagePtr next = node->next;
        free(node);
        node = next;
    }
    *threadCpuUsage = nullptr;
}