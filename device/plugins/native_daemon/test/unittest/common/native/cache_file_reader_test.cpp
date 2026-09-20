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
#include <cstdint>
#include <vector>
#include "cache_file_reader.h"
#include "securec.h"

using namespace testing::ext;

namespace {
constexpr size_t ZERO_TAIL_SIZE = 16;
constexpr uint32_t TEST_RECORD_SIZE = 100;
constexpr uint32_t OVERSIZED_RECORD = 33 * 1024 * 1024;
constexpr uint8_t NON_ZERO_PATTERN_BYTE = 0x5A;

int CreateTempFdWithData(const std::vector<uint8_t>& data)
{
    char path[] = "/data/local/tmp/cache_file_reader_test_XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) {
        return -1;
    }
    unlink(path);
    if (!data.empty() && write(fd, data.data(), data.size()) != static_cast<ssize_t>(data.size())) {
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);
    return fd;
}

std::vector<uint8_t> MakeRecordBuffer(uint32_t recordSize)
{
    std::vector<uint8_t> buffer(sizeof(uint32_t) + recordSize, 0);
    errno_t ret = memcpy_s(buffer.data(), buffer.size(), &recordSize, sizeof(recordSize));
    if (ret != EOK) {
        buffer.clear();
    }
    return buffer;
}

class CacheFileReaderTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

HWTEST_F(CacheFileReaderTest, GetRecordHeadValidRecord, TestSize.Level0)
{
    std::vector<uint8_t> buffer = MakeRecordBuffer(TEST_RECORD_SIZE);
    ASSERT_FALSE(buffer.empty());
    uint32_t totalSize = 0;
    EXPECT_EQ(GetRecordHead(buffer.data(), 0, false, totalSize), RECORD_HEAD_OK);
    EXPECT_EQ(totalSize, TEST_RECORD_SIZE);
}

HWTEST_F(CacheFileReaderTest, GetRecordHeadCrashFileEnd, TestSize.Level0)
{
    std::vector<uint8_t> buffer(sizeof(uint32_t), 0);
    uint32_t totalSize = 0;
    EXPECT_EQ(GetRecordHead(buffer.data(), 0, true, totalSize), RECORD_HEAD_END);
}

HWTEST_F(CacheFileReaderTest, GetRecordHeadInvalidZeroSize, TestSize.Level0)
{
    std::vector<uint8_t> buffer(sizeof(uint32_t), 0);
    uint32_t totalSize = 0;
    EXPECT_EQ(GetRecordHead(buffer.data(), 0, false, totalSize), RECORD_HEAD_INVALID);
}

HWTEST_F(CacheFileReaderTest, GetRecordHeadInvalidOversize, TestSize.Level0)
{
    std::vector<uint8_t> buffer = MakeRecordBuffer(OVERSIZED_RECORD);
    ASSERT_FALSE(buffer.empty());
    uint32_t totalSize = 0;
    EXPECT_EQ(GetRecordHead(buffer.data(), 0, false, totalSize), RECORD_HEAD_INVALID);
}

HWTEST_F(CacheFileReaderTest, GetRecordHeadNullBuffer, TestSize.Level0)
{
    uint32_t totalSize = 0;
    EXPECT_EQ(GetRecordHead(nullptr, 0, false, totalSize), RECORD_HEAD_INVALID);
    EXPECT_EQ(totalSize, 0u);
}

HWTEST_F(CacheFileReaderTest, GetRecordHeadWithRetrySkipsZeroHeads, TestSize.Level0)
{
    std::vector<uint8_t> buffer(2 * sizeof(uint32_t), 0);
    uint32_t validHead = TEST_RECORD_SIZE;
    ASSERT_EQ(memcpy_s(buffer.data() + sizeof(uint32_t), sizeof(uint32_t), &validHead,
        sizeof(validHead)), EOK);
    uint32_t offset = 0;
    uint32_t totalSize = 0;
    EXPECT_TRUE(GetRecordHeadWithRetry(buffer.data(), buffer.size(), offset, totalSize, true));
    EXPECT_EQ(offset, sizeof(uint32_t));
    EXPECT_EQ(totalSize, TEST_RECORD_SIZE);
}

