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
#include "string_help.h"

char* GetDemangleSymbolIndex(const char* mangled)
{
    int status = 0;
    auto demangle = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    if (status) { // status != 0 failed
        return const_cast<char*>(mangled);
    } else {
        return demangle;
    }
}

bool StartWith(const std::string& str, const std::string& res)
{
    if (res.size() > str.size()) {
        return false;
    }
    return str.compare(0, res.length(), res) == 0;
}

bool EndWith(const std::string& str, const std::string& res)
{
    if (res.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - res.size(), res.size(), res) == 0;
}

std::vector<std::string> SplitStringToVec(const std::string& str, const std::string& pat)
{
    std::vector<std::string> result;
    size_t curPos = 0;
    size_t strSize = str.size();
    size_t patSize = pat.size();
    while (curPos < strSize) {
        auto patPos = str.find(pat, curPos);
        if (patPos == std::string::npos) {
            break;
        }
        result.emplace_back(str.substr(curPos, patPos - curPos));
        curPos = patPos + patSize;
    }
    if (curPos < strSize) {
        result.emplace_back(str.substr(curPos));
    }

    return result;
}
