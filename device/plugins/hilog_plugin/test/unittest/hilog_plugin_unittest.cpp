/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <cinttypes>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#include "hilog_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
#if defined(__LP64__)
const std::string DEFAULT_TEST_PATH("/system/lib64/");
#else
const std::string DEFAULT_TEST_PATH("/system/lib/");
#endif
const std::string DEFAULT_RECORD_FILE("/data/local/tmp/");
const int FILE_NAME_LEN = 5;
const int US_PER_S = 1000000;
const int BASE_YEAR = 1900;
const int DEFAULT_WAIT = 10;
const int TIME_HOUR_WIDTH = 5;
constexpr int BUF_MAX_LEN = 32;
uint64_t g_testId;
std::vector<HilogInfo> g_proto;

class HilogPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

long WriteFunc(WriterStruct* writer, const void* data, size_t size)
{
    if (writer == nullptr || data == nullptr || size <= 0) {
        return -1;
    }

    HilogInfo info;
    if (info.ParseFromArray(data, size) <= 0) {
        return -1;
    }
    g_proto.push_back(info);
    return 0;
}

bool FlushFunc(WriterStruct* writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

bool PluginStart(HilogPlugin& plugin, HilogConfig& config)
{
    // serialize
    int size = config.ByteSizeLong();
    std::vector<uint8_t> configData(size);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    CHECK_TRUE(ret > 0, false, "HilogPluginTest: SerializeToArray fail!!!");
    HILOG_INFO(LOG_CORE, "HilogPluginTest: SerializeToArray success");

    // start
    ret = plugin.Start(configData.data(), configData.size());
    CHECK_TRUE(ret == 0, false, "HilogPluginTest: start plugin fail!!!");
    HILOG_INFO(LOG_CORE, "HilogPluginTest: Start success");

    return true;
}

bool RecordFileExist(std::string& file)
{
    char name[FILE_NAME_LEN] = {0};
    time_t nSeconds;
    struct tm* pTM;

    nSeconds = time(nullptr);
    pTM = localtime(&nSeconds);
    if (snprintf_s(name, sizeof(name), sizeof(name) - 1, "%04d", pTM->tm_year + BASE_YEAR) < 0) {
        HILOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
    }
    if (access(DEFAULT_RECORD_FILE.c_str(), F_OK) != 0) {
        return false;
    }

    std::string cmd = "find /data/local/tmp -name \"" + std::string(name) + "*\"";
    char buff[BUF_MAX_LEN] = {0};
    std::unique_ptr<FILE, int (*)(FILE*)> fp(popen(cmd.c_str(), "r"), pclose);
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "%s:: popen error", __func__);
        return false;
    }

    char* pRet = fgets(buff, BUF_MAX_LEN - 1, fp.get());
    CHECK_NOTNULL(pRet, false, "FileCache: fgets Failed, errno(%d)", errno);
    buff[BUF_MAX_LEN - 1] = '\0';
    if (strlen(buff)) {
        file = std::string(buff);
        return true;
    }
    return false;
}

