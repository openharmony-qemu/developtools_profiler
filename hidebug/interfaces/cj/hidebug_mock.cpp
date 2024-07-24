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

#include "cj_ffi/cj_common_ffi.h"

extern "C" {
FFI_EXPORT int FfiHidebugGetPss = 0;
FFI_EXPORT int FfiHidebugGetVss = 0;
FFI_EXPORT int FfiHidebugGetNativeHeapSize = 0;
FFI_EXPORT int FfiHidebugGetNativeHeapAllocatedSize = 0;
FFI_EXPORT int FfiHidebugGetNativeHeapFreeSize = 0;
FFI_EXPORT int FfiHidebugGetSharedDirty = 0;
FFI_EXPORT int FfiHidebugGetPrivateDirty = 0;
FFI_EXPORT int FfiHidebugGetCpuUsage = 0;
FFI_EXPORT int FfiHidebugGetSystemCpuUsage = 0;
FFI_EXPORT int FfiHidebugGetAppThreadCpuUsage = 0;
FFI_EXPORT int FfiHidebugGetSystemMemInfo = 0;
FFI_EXPORT int FfiHidebugGetAppNativeMemInfo = 0;
FFI_EXPORT int FfiHidebugGetAppMemoryLimit = 0;
FFI_EXPORT int FfiHidebugGetServiceDump = 0;
FFI_EXPORT int FfiHidebugStartAppTraceCapture = 0;
FFI_EXPORT int FfiHidebugStopAppTraceCapture = 0;
FFI_EXPORT int FfiHidebugSetAppResourceLimit = 0;
FFI_EXPORT int FfiHidebugIsDebugState = 0;
}
