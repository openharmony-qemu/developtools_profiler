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

#ifndef OHOS_PROFILER_LOGGING_H
#define OHOS_PROFILER_LOGGING_H

#define EXPORT_API __attribute__((visibility("default")))

#undef NDEBUG

#ifndef LOG_TAG
#define LOG_TAG "Hiprofiler"
#endif

#define PROFILER_SUBSYSTEM 0xD002D0C
#ifndef LOG_DOMAIN
#define LOG_DOMAIN PROFILER_SUBSYSTEM
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) ((void)(x))
#endif

#ifdef HAVE_HILOG
#include "hilog/log.h"
#include <string>
#else // HAVE_HILOG
#include <mutex>
#include <string>
#include <securec.h>
#include <stdarg.h>
#if !is_mingw
#include <sys/syscall.h>
#undef getsystid
#define getsystid() syscall(SYS_gettid)
#else
#include "windows.h"
inline long getsystid()
{
    return GetCurrentThreadId();
}
#endif

#include <ctime>
#include <vector>
#include <unistd.h>

static inline long GetTid(void)
{
    return getsystid();
}

enum {
    LOG_UNKNOWN = 0,
    LOG_DEFAULT,
    LOG_VERBOSE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
    LOG_SILENT,
};

namespace {
constexpr int NS_PER_MS_LOG = 1000 * 1000;
}

static inline std::string GetTimeStr();

typedef const char* ConstCharPtr;

static inline int HiLogPrintArgs(int prio, int domain, ConstCharPtr tag, ConstCharPtr fmt, va_list vargs)
{
    static std::mutex mtx;
    static std::vector<std::string> prioNames = {"U", " ", "V", "D", "I", "W", "E", "F", "S"};
    std::unique_lock<std::mutex> lock(mtx);
    int count = fprintf(stderr, "%04x %s %7d %7ld %5s %s ", domain, GetTimeStr().c_str(), getpid(), GetTid(),
                        prioNames[prio].c_str(), tag);
    if (count < 0) {
        return 0;
    }
    count = count + vfprintf(stderr, fmt, vargs) + fprintf(stderr, "\n");
    fflush(stderr);
    return count;
}

static inline int HiLogPrint(int type, int prio, int domain, ConstCharPtr tag, ConstCharPtr fmt, ...)
{
    va_list vargs;
    UNUSED_PARAMETER(type);
    va_start(vargs, fmt);
    int count = HiLogPrintArgs(prio, domain, tag, fmt, vargs);
    va_end(vargs);
    return count;
}

#ifndef LOG_CORE
#define LOG_CORE 0
#endif

#define HILOG_DEBUG(LOG_CORE, fmt, ...) HiLogPrint(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define HILOG_INFO(LOG_CORE, fmt, ...) HiLogPrint(LOG_CORE, LOG_INFO, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define HILOG_WARN(LOG_CORE, fmt, ...) HiLogPrint(LOG_CORE, LOG_WARN, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define HILOG_ERROR(LOG_CORE, fmt, ...) HiLogPrint(LOG_CORE, LOG_ERROR, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)

#endif // HAVE_HILOG

#ifndef NDEBUG
#include <securec.h>
namespace logging {
inline void StringReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    std::string::size_type pos = 0u;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

// let compiler check format string and variable arguments
static inline std::string StringFormat(const char* fmt, ...)  __attribute__((format(printf, 1, 2)));

static inline std::string StringFormat(const char* fmt, ...)
{
    va_list vargs;
    char buf[1024] = {0};

    if (fmt == nullptr) {
        return "";
    }
    std::string format(fmt);
    StringReplace(format, "%{public}", "%");

    va_start(vargs, fmt);
    if (vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, format.c_str(), vargs) < 0) {
        va_end(vargs);
        return "";
    }

    va_end(vargs);
    return buf;
}
}  // logging

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

#ifdef HILOG_INFO
#undef HILOG_INFO
#endif

#ifdef HILOG_WARN
#undef HILOG_WARN
#endif

#ifdef HILOG_ERROR
#undef HILOG_ERROR
#endif

#ifdef HILOG_PRINT
#undef HILOG_PRINT
#endif

#ifdef HAVE_HILOG
#define HILOG_PRINT(type, level, fmt, ...) \
    HiLogPrint(type, level, LOG_DOMAIN, LOG_TAG, "%{public}s", logging::StringFormat(fmt, ##__VA_ARGS__).c_str())
#else
#define HILOG_PRINT(type, level, fmt, ...) \
    HiLogPrint(type, level, LOG_DOMAIN, LOG_TAG, "%s", logging::StringFormat(fmt, ##__VA_ARGS__).c_str())
#endif

#define HILOG_DEBUG(type, fmt, ...) HILOG_PRINT(type, LOG_DEBUG, fmt, ##__VA_ARGS__)
#define HILOG_INFO(type, fmt, ...) HILOG_PRINT(type, LOG_INFO, fmt, ##__VA_ARGS__)
#define HILOG_WARN(type, fmt, ...) HILOG_PRINT(type, LOG_WARN, fmt, ##__VA_ARGS__)
#define HILOG_ERROR(type, fmt, ...) HILOG_PRINT(type, LOG_ERROR, fmt, ##__VA_ARGS__)
#endif  // NDEBUG

#define STD_PTR(K, T) std::K##_ptr<T>

#define NO_RETVAL /* retval */

#define CHECK_NOTNULL(ptr, retval, fmt, ...)                                                                        \
    do {                                                                                                            \
        if (ptr == nullptr) {                                                                                       \
            HILOG_WARN(LOG_CORE, "CHECK_NOTNULL(%s) in %s:%d FAILED, " fmt, #ptr, __func__, \
                       __LINE__, ##__VA_ARGS__);                                                                    \
            return retval;                                                                                          \
        }                                                                                                           \
    } while (0)

#ifndef FUZZ_TEST
#define CHECK_TRUE(expr, retval, fmt, ...)                                                                          \
    do {                                                                                                            \
        if (!(expr)) {                                                                                              \
            HILOG_WARN(LOG_CORE, "CHECK_TRUE(%s) in %s:%d FAILED, " fmt, #expr, __func__, __LINE__, ##__VA_ARGS__); \
            return retval;                                                                                          \
        }                                                                                                           \
    } while (0)
#else
#define CHECK_TRUE(expr, retval, fmt, ...) \
    do {                                   \
        if (!(expr)) {                     \
            return retval;                 \
        }                                  \
    } while (0)
#endif  // FUZZ_TEST

#define RETURN_IF(expr, retval, fmt, ...)             \
    do {                                              \
        if ((expr)) {                                 \
            HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__); \
            return retval;                            \
        }                                             \
    } while (0)

#ifndef HAVE_HILOG
static std::string GetTimeStr()
{
    char timeStr[64];
    struct timespec ts;
    struct tm tmStruct;
    clock_gettime(CLOCK_REALTIME, &ts);
#if !is_mingw
    localtime_r(&ts.tv_sec, &tmStruct);
#else
    CHECK_TRUE(localtime_s(&tmStruct, &ts.tv_sec) == 0, "", "localtime_s FAILED!");
#endif
    size_t used = strftime(timeStr, sizeof(timeStr), "%m-%d %H:%M:%S", &tmStruct);
    (void)snprintf_s(&timeStr[used], sizeof(timeStr) - used, sizeof(timeStr) - used - 1, ".%03ld",
        ts.tv_nsec / NS_PER_MS_LOG);
    return timeStr;
}
#endif // !HAVE_HILOG
#endif // OHOS_PROFILER_LOGGING_H
