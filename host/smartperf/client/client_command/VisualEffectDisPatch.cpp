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

#include "include/VisualEffectDisPatch.h"
#include "interface/GameSchedulePlugin.h"

#include <string>
#include <map>
#include "include/service_plugin.h"
#include <sp_log.h>
#include <dlfcn.h>
#include <memory>
#include "include/sp_utils.h"

namespace OHOS {
namespace SmartPerf {
int32_t GameScheduleEventCallbackImpl::OnGameScheduleEvent(int32_t type, std::map<std::string, std::string> &params)
{
    LOGI("yanzhe recv type = %d", type);
    VisualEffectDisPatch::GetInstance().SetVisualEffectDisPatchType(type);
    for (auto &item : params) {
        VisualEffectDisPatch::GetInstance().GetVisualEffectDisPatchItemData()[item.first] = item.second;
    }
    if (VisualEffectDisPatch::GetInstance().GetSpeciFlag()) {
        return 1;
    } else if (VisualEffectDisPatch::GetInstance().GetCatchTraceFlag()) {
        return captureFileFlag_;
    }
    return 0;
}

std::map<std::string, std::string> VisualEffectDisPatch::ItemData()
{
    LOGI("VisualEffectDisPatch:ItemData map size(%u)", VisualEffectDisPatchItemData.size());
    return VisualEffectDisPatchItemData;
}

std::map<std::string, std::string> &VisualEffectDisPatch::GetVisualEffectDisPatchItemData()
{
    return visualEffectDisPatchItemData;
}

void VisualEffectDisPatch::SetVisualEffectDisPatchType(int32_t type)
{
    LOGI("yanzhe set type = %d", type);
    type_ = type;
}

void VisualEffectDisPatch::SetCollectNum()
{
    if (type_ == captureFileFlag_) {
        LOGI("yanzhe type = 2");
        int collectNum = SPUtilesTye::StringToSometype<int>(visualEffectDisPatchItemData["maxCollectNum"]);
        if (collectNum <= 1 || collectNum >= static_cast<int>(maxFileFlag_)) {
            collectNum = 1;
        }
        std::string sendMessage = "haveSchedule::" + std::to_string(collectNum);
        LOGI("yanzhe send hap sendMessage = %s", sendMessage.c_str());
        ipcCallback_(sendMessage);
    } else if (type_ == 0) {
        LOGI("yanzhe type = 0");
        ipcCallback_("haveSchedule");
    }
}

void VisualEffectDisPatch::StartExecutionOnce(bool isPause)
{
    if (isPause) {
        return;
    }
    RegisterGameScheduleEvent();
}

void VisualEffectDisPatch::FinishtExecutionOnce(bool isPause)
{
    if (isPause) {
        return;
    }
    UnregisterGameEvent();
}

int VisualEffectDisPatch::RegisterGameScheduleEvent()
{
    if (isRegister) {
        WLOGI("VisualEffectDisPatch::RegisterGameScheduleEvent, already register");
        return 0;
    }
    WLOGI("VisualEffectDisPatch::RegisterGameScheduleEvent");
    std::unique_ptr<GameScheduleEventCallback> visualEffectDisPatchCallback =
        std::make_unique<GameScheduleEventCallbackImpl>();
    ServicePluginHandler &servicePluginHandler = ServicePluginHandler::GetInstance();
    void* handle = servicePluginHandler.GetSoHandler(ServicePluginHandler::ServicePluginType::SCHEDULE_PLUGIN);
    if (!handle) {
        WLOGE("Get service plugin handler failed.");
        return -1;
    }
    typedef GameSchedulePlugin *(*GetSchedulePlugin)();
    GetSchedulePlugin servicePlugin = (GetSchedulePlugin)dlsym(handle, createPlugin.c_str());
    if (!servicePlugin) {
        WLOGE("GameServicePlugin Error loading symbol");
        return -1;
    }

    int32_t ret = servicePlugin()->RegisterGameScheduleEvent(std::move(visualEffectDisPatchCallback));
    if (ret == 0) {
        isRegister = true;
        WLOGI("VisualEffectDisPatch::RegisterGameScheduleEvent success");
    } else {
        WLOGE("VisualEffectDisPatch::RegisterGameScheduleEvent failed");
    }
    return ret;
}

int VisualEffectDisPatch::UnregisterGameEvent()
{
    if (!isRegister) {
        return 0;
    }
    WLOGI("VisualEffectDisPatch::UnregisterGameEvent");
    ServicePluginHandler &servicePluginHandler = ServicePluginHandler::GetInstance();
    void* handle = servicePluginHandler.GetSoHandler(ServicePluginHandler::ServicePluginType::SCHEDULE_PLUGIN);
    if (!handle) {
        WLOGE("Get service plugin handler failed.");
        return -1;
    }
    typedef GameSchedulePlugin *(*GetSchedulePlugin)();
    GetSchedulePlugin servicePlugin = (GetSchedulePlugin)dlsym(handle, createPlugin.c_str());
    if (!servicePlugin) {
        WLOGE("GameServicePlugin Error loading symbol");
        return -1;
    }

    int32_t ret = servicePlugin()->UnregisterGameEvent();
    if (ret == 0) {
        isRegister = false;
        WLOGI("VisualEffectDisPatch::UnregisterGameEvent success");
    } else {
        WLOGE("VisualEffectDisPatch::UnregisterGameEvent failed");
    }
    return ret;
}

void VisualEffectDisPatch::SetSpeciFlag()
{
    speciFlag_ = true;
}

void VisualEffectDisPatch::SetCatchTraceFlag()
{
    catchTraceFlag_ = true;
}

bool VisualEffectDisPatch::GetSpeciFlag()
{
    return speciFlag_;
}

bool VisualEffectDisPatch::GetCatchTraceFlag()
{
    return catchTraceFlag_;
}
} // namespace SmartPerf
} // namespace OHOS