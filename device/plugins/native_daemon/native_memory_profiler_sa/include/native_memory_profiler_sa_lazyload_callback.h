/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NATIVE_MEMORY_PROFILER_SA_LAZYLOAD__CALLBACK_H
#define NATIVE_MEMORY_PROFILER_SA_LAZYLOAD__CALLBACK_H
#include "system_ability_load_callback_stub.h"
namespace OHOS::Developtools::NativeDaemon {

class NativeMemoryProfilerSaLazyLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    NativeMemoryProfilerSaLazyLoadCallback();
    ~NativeMemoryProfilerSaLazyLoadCallback();
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    bool CheckLoadSystemAbilityStatus();
    OHOS::sptr<IRemoteObject> GetRemoteObject();
    void ResetStatus();
private:
    bool isRemoteObject_{false};
    OHOS::sptr<IRemoteObject> remoteObject_{nullptr};
    std::mutex callbackMutex_;
};
} // namespace OHOS::Developtools::NativeDaemon
#endif // NATIVE_MEMORY_PROFILER_SA_LAZYLOAD__CALLBACK_H
