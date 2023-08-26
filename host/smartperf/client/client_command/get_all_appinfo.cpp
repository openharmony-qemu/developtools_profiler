/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "include/get_all_appinfo.h"
#include "launcher_service.h"
#include "bundle_mgr_proxy.h"
namespace OHOS {
    namespace SmartPerf {
        void AllAppInfo::getAllAppInfo()
        {
            const int USE_ID = 100;
            std::vector<std::string> validBundleList_;
            std::vector<std::string> validAbilityList_;
            AppExecFwk::LauncherService launcherService;
            std::vector<AppExecFwk::LauncherAbilityInfo> launcherAbilityInfos(0);
            launcherService.GetAllLauncherAbilityInfos(USE_ID, launcherAbilityInfos);
            if (launcherAbilityInfos.size() == 0) {
                std::cout<< "launcherAbilityInfos size is 0" <<std::endl;
                return;
            }
            for (auto item : launcherAbilityInfos) {
                std::string iconPath_ = item.applicationInfo.iconPath;
                std::string bundleName = item.elementName.GetBundleName();
                std::cout << "bundleName: " << bundleName << std::endl;
                std::cout << "AbilityName: " << item.elementName.GetAbilityName() << std::endl;
            }
        }
    }
}