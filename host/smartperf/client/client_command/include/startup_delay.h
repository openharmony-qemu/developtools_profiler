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
#ifndef STARTUP_DELAY_H
#define STARTUP_DELAY_H
#include <thread>
namespace OHOS {
namespace SmartPerf {
class StartUpDelay {
public:
    StartUpDelay();
    ~StartUpDelay();
    void GetTrace(const std::string &traceName) const;
    void GetHisysIdAndKill() const;
    void GetHisysId() const;
    std::string GetPidByPkg(const std::string &curPkgName, std::string* pids = nullptr) const;
    bool KillSpProcess() const;
    bool GetSpClear(bool isKillTestServer) const;
    void ClearOldServer() const;
    std::string ExecuteCommand(const std::vector<const char*> &args) const;
    std::vector<std::string> GetPidParams() const;
    void KillTestSpdaemon(const std::string &line, const std::string &curPid) const;
};
}
}
#endif