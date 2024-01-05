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

#include "runtime_stack_range.h"

#include <csignal>
#include <cstring>
#include <map>
#include <sys/types.h>
#include "get_thread_id.h"
#include "utilities.h"

namespace {
constexpr int BASE_MIN = 2;
constexpr int BASE_CENTRE = 10;
constexpr int BASE_MAX = 16;

struct StackScope {
    const char* start;
    const char* end;
};
struct StandardLibrary {
    StandardLibrary(uint64_t begin, uint64_t end, const std::string& name)
        : soBegin_(begin), soEnd_(end), name_(name)
    {}
    uint64_t soBegin_;
    uint64_t soEnd_;
    std::string name_;
};

static StackScope g_mainStack;
static std::map<std::string, StandardLibrary> g_stdLib;
}  // namespace

static void GetThreadRuntimeStackRange(const char** start, const char** end)
{
    *start = nullptr;
    *end = nullptr;
    pthread_t tid = pthread_self();
    pthread_attr_t attr;
    if (pthread_getattr_np(tid, &attr) == 0) {
        size_t stackSize;
        if (pthread_attr_getstack(&attr, reinterpret_cast<void**>(const_cast<char**>(start)), &stackSize) == 0) {
            *end = *start + stackSize;
        }
        pthread_attr_destroy(&attr);
    }
}

static long long CvtStrToInt(const char* str, int base)
{
    long long result = 0;
    if (base >= BASE_MIN && base <= BASE_CENTRE) {
        while (*str) {
            if (*str >= '0' && *str <= '0' + base - 1) {
                result = result * base + static_cast<long long>((*str) - '0');
            } else {
                break;
            }
            ++str;
        }
    } else if (base > BASE_CENTRE && base <= BASE_MAX) {
        while (*str) {
            if (*str >= '0' && *str <= '0' + base - 1) {
                result = result * base + static_cast<long long>(*str) - '0';
            } else if (*str >= 'a' && *str <= 'a' + base - 0x0a - 1) {
                result = result * base + static_cast<long long>(*str) - 'a' + 0x0a;
            } else if (*str >= 'A' && *str <= 'A' + base - 0x0a - 1) {
                result = result * base + static_cast<long long>(*str) - 'A' + 0x0a;
            } else {
                break;
            }
            ++str;
        }
    } else {
        result = 0;
    }
    return result;
}

static int IsEmptyString(const std::string& str)
{
    size_t idx = 0;
    size_t size = str.size();
    while (idx < size) {
        if (!isspace(static_cast<unsigned char>(str[idx])) && str[idx] != 0) {
            return 0;
        }
        ++idx;
    }
    return 1;
}

static void GetAnUnlimitedLine(FILE* fp, std::string& buf)
{
    if (!fp) {
        buf.resize(0);
        return;
    }
    char* retLine = nullptr;
    if (buf.size() == 0) {
        buf.resize(INIT_LINE_SIZE);
    }
    
    int offset = 0;
    int length = 0;
    do {
        if (offset + length >= static_cast<int>(buf.size())) {
            buf.resize(buf.size() + INC_LINE_SIZE);
        }
        retLine = fgets(&buf[0] + offset, buf.size() - offset, fp);
        if (retLine == nullptr) {
            break;
        }
        length = static_cast<int>(strlen(&buf[0] + offset));
        if (offset + length - 1 >= 0 && buf[offset + length - 1] == '\n') {
            break;
        }
        offset += length;
    } while (1);
}

static bool IsLegalSoName(const std::string &fileName)
{
    if (fileName.front() == '[' or fileName.back() == ']' or
        std::strncmp(fileName.c_str(), "/dev/", sizeof("/dev/")) == 0 or
        std::strncmp(fileName.c_str(), "/memfd:", sizeof("/memfd:")) == 0 or
        std::strncmp(fileName.c_str(), "//anon", sizeof("//anon")) == 0) {
        return false;
    }
    return true;
}

