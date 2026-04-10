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

#ifndef HIDEBUG_DUMP
#define HIDEBUG_DUMP

#include <string>

#include "refbase.h"
#include "singleton.h"

#include "hidebug/hidebug.h"

namespace OHOS {
namespace HiviewDFX {
class HidebugMemDumpListener : public DelayedRefSingleton<HidebugMemDumpListener> {
    DECLARE_DELAYED_REF_SINGLETON(HidebugMemDumpListener);
public:
    HiDebug_ErrorCode RegisterMemDumpListener(const std::string& name, OH_HiDebug_MemDumpListener listener);

    OH_HiDebug_MemDumpListener FindListener(const std::string& name);

    bool TriggerListener(const std::string& name, int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDEBUG_DUMP
