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
#include "utils/terminal_style.h"

#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#define ISATTY  _isatty
#define FILENO  _fileno
#else
#include <unistd.h>
#define ISATTY  isatty
#define FILENO  fileno
#endif

namespace mem_analyzer {
namespace style {

static constexpr const char* ANSI_GREEN  = "\033[32m";
static constexpr const char* ANSI_RED    = "\033[31m";
static constexpr const char* ANSI_YELLOW = "\033[33m";
static constexpr const char* ANSI_BOLD   = "\033[1m";
static constexpr const char* ANSI_DIM    = "\033[2m";
static constexpr const char* ANSI_RESET  = "\033[0m";

static bool g_sInitialized = false;

void Init()
{
#if defined(_WIN32)
    if (g_sInitialized) {
        return;
    }
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(h, &mode)) {
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
    g_sInitialized = true;
#else
    (void)g_sInitialized;
#endif
}

bool IsTty()
{
    return ISATTY(FILENO(stdout)) != 0;
}

bool IsStderrTty()
{
    return ISATTY(FILENO(stderr)) != 0;
}

static std::string wrap(const char* code, const std::string& text)
{
    if (!IsStderrTty()) {
        return text;
    }
    return std::string(code) + text + ANSI_RESET;
}

std::string Green(const std::string& text)
{
    return wrap(ANSI_GREEN, text);
}
std::string Red(const std::string& text)
{
    return wrap(ANSI_RED, text);
}
std::string Yellow(const std::string& text)
{
    return wrap(ANSI_YELLOW, text);
}
std::string Bold(const std::string& text)
{
    return wrap(ANSI_BOLD, text);
}
std::string Dim(const std::string& text)
{
    return wrap(ANSI_DIM, text);
}

} // namespace style
} // namespace mem_analyzer