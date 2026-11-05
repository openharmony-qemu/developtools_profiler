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
#include <string>

namespace mem_analyzer {

enum class Scope {
    APP_ONLY,
    SYSTEM_ONLY,
    APP_AND_SYSTEM,
    MIXED
};

Scope ScopeFromString(const std::string& s);
std::string ScopeToString(Scope s);

// Default values for CLI options
namespace defaults {
    constexpr const char* OUTPUT_DIR = "out";
    constexpr Scope SCOPE = Scope::APP_AND_SYSTEM;
    constexpr const char* SCOPE_STR = "app-and-system";
    constexpr int TRACE_TOP_N = 10;
    constexpr int TSHANDLE_TOP_N = 50;
    constexpr int JSLEAK_TOP_N = 50;
    constexpr const char* JSLEAK_MODE = "all";
    constexpr const char* HTRACE_TYPE = "nativeheap";
    constexpr int PROF_DURATION = 120;
}

} // namespace mem_analyzer
