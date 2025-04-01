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

#ifndef NATIVE_MEMORY_PROFILER_SA_STUB_H
#define NATIVE_MEMORY_PROFILER_SA_STUB_H

#include "iremote_stub.h"
#include "message_parcel.h"
#include "nocopyable.h"

#include "i_native_memory_profiler_sa.h"
#include "native_memory_profiler_sa_interface_code.h"

namespace OHOS::Developtools::NativeDaemon {
class NativeMemoryProfilerSaStub : public IRemoteStub<INativeMemoryProfilerSa> {
public:
    NativeMemoryProfilerSaStub() = default;
    DISALLOW_COPY_AND_MOVE(NativeMemoryProfilerSaStub);
    virtual ~NativeMemoryProfilerSaStub() = default;

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& options) override;

protected:
    int32_t StubStart(MessageParcel &data, MessageParcel &reply);
    int32_t StubStopPid(MessageParcel &data, MessageParcel &reply);
    int32_t StubStopName(MessageParcel &data, MessageParcel &reply);
    int32_t StubDumpFile(MessageParcel &data, MessageParcel &reply);
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // NATIVE_MEMORY_PROFILER_SA_STUB_H