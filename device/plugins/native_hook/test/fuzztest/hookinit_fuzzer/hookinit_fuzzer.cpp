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
    static bool staticBool = false;
    static bool staticBoolArray[2] = {false, true};
    offset = (offset + sizeof(uint8_t)) % size;
    uint8_t choice = data[offset] % 3; // 0: nullptr, 1: &staticBool, 2: staticBoolArray
    
    switch (choice) {
        case 0: // 0: nullptr
            return nullptr;
        case 1: // 1: &staticBool
            return &staticBool;
        case 2: // 2: staticBoolArray
            return &staticBoolArray[0];
        default:
            return nullptr;
    }
}

const char* GenerateRandomString(const uint8_t* data, size_t size, size_t& offset)
{
    static std::vector<std::string> staticStrings = {
        "",
        "default",
        "test_config",
        "malloc_hook_config",
        "fuzz_test_string"
    };
    offset = (offset + sizeof(uint8_t)) % size;
    size_t choice = data[offset] % staticStrings.size();
    return staticStrings[choice].c_str();
}

bool FuzzMallocHookInitialize(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    size_t offset = 0;
    bool* boolPtr = GenerateRandomBoolPtr(data, size, offset);
    const char* charPtr = GenerateRandomString(data, size, offset);
    bool result1 = ohos_malloc_hook_initialize(nullptr, boolPtr, charPtr);
    bool result2 = ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, boolPtr, charPtr);
    bool result3 = ohos_malloc_hook_initialize(nullptr, nullptr, "");
    std::string longStr(256, 'a'); // test input 256
    bool result4 = ohos_malloc_hook_initialize(nullptr, nullptr, longStr.c_str());
    bool result5 = ohos_malloc_hook_initialize(nullptr, nullptr, nullptr);
    return result1 || result2 || result3 || result4 || result5;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzMallocHookInitialize(data, size);
    return 0;
}