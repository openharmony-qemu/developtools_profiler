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
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {

enum class PointerChoice {
    NULL_VALUE,           // 空指针
    SINGLE_VALUE,    // 单个值的指针
    ARRAY_VALUE,      // 数组指针
    COUNT
};

bool* GenerateRandomBoolPtr(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return nullptr;
    }
    static bool staticBool = false;
    static bool staticBoolArray[2] = {false, true};
    FuzzedDataProvider provider(data, size);
    uint8_t choice = provider.ConsumeIntegral<uint8_t>();
    
    switch (choice) {
        case static_cast<uint8_t>(PointerChoice::NULL_VALUE):
            return nullptr;
        case static_cast<uint8_t>(PointerChoice::SINGLE_VALUE):
            return &staticBool;
        case static_cast<uint8_t>(PointerChoice::ARRAY_VALUE):
            return &staticBoolArray[0];
        default:
            return nullptr;
    }
}

std::string GenerateRandomString(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "";
    }
    FuzzedDataProvider provider(data, size);
    return provider.ConsumeRandomLengthString();
}

bool FuzzMallocHookInitialize(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    bool* boolPtr = GenerateRandomBoolPtr(data, size);
    std::string charString = GenerateRandomString(data, size);
    bool result1 = ohos_malloc_hook_initialize(nullptr, boolPtr, charString.c_str());
    bool result2 = ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, boolPtr, charString.c_str());
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