HWTEST_F(CacheFileReaderTest, GetRecordHeadWithRetryExhaustedRestoresOffset, TestSize.Level0)
{
    constexpr uint32_t zeroHeadCount = MAX_ZERO_HEAD_RETRY + 2;
    std::vector<uint8_t> buffer(zeroHeadCount * sizeof(uint32_t), 0);
    uint32_t offset = 0;
    uint32_t totalSize = 0;
    EXPECT_FALSE(GetRecordHeadWithRetry(buffer.data(), buffer.size(), offset, totalSize, true));
    EXPECT_EQ(offset, 0u);

    constexpr uint32_t entryOffset = sizeof(uint32_t) * 2; // mid-buffer entry must also be restored
    offset = entryOffset;
    EXPECT_FALSE(GetRecordHeadWithRetry(buffer.data(), buffer.size(), offset, totalSize, true));
    EXPECT_EQ(offset, entryOffset);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeInvalidBuffer, TestSize.Level0)
{
    CacheFileReadContext ctx;
    ctx.readBuffer = nullptr;
    EXPECT_EQ(GetCacheFileReadSize(ctx), 0u);
    EXPECT_FALSE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeInvalidRemainSize, TestSize.Level0)
{
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    CacheFileReadContext ctx;
    ctx.readBuffer = buffer.data();
    ctx.remainSize = FILE_CACHE_READ_BUFFER_SIZE;
    EXPECT_EQ(GetCacheFileReadSize(ctx), 0u);
    EXPECT_FALSE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeCrashZeroStops, TestSize.Level0)
{
    std::vector<uint8_t> data(CRASH_ZERO_THRESHOLD + ZERO_TAIL_SIZE, 0);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.readBuffer = buffer.data();
    ctx.isCrashFile = true;
    EXPECT_EQ(GetCacheFileReadSize(ctx), 0u);
    EXPECT_TRUE(ctx.readDone);
    close(fd);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeCrashSkipsProbeWhenMidRecord, TestSize.Level0)
{
    std::vector<uint8_t> data(CRASH_ZERO_THRESHOLD + ZERO_TAIL_SIZE, 0);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    constexpr uint32_t bufferedRemainSize = 512;
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.readBuffer = buffer.data();
    ctx.remainSize = bufferedRemainSize;
    ctx.isCrashFile = true;
    EXPECT_EQ(GetCacheFileReadSize(ctx), FILE_CACHE_READ_BUFFER_SIZE - bufferedRemainSize);
    EXPECT_FALSE(ctx.readDone);
    close(fd);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeCrashDataContinues, TestSize.Level0)
{
    std::vector<uint8_t> data(TEST_RECORD_SIZE, NON_ZERO_PATTERN_BYTE);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.readBuffer = buffer.data();
    ctx.isCrashFile = true;
    EXPECT_EQ(GetCacheFileReadSize(ctx), FILE_CACHE_READ_BUFFER_SIZE);
    EXPECT_FALSE(ctx.readDone);
    close(fd);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeExhausted, TestSize.Level0)
{
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    CacheFileReadContext ctx;
    ctx.readBuffer = buffer.data();
    ctx.readfileSize = TEST_RECORD_SIZE;
    ctx.bytesToRead = TEST_RECORD_SIZE;
    EXPECT_EQ(GetCacheFileReadSize(ctx), 0u);
    EXPECT_FALSE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeBoundedByDataSize, TestSize.Level0)
{
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    constexpr uint32_t readOffset = 100;
    constexpr uint32_t totalToRead = 1000;
    CacheFileReadContext ctx;
    ctx.readBuffer = buffer.data();
    ctx.readfileSize = readOffset;
    ctx.bytesToRead = totalToRead;
    EXPECT_EQ(GetCacheFileReadSize(ctx), totalToRead - readOffset);
    EXPECT_FALSE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeBoundedWithRemainSize, TestSize.Level0)
{
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    constexpr uint32_t readOffset = 100;
    constexpr uint32_t bufferedRemainSize = 24;
    constexpr uint32_t totalToRead = 1000;
    CacheFileReadContext ctx;
    ctx.readBuffer = buffer.data();
    ctx.readfileSize = readOffset;
    ctx.remainSize = bufferedRemainSize;
    ctx.bytesToRead = totalToRead;
    EXPECT_EQ(GetCacheFileReadSize(ctx), totalToRead - readOffset - bufferedRemainSize);
    EXPECT_FALSE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, GetCacheFileReadSizeExhaustedWithRemainSize, TestSize.Level0)
{
    std::vector<uint8_t> buffer(FILE_CACHE_READ_BUFFER_SIZE, 0);
    constexpr uint32_t readOffset = 976;
    constexpr uint32_t bufferedRemainSize = 24;
    constexpr uint32_t totalToRead = 1000;
    CacheFileReadContext ctx;
    ctx.readBuffer = buffer.data();
    ctx.readfileSize = readOffset;
    ctx.remainSize = bufferedRemainSize;
    ctx.bytesToRead = totalToRead;
    EXPECT_EQ(GetCacheFileReadSize(ctx), 0u);
    EXPECT_FALSE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, HandleOversizeRecordInvalidBuffer, TestSize.Level0)
{
    CacheFileReadContext ctx;
    ctx.readBuffer = nullptr;
    EXPECT_FALSE(HandleOversizeRecord(ctx));
}

