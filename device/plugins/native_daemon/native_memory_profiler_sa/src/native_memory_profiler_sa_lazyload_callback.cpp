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
#include "native_memory_profiler_sa_lazyload_callback.h"
namespace OHOS::Developtools::NativeDaemon {
NativeMemoryProfilerSaLazyLoadCallback::NativeMemoryProfilerSaLazyLoadCallback()
{
    isRemoteObject_ = false;
    remoteObject_ = nullptr;
}

NativeMemoryProfilerSaLazyLoadCallback::~NativeMemoryProfilerSaLazyLoadCallback()
{
    remoteObject_ = nullptr;
}

void NativeMemoryProfilerSaLazyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        isRemoteObject_ =  (remoteObject != nullptr);
        remoteObject_ = remoteObject;
    }
}

void NativeMemoryProfilerSaLazyLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        isRemoteObject_ = false;
        remoteObject_ = nullptr;
    }
}

bool NativeMemoryProfilerSaLazyLoadCallback::CheckLoadSystemAbilityStatus()
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    return isRemoteObject_;
}

OHOS::sptr<OHOS::IRemoteObject> NativeMemoryProfilerSaLazyLoadCallback::GetRemoteObject()
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    return remoteObject_;
}

void NativeMemoryProfilerSaLazyLoadCallback::ResetStatus()
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    isRemoteObject_ = false;
    remoteObject_ = nullptr;
}

} // namespace OHOS::Developtools::NativeDaemon
