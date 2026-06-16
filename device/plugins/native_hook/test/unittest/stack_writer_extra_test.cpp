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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "stack_writer.h"
#include "logging.h"
#include "hook_common.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr uint32_t BUFFER_SIZE = 1024;
constexpr uint32_t SMB_SIZE = 10 * 4096;
const std::string SMB_NAME = "stackwriterextratest";
const std::string PLUGIN_NAME = "stackwriter_extra";
void* g_smbAddr = nullptr;
int g_smbFd = 0;

int InitShareMemory()
{
    int fd = syscall(SYS_memfd_create, SMB_NAME.c_str(), 0);
    CHECK_TRUE(fd >= 0, -1, "CreateBlock FAIL SYS_memfd_create");

    int check = ftruncate(fd, SMB_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr = mmap(nullptr, SMB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr == (reinterpret_cast<void *>(-1))) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr mmap ERR : %s", buf);
        return -1;
    }

    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr);

    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    return fd;
}

class StackWriterExtraTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp()
    {
        g_smbFd = InitShareMemory();
    }
    void TearDown()
    {
        g_smbFd = 0;
    }
};

bool CheckBuffer(uint8_t* buffer, size_t size)
{
    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr);
    uint8_t* cmpaddr = (uint8_t*)g_smbAddr + sizeof(ShareMemoryBlock::BlockHeader) + header_->info.readOffset_;
    cmpaddr = cmpaddr + sizeof(uint32_t);

    header_->info.readOffset_ = header_->info.writeOffset_.load();
    if (memcmp(buffer, cmpaddr, size) == 0) {
        return true;
    }
    return false;
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutAllParts, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    uint8_t secondExtra[4] = {19, 20, 21, 22};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra), nullptr));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutNoPayload, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    uint8_t secondExtra[4] = {19, 20, 21, 22};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), nullptr, 0,
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra), nullptr));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutNoFirstExtra, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t secondExtra[4] = {19, 20, 21, 22};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), payload, sizeof(payload),
        nullptr, 0, secondExtra, sizeof(secondExtra), nullptr));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutNoSecondExtra, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), nullptr, 0, nullptr));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutOnlyHeader, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0, nullptr));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutNullHeader, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    EXPECT_FALSE(write->WriteWithExtraDataTimeout(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutZeroSize, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    EXPECT_FALSE(write->WriteWithExtraDataTimeout(header, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutNoShareMemory, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, 0, -1, false);
    EXPECT_EQ(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    EXPECT_FALSE(write->WriteWithExtraDataTimeout(header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0, nullptr));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutBlockedMode, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, true);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    uint8_t secondExtra[4] = {19, 20, 21, 22};

    auto callback = []() { return false; };

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra), callback));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutWithDynamicTag, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = 1234;
    baseData->tid = 5678;

    std::string tagName = "libtest_dynamic.so";
    uint8_t firstExtra[32] = {0};
    memcpy_s(firstExtra, sizeof(firstExtra), tagName.c_str(), tagName.size() + 1);

    uint32_t originSize = sizeof(BaseStackRawData);
    uint8_t secondExtra[sizeof(uint32_t)] = {0};
    memcpy_s(secondExtra, sizeof(uint32_t), &originSize, sizeof(uint32_t));

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), nullptr, 0,
        firstExtra, tagName.size() + 1, secondExtra, sizeof(uint32_t), nullptr));
    EXPECT_TRUE(CheckBuffer(header, sizeof(header)));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutMultipleWrites, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t header1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t header2[8] = {11, 12, 13, 14, 15, 16, 17, 18};
    uint8_t firstExtra[4] = {19, 20, 21, 22};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header1, sizeof(header1), nullptr, 0,
        firstExtra, sizeof(firstExtra), nullptr, 0, nullptr));
    EXPECT_TRUE(CheckBuffer(header1, sizeof(header1)));

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header2, sizeof(header2), nullptr, 0,
        firstExtra, sizeof(firstExtra), nullptr, 0, nullptr));
}

HWTEST_F(StackWriterExtraTest, WriteWithExtraDataTimeoutFileCacheMode, TestSize.Level0)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, false);
    write->useFileCache_ = true;
    write->cacheFileWriter_ = std::make_shared<CacheFileWriter>(1);
    write->cacheFileWriter_->Prepare(nullptr);

    int fd = syscall(SYS_memfd_create, "test_cache", 0);
    if (fd >= 0) {
        ftruncate(fd, 4096);
        ASSERT_TRUE(write->cacheFileWriter_->AddCacheFile(0, fd));
    }

    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};

    EXPECT_TRUE(write->WriteWithExtraDataTimeout(header, sizeof(header), nullptr, 0,
        firstExtra, sizeof(firstExtra), nullptr, 0, nullptr));

    write->cacheFileWriter_->Prepare(nullptr);
}
} // namespace