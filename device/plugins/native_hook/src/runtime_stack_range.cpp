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

#include <algorithm>
#include <array>
#include <csignal>
#include <cstring>
#include <map>
#include <sys/types.h>
#include "c/executor_task.h"
#include "get_thread_id.h"
#include "utilities.h"

namespace {
constexpr int BASE_MAX = 16;
constexpr std::array<const char*, 2> DEFAULT_FILTER_LIBS = {"ld-musl", "libc++"};

struct StandardLibrary {
    StandardLibrary(uint64_t begin, uint64_t end, const std::string& name)
        : soBegin_(begin), soEnd_(end), name_(name)
    {}
    uint64_t soBegin_;
    uint64_t soEnd_;
    std::string name_;
};

static std::map<std::string, StandardLibrary> g_stdLib;
static std::map<std::string, StandardLibrary> g_targetLib;
static uintptr_t g_stackMainStart = 0;
static uintptr_t g_stackMainEnd = 0;
}  // namespace

static bool GetMainStackRange(uintptr_t& stackBottom, uintptr_t& stackTop)
{
    stackBottom = g_stackMainStart;
    stackTop = g_stackMainEnd;
    return (stackBottom != 0 && stackTop != 0);
}

static bool GetSubStackRange(uintptr_t& stackBottom, uintptr_t& stackTop)
{
    bool ret = false;
    pthread_attr_t tattr;
    void* base = nullptr;
    size_t size = 0;
    if (pthread_getattr_np(pthread_self(), &tattr) != 0) {
        return ret;
    }
    if (pthread_attr_getstack(&tattr, &base, &size) == 0) {
        stackBottom = reinterpret_cast<uintptr_t>(base);
        stackTop = reinterpret_cast<uintptr_t>(base) + size;
        ret = true;
    }
    pthread_attr_destroy(&tattr);
    return ret;
}

static bool GetSigAltStackRange(uintptr_t& stackBottom, uintptr_t& stackTop)
{
    bool ret = false;
    stack_t altStack;
    if (sigaltstack(nullptr, &altStack) != -1) {
        if ((static_cast<uint32_t>(altStack.ss_flags) & SS_ONSTACK) != 0) {
            stackBottom = reinterpret_cast<uintptr_t>(altStack.ss_sp);
            stackTop = reinterpret_cast<uintptr_t>(altStack.ss_sp) + altStack.ss_size;
            ret = true;
        }
    }
    return ret;
}

static bool GetCoroutineStackRange(uintptr_t& stackBottom, uintptr_t& stackTop)
{
    bool ret = false;
    void* stackAddr = nullptr;
    size_t coroutineStackSize = 0;
    if (ffrt_get_current_coroutine_stack(&stackAddr, &coroutineStackSize)) {
        stackBottom = reinterpret_cast<uintptr_t>(stackAddr);
        stackTop = stackBottom + coroutineStackSize;
        ret = true;
    }
    return ret;
}

static bool MatchFilterLib(const std::string& filePath, const std::string& filterLib)
{
    return !filterLib.empty() && filePath.find(filterLib) != std::string::npos;
}

static std::string TrimTargetLib(const std::string& targetLib)
{
    size_t first = targetLib.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = targetLib.find_last_not_of(" \t\r\n");
    return targetLib.substr(first, last - first + 1);
}

