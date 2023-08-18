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

#include <dlfcn.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "stack_writer.h"
#include "logging.h"

using namespace testing::ext;

namespace {
constexpr uint32_t BUFFER_SIZE = 1024;
constexpr uint32_t SMB_SIZE = 10 * 4096;
const std::string SMB_NAME = "stackwritertest";
const std::string PLUGIN_NAME = "stackwriter";
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
        HILOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr = mmap(nullptr, SMB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr == (reinterpret_cast<void *>(-1))) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr mmap ERR : %s", buf);
        return -1;
    }

    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr);

    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    return fd;
}

class StackWriterTest : public ::testing::Test {
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

    header_->info.readOffset_ = header_->info.writeOffset_;
    if (memcmp(buffer, cmpaddr, size) == 0) {
        return true;
    }
    return false;
}

void RandData(uint8_t* data, int size)
{
    time_t tv = time(nullptr);
    if (tv == -1) {
        tv = 1;
    }
    unsigned int seed = (unsigned int)tv;
    while (--size) {
        data[size] = rand_r(&seed) / (uint8_t)(-1);
    }
}

/**
 * @tc.name: StackWriter
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(StackWriterTest, WriteaNormalTest, TestSize.Level1)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};

    EXPECT_TRUE(write->Write((const void*)buffer1, sizeof(buffer1)));
    EXPECT_TRUE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->Write((const void*)buffer2, sizeof(buffer2)));
    EXPECT_TRUE(CheckBuffer(buffer2, sizeof(buffer2)));
    EXPECT_TRUE(write->Write((const void*)buffer3, sizeof(buffer3)));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));

    EXPECT_FALSE(write->Write(nullptr, 0));
}

/**
 * @tc.name: StackWriter
 * @tc.desc: Write failure process.
 * @tc.type: FUNC
 */
HWTEST_F(StackWriterTest, WriteaFalseTest, TestSize.Level1)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, 0, -1);
    EXPECT_EQ(write->shareMemoryBlock_, nullptr);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};

    EXPECT_FALSE(write->Write((const void*)buffer1, sizeof(buffer1)));
    EXPECT_FALSE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_FALSE(write->Write((const void*)buffer2, sizeof(buffer2)));
    EXPECT_FALSE(CheckBuffer(buffer2, sizeof(buffer2)));
    EXPECT_FALSE(write->Write((const void*)buffer3, sizeof(buffer3)));
    EXPECT_FALSE(CheckBuffer(buffer3, sizeof(buffer3)));

    EXPECT_FALSE(write->Write(nullptr, 0));
}

/**
 * @tc.name: StackWriter
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(StackWriterTest, WriteProcessTest, TestSize.Level1)
{
    uint8_t data[BUFFER_SIZE];
    RandData(data, BUFFER_SIZE);
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);
    long bytes = BUFFER_SIZE;

    EXPECT_GT(write->Write((const void*)data, sizeof(data)), 0);
    EXPECT_TRUE(CheckBuffer(data, BUFFER_SIZE));
    EXPECT_TRUE(write->Flush());
    write->DoStats(bytes);
    write->Report();

    EXPECT_EQ((int)write->bytesCount_, bytes);
    EXPECT_EQ((int)write->bytesPending_, bytes);
    EXPECT_EQ((int)write->writeCount_, 1);
    EXPECT_EQ((int)write->flushCount_, 1);

    EXPECT_TRUE(write->Flush());
    write->DoStats(bytes);
    write->Report();

    EXPECT_EQ((int)write->bytesCount_, bytes + bytes);
    EXPECT_EQ((int)write->bytesPending_, bytes);
    EXPECT_EQ((int)write->writeCount_, 2);
    EXPECT_EQ((int)write->flushCount_, 2);
}

/**
 * @tc.name: StackWriter
 * @tc.desc: Write data to shared memory with blocked mode.
 * @tc.type: FUNC
 */
HWTEST_F(StackWriterTest, WriterSyncTest, TestSize.Level1)
{
    auto write = std::make_shared<StackWriter>(PLUGIN_NAME, SMB_SIZE, g_smbFd, -1, true);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t buffer4[] = {0xCC, 0xDD, 0xBB, 0xEE};

    EXPECT_FALSE(write->WriteWithPayloadTimeout(nullptr, 0, nullptr, 0));
    EXPECT_TRUE(write->WriteWithPayloadTimeout((const void*)buffer1, sizeof(buffer1),
        (const void*)buffer2, sizeof(buffer2)));
    EXPECT_TRUE(CheckBuffer(buffer1, sizeof(buffer1)));
    write->blocked_ = false;
    EXPECT_TRUE(write->WriteWithPayloadTimeout((const void*)buffer3, sizeof(buffer3),
        (const void*)buffer4, sizeof(buffer4)));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));
    EXPECT_FALSE(write->Write(nullptr, 0));
    EXPECT_FALSE(write->WriteTimeout(nullptr, 0));
    EXPECT_TRUE(write->WriteTimeout((const void*)buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->Clear());
}
} // namespace
