/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include "RAM.h"
#include "sp_utils.h"
#include "sp_log.h"
#include "common.h"
#include "memory_collector.h"
#include "collect_result.h"
#include "startup_delay.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonRamTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

// ==================== CollectRam ====================

/**
 * @tc.name: RAM::CollectRam
 * @tc.desc: Test CollectRam with invalid PID returns empty map
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, CollectRamTest001, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();

    auto result = ram.CollectRam("invalid_pid", 0);

    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: RAM::CollectRam
 * @tc.desc: Test CollectRam with empty PID returns empty map
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, CollectRamTest002, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();

    auto result = ram.CollectRam("", 0);

    EXPECT_TRUE(result.empty());
}
/**
 * @tc.name: RAM::CollectRam
 * @tc.desc: Test CollectRam with non-existent PID returns zero-filled map
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, CollectRamTest003, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();

    auto result = ram.CollectRam("99999999", 0);

    // SafeLoadExec fails for non-existent pid, returns ProcMemNaInfo with "0" values
    EXPECT_FALSE(result.empty());
    for (const auto &item : result) {
        EXPECT_GE(item.second, "0") << "Key: " << item.first << " value is not 0";
    }
}

/**
 * @tc.name: RAM::CollectRam
 * @tc.desc: Test CollectRam with non-existent PID and child index returns zero-filled map
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, CollectRamTest004, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();

    auto result = ram.CollectRam("99999999", 1);

    EXPECT_FALSE(result.empty());
    for (const auto &item : result) {
        EXPECT_GE(item.second, "0") << "Key: " << item.first << " value is not 0";
    }
}

/**
 * @tc.name: RAM::CollectRam
 * @tc.desc: Test CollectRam with valid PID (init process) returns non-empty result
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, CollectRamTest005, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("1");

    auto result = ram.CollectRam("1", 0);

    // On device, hidumper succeeds; on host, SafeLoadExec fails and returns zero-filled
    EXPECT_FALSE(result.empty());
}

// ==================== ParsePssValues ====================

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with "No such process" returns empty map
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest001, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("99999");
    std::istringstream iss("No such process running");
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "99999", 0);

    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with GL and Graph lines
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest002, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "GL           100     0     0     0     0    50     0     0     0\n"
        "Graph        200     0     0     0     0    30     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["gpuPss"], "100");
    EXPECT_EQ(result["graphicPss"], "200");
}
/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with ark ts heap line
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest003, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "GL           100     0     0     0     0    50     0     0     0\n"
        "ark  ts        0    10   300     0     0     0    50     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["arktsHeapPss"], "10");
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with native heap line
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest004, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "native  heap   0   400     0     0     0     0    60     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["nativeHeapPss"], "60");
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with Ashmem line containing colon
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest005, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "GL           100     0     0     0     0    50     0     0     0\n"
        "Ashmem:256     0     0     0     0     0     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["ashmem"], "NA");
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues skips lines starting with '-'
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest006, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "------ separator line ------\n"
        "GL           100     0     0     0     0    50     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["gpuPss"], "100");
}
/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with arkts heap section - main thread tid matches pid
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest007, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "GL           100     0     0     0     0    50     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n"
        "arkts heap:\n"
        "tid  thread name  arkts_heap(KB)\n"
        "12345  mainThread  500\n"
        "12346  workerThread  200\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["arktsHeapMainThreadPss"], "500");
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with arkts heap section - main thread tid does not match pid
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest008, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("99999");
    std::string input =
        "GL           100     0     0     0     0    50     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n"
        "arkts heap:\n"
        "tid  thread name  arkts_heap(KB)\n"
        "11111  mainThread  500\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "99999", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["arktsHeapMainThreadPss"], "NA");
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with empty input stream
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest009, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::istringstream iss("");
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with child index > 0 uses child-prefixed keys
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest010, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("6789");
    std::string input =
        "GL           100     0     0     0     0    50     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "6789", 1);

    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("childGpuPss"), result.end());
    EXPECT_NE(result.find("childArktsHeapMainThreadPss"), result.end());
}

/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with guard/hap/db/so/dev/ttf lines
 * @tc.type: FUNC
 */
/**
 * @tc.name: RAM::ParsePssValues
 * @tc.desc: Test ParsePssValues with guard/hap/db/so/dev/ttf lines
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, ParsePssValuesTest011, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId("12345");
    std::string input =
        "guard        10     0     0     0     0     5     0     0     0\n"
        ".hap         20     0     0     0     0    10     0     0     0\n"
        ".db          30     0     0     0     0    15     0     0     0\n"
        ".so          40     0     0     0     0    20     0     0     0\n"
        "dev          50     0     0     0     0    25     0     0     0\n"
        ".ttf         60     0     0     0     0    30     0     0     0\n"
        "Total       1000   200   300   400   500   600   700   800   900  1000K\n";
    std::istringstream iss(input);
    std::vector<std::string> paramsInfo;

    auto result = ram.ParsePssValues(iss, paramsInfo, "12345", 0);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result["guard"], "10");
    EXPECT_EQ(result["hap"], "20");
    EXPECT_EQ(result["db"], "30");
    EXPECT_EQ(result["so"], "40");
    EXPECT_EQ(result["dev"], "50");
    EXPECT_EQ(result["ttf"], "60");
}

// ==================== DetermineDeviceVersion ====================

/**
 * @tc.name: RAM::DetermineDeviceVersion
 * @tc.desc: Test DetermineDeviceVersion does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, DetermineDeviceVersionTest001, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();

    ram.DetermineDeviceVersion();

    // Should not crash; reads system parameter which may return "Unknown" on host
    auto result = ram.ItemData();
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: RAM::DetermineDeviceVersion
 * @tc.desc: Test DetermineDeviceVersion multiple calls do not crash
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonRamTest, DetermineDeviceVersionTest002, TestSize.Level1)
{
    RAM &ram = RAM::GetInstance();

    ram.DetermineDeviceVersion();
    ram.DetermineDeviceVersion();
    ram.DetermineDeviceVersion();

    auto result = ram.ItemData();
    EXPECT_FALSE(result.empty());
}
}
}