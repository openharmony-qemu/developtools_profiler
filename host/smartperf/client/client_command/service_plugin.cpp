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
#include "string"
#include "map"
#include "thread"
#include "include/service_plugin.h"

namespace OHOS {
    namespace SmartPerf {
        std::map<std::string, std::string> ServicePluginHandler::ItemData()
        {
            return std::map<std::string, std::string>();
        }

        void ServicePluginHandler::ServicePluginInit()
        {
            char soFilePathChar[PATH_MAX] = {0x00};
            if ((realpath(PLUGIN_SO_PATH.c_str(), soFilePathChar) == nullptr)) {
                WLOGE("%s is not exist.", PLUGIN_SO_PATH.c_str());
            }

            pluginHandle = dlopen(PLUGIN_SO_PATH.c_str(), RTLD_LAZY);
            if (!pluginHandle) {
                WLOGE("open ServicePlugin so file error.");
            }
        }

        void* ServicePluginHandler::GetSoHandler()
        {
            return pluginHandle;
        }
    }
}

#endif