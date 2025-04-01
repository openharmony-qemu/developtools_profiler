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

#ifndef I_NATIVE_MEMORY_PROFILER_SA_H
#define I_NATIVE_MEMORY_PROFILER_SA_H

#include "iremote_broker.h"
#include "system_ability_definition.h"
#include "native_memory_profiler_sa_interface_code.h"
#include "define_macro.h"
#include "native_memory_profiler_sa_config.h"

namespace OHOS::Developtools::NativeDaemon {
class INativeMemoryProfilerSa : public IRemoteBroker {
public:
    static constexpr int32_t NATIVE_DAEMON_SYSTEM_ABILITY_ID = DFX_SYS_NATIVE_MEMORY_PROFILER_SERVICE_ABILITY_ID;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NativeMemory.INativeMemoryProfilerSa");

    virtual int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) = 0;
    virtual int32_t Stop(uint32_t pid) = 0;
    virtual int32_t Stop(const std::string& name) = 0;
    virtual int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) = 0;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // I_NATIVE_MEMORY_PROFILER_SA_H