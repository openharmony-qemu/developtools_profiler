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
#include "hookinit_fuzzer.h"

namespace OHOS {
bool* GenerateRandomBoolPtr(const uint8_t* data, size_t size, size_t& offset)
{
    if (data == nullptr || size == 0) {
        return nullptr;
    }
    static bool static_bool = false;
    static bool static_bool_array[2] = {false, true};
    offset = (offset + sizeof(uint8_t)) % size;
    uint8_t choice = data[offset] % 3; // 0: nullptr, 1: &static_bool, 2: static_bool_array
    
    switch (choice) {
        case 0:
            return nullptr;
        case 1:
            return &static_bool;
        case 2:
            return &static_bool_array[0];
        default:
            return nullptr;
    }
}

const char* GenerateRandomString(const uint8_t* data, size_t size, size_t& offset)
{
    static std::vector<std::string> static_strings = {
        "",
        "default",
        "test_config",
        "malloc_hook_config",
        "fuzz_test_string"
    };
    offset = (offset + sizeof(uint8_t)) % size;
    size_t choice = data[offset] % static_strings.size();
    return static_strings[choice].c_str();
}

bool FuzzMallocHookInitialize(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    size_t offset = 0;
    bool* bool_ptr = GenerateRandomBoolPtr(data, size, offset);
    const char* char_ptr = GenerateRandomString(data, size, offset);
    bool result1 = ohos_malloc_hook_initialize(nullptr, bool_ptr, char_ptr);
    bool result2 = ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, bool_ptr, char_ptr);
    bool result3 = ohos_malloc_hook_initialize(nullptr, nullptr, "");
    std::string long_str(256, 'a');
    bool result4 = ohos_malloc_hook_initialize(nullptr, nullptr, long_str.c_str());
    bool result5 = ohos_malloc_hook_initialize(nullptr, nullptr, nullptr);
    return result1 || result2 || result3 || result4 || result5;
}
} // namespace OHOS


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzMallocHookInitialize(data, size);
    return 0;
}