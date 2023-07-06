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
#include "string_help.h"
#include <cstdio>
#include <memory.h>
#define UNUSED(expr)             \
    do {                         \
        static_cast<void>(expr); \
    } while (0)

#if !is_mingw
int32_t memcpy_s(void* dest, uint32_t destSize, const void* src, size_t srcSize)
{
    if (srcSize > destSize || src == nullptr || dest == nullptr) {
        return -1;
    } else {
        if (!memcpy(dest, src, srcSize)) {
            printf("memcpy fail\n");
            return -1;
        }
    }
    return 0;
}

int32_t sscanf_s(const char* buffer, const char* format, ...)
{
    va_list ap;
    __builtin_va_start(ap, format);
    int32_t ret = scanf(buffer, format, ap);
    __builtin_va_end(ap);
    return ret;
}

int32_t strncpy_s(char* strDest, size_t destMax, const char* strSrc, size_t count)
{
    (void*)strncpy(strDest, strSrc, destMax);
    return destMax;
}
#endif

void* memset_s(void* dest, size_t destSize, int32_t ch, size_t n)
{
    UNUSED(destSize);
    UNUSED(ch);
    return memset(dest, 0, n);
}

int32_t snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...)
{
    UNUSED(count);
    int32_t ret;
    va_list ap;
    __builtin_va_start(ap, format);
    ret = vsnprintf(strDest, destMax, format, ap);
    __builtin_va_end(ap);
    return ret;
}

int32_t sprintf_s(char* strDest, size_t destMax, const char* format, ...)
{
    va_list ap;
    __builtin_va_start(ap, format);
    int32_t ret = vsnprintf(strDest, destMax, format, ap);
    __builtin_va_end(ap);
    return ret;
}

const char* GetDemangleSymbolIndex(const char* mangled)
{
    int status = 0;
    auto demangle = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    if (status) { // status != 0 failed
        return mangled;
    } else {
        return demangle;
    }
}

int GetProcessorNumFromString(char* str)
{
    int processorNum = 0;
    int lastNum = -1;
    char* s = str;
    while (*s != '\0') {
        if (isdigit(*s)) {
            int currentNum = strtol(s, &s, 10);
            if (lastNum == -1) {
                processorNum++;
            } else {
                processorNum += currentNum - lastNum;
            }
            lastNum = currentNum;
        } else {
            if (*s == ',') {
                lastNum = -1;
            }
            s++;
        }
    }
    return processorNum;
}

bool EndWith(const std::string& str, const std::string& res)
{
    if (res.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - res.size(), res.size(), res) == 0;
}

std::vector<std::string> SplitStringToVec(const std::string& str, const std::string& pat)
{
    std::vector<std::string> result;
    int32_t curPos = 0;
    int32_t patPos = 0;
    int32_t strSize = str.size();
    int32_t patSize = pat.size();
    while (curPos < strSize) {
        patPos = str.find(pat, curPos);
        if (patPos == std::string::npos) {
            break;
        }
        result.emplace_back(str.substr(curPos, patPos - curPos));
        curPos = patPos + patSize;
    }
    if (curPos < strSize) {
        result.emplace_back(str.substr(curPos));
    }

    return result;
}
