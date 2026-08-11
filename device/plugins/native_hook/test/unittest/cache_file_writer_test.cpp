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
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <thread>
#include <unistd.h>
#include <string>
#include <vector>
#define private public
#include "cache_file_writer.h"
#undef private

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr size_t TEST_DATA_SIZE = 128;
const std::string TEST_CACHE_DIR = "/data/local/tmp/cache_file_writer_test/";

#ifdef __aarch64__
static bool IsNonHmKernel()
{
    utsname buf;
    if (uname(&buf) != 0) {
        return false;
    }
    return std::string(buf.release).find("HongMeng") == std::string::npos;
}
#endif

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
    std::vector<char> buffer(TEST_DATA_SIZE + 16, 0);
    ssize_t bytes = read(fd, buffer.data(), buffer.size());
    if (bytes <= 0) {
        return "";
    }
    return std::string(buffer.data(), static_cast<size_t>(bytes));
}

bool AddressInNamedVma(uintptr_t addr, size_t size, const std::string& vmaName)
{
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        uintptr_t start = 0;
        uintptr_t end = 0;
        char dash = 0;
        std::istringstream iss(line);
        if (!(iss >> std::hex >> start >> dash >> end)) {
            continue;
        }
        if (addr >= start && addr + size <= end && line.find(vmaName) != std::string::npos) {
            return true;
        }
    }
    return false;
}

class CacheFileWriterTest : public ::testing::Test {
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

HWTEST_F(CacheFileWriterTest, AddCacheFileAndWrite, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    int fd = CreateAnonCacheFd();
    ASSERT_GE(fd, 0);
    ASSERT_TRUE(writer.AddCacheFile(0, fd));
    uint8_t data[TEST_DATA_SIZE] = {0x5A};
    ASSERT_TRUE(writer.Write(data, sizeof(data)));
    ASSERT_TRUE(writer.Flush());
    EXPECT_GE(writer.currentFileFd_, 0);
    EXPECT_FALSE(ReadAll(writer.currentFileFd_).empty());
}

HWTEST_F(CacheFileWriterTest, WriteFailsWithoutWritableFd, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    uint8_t data[TEST_DATA_SIZE] = {0};
    EXPECT_FALSE(writer.Write(data, sizeof(data)));
}

HWTEST_F(CacheFileWriterTest, RotateReturnsReadyFd, TestSize.Level0)
{
    std::vector<FileEventRecord> events;
    CacheFileWriter writer(1);
    writer.Prepare([&events](uint16_t fileIndex, uint16_t eventType, int fd) {
        FileEventRecord record = {fileIndex, eventType, -1};
        if (fd >= 0) {
            record.dupFd = dup(fd);
        }
        events.push_back(record);
        return true;
    });
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    ASSERT_TRUE(writer.AddCacheFile(1, CreateAnonCacheFd()));
    writer.currentFileFd_ = writer.pendingFileFds_[0];
    writer.pendingFileFds_[0] = -1;
    writer.currentFileIndex_ = 0;
    uint8_t existingData[TEST_DATA_SIZE] = {0x11};
    ASSERT_TRUE(writer.WriteToFile(existingData, sizeof(existingData)));
    writer.currentFileSize_ = 96 * 1024 * 1024;
    uint8_t data[TEST_DATA_SIZE] = {0x33};
    writer.WriteToMemoryCache(data, sizeof(data));
    ASSERT_TRUE(writer.FlushMemoryCacheToFile());
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].eventType, CACHE_FILE_READY);
    EXPECT_EQ(events[0].fileIndex, 0);
    EXPECT_EQ(writer.currentFileIndex_, 1);
    EXPECT_FALSE(ReadAll(events[0].dupFd).empty());
    EXPECT_FALSE(ReadAll(writer.currentFileFd_).empty());
    close(events[0].dupFd);
    writer.Prepare(nullptr);
}

HWTEST_F(CacheFileWriterTest, StopModeSkipsMaxFileSizeRotation, TestSize.Level0)
{
    std::vector<FileEventRecord> events;
    CacheFileWriter writer(1);
    writer.Prepare([&events](uint16_t fileIndex, uint16_t eventType, int fd) {
        FileEventRecord record = {fileIndex, eventType, -1};
        if (fd >= 0) {
            record.dupFd = dup(fd);
        }
        events.push_back(record);
        return true;
    });
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    ASSERT_TRUE(writer.AddCacheFile(1, CreateAnonCacheFd()));
    writer.currentFileFd_ = writer.pendingFileFds_[0];
    writer.pendingFileFds_[0] = -1;
    writer.currentFileIndex_ = 0;
    writer.currentFileSize_ = 96 * 1024 * 1024;
    writer.EnterStopMode();
    uint8_t data[TEST_DATA_SIZE] = {0x66};
    writer.WriteToMemoryCache(data, sizeof(data));
    ASSERT_TRUE(writer.FlushMemoryCacheToFile());
    EXPECT_TRUE(events.empty());
    EXPECT_EQ(writer.currentFileIndex_, 0);
    EXPECT_GT(writer.currentFileSize_, 96 * 1024 * 1024);
    writer.Prepare(nullptr);
}

