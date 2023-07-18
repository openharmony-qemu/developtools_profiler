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

#include "parting_string.h"
namespace SysTuning {
namespace base {
PartingString::PartingString(std::string str, char delimiter) : str_(std::move(str)), delimiter_(delimiter)
{
    begin_ = str_.begin();
    end_ = str_.end();
    cur_ = nullptr;
}

bool PartingString::Next()
{
    while (begin_ != end_) {
        if (*begin_ == delimiter_) {
            ++begin_;
            continue;
        }

        cur_ = begin_.base();
        do {
            if (*begin_ == delimiter_) {
                *(begin_++) = '\0';
                break;
            }
            if (*begin_ == '\0') {
                begin_ = end_;
                break;
            }
        } while (begin_++ != end_);

        if (*cur_ != '\0') {
            return true;
        }

        ++begin_;
    }

    cur_ = nullptr;
    return false;
}
} // namespace base
} // namespace SysTuning
