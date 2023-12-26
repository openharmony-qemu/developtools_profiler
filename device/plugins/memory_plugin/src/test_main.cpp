/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "logging.h"

#pragma clang optimize off

namespace {
const size_t MB_PER_BYTE = 0x100000;
}

int main(int agrc, char* agrv[])
{
    std::vector<char*> cache;
    size_t size = 0;
    char *buf = nullptr;

    for (int i = 1; i < agrc; i++) {
        size = atoi(agrv[i]);
        if (size <= 0) {
            HILOG_INFO(LOG_CORE, "ready malloc size(%zu)Mb invalid", size);
            continue;
        }
        buf = (char *)malloc(size * MB_PER_BYTE);
        if (buf == nullptr) {
            const int bufferSize = 256;
            char buffer[bufferSize] = { 0 };
            strerror_r(errno, buffer, bufferSize);
            HILOG_ERROR(LOG_CORE, "malloc %zu fail, err(%s:%d)", size, buffer, errno);
            continue;
        }
        cache.emplace(cache.begin() + i - 1, buf);
    }
    while (true) {};
    return 0;
}

#pragma clang optimize on
