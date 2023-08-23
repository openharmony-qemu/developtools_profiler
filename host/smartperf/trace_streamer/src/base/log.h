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

#ifndef INCLUDE_TS_BASE_LOGGING_H_
#define INCLUDE_TS_BASE_LOGGING_H_

#include <iostream>
#include <cstring>

// namespace SysTuning {
// namespace base {
#define TS_CRASH                 \
    do {                         \
        __builtin_trap();        \
        __builtin_unreachable(); \
    } while (0)
enum LogLevel { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
const enum LogLevel CURRENT_LOG_LEVEL = LOG_DEBUG;
extern bool g_cleanMode;
#define LOGWITHLEVEL(level, motify, fmt, ...)                                                               \
    do {                                                                                                    \
        if (level >= CURRENT_LOG_LEVEL) {                                                                   \
            if (!g_cleanMode) {                                                                             \
                fprintf(stdout, "[-%c][%s][%d]: " fmt "\n", motify, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            }                                                                                               \
            if (level == LOG_FATAL) {                                                                       \
                TS_CRASH;                                                                                   \
            }                                                                                               \
        }                                                                                                   \
    } while (0)
#define TS_LOGE(fmt, ...) LOGWITHLEVEL(LOG_ERROR, 'E', fmt, ##__VA_ARGS__)
#define TS_LOGF(fmt, ...) LOGWITHLEVEL(LOG_FATAL, 'F', fmt, ##__VA_ARGS__)
#ifdef NDEBUG
#define TS_LOGI(fmt, ...) LOGWITHLEVEL(LOG_DEBUG, 'I', fmt, ##__VA_ARGS__)
#define TS_LOGD(format, ...)
#define TS_ASSERT(x)
#define TS_LOGW(fmt, ...) LOGWITHLEVEL(LOG_WARN, 'W', fmt, ##__VA_ARGS__)
#else
#define TS_LOGD(fmt, ...) LOGWITHLEVEL(LOG_DEBUG, 'D', fmt, ##__VA_ARGS__)
#define TS_LOGI(fmt, ...) LOGWITHLEVEL(LOG_DEBUG, 'I', fmt, ##__VA_ARGS__)
#define TS_LOGW(fmt, ...) LOGWITHLEVEL(LOG_WARN, 'W', fmt, ##__VA_ARGS__)

#define TS_ASSERT(x)  \
    do {              \
        if (!(x)) {   \
            TS_CRASH; \
        }             \
    } while (0)

#endif
// } // namespace base
// } // namespace SysTuning

#endif // INCLUDE_TS_BASE_LOGGING_H_
