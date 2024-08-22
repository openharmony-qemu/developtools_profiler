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

#ifndef HIPERF_HILOG
#define HIPERF_HILOG

#ifndef CONFIG_NO_HILOG
#define HILOG_PUBLIC  "{public}"
#define HILOG_NEWLINE ""
#else
#define HILOG_PUBLIC  ""
#define HILOG_NEWLINE "\n"
#endif

#define FILENAME                                                                                   \
    (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define FORMATED(fmt, ...)                                                                         \
    "[%" HILOG_PUBLIC "s:%" HILOG_PUBLIC "d] %" HILOG_PUBLIC "s# " fmt HILOG_NEWLINE, FILENAME,    \
        __LINE__, __FUNCTION__, ##__VA_ARGS__

#ifndef CONFIG_NO_HILOG
#include "hilog_base/log_base.h"

static constexpr unsigned int BASE_HIPERF_DOMAIN_ID = 0xD000000;
static constexpr unsigned int HITRACE_TAG = 0xd03301;

#endif // CONFIG_NO_HILOG

#endif // HIPERF_HILOG