HWTEST_F(CacheFileWriterTest, WaitForNextCacheFileTimeout, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    writer.currentFileFd_ = writer.pendingFileFds_[0];
    writer.pendingFileFds_[0] = -1;
    writer.currentFileIndex_ = 0;
    writer.currentFileSize_ = 96 * 1024 * 1024;
    uint8_t data[TEST_DATA_SIZE] = {0x77};
    writer.WriteToMemoryCache(data, sizeof(data));
    auto start = std::chrono::steady_clock::now();
    EXPECT_FALSE(writer.FlushMemoryCacheToFile());
    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_GE(elapsed, std::chrono::seconds(3));
}

HWTEST_F(CacheFileWriterTest, StopReturnsReadyThenStop, TestSize.Level0)
{
    std::vector<FileEventRecord> events;
    CacheFileWriter writer(1);
    writer.Prepare([&events](uint16_t fileIndex, uint16_t eventType, int fd) {
        FileEventRecord record = {fileIndex, eventType, -1};
        if (fd >= 0) {
            record.dupFd = dup(fd);
        }
        events.push_back(record);
        return true;
    });
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    uint8_t data[TEST_DATA_SIZE] = {0x22};
    ASSERT_TRUE(writer.Write(data, sizeof(data)));
    ASSERT_TRUE(writer.Flush());
    writer.SetCacheFileWriterEnd();
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].eventType, CACHE_FILE_READY);
    EXPECT_EQ(events[0].fileIndex, 0);
    EXPECT_EQ(events[1].eventType, CACHE_FILE_STOP);
    EXPECT_EQ(events[1].fileIndex, INVALID_CACHE_FILE_INDEX);
    EXPECT_FALSE(ReadAll(events[0].dupFd).empty());
    close(events[0].dupFd);
    writer.Prepare(nullptr);
}

HWTEST_F(CacheFileWriterTest, AddDuplicateSlotFails, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    int dupFd = CreateAnonCacheFd();
    ASSERT_GE(dupFd, 0);
    EXPECT_FALSE(writer.AddCacheFile(0, dupFd));
}

HWTEST_F(CacheFileWriterTest, AddDuplicateSlotFailDoesNotCloseFd, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    int dupFd = CreateAnonCacheFd();
    ASSERT_GE(dupFd, 0);

    EXPECT_FALSE(writer.AddCacheFile(0, dupFd));
    EXPECT_EQ(fcntl(dupFd, F_GETFD), 0);
    close(dupFd);
}

