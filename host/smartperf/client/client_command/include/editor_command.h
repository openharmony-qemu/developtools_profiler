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
#ifndef EDITOR_COMMAND_H
#define EDITOR_COMMAND_H
namespace OHOS {
namespace SmartPerf {
class EditorCommand {
public:
    const int threeParamMore = 3;
    EditorCommand(int argc, std::vector<std::string> v);
    ~EditorCommand() {};
    float ColdStart(std::vector<std::string> v);
    float ColdStartHM(std::vector<std::string> v);
    float HotStart(std::vector<std::string> v);
    static float CompleteTime();
    static float ResponseTime();
    static std::string SlideFPS(std::vector<std::string> v);
    static std::string SlideFps(std::vector<std::string> v);
private:
    bool isOhTest = false;
};
}
}
#endif // SMARTPERF_COMMAND_H