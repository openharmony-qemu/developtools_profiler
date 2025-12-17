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

#include "gtest/gtest.h"
#include <thread>
#include "stack_data_repeater_test.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

const int TEST_MAX_SIZE = 10;
class StackDataRepeaterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        maxSize = TEST_MAX_SIZE;
        stackDataRepeater = std::make_shared<StackDataRepeater>(maxSize);
    }

    void TearDown() override
    {
        stackDataRepeater->Close();
    }

    std::shared_ptr<HookRecord> CreateHookRecord(uint32_t stackSize)
    {
        auto rawStack = std::make_shared<RawStack>();
        rawStack->stackContext = new BaseStackRawData();
        rawStack->stackContext->type = MALLOC_MSG;
        rawStack->stackContext->addr = reinterpret_cast<void*>(0xED);
        rawStack->stackSize = stackSize;
        rawStack->fpDepth = 10; // 10 : fp depth
        rawStack->reportFlag = true;
        rawStack->reduceStackFlag = true;
        return std::make_shared<HookRecord>(rawStack);
    }

    size_t maxSize;
    StackDataRepeaterPtr stackDataRepeater;
};

/*
@tc.name: StackDataRepeaterTest001
@tc.desc: test put raw stack with nullptr and non-empty queue.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest001, TestSize.Level1)
{
    ASSERT_TRUE(stackDataRepeater->PutRawStack(nullptr, true));
}

/*
@tc.name: StackDataRepeaterTest002
@tc.desc: test put raw stack with nullptr and empty queue.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest002, TestSize.Level1)
{
    stackDataRepeater->Close();
    ASSERT_FALSE(stackDataRepeater->PutRawStack(nullptr, true));
}

/*
@tc.name: StackDataRepeaterTest003
@tc.desc: test put raw stack with HookRecord and accurate record.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest003, TestSize.Level1)
{
    auto hookRecord = std::make_shared<HookRecord>();
    ASSERT_TRUE(stackDataRepeater->PutRawStack(hookRecord, true));
}

/*
@tc.name: StackDataRepeaterTest004
@tc.desc: test put raw stack with HookRecord and unaccurate record.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest004, TestSize.Level1)
{
    auto hookRecord = std::make_shared<HookRecord>();
    ASSERT_TRUE(stackDataRepeater->PutRawStack(hookRecord, false));
}

/*
@tc.name: StackDataRepeaterTest005
@tc.desc: test put raw stack with non-empty array and non-empty queue.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest005, TestSize.Level1)
{
    std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
    ASSERT_TRUE(stackDataRepeater->PutRawStackArray(rawDataArray, 0));
}

/*
@tc.name: StackDataRepeaterTest006
@tc.desc: test put raw stack with non-empty array and non-empty queue.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest006, TestSize.Level1)
{
    std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
    rawDataArray[0] = std::make_shared<HookRecord>();
    ASSERT_TRUE(stackDataRepeater->PutRawStackArray(rawDataArray, 1));
}

/*
@tc.name: StackDataRepeaterTest007
@tc.desc: test TakeRawData with timeout.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest007, TestSize.Level1)
{
    bool isTimeOut = false;
    HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE];
    ASSERT_EQ(stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, 1, batchRawStack, 1, isTimeOut), nullptr);
    ASSERT_TRUE(isTimeOut);
}

/*
@tc.name: StackDataRepeaterTest008
@tc.desc: test TakeRawData with non-timeout.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest008, TestSize.Level1)
{
    bool isTimeOut = false;
    HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE];
    auto hookRecord = std::make_shared<HookRecord>();
    stackDataRepeater->PutRawStack(hookRecord, true);
    ASSERT_NE(stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, 1, batchRawStack, 1, isTimeOut), nullptr);
    ASSERT_FALSE(isTimeOut);
}

/*
@tc.name: StackDataRepeaterTest009
@tc.desc: test Close method.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest009, TestSize.Level1)
{
    stackDataRepeater->Close();
    ASSERT_EQ(stackDataRepeater->Size(), 0);
}

/*
@tc.name: StackDataRepeaterTest010
@tc.desc: test Reset method
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest010, TestSize.Level1)
{
    stackDataRepeater->Close();
    stackDataRepeater->Reset();
    ASSERT_FALSE(stackDataRepeater->closed_);
}

/*
@tc.name: StackDataRepeaterTest011
@tc.desc: test Size method
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest011, TestSize.Level1)
{
    auto hookRecord = std::make_shared<HookRecord>();
    stackDataRepeater->PutRawStack(hookRecord, true);
    ASSERT_EQ(stackDataRepeater->Size(), 1);
}

/*
@tc.name: StackDataRepeaterTest012
@tc.desc: test PutRawStackArray with empty rawDataArray and non-empty rawDataQueue.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest012, TestSize.Level1)
{
    auto hookRecord = std::make_shared<HookRecord>();
    stackDataRepeater->PutRawStack(hookRecord, true);
    std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
    ASSERT_TRUE(stackDataRepeater->PutRawStackArray(rawDataArray, 0));
}

/*
@tc.name: StackDataRepeaterTest013
@tc.desc: test PutRawStackArray with rawDataQueue size equal to maxSize using multithreading.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest013, TestSize.Level1)
{
    for (size_t i = 0; i < maxSize; ++i) {
        auto hookRecord = std::make_shared<HookRecord>();
        stackDataRepeater->PutRawStack(hookRecord, true);
    }
    std::thread takeThread([stackDataRepeater = this->stackDataRepeater]() {
        bool isTimeOut = false;
        HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE];
        ASSERT_NE(stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, 1, batchRawStack, 1, isTimeOut), nullptr);
        ASSERT_FALSE(isTimeOut);
    });
    std::thread putThread([stackDataRepeater = this->stackDataRepeater]() {
        std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
        rawDataArray[0] = std::make_shared<HookRecord>();
        ASSERT_TRUE(stackDataRepeater->PutRawStackArray(rawDataArray, 1));
    });
    takeThread.join();
    putThread.join();
    ASSERT_EQ(stackDataRepeater->Size(), maxSize);
}

/*
@tc.name: StackDataRepeaterTest014
@tc.desc: test PutRawStackArray with closed StackDataRepeater.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest014, TestSize.Level1)
{
    stackDataRepeater->Close();
    std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
    ASSERT_FALSE(stackDataRepeater->PutRawStackArray(rawDataArray, 0));
}

/*
@tc.name: StackDataRepeaterTest015
@tc.desc: test PutRawStack matching malloc and free records.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest015, TestSize.Level1)
{
    auto rawStackMalloc = std::make_shared<RawStack>();
    rawStackMalloc->stackContext = new BaseStackRawData();
    rawStackMalloc->stackContext->type = MALLOC_MSG;
    rawStackMalloc->stackContext->addr = reinterpret_cast<void*>(0x1234);
    auto mallocRecord = std::make_shared<HookRecord>(rawStackMalloc);
    stackDataRepeater->PutRawStack(mallocRecord, true);

    auto rawStackFree = std::make_shared<RawStack>();
    rawStackFree->stackContext = new BaseStackRawData();
    rawStackFree->stackContext->type = FREE_MSG;
    rawStackFree->stackContext->addr = reinterpret_cast<void*>(0x1234);
    auto freeRecord = std::make_shared<HookRecord>(rawStackFree);
    ASSERT_TRUE(stackDataRepeater->PutRawStack(freeRecord, true));
}

/*
@tc.name: StackDataRepeaterTest016
@tc.desc: test TaskeRawData with MALLOC_MSG hookData.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest016, TestSize.Level1)
{
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1234);
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    stackDataRepeater->PutRawStack(hookRecord, true);

    bool isTimeOut = false;
    HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE];
    ASSERT_NE(stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, 1, batchRawStack, 1, isTimeOut), nullptr);
    ASSERT_FALSE(isTimeOut);
}

/*
@tc.name: StackDataRepeaterTest017
@tc.desc: check the correctness of the TakeRawData method.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest017, TestSize.Level1)
{
    auto rawStack = std::make_shared<RawStack>();
    auto baseStack = std::make_shared<BaseStackRawData>();
    rawStack->stackContext = baseStack.get();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0xED);
    rawStack->stackSize = 1024; // 1024 : stack size
    rawStack->fpDepth = 10; // 10 : fp depth
    rawStack->reportFlag = true;
    rawStack->reduceStackFlag = true;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    stackDataRepeater->PutRawStack(hookRecord, true);

    bool isTimeOut = false;
    HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE];
    ASSERT_NE(stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, 1, batchRawStack, 1, isTimeOut), nullptr);
    ASSERT_NE(batchRawStack[0], nullptr);
    EXPECT_EQ(batchRawStack[0]->GetRawStack()->stackContext->type, MALLOC_MSG);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(batchRawStack[0]->GetRawStack()->stackContext->addr), 0xED);
    EXPECT_EQ(batchRawStack[0]->GetRawStack()->stackSize, 1024);
    EXPECT_EQ(batchRawStack[0]->GetRawStack()->fpDepth, 10);
    EXPECT_EQ(batchRawStack[0]->GetRawStack()->reportFlag, true);
    EXPECT_EQ(batchRawStack[0]->GetRawStack()->reduceStackFlag, true);
}

/*
@tc.name: StackDataRepeaterTest018
@tc.desc: check the correctness int one thread.
@tc.type: FUNC
*/
HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest018, TestSize.Level1)
{
    std::thread putThread([this]() {
        std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
        for (uint32_t i = 0; i < CACHE_ARRAY_SIZE; i++) {
            rawDataArray[i] = CreateHookRecord(1024 + i);
        }
        ASSERT_TRUE(stackDataRepeater->PutRawStackArray(rawDataArray, CACHE_ARRAY_SIZE));
    });

    std::thread takeThread([this]() {
        bool isTimeOut = false;
        HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE] = {nullptr};
        ASSERT_NE(stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, CACHE_ARRAY_SIZE, batchRawStack, 1, isTimeOut),
                  nullptr);
        ASSERT_FALSE(isTimeOut);
        for (size_t i = 0; i < CACHE_ARRAY_SIZE; i++) {
            EXPECT_TRUE(batchRawStack[i]->IsValid());
            EXPECT_EQ(batchRawStack[i]->GetRawStack()->stackContext->type, MALLOC_MSG);
            EXPECT_EQ(reinterpret_cast<uintptr_t>(batchRawStack[i]->GetRawStack()->stackContext->addr), 0xED);
            EXPECT_EQ(batchRawStack[i]->GetRawStack()->stackSize, 1024 + i);
            EXPECT_EQ(batchRawStack[i]->GetRawStack()->fpDepth, 10);
            EXPECT_EQ(batchRawStack[i]->GetRawStack()->reportFlag, true);
            EXPECT_EQ(batchRawStack[i]->GetRawStack()->reduceStackFlag, true);
            delete batchRawStack[i]->GetRawStack()->stackContext;
        }
    });
    takeThread.join();
    putThread.join();
}

