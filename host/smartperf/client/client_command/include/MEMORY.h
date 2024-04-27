/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#ifndef MEMORY_H
#define MEMORY_H
#include "sp_profiler.h"
#include <string>

enum {
    MEMORY_ONE = 1,
    MEMORY_SECOND,
    MEMORY_THIRD,
    MEMORY_FOURTH,
    MEMORY_FIFTH,
    MEMORY_SIXTH,
    MEMORY_SEVENTH,
    MEMORY_EIGHTH,
    MEMORY_NINTH,
    MEMORY_TENTH,
};

namespace OHOS {
namespace SmartPerf {
class MEMORY : public SpProfiler {
public:
    void SetProcessId(std::string pid);
    std::map<std::string, std::string> GetRamInfo() const;
    std::map<std::string, std::string> GetPssRamInfo(FILE *fd, std::vector<std::string> paramsInfo) const;
    std::map<std::string, std::string> SaveSumRamInfo(std::vector<std::string> paramsInfo) const;
    static MEMORY &GetInstance()
    {
        static MEMORY instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
    void SetPackageName(const std::string &pName);

private:
    MEMORY() {};
    MEMORY(const MEMORY &);
    MEMORY &operator = (const MEMORY &);
    std::string packageName = "";
};
}
}
#endif
