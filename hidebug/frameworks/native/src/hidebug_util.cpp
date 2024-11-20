/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hidebug_util.h"

#include <ctime>
#include <thread>

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Util"

int64_t GetElapsedNanoSecondsSinceBoot()
{
    struct timespec times = {0, 0};
    int res = clock_gettime(CLOCK_BOOTTIME, &times);
    if (res != 0) {
        HILOG_INFO(LOG_CORE, "clock_gettime failed in GetElapsedNanoSecondsSinceBoot.");
        return 0;
    }
    constexpr static int secondToNanosecond = 1 * 1000 * 1000 * 1000;
    return times.tv_sec * secondToNanosecond + times.tv_nsec;
}

int64_t GetNanoSecondsTimestamp()
{
    struct timespec times = {0, 0};
    int res = clock_gettime(CLOCK_REALTIME, &times);
    if (res != 0) {
        HILOG_INFO(LOG_CORE, "clock_gettime failed in GetNanoSecondsTimestamp.");
        return 0;
    }
    constexpr static int secondToNanosecond = 1 * 1000 * 1000 * 1000;
    return times.tv_sec * secondToNanosecond + times.tv_nsec;
}
}
}