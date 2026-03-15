/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <ctime>
#define private public
#include "cache_file_writer.h"
#undef private
#include "hook_common.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr size_t TEST_DATA_SIZE = 1024;
constexpr size_t SMALL_DATA_SIZE = 16;
const std::string TEST_CACHE_DIR = "/data/local/tmp/cache_file_writer_test/";
const std::string TEST_FILE_PREFIX = "native_hook_cache_";

class CacheFileWriterTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        mkdir(TEST_CACHE_DIR.c_str(), 0755);
    }
    static void TearDownTestCase()
    {
        DIR* dir = opendir(TEST_CACHE_DIR.c_str());
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                std::string fullPath = TEST_CACHE_DIR + entry->d_name;
                unlink(fullPath.c_str());
            }
            closedir(dir);
        }
        rmdir(TEST_CACHE_DIR.c_str());
    }
    void SetUp() {}
    void TearDown() {}

protected:
    void CreateTestFile(const std::string& fileName)
    {
        std::string fullPath = TEST_CACHE_DIR + fileName;
        int fd = open(fullPath.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd >= 0) {
            close(fd);
        }
    }

    void RemoveTestFile(const std::string& fileName)
    {
        std::string fullPath = TEST_CACHE_DIR + fileName;
        unlink(fullPath.c_str());
    }

    bool FileExists(const std::string& fileName)
    {
        std::string fullPath = TEST_CACHE_DIR + fileName;
        struct stat st;
        return stat(fullPath.c_str(), &st) == 0;
    }
};

