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

#include "hook_so.h"

#pragma clang optimize off

namespace {
constexpr int DATA_SIZE = 200;
using StaticSpace = struct {
    int data[DATA_SIZE];
};
}

char *DepthMallocSo(int depth, int mallocSize)
{
    if (mallocSize <= 0) {
        return nullptr;
    }
    StaticSpace staticeData;
    if (depth == 0) {
        staticeData.data[0] = 1;
        return new char[mallocSize];
    }
    return (DepthMallocSo(depth - 1, mallocSize));
}

void DepthFreeSo(int depth, char *p)
{
    StaticSpace staticeData;
    if (depth == 0) {
        staticeData.data[0] = 1;
        delete []p;
        return;
    }
    return (DepthFreeSo(depth - 1, p));
}

#pragma clang optimize on