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

#ifndef NATIVE_MEMORY_PROFILER_SA_DEATH_RECIPIENT_H
#define NATIVE_MEMORY_PROFILER_SA_DEATH_RECIPIENT_H

#include "iremote_broker.h"
#include "nocopyable.h"

namespace OHOS::Developtools::NativeDaemon {
class NativeMemoryProfilerSaDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit NativeMemoryProfilerSaDeathRecipient();
    DISALLOW_COPY_AND_MOVE(NativeMemoryProfilerSaDeathRecipient);
    ~NativeMemoryProfilerSaDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // NATIVE_MEMORY_PROFILER_SA_DEATH_RECIPIENT_H