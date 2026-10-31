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
#include "config.h"
#include <unordered_map>

namespace mem_analyzer {

static const std::unordered_map<std::string, Scope> strToScope = {
    {"app-only", Scope::APP_ONLY},
    {"system-only", Scope::SYSTEM_ONLY},
    {"app-and-system", Scope::APP_AND_SYSTEM},
    {"mixed", Scope::MIXED},
};

static const std::unordered_map<Scope, std::string> scopeToStr = {
    {Scope::APP_ONLY, "app-only"},
    {Scope::SYSTEM_ONLY, "system-only"},
    {Scope::APP_AND_SYSTEM, "app-and-system"},
    {Scope::MIXED, "mixed"},
};

Scope ScopeFromString(const std::string& s)
{
    auto it = strToScope.find(s);
    if (it != strToScope.end()) {
        return it->second;
    }
    return Scope::APP_AND_SYSTEM;
}

std::string ScopeToString(Scope s)
{
    auto it = scopeToStr.find(s);
    if (it != scopeToStr.end()) {
        return it->second;
    }
    return "app-and-system";
}

} // namespace mem_analyzer
