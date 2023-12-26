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

#include "native_memory_profiler_sa_death_recipient.h"

#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {
NativeMemoryProfilerSaDeathRecipient::NativeMemoryProfilerSaDeathRecipient() {}

void NativeMemoryProfilerSaDeathRecipient::OnRemoteDied(const OHOS::wptr<OHOS::IRemoteObject> &object)
{
    PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfiler SA has died");
}
} // namespace OHOS::Developtools::NativeDaemon