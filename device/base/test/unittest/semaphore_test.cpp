/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include "i_semaphore.h"
#include "posix_semaphore.h"

namespace {
using namespace testing::ext;

class SemaphoreTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: CtorDtor.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, CtorDtor, TestSize.Level1)
{
    auto semaphore = GetSemaphoreFactory().Create(1);
    EXPECT_NE(semaphore, nullptr);
    EXPECT_TRUE(semaphore->TryWait());
    EXPECT_FALSE(semaphore->TimedWait(1, 0));
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: Wait.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, Wait, TestSize.Level1)
{
    auto semaphore = GetSemaphoreFactory().Create(1);
    ASSERT_NE(semaphore, nullptr);
    EXPECT_TRUE(semaphore->Wait());
    EXPECT_FALSE(semaphore->TryWait());
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: Post.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, Post, TestSize.Level1)
{
    auto semaphore = GetSemaphoreFactory().Create(0);
    ASSERT_NE(semaphore, nullptr);
    EXPECT_TRUE(semaphore->Post());
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: Post.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, WaitPost, TestSize.Level1)
{
    auto readySem = GetSemaphoreFactory().Create(0);
    auto finishSem = GetSemaphoreFactory().Create(0);
    ASSERT_NE(readySem, nullptr);
    ASSERT_NE(finishSem, nullptr);

    auto done = std::make_shared<bool>(false);
    ASSERT_NE(done, nullptr);

    std::thread bgThread([=]() {
        readySem->Wait();
        *done = true;
        finishSem->Post();
    });

    EXPECT_TRUE(readySem->Post());
    EXPECT_TRUE(finishSem->Wait());
    EXPECT_TRUE(*done);

    bgThread.join();
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: CreatePosixSemaphoreFactory.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, CreatePosixSemaphoreFactory, TestSize.Level1)
{
    auto semaphore = GetSemaphoreFactory(POSIX_SEMAPHORE_FACTORY).Create(0);
    EXPECT_NE(semaphore, nullptr);
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: CreatePtheadSemaphoreFactory.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, CreatePtheadSemaphoreFactory, TestSize.Level1)
{
    auto semaphore = GetSemaphoreFactory(PTHREAD_SEMAPHORE_FACTORY).Create(0);
    EXPECT_NE(semaphore, nullptr);
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: test MultiplePost function
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, MultiplePost, TestSize.Level1)
{
    auto semaphore = PosixSemaphoreFactory().Create(0);
    ASSERT_NE(semaphore, nullptr);
    EXPECT_TRUE(semaphore->Post());
    EXPECT_TRUE(semaphore->Post());
    EXPECT_TRUE(semaphore->Post());
    EXPECT_EQ(semaphore->Value(), 3);
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: test MultiThreadWait function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, MultiThreadWait, TestSize.Level1)
{
    auto semaphore = PosixSemaphoreFactory().Create(1);
    ASSERT_NE(semaphore, nullptr);
    std::vector<std::thread> threads;
    const int threadCount = 5;
    std::atomic<int> counter(0);

    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([=, &counter]() {
            semaphore->Wait();
            ++counter;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            semaphore->Post();
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(counter.load(), 5);
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: test TryWaitZero function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, TryWaitZero, TestSize.Level1)
{
    auto semaphore = PosixSemaphoreFactory().Create(0);
    EXPECT_NE(semaphore, nullptr);
    EXPECT_FALSE(semaphore->TryWait());
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: test TimeWaitTimeout function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, TimeWaitTimeout, TestSize.Level1)
{
    auto semaphore = PosixSemaphoreFactory().Create(0);
    EXPECT_NE(semaphore, nullptr);
    EXPECT_FALSE(semaphore->TimedWait(0, 0));
}

/**
 * @tc.name: SemaphoreTest
 * @tc.desc: test Value function.
 * @tc.type: FUNC
 */
HWTEST_F(SemaphoreTest, Value, TestSize.Level1)
{
    auto semaphore = PosixSemaphoreFactory().Create(5);
    EXPECT_NE(semaphore, nullptr);
    EXPECT_EQ(semaphore->Value(), 5);
    semaphore->Wait();
    EXPECT_EQ(semaphore->Value(), 4);
    semaphore->Post();
    EXPECT_EQ(semaphore->Value(), 5);
}
} // namespace