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
namespace style {

/// Enable ANSI escape processing on Windows (call once at startup).
void Init();

/// True if stdout is a terminal (not piped/redirected).
bool IsTty();

/// True if stderr is a terminal (not piped/redirected).
bool IsStderrTty();

/// Wrap text with ANSI color/style. Returns raw text if stderr is not a TTY.
std::string Green(const std::string& text);
std::string Red(const std::string& text);
std::string Yellow(const std::string& text);
std::string Bold(const std::string& text);
std::string Dim(const std::string& text);

} // namespace style
} // namespace mem_analyzer