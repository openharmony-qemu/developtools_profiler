/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#define private public
#include "stack_preprocess.h"
#include "hook_manager.h"
#undef private
#include "native_hook_config.pb.h"
#include "plugin_module_api.h"
#include "native_hook_result_standard.pb.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr size_t BUF_SIZE = 4 * 1024 * 1024;
constexpr size_t MAX_RECORD_SIZE = BUF_SIZE - sizeof(uint32_t);
constexpr uint32_t NODE_TYPE = 1;
constexpr uint64_t NODE_ID = 0x1000;
constexpr uint32_t TAG_ID = 1;

class CacheFileReadTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
    }

    static void TearDownTestCase(void)
    {
    }

    void SetUp()
    {
        StackPreprocess::ClearThreadLocalVariables();
    }

    void TearDown()
    {
        StackPreprocess::ClearThreadLocalVariables();
    }

    std::string CreateTestFile(const std::vector<std::vector<uint8_t>>& records)
    {
        std::string testFile = "/data/local/tmp/cache_file_test_" + std::to_string(GetCurrentPid()) + ".dat";
        int fd = open(testFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ASSERT_GE(fd, 0);

        for (const auto& record : records) {
            uint32_t size = static_cast<uint32_t>(record.size());
            write(fd, &size, sizeof(uint32_t));
            write(fd, record.data(), record.size());
        }
        close(fd);
        return testFile;
    }

    std::vector<uint8_t> CreateValidRecord(size_t dataSize)
    {
        std::vector<uint8_t> record(dataSize);
        memset(record.data(), 0xAA, dataSize);
        return record;
    }

    void RemoveTestFile(const std::string& filePath)
    {
        unlink(filePath.c_str());
    }
};

static RawStackPtr MakeRawStack(uint32_t nodeType, = NODE_TYPE, uint64_t nodeId = NODE_ID, uint32_t tagId = TAG_ID)
{
    auto raw = RawStackPtr(new RawStack(), [](RawStack* p) {
        if (p != nullptr) {
            delete p->stackContext;
            p->stackContext = nullptr;
            delete p;
        }
    });

    raw->stackContext = new BaseStackRawData();
    raw->stackContext->nodeType = nodeType;
    raw->stackContext->nodeId = nodeId;
    raw->stackContext->tagId = tagId;
    raw->stackSize = 1;
    raw->reportFlag = true;
    return raw;
}

