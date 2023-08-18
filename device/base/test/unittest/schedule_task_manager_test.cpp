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
#include <sys/time.h>
#include <thread>

#include "schedule_task_manager.h"

using namespace testing::ext;

namespace {
class ScheduleTaskManagerTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase()
    {
        ScheduleTaskManager::GetInstance().Shutdown();
    }
};

/**
 * @tc.name: base
 * @tc.desc: Single task processing.
 * @tc.type: FUNC
 */
HWTEST_F(ScheduleTaskManagerTest, ScheduleTaskOneshot, TestSize.Level1)
{
    std::atomic<int> count = 0;
    ScheduleTaskManager::ms initalDelay{10};

    ScheduleTaskManager scheduleTaskManager;
    EXPECT_TRUE(scheduleTaskManager.ScheduleTask(
        "task-1", [&]() { count++; }, ScheduleTaskManager::ms{0}, initalDelay));

    std::this_thread::sleep_for(initalDelay + initalDelay);
    EXPECT_EQ(count.load(), 1);

    scheduleTaskManager.Shutdown();
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

    ScheduleTaskManager::ms repeatInterval{100};
    ScheduleTaskManager::ms initalDelay{10};

    ScheduleTaskManager scheduleTaskManager;
    EXPECT_TRUE(scheduleTaskManager.ScheduleTask(
        "task-2",
        [&]() {
            count++;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
        },
        repeatInterval, initalDelay));

    int expected = 0;
    std::this_thread::sleep_for(initalDelay + initalDelay);
    for (int i = 0; i < cnt; i++) {
        expected++;
        struct timeval tv = { 0, 0 };
        gettimeofday(&tv, nullptr);
        EXPECT_EQ(count.load(), expected);
        std::this_thread::sleep_for(repeatInterval);
    }

    scheduleTaskManager.Shutdown();
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

    std::string taskName = "task-3";
    ScheduleTaskManager::ms repeatInterval{100};
    ScheduleTaskManager::ms initalDelay{10};

    ScheduleTaskManager scheduleTaskManager;
    EXPECT_TRUE(scheduleTaskManager.ScheduleTask(
        taskName,
        [&]() {
            count++;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
        },
        repeatInterval));

    int expected = 0;
    std::this_thread::sleep_for(initalDelay);
    for (int i = 0; i < cnt; i++) {
        std::this_thread::sleep_for(repeatInterval);
        expected++;
        struct timeval tv = { 0, 0 };
        gettimeofday(&tv, nullptr);
        EXPECT_EQ(count.load(), expected);
    }

    EXPECT_TRUE(scheduleTaskManager.UnscheduleTask(taskName));
    scheduleTaskManager.Shutdown();
}
} // namespace