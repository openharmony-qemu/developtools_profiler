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
#include <thread>
namespace OHOS {
namespace SmartPerf {
class StartUpDelay {
public:
    StartUpDelay();
    ~StartUpDelay();
    void GetTrace(std::string sessionID, std::string traceName);
    std::thread ThreadGetTrace(std::string sessionID, std::string traceName);
    void GetLayout();
    std::thread ThreadGetLayout();
    void ChangeToBackground();
    void KillCurApp(std::string curPkgName);
    std::vector<std::string> GetPidByPkg(std::string curPkgName);
};
}
}