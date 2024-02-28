/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_HIDEBUG_NATIVE_INTERFACE_H
#define HIVIEWDFX_HIDEBUG_NATIVE_INTERFACE_H

#include <map>
#include <memory>
#include "resource/memory.h"

namespace OHOS {
namespace HiviewDFX {
class HidebugNativeInterface {
public:
    static std::unique_ptr<HidebugNativeInterface> CreateInstance();
    virtual ~HidebugNativeInterface() = default;

    virtual double GetCpuUsage() = 0;
    virtual std::map<uint32_t, double> GetAppThreadCpuUsage() = 0;
    virtual std::string StartAppTraceCapture(uint64_t tags, uint32_t flag, uint32_t limitsize) = 0;
    virtual void StopAppTraceCapture() = 0;
    virtual std::optional<MemoryLimit> GetAppMemoryLimit() = 0;
    virtual std::optional<ProcessMemory> GetAppNativeMemInfo() = 0;
    virtual std::optional<SysMemory> GetSystemMemInfo() = 0;
};
}
}

#endif  // HIVIEWDFX_HIDEBUG_NATIVE_INTERFACE_H