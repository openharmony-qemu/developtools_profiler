/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#pragma once
#include <cctype>
#include <string>

namespace mem_analyzer {

/// 判断字符串是否全为数字（非空且每个字符都是 0-9）。
inline bool AllDigits(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

/// 判断源码路径是否与 nameCache 顶层 key（原始文件路径）匹配：
/// nameCache 顶层 key 无模块前缀（如 "src/main/ets/web/a.ets"），而还原后的
/// 源码路径常带 "module/" 前缀（如 "entry/src/main/ets/web/a.ets"），按路径后缀匹配。
inline bool SourcePathMatches(const std::string& src, const std::string& filePath)
{
    if (src == filePath) {
        return true;
    }
    if (src.size() <= filePath.size()) {
        return false;
    }
    const size_t sepPos = src.size() - filePath.size() - 1;
    return src[sepPos] == '/' && src.compare(sepPos + 1, filePath.size(), filePath) == 0;
}

} // namespace mem_analyzer
