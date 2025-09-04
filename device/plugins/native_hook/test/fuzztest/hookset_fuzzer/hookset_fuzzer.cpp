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
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
constexpr size_t LIMITED_TEST_COUNT_DIV = 100;
class HookSetFuzzer {
public:
    void RecordResult(bool result)
    {
        std::lock_guard<std::mutex> lock(mutex);
        results.push_back(result);
    }
    
    bool GetLastResult() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return results.empty() ? false : results.back();
    }
    
    size_t GetTotalTests() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return results.size();
    }
private:
    mutable std::mutex mutex;
    std::vector<bool> results;
};

static HookSetFuzzer g_testRecorder;

size_t GetTestCount(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    size_t testCount = provider.ConsumeIntegral<size_t>();
    return testCount % LIMITED_TEST_COUNT_DIV; // litmit test count
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
    FuzzedDataProvider provider(data, size);
    bool currentFlag = ohos_malloc_hook_get_hook_flag();
    g_testRecorder.RecordResult(currentFlag);

    for (size_t i = 0; i < testCount; ++i) { 
        bool nextFlag = provider.ConsumeBoolean();
        bool oldFlag = ohos_malloc_hook_set_hook_flag(nextFlag);
        bool retrievedFlag = ohos_malloc_hook_get_hook_flag();
        g_testRecorder.RecordResult(oldFlag);
        g_testRecorder.RecordResult(retrievedFlag);
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

    g_testRecorder.RecordResult(result1);
    g_testRecorder.RecordResult(result2);
    g_testRecorder.RecordResult(result3);
    g_testRecorder.RecordResult(result4);

    for (int i = 0; i < FUZZ_DEFAULT_TEST_COUNT; ++i) {
        bool flag = ohos_malloc_hook_set_hook_flag(true);
        g_testRecorder.RecordResult(flag);
    }

    for (int i = 0; i < FUZZ_DEFAULT_TEST_COUNT; ++i) {
        bool flag = ohos_malloc_hook_set_hook_flag(false);
        g_testRecorder.RecordResult(flag);
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
    return g_testRecorder.GetLastResult();
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzHookSetTests(data, size);
    return 0;
}