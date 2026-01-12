/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#include "hook_guard.h"
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <malloc.h>
#include "hook_common.h"
#include "hook_client.h"
#include "sampling.h"

constexpr int PID_NAMESPACE_ID = 1;
constexpr int MIN_SAMPLER_INTERVAL = 1;

using namespace OHOS::Developtools::NativeDaemon;

namespace HookGuard {

bool IsReady()
{
    return g_hookReady.load();
}

bool IsPidChanged()
{
    if (g_isPidChanged.load()) {
        return true;
    }
    int pid = getpid();
    if (pid == PID_NAMESPACE_ID) {
        return false;
    }
    bool changed = (g_hookPid.load() != 0 && g_hookPid.load() != pid);
    if (changed) {
        g_isPidChanged.store(true);
    }
    return changed;
}

bool ShouldSkipMalloc()
{
    return g_clientConfig.mallocDisable || IsPidChanged();
}

bool ShouldSkipMemtrace()
{
    return !g_clientConfig.memtraceEnable || IsPidChanged();
}

bool ShouldSkipMmap()
{
    return g_clientConfig.mmapDisable || IsPidChanged();
}

bool ShouldFilterBySize(void* ptr, size_t mallocSize)
{
    if (g_clientConfig.largestSize == 0 || g_clientConfig.secondLargestSize == 0) {
        return false;
    }
    if (mallocSize >= g_clientConfig.sampleInterval) {
        return false;
    }

    size_t usableSize = 0;
    if (mallocSize == 0) {
        usableSize = malloc_usable_size(ptr);
    } else {
        std::unique_lock<std::mutex> lock(g_usableSizeMapMutex);
        auto it = g_mallocUsableSizeMap.find(mallocSize);
        if (it == g_mallocUsableSizeMap.end()) {
            usableSize = malloc_usable_size(ptr);
            g_mallocUsableSizeMap[mallocSize] = usableSize;
        } else {
            usableSize = it->second;
        }
    }

    if (usableSize >= g_clientConfig.sampleInterval) {
        return false;
    }

    if ((usableSize == g_clientConfig.largestSize) ||
        (usableSize == g_clientConfig.secondLargestSize) ||
        (usableSize == g_clientConfig.maxGrowthSize)) {
        return false;
    }

    return true;
}

bool ShouldSample(size_t size, pthread_key_t& sampleKey)
{
    if (sampleKey == 10000) { // 10000 : invalid key value
        return false;
    }
    auto* tlsSample = static_cast<Sampling*>(pthread_getspecific(sampleKey));
    if (tlsSample == nullptr) {
        tlsSample = new (std::nothrow) Sampling();
        if (tlsSample == nullptr) {
            return false;
        }
        (void)pthread_setspecific(sampleKey, tlsSample);
    }
    if (g_clientConfig.sampleInterval > MIN_SAMPLER_INTERVAL) {
        tlsSample->InitSampling(g_clientConfig.sampleInterval);
        return tlsSample->StartSampling(size) == 0;
    }
    return false;
}

int CalculateRealSize(int fpStackDepth, bool isAsyncStack)
{
    if (!g_clientConfig.fpunwind) {
        return sizeof(BaseStackRawData) + sizeof(StackRawData::regs);
    }
    if (isAsyncStack) {
        return sizeof(AsyncStackData);
    }
    return sizeof(BaseStackRawData) + (fpStackDepth * sizeof(uint64_t));
}

uint16_t GetPredefinedTagId(unsigned long long mask)
{
    static std::unordered_map<unsigned long long, uint16_t> g_maskTagMap = {
        {RES_GPU_VK,            static_cast<uint16_t>(GPU_VK_INDEX + 1)},
        {RES_GPU_GLES_IMAGE,    static_cast<uint16_t>(GPU_GLES_IMAGE_INDEX + 1)},
        {RES_GPU_GLES_BUFFER,   static_cast<uint16_t>(GPU_GLES_BUFFER_INDEX + 1)},
        {RES_GPU_CL_IMAGE,      static_cast<uint16_t>(GPU_CL_IMAGE_INDEX + 1)},
        {RES_GPU_CL_BUFFER,     static_cast<uint16_t>(GPU_CL_BUFFER_INDEX + 1)},
        {RES_FD_OPEN,           static_cast<uint16_t>(FD_OPEN_INDEX + 1)},
        {RES_FD_EPOLL,          static_cast<uint16_t>(FD_EPOLL_INDEX + 1)},
        {RES_FD_EVENTFD,        static_cast<uint16_t>(FD_EVENTFD_INDEX + 1)},
        {RES_FD_SOCKET,         static_cast<uint16_t>(FD_SOCKET_INDEX + 1)},
        {RES_FD_PIPE,           static_cast<uint16_t>(FD_PIPE_INDEX + 1)},
        {RES_FD_DUP,            static_cast<uint16_t>(FD_DUP_INDEX + 1)},
        {RES_FD_MASK,           static_cast<uint16_t>(FD_MASK_INDEX + 1)},
        {RES_THREAD_PTHREAD,    static_cast<uint16_t>(THREAD_PTHREAD_INDEX + 1)},
        {RES_THREAD_MASK,       static_cast<uint16_t>(THREAD_MASK_INDEX + 1)},
        {RES_JS_HEAP_MASK,      static_cast<uint16_t>(JS_HEAP_MASK_INDEX + 1)},
        {RES_ARKTS_HEAP_MASK,   static_cast<uint16_t>(ARKTS_HEAP_MASK_INDEX + 1)},
        {RES_KMP_HEAP_MASK,     static_cast<uint16_t>(KMP_HEAP_MASK_INDEX + 1)},
        {RES_RN_HEAP_MASK,      static_cast<uint16_t>(RN_HEAP_MASK_INDEX + 1)},
        {RES_DMABUF_MASK,       static_cast<uint16_t>(DMABUF_MASK_INDEX + 1)},
        {RES_ARK_GLOBAL_HANDLE, static_cast<uint16_t>(ARK_GLOBAL_HANDLE_INDEX + 1)}
    };
    auto it = g_maskTagMap.find(mask);
    return it != g_maskTagMap.end() ? it->second : 0;
}

bool IsInDualRange(size_t size, const uint64_t* range)
{
    if (range == nullptr) {
        return false;
    }
    const uint64_t u64Size = static_cast<uint64_t>(size);
    return (u64Size >= range[0] && u64Size <= range[1]) ||
           (u64Size >= range[GPU_RANGE_COUNT] && u64Size <= range[GPU_RANGE_COUNT + 1]);
}

bool CheckSizeRange(size_t size, uint64_t type)
{
    uint64_t* targetRange = nullptr;
    switch (type) {
        case RES_GPU_VK:
            targetRange = g_clientConfig.gpuRange.gpuVk;
            break;
        case RES_GPU_GLES_IMAGE:
            targetRange = g_clientConfig.gpuRange.gpuGlesImage;
            break;
        case RES_GPU_GLES_BUFFER:
            targetRange = g_clientConfig.gpuRange.gpuGlesBuffer;
            break;
        case RES_GPU_CL_IMAGE:
            targetRange = g_clientConfig.gpuRange.gpuClImage;
            break;
        case RES_GPU_CL_BUFFER:
            targetRange = g_clientConfig.gpuRange.gpuClBuffer;
            break;
        default:
            break;
    }
    return IsInDualRange(size, targetRange);
}

bool CheckRestraceConditions(unsigned long long combineVal, size_t size)
{
    static unsigned long long checkRangeType =
        RES_GPU_VK | RES_GPU_GLES_IMAGE | RES_GPU_GLES_BUFFER | RES_GPU_CL_IMAGE | RES_GPU_CL_BUFFER;
    return ((checkRangeType & combineVal) == 0) ? true : CheckSizeRange(size, combineVal);
}

}  // namespace HookGuard
