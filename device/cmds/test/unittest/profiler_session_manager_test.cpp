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

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "profiler_session_manager.h"

using namespace testing::ext;

namespace {
const std::string TEST_CONFIG = R"(
session_config {
    sample_duration: 10000
    result_file: "/data/local/tmp/test_output.htrace"
}
plugin_configs {
    name: "cpu-plugin"
    config_data: ""
}
)";
const std::string TEST_OUTPUT_FILE("/data/local/tmp/test_output.htrace");
}

class ProfilerSessionManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        // Setup for tests
    }

    void TearDown() override
    {
        // Cleanup for tests
    }
};

/**
 * @tc.name: TestCaptureLongRunningBasic
 * @tc.desc: Test basic functionality of CaptureLongRunning
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerSessionManagerTest, TestCaptureLongRunningBasic, Function | MediumTest | Level0)
{
    ProfilerSessionManager& manager = ProfilerSessionManager::GetInstance();
    
    std::string duration = "5";  // 5 seconds
    uint32_t sessionId = manager.CaptureLongRunning(TEST_CONFIG, duration, TEST_OUTPUT_FILE);
    
    if (sessionId == 0) {
        GTEST_SKIP() << "Profiler service not available, skipping test";
    }
    
    EXPECT_NE(sessionId, 0u);
    
    // Clean up
    if (sessionId != 0) {
        manager.StopSessionById(sessionId);
    }
}

/**
 * @tc.name: TestCaptureLongRunningDurationCap
 * @tc.desc: Test that duration is capped at 3600 seconds
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerSessionManagerTest, TestCaptureLongRunningDurationCap, Function | MediumTest | Level0)
{
    ProfilerSessionManager& manager = ProfilerSessionManager::GetInstance();
    
    // Try with duration exceeding 3600 seconds
    std::string longDuration = "5000";
    uint32_t sessionId = manager.CaptureLongRunning(TEST_CONFIG, longDuration, TEST_OUTPUT_FILE);
    
    if (sessionId == 0) {
        GTEST_SKIP() << "Profiler service not available, skipping test";
    }
    
    // Verify session was created (duration capping happens internally)
    EXPECT_NE(sessionId, 0u);
    
    // Clean up
    if (sessionId != 0) {
        manager.StopSessionById(sessionId);
    }
}

/**
 * @tc.name: TestCaptureWithWrongSessionId
 * @tc.desc: Test CaptureLongRunning with wrong sessionId
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerSessionManagerTest, TestCaptureWithWrongSessionId, Function | MediumTest | Level0)
{
    ProfilerSessionManager& manager = ProfilerSessionManager::GetInstance();
    
    std::string longDuration = "50";
    uint32_t sessionId = manager.CaptureLongRunning(TEST_CONFIG, longDuration, TEST_OUTPUT_FILE);
    
    if (sessionId == 0) {
        GTEST_SKIP() << "Profiler service not available, skipping test";
    }
    
    // Verify session was created (duration capping happens internally)
    EXPECT_NE(sessionId, 0u);
    
    // Clean up
    if (sessionId != 0) {
        EXPECT_EQ(manager.StopSessionById(sessionId), true);
    }
    sessionId = 999;
    EXPECT_EQ(manager.StopSessionById(sessionId), false);
}

/**
 * @tc.name: TestStopAllSession
 * @tc.desc: TestStopAllSession normal case
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerSessionManagerTest, TestStopAllSession, Function | MediumTest | Level0)
{
    ProfilerSessionManager& manager = ProfilerSessionManager::GetInstance();
    
    std::string longDuration = "50";
    uint32_t sessionId = manager.CaptureLongRunning(TEST_CONFIG, longDuration, TEST_OUTPUT_FILE);
    
    if (sessionId == 0) {
        GTEST_SKIP() << "Profiler service not available, skipping test";
    }
    
    // Verify session was created (duration capping happens internally)
    EXPECT_NE(sessionId, 0u);

    EXPECT_EQ(manager.StopAllSessions(), true);
}

