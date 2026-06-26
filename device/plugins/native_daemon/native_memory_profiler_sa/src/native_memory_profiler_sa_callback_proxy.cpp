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

#include "native_memory_profiler_sa_callback_proxy.h"

#include "message_parcel.h"
#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr uint32_t ON_PROFILING_RESULT = 0;
}

NativeMemoryProfilerSaCallbackProxy::NativeMemoryProfilerSaCallbackProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<INativeMemoryProfilerSaCallback>(impl) {}

void NativeMemoryProfilerSaCallbackProxy::OnProfilingResult(int32_t pid, int32_t errorCode, const std::string& filePath)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "write token FAIL");
        return;
    }
    if (!data.WriteInt32(pid) || !data.WriteInt32(errorCode) || !data.WriteString(filePath)) {
        PROFILER_LOG_ERROR(LOG_CORE, "write data FAIL");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "remote object is NULL");
        return;
    }
    CHECK_TRUE(remote->SendRequest(ON_PROFILING_RESULT, data, reply, option) == ERR_NONE, NO_RETVAL, "send FAIL");
}
} // namespace OHOS::Developtools::NativeDaemon