uint64_t GetSec(HilogPlugin& plugin, const char* data)
{
    time_t nSeconds = time(nullptr);
    if (nSeconds == 0) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "GetSec: get time failed!, errno(%d:%s)", errno, buf);
        return 0;
    }

    struct tm* pTM = localtime(&nSeconds);
    if (pTM == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "GetSec: get localtime failed!, errno(%d:%s)", errno, buf);
        return 0;
    }

    struct tm tmTime = {0};
    tmTime.tm_year = pTM->tm_year;
    strptime(data, "%m-%d %H:%M:%S", &tmTime);
    const char* pTmp = data + TIME_HOUR_WIDTH;
    long fixHour;
    CHECK_TRUE(plugin.StringToL(pTmp, fixHour), 0, "%s:strtol fixHour failed", __func__);
    if (static_cast<int>(fixHour) != tmTime.tm_hour) { // hours since midnight - [0, 23]
        HILOG_INFO(LOG_CORE, "GetSec: hour(%d) <==> fix hour(%ld)!", tmTime.tm_hour, fixHour);
        tmTime.tm_hour = fixHour;
    }

    return mktime(&tmTime);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test valid full cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestValidFullCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;

    config.set_device_type(Type::TYPE_UNSPECIFIED);
    plugin.SetConfig(config);
    ASSERT_FALSE(plugin.InitHilogCmd());
    EXPECT_STREQ(plugin.GetFullCmd().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test HI3516
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestHI3516, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    int32_t pid = 1;

    config.set_device_type(Type::HI3516);
    config.set_pid(pid);
    config.set_log_level(Level::ERROR);
    plugin.SetConfig(config);
    ASSERT_TRUE(plugin.InitHilogCmd());

    std::string target;
    target = "hilog -P " + std::to_string(pid) + " -L E --format nsec";
    EXPECT_STREQ(plugin.GetFullCmd().c_str(), target.c_str());
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test P40
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestP40, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    int32_t pid = 1;

    config.set_device_type(Type::P40);
    config.set_pid(pid);
    config.set_log_level(Level::ERROR);
    plugin.SetConfig(config);
    ASSERT_TRUE(plugin.InitHilogCmd());

    std::string target;
    target = "hilogcat --pid=" + std::to_string(pid) + " *:E --format nsec";
    EXPECT_STREQ(plugin.GetFullCmd().c_str(), target.c_str());
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test HI3516 and clear
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestHI3516AndClear, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    int32_t pid = 1;

    config.set_device_type(Type::HI3516);
    config.set_pid(pid);
    config.set_log_level(Level::ERROR);
    config.set_need_clear(true);
    plugin.SetConfig(config);
    ASSERT_TRUE(plugin.InitHilogCmd());

    std::string target;
    target = "hilog -P " + std::to_string(pid) + " -L E --format nsec";
    EXPECT_STREQ(plugin.GetFullCmd().c_str(), target.c_str());
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test P40 and clear
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestP40AndClear, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    int32_t pid = 1;

    config.set_device_type(Type::P40);
    config.set_pid(pid);
    config.set_log_level(Level::ERROR);
    config.set_need_clear(true);
    plugin.SetConfig(config);
    ASSERT_TRUE(plugin.InitHilogCmd());

    std::string target;
    target = "hilogcat --pid=" + std::to_string(pid) + " *:E --format nsec";
    EXPECT_STREQ(plugin.GetFullCmd().c_str(), target.c_str());
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test valid level cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestValidLevelCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;

    config.set_log_level(Level::LEVEL_UNSPECIFIED);
    plugin.SetConfig(config);
    EXPECT_STREQ(plugin.GetlevelCmd().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test e level cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestELevelCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;

    config.set_log_level(Level::ERROR);
    plugin.SetConfig(config);
    EXPECT_STREQ(plugin.GetlevelCmd().c_str(), "E");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test i level cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestILevelCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;

    config.set_log_level(Level::INFO);
    plugin.SetConfig(config);
    EXPECT_STREQ(plugin.GetlevelCmd().c_str(), "I");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test d level cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestDLevelCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;

    config.set_log_level(Level::DEBUG);
    plugin.SetConfig(config);
    EXPECT_STREQ(plugin.GetlevelCmd().c_str(), "D");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test w level cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestWLevelCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;

    config.set_log_level(Level::WARN);
    plugin.SetConfig(config);
    EXPECT_STREQ(plugin.GetlevelCmd().c_str(), "W");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test nullptr param
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestNullptrLogLine, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = nullptr;

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, 0, &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), (uint64_t)0);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), (uint64_t)0);
    EXPECT_EQ(info.mutable_detail()->pid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->tid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->level(), (uint32_t)0);
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine1, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.522";

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), (uint64_t)0);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), (uint64_t)0);
    EXPECT_EQ(info.mutable_detail()->pid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->tid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->level(), (uint32_t)0);
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine2, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.149566200\n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 149566200;

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->tid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->level(), (uint32_t)0);
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine3, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.149566200 27953 \n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 149566200;
    uint32_t target = 27953;

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target);
    EXPECT_EQ(info.mutable_detail()->tid(), (uint32_t)0);
    EXPECT_EQ(info.mutable_detail()->level(), (uint32_t)0);
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine4, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.149566200 27953 31750 \n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 149566200;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), (uint32_t)0);
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine5, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.149566200 27953 31750 I\n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 149566200;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine6, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.522 27953 31750 I 00B00 \n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 522;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine7, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.522 27953 31750 I 00B00/ \n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 522;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine8, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.522 27953 31750 I 00B00/HwMSDPMovementImpl: \n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 522;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "HwMSDPMovementImpl");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine9, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.522 27953 31750 I chatty  :\n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 522;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "chatty  ");
    EXPECT_STREQ(info.context().c_str(), "");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test invalid ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestInvalidLogLine, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.522 27953 31750 I 00B00/HwMSDPMovementImpl: mSupportedModule= 0\n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 522;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "HwMSDPMovementImpl");
    EXPECT_STREQ(info.context().c_str(), "mSupportedModule= 0");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestParseLogLine1, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data = "08-30 16:47:16.149566200 27953 31750 I 00B00/HwMSDPMovementImpl: mSupportedModule= 0\n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 149566200;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'I');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "HwMSDPMovementImpl");
    EXPECT_STREQ(info.context().c_str(), "mSupportedModule= 0");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test ParseLogLineInfo
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestParseLogLine2, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    HilogLine info;
    const char* data =
        "08-30 16:47:16.149566200 27953 31750 E chatty  : uid=10194(com.zh.heaptest) identical 2 lines\n";
    uint64_t sec = GetSec(plugin, data);
    uint64_t nsec = 149566200;
    uint32_t target[] = {27953, 31750};

    plugin.SetConfig(config);
    plugin.ParseLogLineInfo(data, strlen(data), &info);
    EXPECT_EQ(info.mutable_detail()->tv_sec(), sec);
    EXPECT_EQ(info.mutable_detail()->tv_nsec(), nsec);
    EXPECT_EQ(info.mutable_detail()->pid(), target[0]);
    EXPECT_EQ(info.mutable_detail()->tid(), target[1]);
    EXPECT_EQ(info.mutable_detail()->level(), 'E');
    EXPECT_STREQ(info.mutable_detail()->tag().c_str(), "chatty  ");
    EXPECT_STREQ(info.context().c_str(), "uid=10194(com.zh.heaptest) identical 2 lines");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstNum
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstNum1, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', '\0', '0'};
    char* cptr = data;

    EXPECT_FALSE(plugin.FindFirstNum(&cptr));
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstNum
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstNum2, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', '1', '\0', '0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.FindFirstNum(&cptr));
    EXPECT_STREQ(cptr, "1");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstNum
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstNum3, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', '1', 'c', '\n', '\0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.FindFirstNum(&cptr));
    EXPECT_STREQ(cptr, "1c\n");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstNum
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstNum4, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', '1', 'c', '\0', '0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.FindFirstNum(&cptr));
    EXPECT_STREQ(cptr, "1c");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstNum
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstNum5, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', '\n', '0'};
    char* cptr = data;

    EXPECT_FALSE(plugin.FindFirstNum(&cptr));
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstSpace
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstSpace1, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', '1', '\0', ' '};
    char* cptr = data;

    EXPECT_FALSE(plugin.FindFirstSpace(&cptr));
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstSpace
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstSpace2, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', ' ', '\0', '0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.FindFirstSpace(&cptr));
    EXPECT_STREQ(cptr, " ");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstSpace
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstSpace3, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'\n', 'a', ' ', '\0', '0'};
    char* cptr = data;

    EXPECT_FALSE(plugin.FindFirstSpace(&cptr));
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstSpace
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstSpace4, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', ' ', ' ', '\0', '0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.FindFirstSpace(&cptr));
    EXPECT_STREQ(cptr, "  ");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test FindFirstSpace
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFindFirstSpace5, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'.', 'b', ' ', ' ', 'c', '\0', '0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.FindFirstSpace(&cptr));
    EXPECT_STREQ(cptr, "  c");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test RemoveSpaces
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestRemoveSpaces1, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {' ', '\0', 'a'};
    char* cptr = data;

    EXPECT_FALSE(plugin.RemoveSpaces(&cptr));
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test RemoveSpaces
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestRemoveSpaces2, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {' ', ' ', 'a', '\0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.RemoveSpaces(&cptr));
    EXPECT_STREQ(cptr, "a");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test RemoveSpaces
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestRemoveSpaces3, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {' ', 'a', 'b', '\0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.RemoveSpaces(&cptr));
    EXPECT_STREQ(cptr, "ab");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test RemoveSpaces
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestRemoveSpaces4, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'\n', ' ', '\0'};
    char* cptr = data;

    EXPECT_FALSE(plugin.RemoveSpaces(&cptr));
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test RemoveSpaces
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestRemoveSpaces5, TestSize.Level1)
{
    HilogPlugin plugin;
    char data[] = {'a', 'b', ' ', 'c', 'd', '\0'};
    char* cptr = data;

    EXPECT_TRUE(plugin.RemoveSpaces(&cptr));
    EXPECT_STREQ(cptr, "ab cd");
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test CustomPopen
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestCustomPopen, TestSize.Level1)
{
    HilogPlugin plugin;
    const char* cmd = nullptr;
    const char* type = nullptr;

    EXPECT_EQ(plugin.CustomPopen(cmd, type), nullptr);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test CustomPopen write
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestCustomPopenNullW, TestSize.Level1)
{
    HilogPlugin plugin;
    const char* cmd = "";
    const char* type = "w";
    FILE* fp = plugin.CustomPopen(cmd, type);

    EXPECT_NE(fp, nullptr);
    plugin.CustomPclose(fp);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test CustomPopen write
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestCustomPopenW, TestSize.Level1)
{
    HilogPlugin plugin;
    const char* cmd = "echo this is a test!";
    const char* type = "w";
    FILE* fp = plugin.CustomPopen(cmd, type);

    EXPECT_NE(fp, nullptr);
    plugin.CustomPclose(fp);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test CustomPopen read
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestCustomPopenNullR, TestSize.Level1)
{
    HilogPlugin plugin;
    const char* cmd = "";
    const char* type = "r";
    FILE* fp = plugin.CustomPopen(cmd, type);

    EXPECT_NE(fp, nullptr);
    plugin.CustomPclose(fp);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test CustomPopen read
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestCustomPopenR, TestSize.Level1)
{
    HilogPlugin plugin;
    const char* cmd = "ls";
    const char* type = "r";
    FILE* fp = plugin.CustomPopen(cmd, type);

    EXPECT_NE(fp, nullptr);
    plugin.CustomPclose(fp);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test CustomPopen write
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFileOperation, TestSize.Level1)
{
    char buff[FILE_NAME_LEN] = {0};
    std::string testPath = "/data/local/tmp/uttestdir/";
    std::string cmd = std::string("mkdir ") + testPath;
    system(cmd.c_str());
    FileCache file(testPath);
    char writeByte[] = "1234";
    int32_t writeLen = static_cast<int32_t>(strlen(writeByte));

    ASSERT_TRUE(file.Open("test.txt"));
    ASSERT_EQ(file.Write(writeByte, writeLen), writeLen);
    ASSERT_GT(FILE_NAME_LEN, writeLen);
    ASSERT_EQ(file.Read(buff), writeLen);
    ASSERT_TRUE(file.Close());
    cmd = std::string("rm -rf ") + testPath;
    system(cmd.c_str());
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: start fail test
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestStartFail, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    g_testId = 1;
    g_proto.erase(g_proto.begin(), g_proto.end());
    // set config
    config.set_device_type(Type::HI3516);

    // test plugin process
    plugin.SetWriter(&writer);

    // serialize
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // start
    EXPECT_NE(plugin.Start(configData.data(), size - 1), 0);
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Framework test
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFramework, TestSize.Level1)
{
    std::string path = DEFAULT_TEST_PATH + std::string("libhilogplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "hilog-plugin");

    g_testId = 1;
    g_proto.erase(g_proto.begin(), g_proto.end());

    // set config
    HilogConfig config;
    config.set_device_type(Type::HI3516);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onRegisterWriterStruct(&writer), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    usleep(US_PER_S * DEFAULT_WAIT); // 10s
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);

    // test proto data
    int protoSize = g_proto.size();
    ASSERT_GT(protoSize, 0);
    g_testId = 1;
    for (int i = 0; i < protoSize; i++) {
        HilogInfo info = g_proto[i];
        for (int j = 0; j < info.info_size(); j++) {
            EXPECT_EQ(info.info(j).id(), g_testId);
            g_testId++;
        }
    }
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test default cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestDefaultCmd, TestSize.Level1)
{
    HilogConfig config;
    HilogPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    g_testId = 1;
    g_proto.erase(g_proto.begin(), g_proto.end());
    // set config
    config.set_device_type(Type::HI3516);

    // test plugin process
    plugin.SetWriter(&writer);
    EXPECT_TRUE(PluginStart(plugin, config));
    usleep(US_PER_S * DEFAULT_WAIT); // 10s
    EXPECT_EQ(plugin.Stop(), 0);

    // test proto data
    int size = g_proto.size();
    ASSERT_GT(size, 0);
    for (int i = 0; i < size; i++) {
        HilogInfo info = g_proto[i];
        for (int j = 0; j < info.info_size(); j++) {
            EXPECT_EQ(info.info(j).id(), g_testId);
            g_testId++;
        }
    }
}

