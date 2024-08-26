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

#ifndef HIDEBUG_FFI_H
#define HIDEBUG_FFI_H

#include <cstdint>
#include "cj_ffi/cj_common_ffi.h"

extern "C" {
    struct CSystemMemInfo {
        uint64_t totalMem;
        uint64_t freeMem;
        uint64_t availableMem;
    };

    struct CThreadCpuUsage {
        uint32_t threadId;
        double cpuUsage;
    };

    struct ThreadCpuUsageArr {
        CThreadCpuUsage *head;
        int64_t size;
    };

    struct CNativeMemInfo {
        uint64_t pss;
        uint64_t vss;
        uint64_t rss;
        uint64_t sharedDirty;
        uint64_t privateDirty;
        uint64_t sharedClean;
        uint64_t privateClean;
    };

    struct CMemoryLimit {
        uint64_t rssLimit;
        uint64_t vssLimit;
    };

    FFI_EXPORT uint64_t FfiHidebugGetPss();
    FFI_EXPORT uint64_t FfiHidebugGetVss();
    FFI_EXPORT uint64_t FfiHidebugGetNativeHeapSize();
    FFI_EXPORT uint64_t FfiHidebugGetNativeHeapAllocatedSize();
    FFI_EXPORT uint64_t FfiHidebugGetNativeHeapFreeSize();
    FFI_EXPORT uint64_t FfiHidebugGetSharedDirty();
    FFI_EXPORT uint64_t FfiHidebugGetPrivateDirty();
    FFI_EXPORT double FfiHidebugGetCpuUsage();
    FFI_EXPORT double FfiHidebugGetSystemCpuUsage(int32_t &code);
    FFI_EXPORT ThreadCpuUsageArr FfiHidebugGetAppThreadCpuUsage(int32_t &code);
    FFI_EXPORT CSystemMemInfo FfiHidebugGetSystemMemInfo(int32_t &code);
    FFI_EXPORT CNativeMemInfo FfiHidebugGetAppNativeMemInfo(int32_t &code);
    FFI_EXPORT CMemoryLimit FfiHidebugGetAppMemoryLimit(int32_t &code);
    FFI_EXPORT int32_t FfiHidebugGetServiceDump(int32_t serviceId, int32_t fd, CArrString args);
    FFI_EXPORT char *FfiHidebugStartAppTraceCapture(CArrUnit tags, int32_t flag, uint32_t limitSize, int32_t &code);
    FFI_EXPORT int32_t FfiHidebugStopAppTraceCapture();
    FFI_EXPORT int32_t FfiHidebugSetAppResourceLimit(const char *type, int32_t value, bool enableDebugLog);
    FFI_EXPORT bool FfiHidebugIsDebugState();
}

#endif // HIDEBUG_FFI_H
