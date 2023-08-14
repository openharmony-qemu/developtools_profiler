/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "string_utils.h"

bool StringUtils::EndsWith(const std::string& str, const std::string& postfix)
{
    if (str.size() < postfix.size()) {
        return false;
    }
    return str.compare(str.size() - postfix.size(), postfix.size(), postfix) == 0;
}

bool StringUtils::StartsWith(const std::string& str, const std::string& prefix)
{
    if (str.size() < prefix.size()) {
        return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool StringUtils::Contains(const std::string& str, const std::string& target)
{
    return str.find(target) != std::string::npos;
}

std::string StringUtils::Strip(const std::string& str)
{
    std::string blanks = " \f\v\t\r\n";

    auto first = str.find_first_not_of(blanks);
    if (first == std::string::npos) {
        return "";
    }

    auto last = str.find_last_not_of(blanks);
    if (last == std::string::npos) {
        return "";
    }
    return str.substr(first, last - first + 1);
}

std::string StringUtils::Join(const std::vector<std::string>& strs, const std::string& sep)
{
    std::string result;
    for (auto& s : strs) {
        result += s;
        result += sep;
    }
    if (result.size() > sep.size()) {
        result.resize(result.size() - sep.size());
    }
    return result;
}

std::vector<std::string> StringUtils::Split(const std::string& str, const std::string& sep)
{
    std::vector<std::string> result;
    if (str.empty() || sep.empty() || str.size() < sep.size()) {
        return result;
    }
    size_t start = 0; // start index
    size_t pos = 0;
    while (start < str.size()) {
        pos = str.find(sep, start);
        if (pos != std::string::npos) {
            result.push_back(str.substr(start, pos - start));
            start = pos + sep.size(); // next start index
        } else {
            // the last part
            result.push_back(str.substr(start));
            break;
        }
    }
    return result;
}