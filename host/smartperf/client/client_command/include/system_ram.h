/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef SYSTEM_RAM_H
#define SYSTEM_RAM_H
#include "sp_profiler.h"
namespace OHOS {
namespace SmartPerf {
class SystemRam : public SpProfiler {
public:
    std::map<std::string, std::string> GetSysRamInfo() const;
    static SystemRam &GetInstance()
    {
        static SystemRam instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;

private:
    SystemRam() {};
    SystemRam(const SystemRam &);
    SystemRam &operator = (const SystemRam &);
};
}
}
#endif