/*
@tc.name: StackDataRepeaterTest019
@tc.desc: test put and take stack data int three threads
@tc.type: FUNC
*/

HWTEST_F(StackDataRepeaterTest, StackDataRepeaterTest019, TestSize.Level1)
{
    std::vector<std::thread> putThreads;
    for (size_t i = 0; i < 3; i++) { // 3: thread number
        putThreads.emplace_back([this]() {
            std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> rawDataArray;
            for (uint32_t i = 0; i < CACHE_ARRAY_SIZE; i++) {
                rawDataArray[i] = CreateHookRecord(1024 + i);
            }
            ASSERT_TRUE(stackDataRepeater->PutRawStackArray(rawDataArray, CACHE_ARRAY_SIZE));
        });
    }
    std::vector<std::thread> takeThreads;
    for (size_t i = 0; i < 3; i++) { // 3: thread number
        takeThreads.emplace_back([this]() {
            bool isTimeOut = false;
            HookRecordPtr batchRawStack[CACHE_ARRAY_SIZE] = {nullptr};
            ASSERT_NE(
                stackDataRepeater->TakeRawData(100, CLOCK_REALTIME, CACHE_ARRAY_SIZE, batchRawStack, 1, isTimeOut),
                nullptr);
            ASSERT_FALSE(isTimeOut);
            for (size_t i = 0; i < CACHE_ARRAY_SIZE; i++) {
                EXPECT_TRUE(batchRawStack[i]->IsValid());
                EXPECT_EQ(batchRawStack[i]->GetRawStack()->stackContext->type, MALLOC_MSG);
                EXPECT_EQ(reinterpret_cast<uintptr_t>(batchRawStack[i]->GetRawStack()->stackContext->addr), 0xED);
                EXPECT_EQ(batchRawStack[i]->GetRawStack()->stackSize, 1024 + i);
                EXPECT_EQ(batchRawStack[i]->GetRawStack()->fpDepth, 10);
                EXPECT_EQ(batchRawStack[i]->GetRawStack()->reportFlag, true);
                EXPECT_EQ(batchRawStack[i]->GetRawStack()->reduceStackFlag, true);
                delete batchRawStack[i]->GetRawStack()->stackContext;
            }
        });
    }
    for (auto& thread : putThreads) {
        thread.join();
    }
    for (auto& thread : takeThreads) {
        thread.join();
    }
}