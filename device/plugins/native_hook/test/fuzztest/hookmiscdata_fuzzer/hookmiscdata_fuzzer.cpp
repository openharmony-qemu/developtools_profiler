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
#include "hookmiscdata_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
enum class MiscType : uint32_t {
    JS_STACK_DATA = 1,
    OTHER_TYPE = 1000,
};

enum class PointerChoice {
    NULL_CHOICE = 1,
    SINGLE_VALUE,
    EMPTY_STRING,
    ALIGNED_BUFFER,
    COUNT
};

enum class StackChoice {
    JS_STACK_DATA = 0,
    OTHER_TYPE,
    EDGE_VALUE,
    MAX_VALUE,
    COUNT
};

constexpr size_t DEFAULT_BUFFER_SIZE = 64;
constexpr size_t DEFAULT_ALIGNED_SIZE = 8;
constexpr size_t MAX_STACK_SIZE = 1024 * 1024;

uint64_t GenerateRandomId(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    return provider.ConsumeIntegral<uint64_t>();
}

const char* GenerateRandomStackPtr(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    uint8_t choice = provider.ConsumeIntegral<uint8_t>();
    
    switch (choice) {
        case static_cast<uint8_t>(PointerChoice::NULL_CHOICE):
            return nullptr;
        case static_cast<uint8_t>(PointerChoice::SINGLE_VALUE): {
            static char buffer[DEFAULT_BUFFER_SIZE];
            return buffer;
        }
        case static_cast<uint8_t>(PointerChoice::EMPTY_STRING):
            return "";
        case static_cast<uint8_t>(PointerChoice::ALIGNED_BUFFER): {
            static char alignedBuffer[DEFAULT_BUFFER_SIZE] __attribute__((aligned(DEFAULT_ALIGNED_SIZE)));
            return alignedBuffer;
        }
        default:
            return nullptr;
    }
}

size_t GenerateRandomStackSize(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    size_t stackSize = provider.ConsumeIntegral<size_t>();
    if (stackSize > MAX_STACK_SIZE) {
        stackSize = MAX_STACK_SIZE;
    }
    
    return stackSize;
}

uint32_t GenerateRandomType(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    uint8_t choiceRaw = provider.ConsumeIntegral<uint8_t>();
    uint8_t choice = choiceRaw % static_cast<uint8_t>(StackChoice::COUNT);
    
    switch (choice) {
        case static_cast<uint8_t>(StackChoice::JS_STACK_DATA):
            return static_cast<uint32_t>(MiscType::JS_STACK_DATA);
        case static_cast<uint8_t>(StackChoice::OTHER_TYPE):
            return static_cast<uint32_t>(MiscType::OTHER_TYPE);
        case static_cast<uint8_t>(StackChoice::EDGE_VALUE):
            return 0xFFFFFFFF;
        case static_cast<uint8_t>(StackChoice::MAX_VALUE):
            return 0;
        default:
            return static_cast<uint32_t>(MiscType::JS_STACK_DATA);
    }
}

bool FuzzMallocHookSendHookMiscData(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint64_t) + sizeof(size_t) + sizeof(uint32_t)) {
        return false;
    }
    uint64_t id = GenerateRandomId(data, size);
    const char* stackPtr = GenerateRandomStackPtr(data, size);
    size_t stackSize = GenerateRandomStackSize(data, size);
    uint32_t type = GenerateRandomType(data, size);
    bool result1 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, stackSize, type);
    bool result2 = ohos_malloc_hook_send_hook_misc_data(
        id, stackPtr, stackSize, static_cast<uint32_t>(MiscType::JS_STACK_DATA));
    bool result3 = ohos_malloc_hook_send_hook_misc_data(id, nullptr, 0, type);
    bool result4 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, MAX_STACK_SIZE, type);
    bool result5 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, stackSize, 0xFFFFFFFF);
    return result1 || result2 || result3 || result4 || result5;
}

} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzMallocHookSendHookMiscData(data, size);
    return 0;
}