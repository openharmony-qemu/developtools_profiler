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

#ifndef I_NATIVE_MEMORY_PROFILER_SA_CALLBACK_H
#define I_NATIVE_MEMORY_PROFILER_SA_CALLBACK_H

#include <string>

#include "iremote_broker.h"

namespace OHOS::Developtools::NativeDaemon {
class INativeMemoryProfilerSaCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NativeMemory.INativeMemoryProfilerSaCallback");

    virtual void OnProfilingResult(int32_t pid, int32_t errorCode, const std::string& filePath) = 0;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // I_NATIVE_MEMORY_PROFILER_SA_CALLBACK_H
