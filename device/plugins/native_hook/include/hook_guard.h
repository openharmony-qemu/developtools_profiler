/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#ifndef HOOK_GUARD_H
#define HOOK_GUARD_H

#include <cstddef>
#include <cstdint>
#include <pthread.h>

namespace HookGuard {
    bool IsReady();
    bool IsPidChanged();
    bool ShouldSkipMalloc();
    bool ShouldSkipMmap();
    bool ShouldSkipMemtrace();
    bool ShouldFilterBySize(void* ptr, size_t size);
    bool ShouldSample(size_t size, pthread_key_t& sampleKey);
    int CalculateRealSize(int fpStackDepth, bool isAsyncStack = false);
    uint16_t GetPredefinedTagId(unsigned long long mask);
    bool CheckRestraceConditions(unsigned long long combineVal, size_t size);
}

#endif  // HOOK_GUARD_H