static std::vector<std::string> SplitTargetLibs(const std::string& targetLibs)
{
    std::vector<std::string> libs;
    size_t start = 0;
    while (start <= targetLibs.size()) {
        size_t end = targetLibs.find(',', start);
        std::string lib = TrimTargetLib(targetLibs.substr(start, end - start));
        if (!lib.empty()) {
            libs.emplace_back(lib);
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return libs;
}

bool IsLegalSoName(const char *fileName)
{
    if (fileName == nullptr) {
        return false;
    }
    size_t fileNameLength = strlen(fileName);
    if (fileNameLength == 0) {
        return false;
    }
    if (fileName[0] == '[' || fileName[strlen(fileName) - 1] == ']' ||
        std::strncmp(fileName, "/dev/", sizeof("/dev/") - 1) == 0 ||
        std::strncmp(fileName, "/memfd:", sizeof("/memfd:") - 1) == 0 ||
        std::strncmp(fileName, "//anon", sizeof("//anon") - 1) == 0) {
        return false;
    }
    return true;
}

bool IsFilterLibrary(const std::string& filePath, const std::vector<std::string>& filterLibList)
{
    for (const auto* defaultLib : DEFAULT_FILTER_LIBS) {
        if (MatchFilterLib(filePath, defaultLib)) {
            return true;
        }
    }
    for (const auto& filterLib : filterLibList) {
        if (MatchFilterLib(filePath, filterLib)) {
            return true;
        }
    }
    return false;
}

bool IsTargetLibrary(const std::string& filePath, const std::string& targetLibs)
{
    std::vector<std::string> libs = SplitTargetLibs(targetLibs);
    for (const auto& targetLib : libs) {
        if (MatchFilterLib(filePath, targetLib)) {
            return true;
        }
    }
    return false;
}

static void GetStandardLibraryRange(std::string& line, bool targeted = false)
{
    line.resize(strlen(line.c_str()));
    std::vector<std::string> mapTokens = OHOS::Developtools::NativeDaemon::StringSplit(line, " ");
    const std::string& soRange = mapTokens.front();
    std::string& soName = mapTokens.back();
    if (IsLegalSoName(soName.c_str())) {
        std::string::size_type concatPos = soRange.find('-');
        uint64_t soStart = static_cast<uint64_t>(strtoll(soRange.c_str(), nullptr, BASE_MAX));
        uint64_t soEnd = static_cast<uint64_t>(strtoll(soRange.c_str() + concatPos + 1, nullptr, BASE_MAX));
        auto iter = g_stdLib.begin();
        bool isExit = false;
        if (targeted) {
            std::tie(iter, isExit) = g_targetLib.try_emplace(soName, StandardLibrary(soStart, soEnd, soName));
        } else {
            std::tie(iter, isExit) = g_stdLib.try_emplace(soName, StandardLibrary(soStart, soEnd, soName));
        }
        if (!isExit) {
            if (iter->second.soBegin_ > soStart) {
                iter->second.soBegin_ = soStart;
            } else if (iter->second.soEnd_ < soEnd) {
                iter->second.soEnd_ = soEnd;
            }
        }
    }
}

bool ParseTargetedMaps(std::vector<std::pair<uint64_t, uint64_t>>& targetLibRange,
                       const std::string& targetLibs)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    bool ret = false;
    if (fp == nullptr) {
        return ret;
    }
    g_targetLib.clear();
    char mapInfo[256] = {0}; // 256: map info size
    while (fgets(mapInfo, sizeof(mapInfo), fp) != nullptr) {
        if (IsTargetLibrary(mapInfo, targetLibs)) {
            std::string lineStr = mapInfo;
            GetStandardLibraryRange(lineStr, true);
            ret = true;
        }
    }
    if (fclose(fp) != 0) {
        printf("fclose failed.\n");
    }
    targetLibRange.clear();
    for (const auto& [soName, stdLibrary]: g_targetLib) {
        targetLibRange.emplace_back(stdLibrary.soBegin_, stdLibrary.soEnd_);
    }
    return ret;
}

void UpdateTargetLibraryRange(std::vector<std::pair<uint64_t, uint64_t>>& targetLibRange,
                              uint64_t soStart, uint64_t soEnd)
{
    for (auto& range : targetLibRange) {
        if (soStart > range.second || soEnd < range.first) {
            continue;
        }
        range.first = std::min(range.first, soStart);
        range.second = std::max(range.second, soEnd);
        return;
    }
    targetLibRange.emplace_back(soStart, soEnd);
}

bool GetRuntimeStackRange(const uintptr_t stackPtr, uintptr_t& stackBottom, uintptr_t& stackTop, bool isMainThread)
{
    bool ret = false;
    if (isMainThread) {
        ret = GetMainStackRange(stackBottom, stackTop);
    } else {
        ret = GetSubStackRange(stackBottom, stackTop);
        if (stackPtr < stackBottom || stackPtr >= stackTop) {
            ret = GetSigAltStackRange(stackBottom, stackTop);
        }
    }
    if (stackPtr < stackBottom || stackPtr >= stackTop) {
        ret = GetCoroutineStackRange(stackBottom, stackTop);
    }
    return ret && (stackPtr >= stackBottom && stackPtr < stackTop);
}

bool ParseSelfMaps(std::vector<std::pair<uint64_t, uint64_t>>& filterStaLibRange,
                   const std::vector<std::string>& filterLibList)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    bool ret = false;
    if (fp == nullptr) {
        return ret;
    }
    char mapInfo[256] = {0}; // 256: map info size
    int pos = 0;
    uint64_t begin = 0;
    uint64_t end = 0;
    uint64_t offset = 0;
    char perms[5] = {0}; // 5:rwxp
    while (fgets(mapInfo, sizeof(mapInfo), fp) != nullptr) {
        if (strstr(mapInfo, "[stack]") != nullptr) {
            if (sscanf_s(mapInfo, "%" SCNxPTR "-%" SCNxPTR " %4S %" SCNxPTR " %*X:%*X %*d%n", &begin, &end,
                &perms, sizeof(perms), &offset, &pos) != 4) { // 4:scan size
                    continue;
            }
            g_stackMainStart = static_cast<uintptr_t>(begin);
            g_stackMainEnd = static_cast<uintptr_t>(end);
            ret = true;
        } else if (IsFilterLibrary(mapInfo, filterLibList)) {
            std::string lineStr = mapInfo;
            GetStandardLibraryRange(lineStr);
        }
    }
    if (fclose(fp) != 0) {
        printf("fclose failed.\n");
    }
    for (const auto& [soName, stdLibrary]: g_stdLib) {
        filterStaLibRange.emplace_back(stdLibrary.soBegin_, stdLibrary.soEnd_);
    }
    return ret;
}

void ParseEvent(const std::string& filePath, std::vector<std::pair<uint64_t, uint64_t>>& filterStaLibRange,
                const NameData& curRawData)
{
    if (curRawData.addr == nullptr) {
        return;
    }
    uint64_t soStart = reinterpret_cast<uint64_t>(curRawData.addr);
    uint64_t soEnd = soStart + static_cast<uint64_t>(curRawData.mallocSize);
    auto [iter, success] = g_stdLib.try_emplace(filePath, StandardLibrary(soStart, soEnd, filePath));
    if (!success) {
        if (iter->second.soBegin_ > soStart) {
            iter->second.soBegin_ = soStart;
        } else if (iter->second.soEnd_ < soEnd) {
            iter->second.soEnd_ = soEnd;
        }
        auto it = filterStaLibRange.rbegin();
        bool found = false;
        for (; it != filterStaLibRange.rend(); ++it) {
            if (it->first == iter->second.soBegin_) {
                    found = true;
                    break;
            }
        }
        if (found) {
            it->first = iter->second.soBegin_;
            it->second = iter->second.soEnd_;
        }
    } else {
        filterStaLibRange.emplace_back(iter->second.soBegin_, iter->second.soEnd_);
    }
}
