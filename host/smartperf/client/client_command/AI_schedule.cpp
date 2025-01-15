/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "AI_schedule.h"

#include <string>
#include <map>
#include <service_plugin.h>
#include <sp_log.h>
#include <dlfcn.h>

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> AISchedule::ItemData()
{
    aiScheduleParams[AI_SCHEDULE_PARAM] = "1";
     ServicePluginHandler &servicePluginHandler = ServicePluginHandler::GetInstance();

    void* serviceHandle = servicePluginHandler.GetSoHandler(servicePluginType::GAME_PLUGIN);
    if (serviceHandle == nullptr) {
        WLOGE("Get service plugin handler failed.");
        return std::map<std::string, std::string>();
    }

    void* handle = servicePluginHandler.GetSoHandler();
    if (!handle) {
        WLOGE("Get service plugin handler failed.");
        return;
    }

    typedef GpuCounterPlugin *(*GetServicePlugin)();
    GetServicePlugin servicePlugin = (GetServicePlugin)dlsym(handle, CREATE_PLUGIN.c_str());
    if (!servicePlugin) {
        WLOGE("GpuCounterPlugin Error loading symbol");
        return;
    }
    return servicePlugin()->GetSystemFunctionStatus(aiScheduleParams);
}
} // namespace SmartPerf
} // namespace OHOS