HWTEST_F(CacheFileWriterTest, WriteWithPayloadStoresCombinedRecord, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    ASSERT_TRUE(writer.WriteWithPayload(header, sizeof(header), payload, sizeof(payload)));
    ASSERT_TRUE(writer.Flush());
    std::string content = ReadAll(writer.currentFileFd_);
    EXPECT_GT(content.size(), sizeof(uint32_t) + sizeof(header));
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataAllParts, TestSize.Level0)
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
    EXPECT_GE(content.size(), sizeof(uint32_t) + expectedTotalSize);
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataNoPayload, TestSize.Level0)
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

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataNoFirstExtra, TestSize.Level0)
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

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataNoSecondExtra, TestSize.Level0)
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

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataOnlyHeader, TestSize.Level0)
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

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataNullHeader, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    EXPECT_FALSE(writer.WriteWithExtraData(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataZeroSize, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    EXPECT_FALSE(writer.WriteWithExtraData(header, 0, nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataExceedsCacheSize, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    size_t hugeSize = writer.memoryCacheDataSizeLimit_ + 1;
    std::vector<uint8_t> hugeData(hugeSize, 0);
    
    EXPECT_FALSE(writer.WriteWithExtraData(hugeData.data(), hugeSize, nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataBeforeInit, TestSize.Level0)
{
    CacheFileWriter writer(1);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    EXPECT_FALSE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataAfterEnd, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ASSERT_TRUE(writer.Write(header, sizeof(header)));
    
    writer.SetCacheFileWriterEnd();
    
    EXPECT_FALSE(writer.WriteWithExtraData(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0));
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataMultipleWrites, TestSize.Level0)
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

/*
 * @tc.name: UpdateBackpressureStateSetsDelay
 * @tc.desc: test UpdateBackpressureState sets currentBackpressureDelay_ correctly.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, UpdateBackpressureStateSetsDelay, TestSize.Level0)
{
    CacheFileWriter writer(1);
    
    BackpressureStateMsg state;
    state.recommendedDelayNs = 50000;
    
    writer.UpdateBackpressureState(state);
    
    EXPECT_EQ(writer.currentBackpressureDelay_.load(), 50000u);
}

/*
 * @tc.name: UpdateBackpressureStateOverwritesPrevious
 * @tc.desc: test UpdateBackpressureState overwrites previous delay value.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, UpdateBackpressureStateOverwritesPrevious, TestSize.Level0)
{
    CacheFileWriter writer(1);
    
    BackpressureStateMsg state1;
    state1.recommendedDelayNs = 20000;
    writer.UpdateBackpressureState(state1);
    EXPECT_EQ(writer.currentBackpressureDelay_.load(), 20000u);
    
    BackpressureStateMsg state2;
    state2.recommendedDelayNs = 80000;
    writer.UpdateBackpressureState(state2);
    EXPECT_EQ(writer.currentBackpressureDelay_.load(), 80000u);
}

/*
 * @tc.name: ShouldApplyBackpressureReturnsFalseWhenStopMode
 * @tc.desc: test ShouldApplyBackpressure returns false when stopModeEnabled_ is true.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ShouldApplyBackpressureReturnsFalseWhenStopMode, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.stopModeEnabled_ = true;
    writer.currentBackpressureDelay_ = 50000;
    
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
}

/*
 * @tc.name: ShouldApplyBackpressureReturnsFalseWhenEnded
 * @tc.desc: test ShouldApplyBackpressure returns false when isCacheFileEnded_ is true.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ShouldApplyBackpressureReturnsFalseWhenEnded, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.isCacheFileEnded_ = true;
    writer.currentBackpressureDelay_ = 50000;
    
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
}

/*
 * @tc.name: ShouldApplyBackpressureReturnsFalseWhenZeroDelay
 * @tc.desc: test ShouldApplyBackpressure returns false when currentBackpressureDelay_ is zero.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ShouldApplyBackpressureReturnsFalseWhenZeroDelay, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = 0;
    
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
}

/*
 * @tc.name: ShouldApplyBackpressureSamplingRate
 * @tc.desc: test ShouldApplyBackpressure returns true only at sample rate intervals.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ShouldApplyBackpressureSamplingRate, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = 50000;
    writer.delayCount_ = 0;
    
    uint32_t trueCount = 0;
    uint32_t falseCount = 0;
    
    for (int i = 0; i < 100; ++i) {
        if (writer.ShouldApplyBackpressure()) {
            trueCount++;
        } else {
            falseCount++;
        }
    }
    
    EXPECT_EQ(trueCount, 100 / BACKPRESSURE_SAMPLE_SIZE);
    EXPECT_GT(falseCount, 0u);
}

/*
 * @tc.name: ShouldApplyBackpressureCounterOverflow
 * @tc.desc: test ShouldApplyBackpressure handles counter overflow correctly.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ShouldApplyBackpressureCounterOverflow, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = 50000;
    writer.delayCount_ = UINT64_MAX - 1;
    
    EXPECT_TRUE(writer.ShouldApplyBackpressure());
    EXPECT_EQ(writer.delayCount_.load(), 0);
}

/*
 * @tc.name: ShouldApplyBackpressureConsecutiveCalls
 * @tc.desc: test ShouldApplyBackpressure behavior across consecutive calls.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ShouldApplyBackpressureConsecutiveCalls, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = 50000;
    writer.delayCount_ = 0;
    
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
    EXPECT_TRUE(writer.ShouldApplyBackpressure());
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
}

/*
 * @tc.name: ApplyBackpressureDelayAppliesNonZeroDelay
 * @tc.desc: test ApplyBackpressureDelay applies delay when delay is non-zero and within limit.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ApplyBackpressureDelayAppliesNonZeroDelay, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = BACKPRESSURE_HIGH_DELAY_NS;
    writer.stopModeEnabled_ = false;
    writer.isCacheFileEnded_ = false;
    
    auto start = std::chrono::steady_clock::now();
    writer.ApplyBackpressureDelay();
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_GE(elapsed, std::chrono::microseconds(50));
    EXPECT_LT(elapsed, std::chrono::milliseconds(10));
}

/*
 * @tc.name: ApplyBackpressureDelaySkipsDelayAboveLimit
 * @tc.desc: test ApplyBackpressureDelay skips delay when currentBackpressureDelay_ > BACKPRESSURE_HIGH_DELAY_NS.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ApplyBackpressureDelaySkipsDelayAboveLimit, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = BACKPRESSURE_HIGH_DELAY_NS + 1000;
    writer.stopModeEnabled_ = false;
    writer.isCacheFileEnded_ = false;
    
    auto start = std::chrono::steady_clock::now();
    writer.ApplyBackpressureDelay();
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_LT(elapsed, std::chrono::milliseconds(10));
}

/*
 * @tc.name: ApplyBackpressureDelayStopModeFlagCheck
 * @tc.desc: test ApplyBackpressureDelay checks stopModeEnabled_ flag during wait loop.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ApplyBackpressureDelayStopModeFlagCheck, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = BACKPRESSURE_HIGH_DELAY_NS;
    writer.stopModeEnabled_ = true;
    writer.isCacheFileEnded_ = false;
    
    auto start = std::chrono::steady_clock::now();
    writer.ApplyBackpressureDelay();
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_LT(elapsed, std::chrono::milliseconds(10));
}

/*
 * @tc.name: ApplyBackpressureDelayCacheEndedFlagCheck
 * @tc.desc: test ApplyBackpressureDelay checks isCacheFileEnded_ flag during wait loop.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ApplyBackpressureDelayCacheEndedFlagCheck, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = BACKPRESSURE_HIGH_DELAY_NS;
    writer.stopModeEnabled_ = false;
    writer.isCacheFileEnded_ = true;
    
    auto start = std::chrono::steady_clock::now();
    writer.ApplyBackpressureDelay();
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_LT(elapsed, std::chrono::milliseconds(10));
}

/*
 * @tc.name: ApplyBackpressureDelayZeroDelayCheck
 * @tc.desc: test ApplyBackpressureDelay checks zero delay first.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, ApplyBackpressureDelayZeroDelayCheck, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.currentBackpressureDelay_ = 0;
    writer.stopModeEnabled_ = false;
    writer.isCacheFileEnded_ = false;
    
    auto start = std::chrono::steady_clock::now();
    writer.ApplyBackpressureDelay();
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_LT(elapsed, std::chrono::milliseconds(10));
}

/*
 * @tc.name: BackpressureIntegrationTest
 * @tc.desc: test backpressure mechanism integrated with write operations.
 * @tc.type: FUNC
 */
HWTEST_F(CacheFileWriterTest, BackpressureIntegrationTest, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    
    BackpressureStateMsg state;
    state.recommendedDelayNs = 20000;
    writer.UpdateBackpressureState(state);
    
    EXPECT_EQ(writer.currentBackpressureDelay_.load(), 20000u);
    
    writer.delayCount_ = BACKPRESSURE_SAMPLE_SIZE - 1;
    EXPECT_TRUE(writer.ShouldApplyBackpressure());
    
    writer.currentBackpressureDelay_ = 0;
    EXPECT_FALSE(writer.ShouldApplyBackpressure());
}

HWTEST_F(CacheFileWriterTest, MemoryCacheIsOffHeapAndNamedVma, TestSize.Level0)
{
#ifdef __aarch64__
    if (IsNonHmKernel()) {
        GTEST_SKIP() << "Skip on non-HM kernel";
    }
#endif
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.memoryCache_ != nullptr);
    ASSERT_GT(writer.memoryCacheCapacity_, 0u);
    EXPECT_EQ(writer.memoryCacheSize_, 0u);
    bool inNamedVma = AddressInNamedVma(reinterpret_cast<uintptr_t>(writer.memoryCache_),
                                        writer.memoryCacheCapacity_, "hooknativesmb");
    EXPECT_TRUE(inNamedVma);
}

HWTEST_F(CacheFileWriterTest, WriteWithExtraDataRollsBackOnPartialCopy, TestSize.Level0)
{
    CacheFileWriter writer(1);
    writer.Prepare(nullptr);
    ASSERT_TRUE(writer.AddCacheFile(0, CreateAnonCacheFd()));
    // Shrink the reported capacity so a later segment overflows the bounds
    // check after the length prefix and header have already been copied.
    writer.memoryCacheCapacity_ = sizeof(uint32_t) + 8;
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    EXPECT_FALSE(writer.WriteWithExtraData(header, sizeof(header), payload, sizeof(payload),
        nullptr, 0, nullptr, 0));
    EXPECT_EQ(writer.memoryCacheSize_, 0u);
}
} // namespace
