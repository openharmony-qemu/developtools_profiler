/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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
#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#define private public
#include "cache_file_writer.h"
#include "hook_common.h"
#undef private

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr size_t TEST_DATA_SIZE = 128;
const std::string TEST_CACHE_DIR = "/data/local/tmp/cache_file_writer_extra_test/";

struct FileEventRecord {
    uint16_t fileIndex = INVALID_CACHE_FILE_INDEX;
    uint16_t eventType = CACHE_FILE_STOP;
    int dupFd = -1;
};

int CreateAnonCacheFd()
{
    std::string fileTemplate = TEST_CACHE_DIR + "cache_fd_XXXXXX";
    std::vector<char> filePath(fileTemplate.begin(), fileTemplate.end());
    filePath.push_back('\0');
    int fd = mkstemp(filePath.data());
    if (fd >= 0) {
        unlink(filePath.data());
    }
    return fd;
}

std::string ReadAll(int fd)
{
    lseek(fd, 0, SEEK_SET);
    std::vector<char> buffer(TEST_DATA_SIZE + 64, 0);
    ssize_t bytes = read(fd, buffer.data(), buffer.size());
    if (bytes <= 0) {
        return "";
    }
    return std::string(buffer.data(), static_cast<size_t>(bytes));
}

class CacheFileWriterExtraTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        mkdir(TEST_CACHE_DIR.c_str(), 0755);
    }

    static void TearDownTestCase()
    {
        rmdir(TEST_CACHE_DIR.c_str());
    }
};

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataAllParts, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    uint8_t secondExtra[4] = {19, 20, 21, 22};
    
    ASSERT_TRUE(writer.WriteWithExtraData(header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra)));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    uint32_t expectedTotalSize = sizeof(header) + sizeof(payload) + sizeof(firstExtra) + sizeof(secondExtra);
    EXPECT_GT(content.size(), sizeof(uint32_t) + expectedTotalSize);
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataNoPayload, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    uint8_t secondExtra[4] = {19, 20, 21, 22};
    
    ASSERT_TRUE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0,
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra)));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataNoFirstExtra, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t secondExtra[4] = {19, 20, 21, 22};
    
    ASSERT_TRUE(writer.WriteWithExtraData(header, sizeof(header), payload, sizeof(payload),
        nullptr, 0, secondExtra, sizeof(secondExtra)));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataNoSecondExtra, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    
    ASSERT_TRUE(writer.WriteWithExtraData(header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), nullptr, 0));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataOnlyHeader, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    ASSERT_TRUE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataNullHeader, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    EXPECT_FALSE(writer.WriteWithExtraData(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataZeroSize, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    EXPECT_FALSE(writer.WriteWithExtraData(header, 0, nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataExceedsCacheSize, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    size_t hugeSize = writer.cacheMemorySize_ + 1;
    std::vector<uint8_t> hugeData(hugeSize, 0);
    
    EXPECT_FALSE(writer.WriteWithExtraData(hugeData.data(), hugeSize, nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataBeforeInit, TestSize.Level0)
{
    CacheFileWriter writer(1);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    EXPECT_FALSE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataAfterEnd, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ASSERT_TRUE(writer.Write(header, sizeof(header)));
    
    writer.SetCacheFileWriterEnd();
    
    EXPECT_FALSE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataDynamicTagFormat, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    std::string tagName = "libtest.so";
    uint8_t firstExtra[32] = {0};
    memcpy_s(firstExtra, sizeof(firstExtra), tagName.c_str(), tagName.size() + 1);
    
    uint32_t originSize = sizeof(BaseStackRawData);
    uint8_t secondExtra[sizeof(uint32_t)] = {0};
    memcpy_s(secondExtra, sizeof(uint32_t), &originSize, sizeof(uint32_t));
    
    ASSERT_TRUE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0,
        firstExtra, tagName.size() + 1, secondExtra, sizeof(uint32_t)));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t));
}

HWTEST_F(CacheFileWriterExtraTest, WriteWithExtraDataMultipleWrites, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t header2[8] = {11, 12, 13, 14, 15, 16, 17, 18};
    uint8_t firstExtra[4] = {19, 20, 21, 22};
    uint8_t secondExtra[4] = {23, 24, 25, 26};
    
    ASSERT_TRUE(writer.WriteWithExtraData(header1, sizeof(header1), nullptr, 0,
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra)));
    ASSERT_TRUE(writer.WriteWithExtraData(header2, sizeof(header2), nullptr, 0,
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra)));
    ASSERT_TRUE(writer.Flush());
    
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t) * 2);
}
} // namespace