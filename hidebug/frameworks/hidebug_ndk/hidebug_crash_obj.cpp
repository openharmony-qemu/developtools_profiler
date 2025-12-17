/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hidebug/hidebug.h"

#include "dfx_signal_handler.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Crash_Obj"

uint64_t OH_HiDebug_SetCrashObj(HiDebug_CrashObjType type, void* addr)
{
    if (addr == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to set crashObj, addr is nullptr");
        return 0;
    }

    if (type < HIDEBUG_CRASHOBJ_STRING || type > HIDEBUG_CRASHOBJ_MEMORY_4096B) {
        HILOG_ERROR(LOG_CORE, "failed to set crashObj, invalid type: %d", static_cast<int>(type));
        return 0;
    }

    if (sizeof(uintptr_t) != sizeof(uint64_t)) {
        HILOG_ERROR(LOG_CORE, "failed to set crashObj on non 64-bit platform");
        return 0;
    }

    uintptr_t crashObj = DFX_SetCrashObj(type, reinterpret_cast<uintptr_t>(addr));
    if (crashObj == 0) {
        HILOG_INFO(LOG_CORE, "set crashObj return 0");
    }
    return static_cast<uint64_t>(crashObj);
}

void OH_HiDebug_ResetCrashObj(uint64_t crashObj)
{
    if (crashObj == 0) {
        HILOG_INFO(LOG_CORE, "reset crashObj to 0");
    }

    if (sizeof(uintptr_t) != sizeof(uint64_t)) {
        HILOG_ERROR(LOG_CORE, "failed to reset crashObj on non 64-bit platform");
        return;
    }
    DFX_ResetCrashObj(static_cast<uintptr_t>(crashObj));
}
