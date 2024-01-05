/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "native_memory_profiler_sa_stub.h"

#include "string_ex.h"
#include "logging.h"

#include <unistd.h>

namespace OHOS::Developtools::NativeDaemon {
int32_t NativeMemoryProfilerSaStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& options)
{
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != INativeMemoryProfilerSa::GetDescriptor()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Get unexpect descriptor:%s", Str16ToStr8(descriptor).c_str());
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::START) : {
            return StubStart(data, reply);
        }
        case static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID) : {
            return StubStopPid(data, reply);
        }
        case static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_NAME) : {
            return StubStopName(data, reply);
        }
        case static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA) : {
            return StubDumpFile(data, reply);
        }
        default : {
            PROFILER_LOG_ERROR(LOG_CORE, "Unknown code:%u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, options);
        }
    }
}

int32_t NativeMemoryProfilerSaStub::StubStart(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    if (!NativeMemoryProfilerSaConfig::Unmarshalling(data, config)) {
        return RET_ERR;
    }
    return Start(config);
}

int32_t NativeMemoryProfilerSaStub::StubStopPid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t pid = 0;
    READUINT32(data, pid, RET_ERR);
    return Stop(pid);
}

int32_t NativeMemoryProfilerSaStub::StubStopName(MessageParcel &data, MessageParcel &reply)
{
    std::string processName;
    READSTRING(data, processName, RET_ERR);
    return Stop(processName);
}

int32_t NativeMemoryProfilerSaStub::StubDumpFile(MessageParcel &data, MessageParcel &reply)
{
    uint32_t fd = static_cast<uint32_t>(data.ReadFileDescriptor());
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    if (!NativeMemoryProfilerSaConfig::Unmarshalling(data, config)) {
        close(fd);
        return RET_ERR;
    }
    return DumpData(fd, config);
}
} // namespace OHOS::Developtools::NativeDaemon