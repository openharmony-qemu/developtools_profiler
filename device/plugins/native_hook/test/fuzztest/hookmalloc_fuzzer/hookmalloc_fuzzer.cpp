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
#include "hookmalloc_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
constexpr unsigned int WAIT_THREAD_TIME = 3;
constexpr size_t MAX_MALLOC_SIZE = 1024 * 1024 * 1024; // bigger may over rss litmit
bool FuzzMallocHookTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    FuzzedDataProvider provider(data, size);
    auto sizeHook = provider.ConsumeIntegral<size_t>() % MAX_MALLOC_SIZE;
    ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr);
    ohos_malloc_hook_on_start(nullptr);

    void* mallocBlack = ohos_malloc_hook_malloc(sizeHook);
    void* reallocBlack = ohos_malloc_hook_realloc(mallocBlack, sizeHook * 2);
    void* callocBlack = ohos_malloc_hook_calloc(sizeHook, sizeHook * 2);
    void* vallocBlack = ohos_malloc_hook_valloc(sizeHook);
    void* alignedAllocBlack = ohos_malloc_hook_aligned_alloc(sizeHook, sizeHook * 2);

    ohos_malloc_hook_free(alignedAllocBlack);
    ohos_malloc_hook_free(vallocBlack);
    ohos_malloc_hook_free(callocBlack);
    ohos_malloc_hook_free(reallocBlack);
    
    ohos_malloc_hook_on_end();
    sleep(WAIT_THREAD_TIME);
    
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzMallocHookTest(data, size);
    return 0;
}