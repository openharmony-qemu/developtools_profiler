/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INCLUDE_BASE_STRING_SPLITTER_H_
#define INCLUDE_BASE_STRING_SPLITTER_H_

#include <string>

namespace SysTuning {
namespace base {
class PartingString {
public:
    PartingString(std::string str, char delimiter);

    bool Next();

    char* GetCur()
    {
        return cur_;
    }

private:
    PartingString(const PartingString&) = delete;
    PartingString& operator=(const PartingString&) = delete;

    std::string str_;
    char* cur_ = nullptr;
    std::string::iterator begin_;
    std::string::iterator end_;
    const char delimiter_;
};
} // namespace base
} // namespace SysTuning

#endif
