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

#ifndef NETWORK_H
#define NETWORK_H
#include "sp_profiler.h"
namespace OHOS {
namespace SmartPerf {
class Network : public SpProfiler {
public:
    std::map<std::string, std::string> ItemData() override;
    static Network &GetInstance()
    {
        static Network instance;
        return instance;
    }
    std::map<std::string, std::string> GetNetworkInfo();

private:
    Network() {};
    Network(const Network &);
    Network &operator = (const Network &);
    long long allTx = 0;
    long long allRx = 0;
    long long curTx = 0;
    long long curRx = 0;
    long long diffTx = 0;
    long long diffRx = 0;
    long long prevTx = 0;
    long long prevRx = 0;
    bool isFirst = true;
};
}
}
#endif // NETWORK_H
