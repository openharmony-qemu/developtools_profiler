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
#include "config.hpp"
#include <unordered_map>

namespace mem_analyzer {

static const std::unordered_map<std::string, Scope> str_to_scope = {
    {"app-only", Scope::AppOnly},
    {"system-only", Scope::SystemOnly},
    {"app-and-system", Scope::AppAndSystem},
    {"mixed", Scope::Mixed},
};

static const std::unordered_map<Scope, std::string> scope_to_str = {
    {Scope::AppOnly, "app-only"},
    {Scope::SystemOnly, "system-only"},
    {Scope::AppAndSystem, "app-and-system"},
    {Scope::Mixed, "mixed"},
};

Scope scope_from_string(const std::string& s) {
    auto it = str_to_scope.find(s);
    if (it != str_to_scope.end()) return it->second;
    return Scope::AppAndSystem;
}

std::string scope_to_string(Scope s) {
    auto it = scope_to_str.find(s);
    if (it != scope_to_str.end()) return it->second;
    return "app-and-system";
}

} // namespace mem_analyzer