HWTEST_F(CacheFileReaderTest, HandleOversizeRecordCrashStops, TestSize.Level0)
{
    std::vector<uint8_t> buffer = MakeRecordBuffer(TEST_RECORD_SIZE);
    ASSERT_FALSE(buffer.empty());
    CacheFileReadContext ctx;
    ctx.readBuffer = buffer.data();
    ctx.isCrashFile = true;
    ctx.readDone = false;
    EXPECT_TRUE(HandleOversizeRecord(ctx));
    EXPECT_TRUE(ctx.readDone);
}

HWTEST_F(CacheFileReaderTest, HandleOversizeRecordSkipsRecord, TestSize.Level0)
{
    std::vector<uint8_t> buffer = MakeRecordBuffer(TEST_RECORD_SIZE);
    ASSERT_FALSE(buffer.empty());
    std::vector<uint8_t> data(TEST_RECORD_SIZE * 2, 0);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    constexpr uint32_t remainSizeValue = 20;
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.readBuffer = buffer.data();
    ctx.remainSize = remainSizeValue;
    ctx.isCrashFile = false;
    ctx.readDone = false;
    EXPECT_TRUE(HandleOversizeRecord(ctx));
    EXPECT_FALSE(ctx.readDone);
    EXPECT_EQ(ctx.readfileSize, static_cast<uint64_t>(sizeof(uint32_t)) + TEST_RECORD_SIZE);
    EXPECT_EQ(ctx.remainSize, 0u);
    EXPECT_EQ(lseek(fd, 0, SEEK_CUR),
        static_cast<off_t>(TEST_RECORD_SIZE) + static_cast<off_t>(sizeof(uint32_t)) -
        static_cast<off_t>(remainSizeValue));
    close(fd);
}

HWTEST_F(CacheFileReaderTest, ReadCacheFileHeaderValid, TestSize.Level0)
{
    uint32_t headerValue = TEST_RECORD_SIZE;
    std::vector<uint8_t> data(DATA_SIZE_HEADER_LEN, 0);
    ASSERT_EQ(memcpy_s(data.data(), data.size(), &headerValue, sizeof(headerValue)), EOK);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    uint32_t readSize = 0;
    EXPECT_TRUE(ReadCacheFileHeader(fd, readSize));
    EXPECT_EQ(readSize, static_cast<uint64_t>(TEST_RECORD_SIZE));
    EXPECT_EQ(lseek(fd, 0, SEEK_CUR), static_cast<off_t>(DATA_SIZE_HEADER_LEN));
    close(fd);
}

HWTEST_F(CacheFileReaderTest, ReadCacheFileHeaderShortFile, TestSize.Level0)
{
    std::vector<uint8_t> data(DATA_SIZE_HEADER_LEN - 1, 0);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    uint32_t readSize = 1;
    EXPECT_FALSE(ReadCacheFileHeader(fd, readSize));
    EXPECT_EQ(readSize, 0u);
    close(fd);
}

