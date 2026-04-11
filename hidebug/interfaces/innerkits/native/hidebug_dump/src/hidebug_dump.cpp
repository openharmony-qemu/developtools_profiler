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

#include "hidebug_dump.h"

#include <cstring>
#include <shared_mutex>
#include <unordered_map>

#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Dump"

namespace {
using ListenerMap = std::unordered_map<std::string, OH_HiDebug_MemDumpListener>;

ListenerMap& GetListenerMap()
{
    static ListenerMap listenerMap;
    return listenerMap;
}

std::shared_mutex& GetListenerMutex()
{
    static std::shared_mutex listenerMutex;
    return listenerMutex;
}
} // namespace

namespace OHOS {
namespace HiviewDFX {
HidebugMemDumpListener::HidebugMemDumpListener() = default;

HidebugMemDumpListener::~HidebugMemDumpListener() = default;

HiDebug_ErrorCode HidebugMemDumpListener::RegisterMemDumpListener(
    const std::string& name, OH_HiDebug_MemDumpListener listener)
{
    if (name.empty() || listener == nullptr) {
        return HiDebug_ErrorCode::HIDEBUG_INVALID_ARGUMENT;
    }
    std::unique_lock<std::shared_mutex> lock(GetListenerMutex());
    auto& listenerMap = GetListenerMap();
    auto [it, inserted] = listenerMap.emplace(name, listener);
    if (!inserted) {
        HILOG_ERROR(LOG_CORE, "MemDumpListener already exists, name: %{public}s", name.c_str());
        return HiDebug_ErrorCode::HIDEBUG_INVALID_ARGUMENT;
    }

    HILOG_INFO(LOG_CORE, "MemDumpListener registered, name: %{public}s", name.c_str());
    return HiDebug_ErrorCode::HIDEBUG_SUCCESS;
}

OH_HiDebug_MemDumpListener HidebugMemDumpListener::FindListener(const std::string& name)
{
    std::shared_lock<std::shared_mutex> lock(GetListenerMutex());
    auto& listenerMap = GetListenerMap();
    auto it = listenerMap.find(name);
    if (it != listenerMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool HidebugMemDumpListener::TriggerListener(
    const std::string& name, int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg)
{
    if (name.empty()) {
        HILOG_ERROR(LOG_CORE, "TriggerListener failed, Listener name is empty");
        return false;
    }
    if (fd < 0) {
        HILOG_ERROR(LOG_CORE, "TriggerListener failed, Invalid file descriptor");
        return false;
    }
    if (arg == nullptr || strlen(arg) == 0) {
        HILOG_ERROR(LOG_CORE, "TriggerListener failed, Argument is null or empty");
        return false;
    }

    OH_HiDebug_MemDumpListener listener = FindListener(name);
    if (listener == nullptr) {
        HILOG_ERROR(LOG_CORE, "TriggerListener failed, MemDumpListener not found, name: %{public}s", name.c_str());
        return false;
    }

    HILOG_INFO(LOG_CORE, "MemDumpListener triggered, name: %{public}s", name.c_str());
    return listener(fd, tag, arg);
}
} // namespace HiviewDFX
} // namespace OHOS
