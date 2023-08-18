/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include "utilities.h"

#include <fstream>
#include <iostream>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
std::string ReadFileToString(const std::string &fileName)
{
    std::ifstream inputString(fileName, std::ios::in);
    if (!inputString or !inputString.is_open()) {
        return EMPTY_STRING;
    }
    std::istreambuf_iterator<char> firstIt = {inputString};
    std::istreambuf_iterator<char> lastIt = {};

    std::string content(firstIt, lastIt);
    return content;
}

std::vector<std::string> StringSplit(std::string source, const std::string &split)
{
    std::vector<std::string> result;

    // find
    if (!split.empty()) {
        size_t pos = 0;
        while ((pos = source.find(split)) != std::string::npos) {
            // split
            std::string token = source.substr(0, pos);
            if (!token.empty()) {
                result.push_back(token);
            }
            source.erase(0, pos + split.length());
        }
    }
    // add last token
    if (!source.empty()) {
        result.push_back(source);
    }
    return result;
}

bool StringEndsWith(const std::string &string, const std::string &with)
{
    if (string.empty()) {
        // empty string only end with empty string
        if (with.empty()) {
            return true;
        } else {
            return false;
        }
    }
    return string.rfind(with) == (string.length() - with.length());
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