HWTEST_F(CacheFileReaderTest, ReadCacheFileHeaderInvalidFd, TestSize.Level0)
{
    uint32_t readSize = 1;
    EXPECT_FALSE(ReadCacheFileHeader(-1, readSize));
    EXPECT_EQ(readSize, 0u);
}

HWTEST_F(CacheFileReaderTest, SanitizeCacheDataSizeBranches, TestSize.Level0)
{
    uint32_t dataSize = MAX_CACHE_DATA_SIZE;
    EXPECT_TRUE(SanitizeCacheDataSize(dataSize));
    EXPECT_EQ(dataSize, MAX_CACHE_DATA_SIZE);

    dataSize = MAX_CACHE_DATA_SIZE + 1;
    EXPECT_FALSE(SanitizeCacheDataSize(dataSize));
    EXPECT_EQ(dataSize, 0u);

    dataSize = 0;
    EXPECT_TRUE(SanitizeCacheDataSize(dataSize));
    EXPECT_EQ(dataSize, 0u);
}

HWTEST_F(CacheFileReaderTest, ReadCacheFileHeaderOversizeFallsBackToCrash, TestSize.Level0)
{
    uint32_t headerValue = static_cast<uint32_t>(MMAP_FILE_SIZE);
    std::vector<uint8_t> data(DATA_SIZE_HEADER_LEN, 0);
    ASSERT_EQ(memcpy_s(data.data(), data.size(), &headerValue, sizeof(headerValue)), EOK);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    uint32_t readSize = 1;
    EXPECT_TRUE(ReadCacheFileHeader(fd, readSize));
    ASSERT_FALSE(SanitizeCacheDataSize(readSize));
    EXPECT_EQ(readSize, 0u);
    close(fd);
}

HWTEST_F(CacheFileReaderTest, IsBufferAllZeroBranches, TestSize.Level0)
{
    std::vector<uint8_t> zeros(CRASH_ZERO_THRESHOLD, 0);
    std::vector<uint8_t> mixed(CRASH_ZERO_THRESHOLD, 0);
    mixed[CRASH_ZERO_THRESHOLD - 1] = NON_ZERO_PATTERN_BYTE;
    EXPECT_FALSE(IsBufferAllZero(nullptr, zeros.size()));
    EXPECT_FALSE(IsBufferAllZero(zeros.data(), 0));
    EXPECT_TRUE(IsBufferAllZero(zeros.data(), zeros.size()));
    EXPECT_FALSE(IsBufferAllZero(mixed.data(), mixed.size()));
}

HWTEST_F(CacheFileReaderTest, IsNextCacheDataAllZeroAtZeroRegion, TestSize.Level0)
{
    std::vector<uint8_t> data(CRASH_ZERO_THRESHOLD + ZERO_TAIL_SIZE, 0);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    EXPECT_TRUE(IsNextCacheDataAllZero(fd));
    close(fd);
}

HWTEST_F(CacheFileReaderTest, IsNextCacheDataAllZeroAtDataRegion, TestSize.Level0)
{
    std::vector<uint8_t> data(CRASH_ZERO_THRESHOLD + ZERO_TAIL_SIZE, NON_ZERO_PATTERN_BYTE);
    int fd = CreateTempFdWithData(data);
    ASSERT_GE(fd, 0);
    EXPECT_FALSE(IsNextCacheDataAllZero(fd));
    close(fd);
}

HWTEST_F(CacheFileReaderTest, IsNextCacheDataAllZeroAtEof, TestSize.Level0)
{
    int fd = CreateTempFdWithData({});
    ASSERT_GE(fd, 0);
    EXPECT_TRUE(IsNextCacheDataAllZero(fd));
    close(fd);
}

HWTEST_F(CacheFileReaderTest, IsNextCacheDataAllZeroInvalidFd, TestSize.Level0)
{
    EXPECT_TRUE(IsNextCacheDataAllZero(-1));
}
} // namespace
