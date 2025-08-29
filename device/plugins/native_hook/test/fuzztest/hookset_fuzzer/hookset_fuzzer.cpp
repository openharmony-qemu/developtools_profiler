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
class TestResultRecorder {
private:
    mutable std::mutex mutex;  // 添加 mutable 关键字
    std::vector<bool> results;
    
public:
    void record_result(bool result) {
        std::lock_guard<std::mutex> lock(mutex);
        results.push_back(result);
    }
    
    bool get_last_result() const {
        std::lock_guard<std::mutex> lock(mutex);
        return results.empty() ? false : results.back();
    }
    
    size_t get_total_tests() const {
        std::lock_guard<std::mutex> lock(mutex);
        return results.size();
    }
};

static TestResultRecorder g_test_recorder;

size_t GetTestCount(const uint8_t* data, size_t size)
{
    if (size >= sizeof(size_t)) {
        size_t test_count;
        memcpy_s(&test_count, sizeof(test_count), data, sizeof(test_count));
        return test_count % 100; // litmit test count
    }
    return 10; // default test count
}

bool VerifyStateConsistency(bool expected_flag, bool actual_flag, bool old_flag, bool current_flag)
{
    bool result = true;
    result &= (actual_flag == expected_flag);
    result &= (old_flag == current_flag);
    return result;
}


void HookSetTests(size_t test_count, const uint8_t* data, size_t size)
{
    bool current_flag = ohos_malloc_hook_get_hook_flag();
    g_test_recorder.record_result(current_flag);

    for (size_t i = 0; i < test_count; ++i) {
        bool next_flag = (data[i % size] & 0x01) != 0;
        bool old_flag = ohos_malloc_hook_set_hook_flag(next_flag);
        bool retrieved_flag = ohos_malloc_hook_get_hook_flag();
        g_test_recorder.record_result(old_flag);
        g_test_recorder.record_result(retrieved_flag);
        if (!VerifyStateConsistency(next_flag, retrieved_flag, old_flag, current_flag)) {
            return;
        }
        current_flag = next_flag;
    }
}

void HookSetBoundaryTests()
{
    bool result1 = ohos_malloc_hook_set_hook_flag(true);
    bool result2 = ohos_malloc_hook_get_hook_flag();
    bool result3 = ohos_malloc_hook_set_hook_flag(false);
    bool result4 = ohos_malloc_hook_get_hook_flag();

    g_test_recorder.record_result(result1);
    g_test_recorder.record_result(result2);
    g_test_recorder.record_result(result3);
    g_test_recorder.record_result(result4);

    for (int i = 0; i < 5; ++i) {
        bool flag = ohos_malloc_hook_set_hook_flag(true);
        g_test_recorder.record_result(flag);
    }

    for (int i = 0; i < 5; ++i) {
        bool flag = ohos_malloc_hook_set_hook_flag(false);
        g_test_recorder.record_result(flag);
    }
}

void HookSetMultithreadedTests(size_t test_count)
{
    std::vector<std::thread> threads;
    const int thread_count = 4;
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([i, test_count]() {
            for (size_t j = 0; j < test_count; ++j) {
                bool flag = (j + i) % 2 == 0;
                bool old_flag = ohos_malloc_hook_set_hook_flag(flag);
                bool current_flag = ohos_malloc_hook_get_hook_flag();
                
                g_test_recorder.record_result(old_flag);
                g_test_recorder.record_result(current_flag);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

bool FuzzHookSetTests(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }

    size_t test_count = GetTestCount(data, size);
    HookSetTests(test_count, data, size);
    HookSetBoundaryTests();
    // HookSetMultithreadedTests(test_count);
    return g_test_recorder.get_last_result();
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzHookSetTests(data, size);
    return 0;
}