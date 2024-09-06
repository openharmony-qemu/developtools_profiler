/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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


#include "native_memory_profiler_sa_proxy.h"

#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {
NativeMemoryProfilerSaProxy::NativeMemoryProfilerSaProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INativeMemoryProfilerSa>(impl) {}

int32_t NativeMemoryProfilerSaProxy::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed to write descriptor");
        return RET_ERR;
    }
    CHECK_TRUE(config->Marshalling(data), RET_ERR, "NativeMemoryProfilerSaConfig marshalling failed");

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::START),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed to write descriptor");
        return RET_ERR;
    }

    CHECK_TRUE(config->Marshalling(data), RET_ERR, "NativeMemoryProfilerSaConfig marshalling failed");
    if (!data.WriteFileDescriptor(fd)) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed to write fd");
        return RET_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::Stop(uint32_t pid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed to write descriptor");
        return RET_ERR;
    }
    WRITEUINT32(data, pid, RET_ERR);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::Stop(const std::string& name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed to write descriptor");
        return RET_ERR;
    }
    WRITESTRING(data, name, RET_ERR);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_NAME),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed");
        return ret;
    }
    return RET_OK;
}
} // namespace OHOS::Developtools::NativeDaemon