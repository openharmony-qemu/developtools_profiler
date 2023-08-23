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

#ifndef INCLUDE_BASE_STRING_TO_NUMERICAL_H_
#define INCLUDE_BASE_STRING_TO_NUMERICAL_H_

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <typeinfo>
#include "log.h"
namespace SysTuning {
namespace base {
enum IntegerRadixType { INTEGER_RADIX_TYPE_DEC = 10, INTEGER_RADIX_TYPE_HEX = 16 };
inline uint16_t GetNameASCIISumNoNum(const std::string& str)
{
    uint32_t sum = 0;
    int32_t len = str.length() - 1;
    while (len >= 0) {
        sum += std::isdigit(str.at(len)) ? 0 : str.at(len);
        len--;
    }
    return sum % INTEGER_RADIX_TYPE_HEX;
}

inline std::string number(uint64_t value, int32_t base = INTEGER_RADIX_TYPE_DEC)
{
    std::stringstream ss;
    if (base == INTEGER_RADIX_TYPE_DEC) {
        ss << std::dec << value;
    } else if (base == INTEGER_RADIX_TYPE_HEX) {
        ss << std::hex << value;
    }
    return ss.str();
}

template <typename T>
std::optional<T> StrToInt(const std::string& str, int32_t base = INTEGER_RADIX_TYPE_DEC)
{
    if (!str.empty()) {
        char* endPtr = nullptr;
        T value;
        if constexpr (std::is_same_v<T, uint32_t>) {
            value = static_cast<T>(std::strtoul(str.c_str(), &endPtr, base));
        } else if constexpr (std::is_same_v<T, int32_t>) {
            value = static_cast<T>(std::strtol(str.c_str(), &endPtr, base));
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            value = static_cast<T>(std::strtoull(str.c_str(), &endPtr, base));
        } else if constexpr (std::is_same_v<T, int64_t>) {
            value = static_cast<T>(std::strtoll(str.c_str(), &endPtr, base));
        }
        if (!*endPtr) {
            return std::make_optional(value);
        }
    }
    TS_LOGD("Illegal value: %s", str.c_str());
    return std::nullopt;
}

inline std::optional<double> StrToDouble(const std::string& str)
{
    if (!str.empty()) {
#ifdef WIN32_
        char* end = nullptr;
        double value = std::strtod(str.c_str(), &end);
#else
        double value = std::stod(str);
#endif
        return std::make_optional(value);
    }
    return std::nullopt;
}
} // namespace base
} // namespace SysTuning

#endif // INCLUDE_TUNING_EXT_BASE_STRING_UTILS_H_
