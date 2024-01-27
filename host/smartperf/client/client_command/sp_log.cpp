/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include "sp_log.h"

#include "securec.h"

#ifdef HI_LOG_ENABLE
#include "hilog/log.h"
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#else
#include <cstdio>
#endif

namespace OHOS {
namespace SmartPerf {
const int32_t LOG_MAX_LEN = 10000;

static void SpLogOut(SpLogLevel logLevel, const char *logBuf)
{
#ifdef HI_LOG_ENABLE
    LogLevel hiLogLevel = LOG_INFO;
    switch (logLevel) {
        case SP_LOG_DEBUG:
            hiLogLevel = LOG_DEBUG;
            break;
        case SP_LOG_INFO:
            hiLogLevel = LOG_INFO;
            break;
        case SP_LOG_WARN:
            hiLogLevel = LOG_WARN;
            break;
        case SP_LOG_ERROR:
            hiLogLevel = LOG_ERROR;
            break;
        default:
            break;
    }
    (void)HiLogPrint(LOG_CORE, hiLogLevel, LOG_DOMAIN, "SP_daemon", "%{public}s", logBuf);
#else
    switch (logLevel) {
        case SP_LOG_DEBUG:
            printf("[D]%s\n", logBuf);
            break;
        case SP_LOG_INFO:
            printf("[I]%s\n", logBuf);
            break;
        case SP_LOG_WARN:
            printf("[W]%s\n", logBuf);
            break;
        case SP_LOG_ERROR:
            printf("[E]%s\n", logBuf);
            break;
        default:
            break;
    }
#endif
}

void SpLog(SpLogLevel logLevel, const char *fmt, ...)
{
    char logBuf[LOG_MAX_LEN] = {0};
    va_list arg;
    int32_t ret = memset_s(&arg, sizeof(va_list), 0, sizeof(va_list));
    if (ret != 0) {
        SpLogOut(logLevel, "SP log memset_s error.");
        return;
    }
    va_start(arg, fmt);
    ret = vsprintf_s(logBuf, sizeof(logBuf), fmt, arg);
    va_end(arg);
    if (ret < 0) {
        SpLogOut(logLevel, "SP log length error.");
        return;
    }
    SpLogOut(logLevel, logBuf);
}
} // namespace SmartPerf
} // namespace OHOS