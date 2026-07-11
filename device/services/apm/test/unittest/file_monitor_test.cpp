/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>

#include "file_monitor.h"

using namespace testing::ext;

namespace {
const std::string TEST_LISTEN_DIR = "/data/local/tmp";
const std::string TEST_FILE_PREFIX = "fm_test_file_";
const std::string INVALID_DIR = "/data/local/tmp/non_existent_dir_for_file_monitor_test";
const int32_t SHORT_TIMEOUT_MS = 2000;
const int32_t ZERO_TIMEOUT_MS = 0;
}

class FileMonitorTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ConstructorWithValidDir
 * @tc.desc: Test FileMonitor can be constructed with a valid directory
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, ConstructorWithValidDir, TestSize.Level1)
{
    auto filter = [](std::string& filename) -> bool {
        return filename.find(TEST_FILE_PREFIX) != std::string::npos;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    SUCCEED();
}

/**
 * @tc.name: ConstructorWithInvalidDir
 * @tc.desc: Test FileMonitor construction with invalid directory does not crash
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, ConstructorWithInvalidDir, TestSize.Level1)
{
    auto filter = [](std::string& filename) -> bool {
        return true;
    };
    FileMonitor monitor(INVALID_DIR, filter);
    monitor.Finalize();
    SUCCEED();
}

/**
 * @tc.name: StartAndFinalize
 * @tc.desc: Test Start and Finalize lifecycle
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, StartAndFinalize, TestSize.Level1)
{
    auto filter = [](std::string& filename) -> bool {
        return true;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());
    monitor.Finalize();
    SUCCEED();
}

/**
 * @tc.name: FinalizeIdempotent
 * @tc.desc: Test Finalize can be called multiple times without crash
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, FinalizeIdempotent, TestSize.Level1)
{
    auto filter = [](std::string& filename) -> bool {
        return true;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());
    monitor.Finalize();
    monitor.Finalize();
    SUCCEED();
}

/**
 * @tc.name: DestructorCallsFinalize
 * @tc.desc: Test destructor properly cleans up resources
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, DestructorCallsFinalize, TestSize.Level1)
{
    auto monitor = std::make_unique<FileMonitor>(TEST_LISTEN_DIR, [](std::string&) -> bool {
        return true;
    });
    EXPECT_TRUE(monitor->Start());
    monitor.reset();
    SUCCEED();
}

/**
 * @tc.name: GetResultFileTimeout
 * @tc.desc: Test GetResultFile returns empty string when timeout expires
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, GetResultFileTimeout, TestSize.Level1)
{
    auto filter = [](std::string& filename) -> bool {
        return false;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());
    std::string result = monitor.GetResultFile(SHORT_TIMEOUT_MS);
    EXPECT_TRUE(result.empty());
    monitor.Finalize();
}

/**
 * @tc.name: GetResultFileZeroTimeout
 * @tc.desc: Test GetResultFile(0) returns immediately when the matching file has already been detected
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, GetResultFileZeroTimeout, TestSize.Level1)
{
    std::string testFilename = "fm_zero_timeout_" + std::to_string(getpid()) + ".txt";
    std::string testFilepath = TEST_LISTEN_DIR + "/" + testFilename;

    auto filter = [&testFilename](std::string& filename) -> bool {
        return filename == testFilename;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    {
        std::ofstream ofs(testFilepath, std::ios::trunc);
        ofs << "test content for zero timeout";
        ofs.close();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto start = std::chrono::steady_clock::now();
    std::string result = monitor.GetResultFile(ZERO_TIMEOUT_MS);
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_EQ(result, testFilename);
    EXPECT_LT(elapsed, SHORT_TIMEOUT_MS);
    monitor.Finalize();
    std::filesystem::remove(testFilepath);
}

/**
 * @tc.name: DetectMatchingFile
 * @tc.desc: Test GetResultFile returns filename when a matching file is written
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, DetectMatchingFile, TestSize.Level1)
{
    std::string testFilename = TEST_FILE_PREFIX + std::to_string(getpid()) + ".txt";
    std::string testFilepath = TEST_LISTEN_DIR + "/" + testFilename;

    auto filter = [&testFilename](std::string& filename) -> bool {
        return filename == testFilename;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    {
        std::ofstream ofs(testFilepath, std::ios::trunc);
        ofs << "test content";
        ofs.close();
    }

    std::string result = monitor.GetResultFile(5000);
    EXPECT_EQ(result, testFilename);
    monitor.Finalize();
    std::filesystem::remove(testFilepath);
}

/**
 * @tc.name: FilterRejectsNonMatchingFile
 * @tc.desc: Test GetResultFile ignores files that do not match the filter
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, FilterRejectsNonMatchingFile, TestSize.Level1)
{
    std::string rejectFilename = "fm_rejected_" + std::to_string(getpid()) + ".txt";
    std::string rejectFilepath = TEST_LISTEN_DIR + "/" + rejectFilename;

    auto filter = [&rejectFilename](std::string& filename) -> bool {
        return false;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    {
        std::ofstream ofs(rejectFilepath, std::ios::trunc);
        ofs << "rejected content";
        ofs.close();
    }

    std::string result = monitor.GetResultFile(SHORT_TIMEOUT_MS);
    EXPECT_TRUE(result.empty());
    monitor.Finalize();
    std::filesystem::remove(rejectFilepath);
}

/**
 * @tc.name: DetectFirstMatchingAmongMany
 * @tc.desc: Test GetResultFile returns the first matching file when multiple are written
 * @tc.type: FUNC
 */
HWTEST_F(FileMonitorTest, DetectFirstMatchingAmongMany, TestSize.Level1)
{
    std::string matchFilename = "fm_first_match_" + std::to_string(getpid()) + ".txt";
    std::string matchFilepath = TEST_LISTEN_DIR + "/" + matchFilename;
    std::string rejectFilename = "fm_reject_" + std::to_string(getpid()) + ".txt";
    std::string rejectFilepath = TEST_LISTEN_DIR + "/" + rejectFilename;

    auto filter = [&matchFilename](std::string& filename) -> bool {
        return filename == matchFilename;
    };
    FileMonitor monitor(TEST_LISTEN_DIR, filter);
    EXPECT_TRUE(monitor.Start());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    {
        std::ofstream ofs(rejectFilepath, std::ios::trunc);
        ofs << "rejected";
        ofs.close();
    }

    {
        std::ofstream ofs(matchFilepath, std::ios::trunc);
        ofs << "matched";
        ofs.close();
    }

    std::string result = monitor.GetResultFile(5000);
    EXPECT_EQ(result, matchFilename);
    monitor.Finalize();
    std::filesystem::remove(matchFilepath);
    std::filesystem::remove(rejectFilepath);
}
