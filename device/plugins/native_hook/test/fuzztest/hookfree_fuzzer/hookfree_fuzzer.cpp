/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "hookfree_fuzzer.h"

namespace OHOS {
bool FuzzMallocHookFree(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr);
    ohos_malloc_hook_on_start(nullptr);
    if (p != nullptr) {
        ohos_malloc_hook_free(p);
    } else {
        return false;
    }

    return true;
}
} // namespace OHOS


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzMallocHookFree(data, size);
    return 0;
}