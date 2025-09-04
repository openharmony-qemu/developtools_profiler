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
#include "hookset_fuzzer.h"

namespace OHOS {
class HookSetFuzzer {
private:
    mutable std::mutex mutex;  // 添加 mutable 关键字
    std::vector<bool> results;
    
public:
    void RecordResult(bool result) {
        std::lock_guard<std::mutex> lock(mutex);
        results.push_back(result);
    }
    
    bool GetLastResult() const {
        std::lock_guard<std::mutex> lock(mutex);
        return results.empty() ? false : results.back();
    }
    
    size_t GetTotalTests() const {
        std::lock_guard<std::mutex> lock(mutex);
        return results.size();
    }
};

static HookSetFuzzer g_test_recorder;

size_t GetTestCount(const uint8_t* data, size_t size)
{
    if (size >= sizeof(size_t)) {
        size_t testCount;
        if (memcpy_s(&testCount, sizeof(testCount), data, sizeof(testCount) != EOK)) {
            return 10; // default test count
        }
        return testCount % 100; // litmit test count
    }
    return 10; // default test count
}

bool VerifyStateConsistency(bool expectedFlag, bool actualFlag, bool oldFlag, bool currentFlag)
{
    bool result = true;
    result &= (actualFlag == expectedFlag);
    result &= (oldFlag == currentFlag);
    return result;
}


void HookSetTests(size_t testCount, const uint8_t* data, size_t size)
{
    bool currentFlag = ohos_malloc_hook_get_hook_flag();
    g_test_recorder.RecordResult(currentFlag);

    for (size_t i = 0; i < testCount; ++i) {
        bool nextFlag = (data[i % size] & 0x01) != 0;
        bool oldFlag = ohos_malloc_hook_set_hook_flag(nextFlag);
        bool retrievedFlag = ohos_malloc_hook_get_hook_flag();
        g_test_recorder.RecordResult(oldFlag);
        g_test_recorder.RecordResult(retrievedFlag);
        if (!VerifyStateConsistency(nextFlag, retrievedFlag, oldFlag, currentFlag)) {
            return;
        }
        currentFlag = nextFlag;
    }
}

void HookSetBoundaryTests()
{
    bool result1 = ohos_malloc_hook_set_hook_flag(true);
    bool result2 = ohos_malloc_hook_get_hook_flag();
    bool result3 = ohos_malloc_hook_set_hook_flag(false);
    bool result4 = ohos_malloc_hook_get_hook_flag();

    g_test_recorder.RecordResult(result1);
    g_test_recorder.RecordResult(result2);
    g_test_recorder.RecordResult(result3);
    g_test_recorder.RecordResult(result4);

    for (int i = 0; i < 5; ++i) { // 5 times
        bool flag = ohos_malloc_hook_set_hook_flag(true);
        g_test_recorder.RecordResult(flag);
    }

    for (int i = 0; i < 5; ++i) { // 5 times
        bool flag = ohos_malloc_hook_set_hook_flag(false);
        g_test_recorder.RecordResult(flag);
    }
}

bool FuzzHookSetTests(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    size_t testCount = GetTestCount(data, size);
    HookSetTests(testCount, data, size);
    HookSetBoundaryTests();
    return g_test_recorder.GetLastResult();
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzHookSetTests(data, size);
    return 0;
}