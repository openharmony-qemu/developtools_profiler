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
    void GetTrace(const std::string &sessionID, const std::string &traceName) const;
    std::thread ThreadGetTrace(const std::string &sessionID, const std::string &traceName) const;
    void GetLayout();
    std::thread ThreadGetLayout();
    void InputEvent(const std::string &point);
    std::thread ThreadInputEvent(const std::string &point);
    void GetHisysIdAndKill() const;
    std::thread ThreadGetHisysIds() const;
    void GetHisysId() const;
    std::thread ThreadGetHisysId() const;
    void ChangeToBackground();
    std::string GetPidByPkg(const std::string &curPkgName);
    bool GetSpTcp();
    bool GetSpClear();
    void InitXY2(const std::string &curAppName, const std::string &fileName, const std::string &appPkgName);
    void InitXY(const std::string &curAppName, const std::string &fileName);
    std::string pointXY = "0 0";
};
}
}
#endif