/**
 * @tc.name: hilog plugin
 * @tc.desc: Test cmd
 * @tc.type: FUNC
 */
HWTEST_F(HilogPluginTest, TestFullCmd, TestSize.Level1)
{
    std::string oldFile = "";
    if (RecordFileExist(oldFile)) {
        std::string cmd = std::string("rm ") + oldFile;
        system(cmd.c_str());
    }

    HilogConfig config;
    HilogPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    g_testId = 1;
    g_proto.erase(g_proto.begin(), g_proto.end());
    // set config
    config.set_device_type(Type::HI3516);
    config.set_need_record(true);

    // test plugin process
    plugin.SetWriter(&writer);
    EXPECT_TRUE(PluginStart(plugin, config));
    usleep(US_PER_S * DEFAULT_WAIT); // 10s

    // test proto data
    int size = g_proto.size();
    ASSERT_GT(size, 0);
    for (int i = 0; i < size; i++) {
        HilogInfo info = g_proto[i];
        for (int j = 0; j < info.info_size(); j++) {
            EXPECT_EQ(info.info(j).id(), g_testId);
            g_testId++;
        }
    }
    // test record file
    std::string file;
    ASSERT_TRUE(RecordFileExist(file));
    file = std::string("rm ") + file;
    system(file.c_str());
    EXPECT_EQ(plugin.Stop(), 0);
}
} // namespace
