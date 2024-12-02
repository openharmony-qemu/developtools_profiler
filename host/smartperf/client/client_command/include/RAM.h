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
#ifndef RAM_H
#define RAM_H
#include "sp_profiler.h"
#include <string>

enum {
    RAM_ONE = 1,
    RAM_SECOND,
    RAM_THIRD,
    RAM_FOURTH,
    RAM_FIFTH,
    RAM_SIXTH,
    RAM_SEVENTH,
    RAM_EIGHTH,
    RAM_NINTH,
    RAM_TENTH,
};

namespace OHOS {
namespace SmartPerf {
class RAM : public SpProfiler {
public:
    std::map<std::string, std::string> GetSysRamInfo() const;
    std::map<std::string, std::string> GetRamInfo() const;
    std::map<std::string, std::string> GetPssRamInfo(FILE *fd, std::vector<std::string> paramsInfo) const;
    std::map<std::string, std::string> ParsePssValues(FILE *fd, std::vector<std::string> &paramsInfo) const;
    std::map<std::string, std::string> SaveSumRamInfo(std::vector<std::string> paramsInfo) const;
    std::map<std::string, std::string> ProcMemNaInfo() const;
    static RAM &GetInstance()
    {
        static RAM instance;
        return instance;
    }
    std::map<std::string, std::string> ItemData() override;
    void SetPackageName(const std::string &pName);
    void ThreadGetPss() const;
    void TriggerGetPss() const;
    void SetFirstFlag();
    void SetProcessId(const std::string &pid);
private:
    RAM() {};
    RAM(const RAM &);
    RAM &operator = (const RAM &);
    std::string packageName = "";
    std::string processId = "";
};
}
}
#endif
