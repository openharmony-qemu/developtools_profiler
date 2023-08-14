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

#include "command_param.h"
#include <algorithm>
#include <list>

void CommandParam::AddFilter(const std::string& filterName)
{
    if (IsInFilter(filterName)) {
        return;
    }
    paramFilter_.push_back(filterName);
}
bool CommandParam::IsInFilter(const std::string& filterName)
{
    return std::any_of(paramFilter_.begin(), paramFilter_.end(),
                       [filterName](const std::string& s) { return s == filterName; });
}