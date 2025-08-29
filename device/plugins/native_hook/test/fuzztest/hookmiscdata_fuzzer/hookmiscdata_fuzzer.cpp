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

namespace OHOS {
enum class MISC_TYPE : uint32_t {
    JS_STACK_DATA = 1,
    OTHER_TYPE = 1000,
};
uint64_t GenerateRandomId(const uint8_t* data, size_t size, size_t& offset)
{
    offset = (offset + sizeof(uint64_t)) % size;
    
    uint64_t id;
    memcpy_s(&id, sizeof(id), data + offset, sizeof(id));
    return id;
}

const char* GenerateRandomStackPtr(const uint8_t* data, size_t size, size_t& offset)
{
    offset = (offset + sizeof(uint8_t)) % size;
    uint8_t choice = data[offset] % 4; // 0: nullptr, 1: 有效指针, 2: 空字符串, 3: 特殊值
    
    switch (choice) {
        case 0:
            return nullptr;
        case 1: {
            static char buffer[64];
            return buffer;
        }
        case 2:
            return "";
        case 3: {
            static char aligned_buffer[64] __attribute__((aligned(8)));
            return aligned_buffer;
        }
        default:
            return nullptr;
    }
}

size_t GenerateRandomStackSize(const uint8_t* data, size_t size, size_t& offset)
{
    offset = (offset + sizeof(size_t)) % size;
    
    size_t stackSize;
    memcpy_s(&stackSize, sizeof(stackSize), data + offset, sizeof(stackSize));
    
    if (stackSize > 1024 * 1024) {
        stackSize = 1024 * 1024;
    }
    
    return stackSize;
}

uint32_t GenerateRandomType(const uint8_t* data, size_t size, size_t& offset)
{
    offset = (offset + sizeof(uint8_t)) % size;
    uint8_t choice = data[offset] % 4; // 0: JS_STACK_DATA, 1: 其他类型, 2: 边界值, 3: 最大值
    
    switch (choice) {
        case 0:
            return static_cast<uint32_t>(MISC_TYPE::JS_STACK_DATA);
        case 1:
            return static_cast<uint32_t>(MISC_TYPE::OTHER_TYPE);
        case 2:
            return 0xFFFFFFFF;
        case 3:
            return 0;
        default:
            return static_cast<uint32_t>(MISC_TYPE::JS_STACK_DATA);
    }
}

bool FuzzMallocHookSendHookMiscData(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint64_t) + sizeof(size_t) + sizeof(uint32_t)) {
        return false;
    }
    size_t offset = 0;
    uint64_t id = GenerateRandomId(data, size, offset);
    const char* stackPtr = GenerateRandomStackPtr(data, size, offset);
    size_t stackSize = GenerateRandomStackSize(data, size, offset);
    uint32_t type = GenerateRandomType(data, size, offset);
    bool result1 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, stackSize, type);
    bool result2 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, stackSize, 
                                                       static_cast<uint32_t>(MISC_TYPE::JS_STACK_DATA));
    bool result3 = ohos_malloc_hook_send_hook_misc_data(id, nullptr, 0, type);
    bool result4 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, 1024 * 1024, type);
    bool result5 = ohos_malloc_hook_send_hook_misc_data(id, stackPtr, stackSize, 0xFFFFFFFF);
    return result1 || result2 || result3 || result4 || result5;
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzMallocHookSendHookMiscData(data, size);
    return 0;
}