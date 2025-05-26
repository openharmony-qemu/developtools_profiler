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
#include "securec.h"

double OH_HiDebug_GetAppCpuUsage()
{
    return OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetCpuUsage();
}

double OH_HiDebug_GetSystemCpuUsage()
{
    auto cpuUsageOptional = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetSystemCpuUsage();
    if (cpuUsageOptional) {
        return cpuUsageOptional.value();
    }
    return 0;
}

HiDebug_ThreadCpuUsagePtr OH_HiDebug_GetAppThreadCpuUsage()
{
    auto threadMap = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
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

void OH_HiDebug_GetAppMemoryLimit(HiDebug_MemoryLimit *memoryLimit)
{
    if (!memoryLimit) {
        return;
    }
    auto collectResult = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
    if (!collectResult) {
        return;
    }
    memoryLimit->vssLimit = collectResult->vssLimit;
    memoryLimit->rssLimit = collectResult->rssLimit;
}

void OH_HiDebug_GetAppNativeMemInfo(HiDebug_NativeMemInfo *nativeMemInfo)
{
    if (!nativeMemInfo) {
        return;
    }
    auto nativeMemoryInfo = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetAppNativeMemInfo();
    if (!nativeMemoryInfo) {
        return;
    }

    nativeMemInfo->pss = static_cast<uint32_t>(nativeMemoryInfo->pss);
    nativeMemInfo->vss = static_cast<uint32_t>(nativeMemoryInfo->vss);
    nativeMemInfo->rss = static_cast<uint32_t>(nativeMemoryInfo->rss);
    nativeMemInfo->sharedDirty = static_cast<uint32_t>(nativeMemoryInfo->sharedDirty);
    nativeMemInfo->privateDirty = static_cast<uint32_t>(nativeMemoryInfo->privateDirty);
    nativeMemInfo->sharedClean = static_cast<uint32_t>(nativeMemoryInfo->sharedClean);
    nativeMemInfo->privateClean = static_cast<uint32_t>(nativeMemoryInfo->privateClean);
}

void OH_HiDebug_GetSystemMemInfo(HiDebug_SystemMemInfo *systemMemInfo)
{
    if (!systemMemInfo) {
        return;
    }
    auto sysMemInfo = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetSystemMemInfo();
    if (!sysMemInfo) {
        return;
    }

    systemMemInfo->totalMem = static_cast<uint32_t>(sysMemInfo->memTotal);
    systemMemInfo->freeMem = static_cast<uint32_t>(sysMemInfo->memFree);
    systemMemInfo->availableMem = static_cast<uint32_t>(sysMemInfo->memAvailable);
}

HiDebug_ErrorCode OH_HiDebug_StartAppTraceCapture(HiDebug_TraceFlag flag,
    uint64_t tags, uint32_t limitSize, char* fileName, uint32_t length)
{
    if (fileName == nullptr) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    auto& nativeInterface = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance();
    std::string file;
    auto ret = nativeInterface.StartAppTraceCapture(tags, flag, limitSize, file);
    if (ret != HIDEBUG_SUCCESS) {
        return ret;
    }
    if (strcpy_s(fileName, length, file.c_str()) != EOK) {
        nativeInterface.StopAppTraceCapture();
        return HIDEBUG_INVALID_ARGUMENT;
    }
    return HIDEBUG_SUCCESS;
}


HiDebug_ErrorCode OH_HiDebug_StopAppTraceCapture()
{
    return OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().StopAppTraceCapture();
}

HiDebug_ErrorCode OH_HiDebug_GetGraphicsMemory(uint32_t *value)
{
    if (value == nullptr) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    std::optional<int32_t> ret = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetGraphicsMemory();
    if (!ret || ret < 0) {
        return HIDEBUG_TRACE_ABNORMAL;
    }
    *value = static_cast<uint32_t>(ret.value());
    return HIDEBUG_SUCCESS;
}