/*
 * @tc.name: ConstructorTest
 * @tc.desc: Test CacheFileWriter constructor.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ConstructorTest_01, TestSize.Level0)
{
    auto writer = std::make_shared<CacheFileWriter>(0, 0);
    EXPECT_TRUE(writer != nullptr);
    EXPECT_TRUE(writer->cacheMemorySize_ == DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES);
}

/*
 * @tc.name: ConstructorTest
 * @tc.desc: Test CacheFileWriter constructor.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ConstructorTest_02, TestSize.Level0)
{
    uint32_t size = MIN_FILE_CACHE_MEMORY_SIZE;
    auto writer = std::make_shared<CacheFileWriter>(0, size);
    EXPECT_TRUE(writer != nullptr);
    EXPECT_TRUE(writer->cacheMemorySize_ == size * MB_TO_BYTES);
}

/*
 * @tc.name: PrepareTest
 * @tc.desc: Test CacheFileWriter::Prepare with callback.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, PrepareTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();
    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    bool callbackCalled = false;
    auto callback = [&callbackCalled](uint16_t fileIndex, uint16_t eventType, int fd) {
        callbackCalled = true;
    };
    writer->Prepare(callback);
    EXPECT_TRUE(writer->fileEventCallback_ != nullptr);
}

/*
 * @tc.name: PrepareWithNullCallbackTest
 * @tc.desc: Test CacheFileWriter::Prepare with null callback.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, PrepareWithNullCallbackTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->Prepare(nullptr);
    EXPECT_TRUE(writer->fileEventCallback_ == nullptr);
}

/*
 * @tc.name: WriteNormalTest
 * @tc.desc: Test CacheFileWriter::Write with normal data.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteNormalTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    uint8_t data[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->Write(data, sizeof(data)));
}

/*
 * @tc.name: WriteWithInvalidDataTest
 * @tc.desc: Test CacheFileWriter::Write with invalid data.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithInvalidDataTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t data[TEST_DATA_SIZE];
    EXPECT_FALSE(writer->Write(nullptr, TEST_DATA_SIZE));
    EXPECT_FALSE(writer->Write(data, 0));
    EXPECT_FALSE(writer->Write(data, DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES + 1));
}

/*
 * @tc.name: WriteWithoutPrepareTest
 * @tc.desc: Test CacheFileWriter::Write without Prepare.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithoutPrepareTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    uint8_t data[TEST_DATA_SIZE];
    EXPECT_FALSE(writer->Write(data, sizeof(data)));
}

/*
 * @tc.name: WriteWithPayloadNormalTest
 * @tc.desc: Test CacheFileWriter::WriteWithPayload with normal data.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithPayloadNormalTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    uint8_t data[TEST_DATA_SIZE];
    uint8_t payload[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->WriteWithPayload(data, sizeof(data), payload, sizeof(payload)));
}

/*
 * @tc.name: WriteWithPayloadNullDataTest
 * @tc.desc: Test CacheFileWriter::WriteWithPayload with null data.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithPayloadNullDataTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t payload[SMALL_DATA_SIZE];
    EXPECT_FALSE(writer->WriteWithPayload(nullptr, TEST_DATA_SIZE, payload, sizeof(payload)));
}

/*
 * @tc.name: WriteWithPayloadZeroSizeTest
 * @tc.desc: Test CacheFileWriter::WriteWithPayload with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithPayloadZeroSizeTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t data[SMALL_DATA_SIZE];
    uint8_t payload[SMALL_DATA_SIZE];
    EXPECT_FALSE(writer->WriteWithPayload(data, 0, payload, sizeof(payload)));
}

/*
 * @tc.name: WriteWithPayloadNullPayloadTest
 * @tc.desc: Test CacheFileWriter::WriteWithPayload with null payload.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithPayloadNullPayloadTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t data[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->WriteWithPayload(data, sizeof(data), nullptr, 0));
}

/*
 * @tc.name: WriteWithPayloadOversizedDataTest
 * @tc.desc: Test CacheFileWriter::WriteWithPayload with oversized data.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteWithPayloadOversizedDataTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t data[TEST_DATA_SIZE];
    uint8_t payload[SMALL_DATA_SIZE];
    EXPECT_FALSE(writer->WriteWithPayload(data, DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES + 1,
        payload, sizeof(payload)));
}

/*
 * @tc.name: FlushTest
 * @tc.desc: Test CacheFileWriter::Flush with data in cache.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, FlushTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t data[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->Write(data, sizeof(data)));
    EXPECT_TRUE(writer->Flush());
}

/*
 * @tc.name: FlushEmptyCacheTest
 * @tc.desc: Test CacheFileWriter::Flush with empty cache.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, FlushEmptyCacheTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    EXPECT_TRUE(writer->Flush());
}

/*
 * @tc.name: SetCacheFileWriterEndTest
 * @tc.desc: Test CacheFileWriter::SetCacheFileWriterEnd.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, SetCacheFileWriterEndTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    uint8_t data[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->Write(data, sizeof(data)));
    writer->SetCacheFileWriterEnd();
    EXPECT_TRUE(writer->Flush());
}

/*
 * @tc.name: GetCacheFilePathTest
 * @tc.desc: Test CacheFileWriter::GetCacheFilePath.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, GetCacheFilePathTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    std::string path = writer->GetCacheFilePath(1);
    EXPECT_TRUE(path.find(TEST_FILE_PREFIX) != std::string::npos);
    EXPECT_TRUE(path.find("1") != std::string::npos);
}

/*
 * @tc.name: WriteToFileTest
 * @tc.desc: Test CacheFileWriter::WriteToFile with valid fd.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteToFileTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    uint8_t data[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->WriteToFile(data, sizeof(data)));
}

/*
 * @tc.name: WriteToFileWithInvalidFdTest
 * @tc.desc: Test CacheFileWriter::WriteToFile with invalid fd.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteToFileWithInvalidFdTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    uint8_t data[TEST_DATA_SIZE];
    EXPECT_TRUE(writer->WriteToFile(data, sizeof(data)));
}

/*
 * @tc.name: WriteToFileZeroSizeTest
 * @tc.desc: Test CacheFileWriter::WriteToFile with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, WriteToFileZeroSizeTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    uint8_t data[SMALL_DATA_SIZE];
    EXPECT_TRUE(writer->WriteToFile(data, 0));
}

/*
 * @tc.name: CreateNewCacheFileTest
 * @tc.desc: Test CacheFileWriter::CreateNewCacheFile.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, CreateNewCacheFileTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    EXPECT_TRUE(writer->CreateNewCacheFile());
    EXPECT_GE(writer->currentFileFd_, 0);
}

/*
 * @tc.name: FlushMemoryCacheToFileEmptyTest
 * @tc.desc: Test CacheFileWriter::FlushMemoryCacheToFile with empty cache.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, FlushMemoryCacheToFileEmptyTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);
    EXPECT_TRUE(writer->FlushMemoryCacheToFile());
}

/*
 * @tc.name: FlushMemoryCacheToFileWithInvalidFdTest
 * @tc.desc: Test CacheFileWriter::FlushMemoryCacheToFile with invalid fd.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, FlushMemoryCacheToFileWithInvalidFdTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    uint8_t data[SMALL_DATA_SIZE];
    writer->WriteToMemoryCache(data, sizeof(data));
    EXPECT_FALSE(writer->FlushMemoryCacheToFile());
}

/*
 * @tc.name: FlushMemoryCacheToFileWithCacheFileEndedTest
 * @tc.desc: Test CacheFileWriter::FlushMemoryCacheToFile with cache file ended flag.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, FlushMemoryCacheToFileWithCacheFileEndedTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    uint8_t data[TEST_DATA_SIZE];
    writer->WriteToMemoryCache(data, sizeof(data));
    writer->SetCacheFileWriterEnd();
    EXPECT_TRUE(writer->FlushMemoryCacheToFile());
}

/*
 * @tc.name: FlushMemoryCacheToFileWithMaxSizeTest
 * @tc.desc: Test CacheFileWriter::FlushMemoryCacheToFile when exceeding max file size.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, FlushMemoryCacheToFileWithMaxSizeTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    writer->Prepare(nullptr);

    // Simulate near max file size
    writer->currentFileSize_ = 96 * 1024 * 1024; // 96MB;
    uint8_t data[TEST_DATA_SIZE];
    writer->WriteToMemoryCache(data, sizeof(data));
    EXPECT_TRUE(writer->FlushMemoryCacheToFile());
}

/*
 * @tc.name: NotifyFileEventTest
 * @tc.desc: Test CacheFileWriter::NotifyFileEvent.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, NotifyFileEventTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    EXPECT_TRUE(writer != nullptr);
    writer->NotifyFileEvent(1, CACHE_FILE_CREATED);
    writer->NotifyFileEvent(1, CACHE_FILE_READY);
    writer->NotifyFileEvent(1, CACHE_FILE_STOP);
    writer->NotifyFileEvent(1, CACHE_FILE_STOP + 1);
    EXPECT_TRUE(writer->eventNotifier_ != nullptr);
}


/*
 * @tc.name: NotifyFileEventTest
 * @tc.desc: Test CacheFileWriter::NotifyFileEvent with invalid eventNotifier_.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, NotifyFileEventInvalidTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    EXPECT_TRUE(writer != nullptr);
    writer->eventNotifier_ = nullptr;
    writer->NotifyFileEvent(1, CACHE_FILE_CREATED);
    writer->NotifyFileEvent(1, CACHE_FILE_READY);
    writer->NotifyFileEvent(1, CACHE_FILE_STOP);
    writer->NotifyFileEvent(1, CACHE_FILE_STOP + 1);
    EXPECT_TRUE(writer->eventNotifier_ == nullptr);
}

/*
 * @tc.name: CleanOldCacheFilesTest
 * @tc.desc: Test CacheFileWriter::CleanOldCacheFiles.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, CleanOldCacheFilesTest, TestSize.Level0)
{
    auto eventNotifier = EventNotifier::Create(0, 0);
    EXPECT_TRUE(eventNotifier != nullptr);
    int eventFd = eventNotifier->GetFd();

    // Create some test cache files
    CreateTestFile("native_hook_cache_test1.bin");
    CreateTestFile("native_hook_cache_test2.bin");
    CreateTestFile("other_file.txt"); // Should not be cleaned

    auto writer = std::make_shared<CacheFileWriter>(eventFd, 0);
    writer->cacheFileDir_ = TEST_CACHE_DIR;
    EXPECT_TRUE(writer->CleanOldCacheFiles());
    EXPECT_FALSE(FileExists("native_hook_cache_test1.bin"));
    EXPECT_FALSE(FileExists("native_hook_cache_test2.bin"));
    EXPECT_TRUE(FileExists("other_file.txt"));
}
} // namespace
