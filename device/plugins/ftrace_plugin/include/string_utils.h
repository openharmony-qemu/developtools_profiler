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
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

class StringUtils {
public:
    static bool EndsWith(const std::string& str, const std::string& postfix);
    static bool StartsWith(const std::string& str, const std::string& prefix);
    static bool Contains(const std::string& str, const std::string& target);
    static std::string Strip(const std::string& str);
    static std::string Join(const std::vector<std::string>& strs, const std::string& sep);
    static std::vector<std::string> Split(const std::string& str, const std::string& sep);
};

#endif // STRING_UTILS_H