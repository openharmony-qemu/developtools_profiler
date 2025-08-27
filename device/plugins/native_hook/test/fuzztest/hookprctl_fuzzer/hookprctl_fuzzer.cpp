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


namespace OHOS {
int GenerateRandomOption(const uint8_t* data, size_t size, size_t& offset)
{
    offset = (offset + sizeof(int)) % size;
    
    int option;
    memcpy_s(&option, sizeof(option), data + offset, sizeof(option));
    if (option < 0 || option > 0x10000) { // 0x1000 cover all value
        option = PR_SET_NAME;
    }
    
    return option;
}

unsigned long GenerateRandomArg(const uint8_t* data, size_t size, size_t& offset)
{
    offset = (offset + sizeof(unsigned long)) % size;
    
    unsigned long arg;
    if (memcpy_s(&arg, sizeof(arg), data + offset, sizeof(arg)) != EOK) {
        return 0;
    }
    return arg;
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
    if (data == nullptr || size < sizeof(int) + 4 * sizeof(unsigned long)) {
        return false;
    }
    size_t offset = 0;
    int option = GenerateRandomOption(data, size, offset);
    unsigned long arg2 = GenerateRandomArg(data, size, offset);
    unsigned long arg3 = GenerateRandomArg(data, size, offset);
    unsigned long arg4 = GenerateRandomArg(data, size, offset);
    unsigned long arg5 = GenerateRandomArg(data, size, offset);
    auto common_options = GetCommonPrctlOptions();
    int result1 = ohos_malloc_hook_prctl(option, arg2, arg3, arg4, arg5);
    for (int opt : common_options) {
        int result = ohos_malloc_hook_prctl(opt, arg2, arg3, arg4, arg5);
        (void)result;
    }
    int result2 = ohos_malloc_hook_prctl(0, 0, 0, 0, 0);
    int result3 = ohos_malloc_hook_prctl(INT_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX);
    int result4 = ohos_malloc_hook_prctl(PR_SET_NAME, (unsigned long)"test_process", 0, 0, 0);
    return (result1 != -1) || (result2 != -1) || (result3 != -1) || (result4 != -1);
}


} // namespace OHOS


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzMallocHookPrctl(data, size);
    return 0;
}