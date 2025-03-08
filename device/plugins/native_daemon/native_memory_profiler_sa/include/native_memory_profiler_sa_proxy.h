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

#ifndef NATIVE_MEMORY_PROFILER_SA_PROXY_H
#define NATIVE_MEMORY_PROFILER_SA_PROXY_H

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "system_ability.h"

#include "i_native_memory_profiler_sa.h"
#include "native_memory_profiler_sa_interface_code.h"

namespace OHOS::Developtools::NativeDaemon {
class NativeMemoryProfilerSaProxy : public IRemoteProxy<INativeMemoryProfilerSa> {
public:
    explicit NativeMemoryProfilerSaProxy(const sptr<IRemoteObject> &impl);
    virtual ~NativeMemoryProfilerSaProxy() = default;

    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override;
    int32_t Stop(uint32_t pid) override;
    int32_t Stop(const std::string& name) override;
    int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override;
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, std::string& replyStats) override;
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, MessageParcel &reply) override { return 0; }
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // NATIVE_MEMORY_PROFILER_SA_PROXY_H