/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef GETLOG_H
#define GETLOG_H
#include "sp_profiler.h"
namespace OHOS {
namespace SmartPerf {
class GetLog : public SpProfiler {
public:
    std::map<std::string, std::string> ItemData() override;
    static GetLog &GetInstance()
    {
        static GetLog instance;
        return instance;
    }

private:
    GetLog() {};
    GetLog(const GetLog &);
    GetLog &operator = (const GetLog &);
};
}
}
#endif // NETWORK_H
