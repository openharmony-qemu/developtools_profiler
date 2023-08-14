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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <thread>

#include "i_semaphore.h"

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
} // namespace