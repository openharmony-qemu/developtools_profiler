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
#include <string>

#include "hidebug/hidebug_type.h"
#include "hidebug_native_interface.h"
#include "securec.h"
#ifdef HOOK_ENABLE
#include "musl_preinit_common.h"
static volatile struct MallocDispatchType *g_lastDispatchTable = nullptr;
static struct HiDebug_MallocDispatch g_defaultDispatchDatable = {
    .malloc = __libc_malloc_default_dispatch.malloc,
    .calloc = __libc_malloc_default_dispatch.calloc,
    .realloc = __libc_malloc_default_dispatch.realloc,
    .free = __libc_malloc_default_dispatch.free,
    .mmap = __libc_malloc_default_dispatch.mmap,
    .munmap = __libc_malloc_default_dispatch.munmap,
};
static struct MallocDispatchType g_currentDispatchDatable = __libc_malloc_default_dispatch;
#endif
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
    auto memoryLimitOption = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
    if (memoryLimitOption) {
        memoryLimit->vssLimit = memoryLimitOption->vssLimit;
        memoryLimit->rssLimit = memoryLimitOption->rssLimit;
    }
}

void OH_HiDebug_GetAppNativeMemInfoWithCache(HiDebug_NativeMemInfo *nativeMemInfo, bool forceRefresh)
{
    if (!nativeMemInfo) {
        return;
    }
    auto memInfoOption = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(!forceRefresh);
    if (memInfoOption) {
        nativeMemInfo->pss = memInfoOption->pss;
        nativeMemInfo->rss = memInfoOption->rss;
        nativeMemInfo->sharedDirty = memInfoOption->sharedDirty;
        nativeMemInfo->privateDirty = memInfoOption->privateDirty;
        nativeMemInfo->sharedClean = memInfoOption->sharedClean;
        nativeMemInfo->privateClean = memInfoOption->privateClean;
        nativeMemInfo->vss = memInfoOption->vss;
    }
}

void OH_HiDebug_GetAppNativeMemInfo(HiDebug_NativeMemInfo *nativeMemInfo)
{
    OH_HiDebug_GetAppNativeMemInfoWithCache(nativeMemInfo, true);
}

void OH_HiDebug_GetSystemMemInfo(HiDebug_SystemMemInfo *systemMemInfo)
{
    if (!systemMemInfo) {
        return;
    }
    auto sysMemInfo = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetSystemMemInfo();
    if (sysMemInfo) {
        systemMemInfo->totalMem = sysMemInfo->totalMem;
        systemMemInfo->freeMem = sysMemInfo->freeMem;
        systemMemInfo->availableMem = sysMemInfo->availableMem;
    }
}

HiDebug_ErrorCode ConvertTraceErrorCode(OHOS::HiviewDFX::TraceErrorCode traceErrorCode)
{
    switch (traceErrorCode) {
        case OHOS::HiviewDFX::TRACE_SUCCESS:
            return HIDEBUG_SUCCESS;
        case OHOS::HiviewDFX::TRACE_INVALID_ARGUMENT:
            return HIDEBUG_INVALID_ARGUMENT;
        case OHOS::HiviewDFX::TRACE_CAPTURED_ALREADY:
            return HIDEBUG_TRACE_CAPTURED_ALREADY;
        case OHOS::HiviewDFX::TRACE_NO_PERMISSION:
            return HIDEBUG_NO_PERMISSION;
        case OHOS::HiviewDFX::NO_TRACE_RUNNING:
            return HIDEBUG_NO_TRACE_RUNNING;
        default:
            return HIDEBUG_TRACE_ABNORMAL;
    }
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
    if (ret != OHOS::HiviewDFX::TRACE_SUCCESS) {
        return ConvertTraceErrorCode(ret);
    }
    if (strcpy_s(fileName, length, file.c_str()) != EOK) {
        nativeInterface.StopAppTraceCapture();
        return HIDEBUG_INVALID_ARGUMENT;
    }
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_StopAppTraceCapture()
{
    return ConvertTraceErrorCode(OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().StopAppTraceCapture());
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

HiDebug_ErrorCode OH_HiDebug_GetGraphicsMemorySummary(uint32_t interval, HiDebug_GraphicsMemorySummary *summary)
{
    if (summary == nullptr) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    auto ret = OHOS::HiviewDFX::HidebugNativeInterface::GetInstance().GetGraphicsMemorySummary(interval);
    if (!ret) {
        return HIDEBUG_TRACE_ABNORMAL;
    }
    summary->gl = ret->gl;
    summary->graph = ret->graph;
    return HIDEBUG_SUCCESS;
}

HiDebug_ErrorCode OH_HiDebug_SetMallocDispatchTable(struct HiDebug_MallocDispatch *newDispatchTable)
{
#ifdef HOOK_ENABLE
    if (newDispatchTable == nullptr) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    volatile struct MallocDispatchType *lastDispatchTable = (struct MallocDispatchType *)atomic_load_explicit(
        &__musl_libc_globals.current_dispatch_table, memory_order_acquire);
    if (lastDispatchTable != nullptr) {
        g_lastDispatchTable = lastDispatchTable;
    }
    if (newDispatchTable->malloc != nullptr) {
        g_currentDispatchDatable.malloc = newDispatchTable->malloc;
    }
    if (newDispatchTable->free != nullptr) {
        g_currentDispatchDatable.free = newDispatchTable->free;
    }
    if (newDispatchTable->mmap != nullptr) {
        g_currentDispatchDatable.mmap = newDispatchTable->mmap;
    }
    if (newDispatchTable->munmap != nullptr) {
        g_currentDispatchDatable.munmap = newDispatchTable->munmap;
    }
    if (newDispatchTable->calloc != nullptr) {
        g_currentDispatchDatable.calloc = newDispatchTable->calloc;
    }
    if (newDispatchTable->realloc != nullptr) {
        g_currentDispatchDatable.realloc = newDispatchTable->realloc;
    }
    atomic_store_explicit(&__custom_hook_flag, (volatile bool)true, memory_order_seq_cst);
    atomic_exchange_explicit(&__musl_libc_globals.current_dispatch_table,
                             (volatile const long long)&g_currentDispatchDatable, memory_order_acq_rel);
    return HIDEBUG_SUCCESS;
#else
    return HIDEBUG_INVALID_ARGUMENT;
#endif
}

HiDebug_MallocDispatch* OH_HiDebug_GetDefaultMallocDispatchTable(void)
{
#ifdef HOOK_ENABLE
    return &g_defaultDispatchDatable;
#else
    return nullptr;
#endif
}

void OH_HiDebug_RestoreMallocDispatchTable(void)
{
#ifdef HOOK_ENABLE
    atomic_store_explicit(&__custom_hook_flag, (volatile bool)false, memory_order_seq_cst);
    atomic_store_explicit(&__musl_libc_globals.current_dispatch_table, (volatile const long long)g_lastDispatchTable,
                          memory_order_seq_cst);
    g_currentDispatchDatable = __libc_malloc_default_dispatch;
    g_lastDispatchTable = nullptr;
#endif
}