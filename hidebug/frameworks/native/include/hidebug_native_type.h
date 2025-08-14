/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_HIDEBUG_NATIVE_TYPE_H
#define HIVIEWDFX_HIDEBUG_NATIVE_TYPE_H
#include <cstdint>

namespace OHOS {
namespace HiviewDFX {

constexpr int NATIVE_SUCCESS = 0;
constexpr int NATIVE_FAIL = -1;

enum TraceErrorCode {
    /** Success */
    TRACE_SUCCESS = NATIVE_SUCCESS,
    /** Invalid argument */
    TRACE_INVALID_ARGUMENT = 401,
    /** Have already capture trace */
    TRACE_CAPTURED_ALREADY = 11400102,
    /** No write permission on the file */
    TRACE_NO_PERMISSION = 11400103,
    /** The status of the trace is abnormal */
    TRACE_ABNORMAL = 11400104,
    /** No trace running */
    NO_TRACE_RUNNING = 11400105,
};

struct NativeMemInfo {
    /**
     * Process proportional set size memory, in kibibytes
     */
    uint32_t pss = 0;
    /**
     * Virtual set size memory, in kibibytes
     */
    uint32_t vss = 0;
    /**
     * Resident set size, in kibibytes
     */
    uint32_t rss = 0;
    /**
     * The size of the shared dirty memory, in kibibytes
     */
    uint32_t sharedDirty = 0;
    /**
     * The size of the private dirty memory, in kibibytes
     */
    uint32_t privateDirty = 0;
    /**
     * The size of the shared clean memory, in kibibytes
     */
    uint32_t sharedClean = 0;
    /**
     * The size of the private clean memory, in kibibytes
     */
    uint32_t privateClean = 0;
};

struct MemoryLimitInfo {
    /**
     * The limit of the application process's resident set, in kibibytes
     */
    uint64_t rssLimit = 0;
    /**
     * The limit of the application process's virtual memory, in kibibytes
     */
    uint64_t vssLimit = 0;
};

struct SystemMemoryInfo {
    /**
     * Total system memory size, in kibibytes
     */
    uint32_t totalMem = 0;
    /**
     * System free memory size, in kibibytes
     */
    uint32_t freeMem = 0;
    /**
     * System available memory size, in kibibytes
     */
    uint32_t availableMem = 0;
};
}
}

#endif  // HIVIEWDFX_HIDEBUG_NATIVE_TYPE_H