static void GetStandardLibraryRange(std::string& line)
{
    line.resize(strlen(line.c_str()));
    std::vector<std::string> mapTokens = OHOS::Developtools::NativeDaemon::StringSplit(line, " ");
    const std::string& soRange = mapTokens.front();
    std::string& soName = mapTokens.back();
    if (IsLegalSoName(soName)) {
        std::string::size_type concatPos = soRange.find('-');
        uint64_t soStart = static_cast<uint64_t>(strtoll(soRange.c_str(), nullptr, BASE_MAX));
        uint64_t soEnd = static_cast<uint64_t>(strtoll(soRange.c_str() + concatPos + 1, nullptr, BASE_MAX));
        auto [iter, isExit] = g_stdLib.try_emplace(soName, StandardLibrary(soStart, soEnd, soName));
        if (!isExit) {
            if (iter->second.soBegin_ > soStart) {
                iter->second.soBegin_ = soStart;
            } else if (iter->second.soEnd_ < soEnd) {
                iter->second.soEnd_ = soEnd;
            }
        }
    }
}

void GetMainThreadRuntimeStackRange(std::vector<std::pair<uint64_t, uint64_t>>& filterStaLibRange)
{
    std::string line;
    FILE* fp = fopen("/proc/self/maps", "re");
    if (fp == nullptr) {
        return;
    }
    while (!feof(fp)) {
        line.clear();
        GetAnUnlimitedLine(fp, line);
        if (IsEmptyString(line)) {
            continue;
        }
        if (line.find("[stack]") != std::string::npos) {
            std::string::size_type concatPos = line.find('-');
            if (concatPos != std::string::npos) {
                g_mainStack.start = reinterpret_cast<char*>(CvtStrToInt(line.c_str(), BASE_MAX));
                g_mainStack.end = reinterpret_cast<char*>(CvtStrToInt(line.c_str() + concatPos + 1, BASE_MAX));
            }
        } else if (line.find("ld-musl") != std::string::npos || line.find("libc++") != std::string::npos) {
            GetStandardLibraryRange(line);
        }
    }
    if (fclose(fp) != 0) {
        printf("fclose failed.\n");
    }
    for (const auto& [soName, stdLibrary]: g_stdLib) {
        filterStaLibRange.emplace_back(stdLibrary.soBegin_, stdLibrary.soEnd_);
    }
}

static bool IfContained(const char* start, const char* end, const char* ptr)
{
    bool ret = (ptr >= start && ptr < end);
    return ret;
}

static void GetRuntimeSigalAltStackRange(char** start, char** end)
{
    *start = nullptr;
    *end = nullptr;

    stack_t altStack;

    if (sigaltstack(nullptr, &altStack) != -1) {
        if ((altStack.ss_flags & SS_ONSTACK) != 0) {
            *start = reinterpret_cast<char*>(altStack.ss_sp);
            *end = reinterpret_cast<char*>(altStack.ss_sp) + altStack.ss_size;
        }
    }
}

static bool IfSubThread(pid_t pid, pid_t tid)
{
    return pid != tid;
}

void GetRuntimeStackEnd(const char* stackptr, const char** end, pid_t pid, pid_t tid)
{
    const char* start = nullptr;
    *end = nullptr;
    bool isSubThread = IfSubThread(pid, tid);
    if (isSubThread) {
        GetThreadRuntimeStackRange(&start, end);
    } else {
        start = g_mainStack.start;
        *end = g_mainStack.end;
    }
    if (!IfContained(start, *end, stackptr)) {
        char *sigStackStart = nullptr;
        char *sigStackEnd = nullptr;
        GetRuntimeSigalAltStackRange(&sigStackStart, &sigStackEnd);
        if (IfContained(sigStackStart, sigStackEnd, stackptr)) {
            *end = sigStackEnd;
        } else if (!(!isSubThread && stackptr < *end)) {
            *end = nullptr;
        }
    }
}
