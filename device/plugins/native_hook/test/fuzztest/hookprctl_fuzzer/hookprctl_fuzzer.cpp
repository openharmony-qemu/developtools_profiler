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
#include "hookprctl_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
constexpr unsigned int WAIT_THREAD_TIME = 3;
unsigned long GenerateLong(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    auto argLong = provider.ConsumeIntegral<unsigned long>();
    return argLong;
}

std::vector<int> GetCommonPrctlOptions()
{
    return {
        PR_SET_NAME,           // 设置进程名称
        PR_GET_NAME,           // 获取进程名称
        PR_SET_DUMPABLE,       // 设置进程可转储
        PR_GET_DUMPABLE,       // 获取进程可转储状态
        PR_SET_PDEATHSIG,      // 设置父进程死亡信号
        PR_GET_PDEATHSIG,      // 获取父进程死亡信号
        PR_SET_SECCOMP,        // 设置seccomp模式
        PR_GET_SECCOMP,        // 获取seccomp模式
        PR_GET_TID_ADDRESS,    // 获取TID地址
        PR_SET_TIMERSLACK,     // 设置定时器松弛值
        PR_GET_TIMERSLACK,     // 获取定时器松弛值
        PR_SET_CHILD_SUBREAPER, // 设置子进程reaper
        PR_GET_CHILD_SUBREAPER  // 获取子进程reaper状态
    };
}

bool FuzzMallocHookPrctl(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    unsigned long arg2 = GenerateLong(data, size);
    unsigned long arg3 = GenerateLong(data, size);
    unsigned long arg4 = GenerateLong(data, size);
    unsigned long arg5 = GenerateLong(data, size);
    auto commonOptions = GetCommonPrctlOptions();
    int result1 = -1;
    ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr);
    ohos_malloc_hook_on_start(nullptr);
    void* mallocBlack = ohos_malloc_hook_malloc(1024); // 1024 malloc default value
    for (int opt : commonOptions) {
        result1 = ohos_malloc_hook_prctl(opt, arg2, arg3, arg4, arg5);
    }
    int result2 = ohos_malloc_hook_prctl(0, 0, 0, 0, 0);
    int result3 = ohos_malloc_hook_prctl(INT_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX);
    ohos_malloc_hook_free(mallocBlack);
    ohos_malloc_hook_on_end();
    sleep(WAIT_THREAD_TIME);
    return (result1 != -1) || (result2 != -1) || (result3 != -1);
}

} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzMallocHookPrctl(data, size);
    return 0;
}