/*
 * @tc.name: ParseRecordsFromBufferNormalRecords
 * @tc.desc: test ParseRecordsFromBuffer with normal records in one buffer.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferNormalRecords, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> record1 = CreateValidRecord(100);
    std::vector<uint8_t> record2 = CreateValidRecord(200);
    std::vector<uint8_t> record3 = CreateValidRecord(150);

    size_t totalSize = sizeof(uint32_t) + record1.size() +
                      sizeof(uint32_t) + record2.size() +
                      sizeof(uint32_t) + record3.size();

    std::vector<uint8_t> buffer(totalSize);
    size_t offset = 0;

    uint32_t size1 = static_cast<uint32_t>(record1.size());
    memcpy(buffer.data() + offset, &size1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record1.data(), record1.size());
    offset += record1.size();

    uint32_t size2 = static_cast<uint32_t>(record2.size());
    memcpy(buffer.data() + offset, &size2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record2.data(), record2.size());
    offset += record2.size();

    uint32_t size3 = static_cast<uint32_t>(record3.size());
    memcpy(buffer.data() + offset, &size3, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record3.data(), record3.size());

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;
    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                            bufferOffset, readfileSize);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, buffer.size());
    EXPECT_EQ(readfileSize, buffer.size());
}

/*
 * @tc.name: ParseRecordsFromBufferCrossBoundary
 * @tc.desc: test ParseRecordsFromBuffer with record crossing buffer boundary.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferCrossBoundary, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> record1 = CreateValidRecord(100);
    std::vector<uint8_t> record2 = CreateValidRecord(200);

    size_t totalSize = sizeof(uint32_t) + record1.size() +
                      sizeof(uint32_t) + record2.size();

    std::vector<uint8_t> buffer(totalSize);
    size_t offset = 0;

    uint32_t size1 = static_cast<uint32_t>(record1.size());
    memcpy(buffer.data() + offset, &size1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record1.data(), record1.size());
    offset += record1.size();

    uint32_t size2 = static_cast<uint32_t>(record2.size());
    memcpy(buffer.data() + offset, &size2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record2.data(), record2.size());

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size() - 50,
                                            bufferOffset, readfileSize);

    EXPECT_TRUE(ret);
    EXPECT_GT(bufferOffset, 0);
    EXPECT_LT(bufferOffset, buffer.size());
    EXPECT_GT(readfileSize, 0);
}

/*
 * @tc.name: ParseRecordsFromBufferIncompleteRecord
 * @tc.desc: test ParseRecordsFromBuffer with incomplete record at end.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferIncompleteRecord, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> record1 = CreateValidRecord(100);
    std::vector<uint8_t> record2 = CreateValidRecord(200);

    size_t partialSize = sizeof(uint32_t) + record1.size() + sizeof(uint32_t) + 50;

    std::vector<uint8_t> buffer(partialSize);
    size_t offset = 0;

    uint32_t size1 = static_cast<uint32_t>(record1.size());
    memcpy(buffer.data() + offset, &size1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record1.data(), record1.size());
    offset += record1.size();

    uint32_t size2 = static_cast<uint32_t>(record2.size());
    memcpy(buffer.data() + offset, &size2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record2.data(), 50);

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                            bufferOffset, readfileSize);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, sizeof(uint32_t) + record1.size() + sizeof(uint32_t));
    EXPECT_EQ(readfileSize, sizeof(uint32_t) + record1.size());
}

/*
 * @tc.name: ParseRecordsFromBufferEmptyBuffer
 * @tc.desc: test ParseRecordsFromBuffer with empty buffer.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferEmptyBuffer, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> buffer;
    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                            bufferOffset, readfileSize);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, 0);
    EXPECT_EQ(readfileSize, 0);
}

/*
 * @tc.name: ParseRecordsFromBufferInvalidSizeZero
 * @tc.desc: test ParseRecordsFromBuffer with totalSize = 0.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferInvalidSizeZero, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> buffer(sizeof(uint32_t));
    uint32_t size = 0;
    memcpy(buffer.data(), &size, sizeof(uint32_t));

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                            bufferOffset, readfileSize);

    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ParseRecordsFromBufferInvalidSizeTooLarge
 * @tc.desc: test ParseRecordsFromBuffer with totalSize > MAX_RECORD_SIZE.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferInvalidSizeTooLarge, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> buffer(sizeof(uint32_t));
    uint32_t size = static_cast<uint32_t>(MAX_RECORD_SIZE) + 1;
    memcpy(buffer.data(), &size, sizeof(uint32_t));

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                            bufferOffset, readfileSize);

    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ParseRecordsFromBufferOnlySizeHeader
 * @tc.desc: test ParseRecordsFromBuffer with only size header.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseRecordsFromBufferOnlySizeHeader, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint8_t> buffer(sizeof(uint32_t));
    uint32_t size = 100;
    memcpy(buffer.data(), &size, sizeof(uint32_t));

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = preprocess.ParseRecordsFromBuffer(buffer.data(), buffer.size(),
                                            bufferOffset, readfileSize);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, 0);
    EXPECT_EQ(readfileSize, 0);
}

/*
 * @tc.name: ParseCacheRecordsNormalRecords
 * @tc.desc: test ParseCacheRecords with normal records in one buffer.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsNormalRecords, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> record1 = CreateValidRecord(100);
    std::vector<uint8_t> record2 = CreateValidRecord(200);
    std::vector<uint8_t> record3 = CreateValidRecord(150);

    size_t totalSize = sizeof(uint32_t) + record1.size() +
                      sizeof(uint32_t) + record2.size() +
                      sizeof(uint32_t) + record3.size();

    std::vector<uint8_t> buffer(totalSize);
    size_t offset = 0;

    uint32_t size1 = static_cast<uint32_t>(record1.size());
    memcpy(buffer.data() + offset, &size1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record1.data(), record1.size());
    offset += record1.size();

    uint32_t size2 = static_cast<uint32_t>(record2.size());
    memcpy(buffer.data() + offset, &size2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record2.data(), record2.size());
    offset += record2.size();

    uint32_t size3 = static_cast<uint32_t>(record3.size());
    memcpy(buffer.data() + offset, &size3, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record3.data(), record3.size());

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size(),
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, buffer.size());
    EXPECT_EQ(readfileSize, buffer.size());
}

/*
 * @tc.name: ParseCacheRecordsCrossBoundary
 * @tc.desc: test ParseCacheRecords with record crossing buffer boundary.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsCrossBoundary, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> record1 = CreateValidRecord(100);
    std::vector<uint8_t> record2 = CreateValidRecord(200);

    size_t totalSize = sizeof(uint32_t) + record1.size() +
                      sizeof(uint32_t) + record2.size();

    std::vector<uint8_t> buffer(totalSize);
    size_t offset = 0;

    uint32_t size1 = static_cast<uint32_t>(record1.size());
    memcpy(buffer.data() + offset, &size1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record1.data(), record1.size());
    offset += record1.size();

    uint32_t size2 = static_cast<uint32_t>(record2.size());
    memcpy(buffer.data() + offset, &size2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record2.data(), record2.size());

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size() - 50,
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_TRUE(ret);
    EXPECT_GT(bufferOffset, 0);
    EXPECT_LT(bufferOffset, buffer.size());
    EXPECT_GT(readfileSize, 0);
}

/*
 * @tc.name: ParseCacheRecordsIncompleteRecord
 * @tc.desc: test ParseCacheRecords with incomplete record at end.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsIncompleteRecord, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> record1 = CreateValidRecord(100);
    std::vector<uint8_t> record2 = CreateValidRecord(200);

    size_t partialSize = sizeof(uint32_t) + record1.size() + sizeof(uint32_t) + 50;

    std::vector<uint8_t> buffer(partialSize);
    size_t offset = 0;

    uint32_t size1 = static_cast<uint32_t>(record1.size());
    memcpy(buffer.data() + offset, &size1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record1.data(), record1.size());
    offset += record1.size();

    uint32_t size2 = static_cast<uint32_t>(record2.size());
    memcpy(buffer.data() + offset, &size2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer.data() + offset, record2.data(), 50);

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size(),
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, sizeof(uint32_t) + record1.size() + sizeof(uint32_t));
    EXPECT_EQ(readfileSize, sizeof(uint32_t) + record1.size());
}

/*
 * @tc.name: ParseCacheRecordsEmptyBuffer
 * @tc.desc: test ParseCacheRecords with empty buffer.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsEmptyBuffer, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> buffer;
    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size(),
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, 0);
    EXPECT_EQ(readfileSize, 0);
}

/*
 * @tc.name: ParseCacheRecordsInvalidSizeZero
 * @tc.desc: test ParseCacheRecords with totalSize = 0.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsInvalidSizeZero, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> buffer(sizeof(uint32_t));
    uint32_t size = 0;
    memcpy(buffer.data(), &size, sizeof(uint32_t));

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size(),
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ParseCacheRecordsInvalidSizeTooLarge
 * @tc.desc: test ParseCacheRecords with totalSize > MAX_RECORD_SIZE.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsInvalidSizeTooLarge, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> buffer(sizeof(uint32_t));
    uint32_t size = static_cast<uint32_t>(MAX_RECORD_SIZE) + 1;
    memcpy(buffer.data(), &size, sizeof(uint32_t));

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size(),
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ParseCacheRecordsOnlySizeHeader
 * @tc.desc: test ParseCacheRecords with only size header.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ParseCacheRecordsOnlySizeHeader, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<uint8_t> buffer(sizeof(uint32_t));
    uint32_t size = 100;
    memcpy(buffer.data(), &size, sizeof(uint32_t));

    size_t bufferOffset = 0;
    uint64_t readfileSize = 0;

    bool ret = hookManager.ParseCacheRecords(hookCtx, buffer.data(), buffer.size(),
                                          bufferOffset, readfileSize, MAX_RECORD_SIZE);

    EXPECT_TRUE(ret);
    EXPECT_EQ(bufferOffset, 0);
    EXPECT_EQ(readfileSize, 0);
}

/*
 * @tc.name: TakeResultsFromCacheFileEmptyFile
 * @tc.desc: test TakeResultsFromCacheFile with empty file.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, TakeResultsFromCacheFileEmptyFile, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::string testFile = "/data/local/tmp/cache_empty_test_" + std::to_string(getpid()) + ".dat";
    int fd = open(testFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    close(fd);

    fd = open(testFile.c_str(), O_RDONLY);
    preprocess.TakeResultsFromCacheFile(fd);

    unlink(testFile.c_str());
}

/*
 * @tc.name: TakeResultsFromCacheFileMultipleRecords
 * @tc.desc: test TakeResultsFromCacheFile with multiple records.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, TakeResultsFromCacheFileMultipleRecords, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<std::vector<uint8_t>> records;
    records.push_back(CreateValidRecord(100));
    records.push_back(CreateValidRecord(200));
    records.push_back(CreateValidRecord(150));

    std::string testFile = CreateTestFile(records);
    int fd = open(testFile.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    preprocess.TakeResultsFromCacheFile(fd);
    close(fd);
    RemoveTestFile(testFile);
}

/*
 * @tc.name: TakeResultsFromCacheFileLargeRecords
 * @tc.desc: test TakeResultsFromCacheFile with large records.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, TakeResultsFromCacheFileLargeRecords, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<std::vector<uint8_t>> records;
    records.push_back(CreateValidRecord(1024));
    records.push.push(CreateValidRecord(2048));
    records.push_back(CreateValidRecord(4096));

    std::string testFile = CreateTestFile(records);
    int fd = open(testFile.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    preprocess.TakeResultsFromCacheFile(fd);
    close(fd);
    RemoveTestFile(testFile);
}

/*
 * @tc.name: TakeResultsFromCacheFileInvalidFd
 * @tc.desc: test TakeResultsFromCacheFile with invalid fd.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, TakeResultsFromCacheFileInvalidFd, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.TakeResultsFromCacheFile(-1);
}

/*
 * @tc.name: ReadFromCacheFileEmptyFile
 * @tc.desc: test ReadFromCacheFile with empty file.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ReadFromCacheFileEmptyFile, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::string testFile = "/data/local/tmp/cache_empty_test_" + std::to_string(getpid()) + ".dat";
    int fd = open(testFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    close(fd);

    fd = open(testFile.c_str(), O_RDONLY);
    hookManager.ReadFromCacheFile(hookCtx, fd);

    unlink(testFile.c_str());
}

/*
 * @tc.name: ReadFromCacheFileMultipleRecords
 * @tc.desc: test ReadFromCacheFile with multiple records.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ReadFromCacheFileMultipleRecords, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<std::vector<uint8_t>> records;
    records.push_back(CreateValidRecord(100));
    records.push_back(CreateValidRecord(200));
    records.push_back(CreateValidRecord(150));

    std::string testFile = CreateTestFile(records);
    int fd = open(testFile.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    hookManager.ReadFromCacheFile(hookCtx, fd);
    close(fd);
    RemoveTestFile(testFile);
}

/*
 * @tc.name: ReadFromCacheFileLargeRecords
 * @tc.desc: test ReadFromCacheFile with large records.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ReadFromCacheFileLargeRecords, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    std::vector<std::vector<uint8_t>> records;
    records.push_back(CreateValidRecord(1024));
    records.push_back(CreateValidRecord(2048));
    records.push_back(CreateValidRecord(4096));

    std::string testFile = CreateTestFile(records);
    int fd = open(testFile.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    hookManager.ReadFromCacheFile(hookCtx, fd);
    close(fd);
    RemoveTestFile(testFile);
}

/*
 * @tc.name: ReadFromCacheFileInvalidFd
 * @tc.desc: test ReadFromCacheFile with invalid fd.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileReadTest, ReadFromCacheFileInvalidFd, TestSize.Level0)
{
    HookManager hookManager;
    auto hookCtx = std::make_shared<HookManager::HookManagerCtx>(getpid());

    hookManager.ReadFromCacheFile(hookCtx, -1);
}
} // namespace
