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
#ifndef LOCK_FREQUENCY_H
#define LOCK_FREQUENCY_H
#include "sp_profiler.h"
#include <string>
#include <map>
namespace OHOS {
namespace SmartPerf {
class LockFrequency : public SpProfiler {
public:
    static LockFrequency &GetInstance()
    {
        static LockFrequency instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
    void LockingThread();
    void SetIsCollecting(bool state);

private:
    LockFrequency() {};
    LockFrequency(const LockFrequency &);
    LockFrequency &operator = (const LockFrequency &);

    bool isCollecting = false;
    const std::string pluginSoPath = "system/lib64/libtest_server_client.z.so";
    const std::string lockFunction = "FrequencyLockPlugin";
};
}
}
#endif