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
#include <sys/time.h>
#include <thread>

#include "schedule_task_manager.h"

using namespace testing::ext;

namespace {
class ScheduleTaskManagerTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: base
 * @tc.desc: Single task processing.
 * @tc.type: FUNC
 */
HWTEST_F(ScheduleTaskManagerTest, ScheduleTaskOneshot, TestSize.Level1)
{
    std::atomic<int> count = 0;
    uint64_t initalDelay = 10; // 10ms

    ScheduleTaskManager scheduleTaskManager;
    EXPECT_NE(scheduleTaskManager.ScheduleTask([&]() { count++; }, initalDelay, true), -1);

    std::this_thread::sleep_for(std::chrono::milliseconds(initalDelay + initalDelay));
    EXPECT_EQ(count.load(), 1);
}

/**
 * @tc.name: base
 * @tc.desc: Repetitive task processing.
 * @tc.type: FUNC
 */
HWTEST_F(ScheduleTaskManagerTest, ScheduleTaskRepeated, TestSize.Level1)
{
    std::atomic<int> count = 0;
    constexpr int cnt = 5;
    constexpr int thresh = 1;

    uint64_t repeatInterval = 100;

    ScheduleTaskManager scheduleTaskManager;
    EXPECT_NE(scheduleTaskManager.ScheduleTask(
        [&]() {
            count++;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
        },
    repeatInterval), -1);

    int expected = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    for (int i = 0; i < cnt; i++) {
        expected++;
        struct timeval tv = { 0, 0 };
        gettimeofday(&tv, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(repeatInterval));
    }
    EXPECT_LE(abs(count.load() - expected), thresh);
}

/**
 * @tc.name: base
 * @tc.desc: Unschedule Task.
 * @tc.type: FUNC
 */
HWTEST_F(ScheduleTaskManagerTest, UnscheduleTask, TestSize.Level1)
{
    std::atomic<int> count = 0;
    constexpr int cnt = 5;
    constexpr int thresh = 1;
    int32_t taskFd = -1;
    uint64_t repeatInterval = 100;
    uint64_t initalDelay = 10;

    ScheduleTaskManager scheduleTaskManager;
    taskFd = scheduleTaskManager.ScheduleTask(
        [&]() {
            count++;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
        },
        repeatInterval);
    EXPECT_NE(taskFd, -1);

    int expected = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(initalDelay));
    for (int i = 0; i < cnt; i++) {
        expected++;
        struct timeval tv = { 0, 0 };
        gettimeofday(&tv, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(repeatInterval));
    }
    EXPECT_LE(abs(count.load() - expected), thresh);
    EXPECT_TRUE(scheduleTaskManager.UnscheduleTask(taskFd));
}
} // namespace