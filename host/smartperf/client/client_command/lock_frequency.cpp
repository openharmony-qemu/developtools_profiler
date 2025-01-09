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

#include "include/lock_frequency.h"
#include <thread>
#include <chrono>
#include <dlfcn.h>
#include "include/sp_log.h"

namespace OHOS {
namespace SmartPerf {
    std::map<std::string, std::string> LockFrequency::ItemData()
    {
        return std::map<std::string, std::string>();
    }
    void LockFrequency::LockingThread()
    {
        LOGD("Lock frequency thread create");
        const int loopLockTime = 4000;
        char soFilePathChar[PATH_MAX] = {0x00};
        if ((realpath(pluginSoPath.c_str(), soFilePathChar) == nullptr)) {
            LOGE("%s is not exist.", pluginSoPath.c_str());
            return;
        }
        void* handle = dlopen(soFilePathChar, RTLD_LAZY);
        if (!handle) {
            LOGE("open TestServerPlugin so file error.");
            return;
        }
        typedef int32_t (*GetLockFreq)();
        GetLockFreq testServerPlugin = (GetLockFreq)dlsym(handle, lockFunction.c_str());
        if (!testServerPlugin) {
            LOGE("testServerPlugin Error loading symbol");
            return;
        }
        while (isCollecting) {
            testServerPlugin();
            std::this_thread::sleep_for(std::chrono::milliseconds(loopLockTime));
        }

        LOGD("Lock frequency thread end");
    }

    void LockFrequency::SetIsCollecting(bool state)
    {
        isCollecting = state;
    }
}
}
