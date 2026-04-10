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

#include "include/VisualEffectDispatch.h"
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
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(type);
    for (auto &item : params) {
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData()[item.first] = item.second;
    }
    if (VisualEffectDispatch::GetInstance().GetSpeciFlag()) {
        return 1;
    } else if (VisualEffectDispatch::GetInstance().GetCatchTraceFlag()) {
        return 2;
    }
    return 0;
}

std::map<std::string, std::string> VisualEffectDispatch::ItemData()
{
    LOGI("VisualEffectDispatch:ItemData map size(%u)", visualEffectDispatchItemData.size());
    return visualEffectDispatchItemData;
}

std::map<std::string, std::string> &VisualEffectDispatch::GetVisualEffectDispatchItemData()
{
    return visualEffectDispatchItemData;
}

void VisualEffectDispatch::SetVisualEffectDispatchType(int32_t type)
{
    LOGI("yanzhe set type = %d", type);
    type_ = type;
}

void VisualEffectDispatch::SetCollectNum()
{
    if (type_ == 2) {
        LOGI("yanzhe type = 2");
        int collectNum = SPUtilesTye::StringToSometype<int>(visualEffectDispatchItemData["maxCollectNum"]);
        if (collectNum <= 1 || collectNum >= 5) {
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

void VisualEffectDispatch::StartExecutionOnce(bool isPause)
{
    if (isPause) {
        return;
    }
    RegisterGameScheduleEvent();
}

void VisualEffectDispatch::FinishtExecutionOnce(bool isPause)
{
    if (isPause) {
        return;
    }
    UnregisterGameEvent();
}

int VisualEffectDispatch::RegisterGameScheduleEvent()
{
    if (isRegister) {
        WLOGI("VisualEffectDispatch::RegisterGameScheduleEvent, already register");
        return 0;
    }
    WLOGI("VisualEffectDispatch::RegisterGameScheduleEvent");
    std::unique_ptr<GameScheduleEventCallback> visualEffectDispatchCallback =
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

    int32_t ret = servicePlugin()->RegisterGameScheduleEvent(std::move(visualEffectDispatchCallback));
    if (ret == 0) {
        isRegister = true;
        WLOGI("VisualEffectDispatch::RegisterGameScheduleEvent success");
    } else {
        WLOGE("VisualEffectDispatch::RegisterGameScheduleEvent failed");
    }
    return ret;
}

int VisualEffectDispatch::UnregisterGameEvent()
{
    if (!isRegister) {
        return 0;
    }
    WLOGI("VisualEffectDispatch::UnregisterGameEvent");
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
        WLOGI("VisualEffectDispatch::UnregisterGameEvent success");
    } else {
        WLOGE("VisualEffectDispatch::UnregisterGameEvent failed");
    }
    return ret;
}

void VisualEffectDispatch::SetSpeciFlag()
{
    speciFlag_ = true;
}

void VisualEffectDispatch::SetCatchTraceFlag()
{
    catchTraceFlag_ = true;
}

bool VisualEffectDispatch::GetSpeciFlag()
{
    return speciFlag_;
}

bool VisualEffectDispatch::GetCatchTraceFlag()
{
    return catchTraceFlag_;
}
} // namespace SmartPerf
} // namespace OHOS