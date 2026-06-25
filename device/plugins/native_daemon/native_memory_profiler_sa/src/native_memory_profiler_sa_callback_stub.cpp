/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "native_memory_profiler_sa_callback_stub.h"

#include "string_ex.h"
#include "logging.h"
#include "define_macro.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr uint32_t ON_PROFILING_RESULT = 0;
}

int32_t NativeMemoryProfilerSaCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != GetDescriptor()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Invalid descriptor");
        return ERR_INVALID_STATE;
    }
    if (code != ON_PROFILING_RESULT) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    int32_t pid = 0;
    int32_t errorCode = 0;
    std::string filePath;
    if (!data.ReadInt32(pid) || !data.ReadInt32(errorCode) || !data.ReadString(filePath)) {
        PROFILER_LOG_ERROR(LOG_CORE, "read data FAIL");
        return RET_ERR;
    }
    OnProfilingResult(pid, errorCode, filePath);
    return ERR_NONE;
}
} // namespace OHOS::Developtools::NativeDaemon
