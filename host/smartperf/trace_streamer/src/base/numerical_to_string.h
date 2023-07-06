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
#ifndef INCLUDE_BASE_NUMERICAL_TO_STRING_H_
#define INCLUDE_BASE_NUMERICAL_TO_STRING_H_
#include <string>
#include "string_to_numerical.h"
#include "ts_common.h"

namespace SysTuning {
namespace base {
// Put uint_ 64 type numbers are converted to hexadecimal numbers and expressed as strings.
inline std::string Uint64ToHexText(uint64_t inputNumber)
{
    if (inputNumber == INVALID_UINT64) {
        return "";
    }
    std::string str = "0x" + base::number(inputNumber, base::INTEGER_RADIX_TYPE_HEX);
    return str;
}
} // namespace base
} // namespace SysTuning

#endif // INCLUDE_BASE_NUMERICAL_TO_STRING_H_
