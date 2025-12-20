/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <cstring>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "plugin_service_types.pb.h"
#include "share_memory_block.h"
#include <log_base.h>

using namespace testing::ext;

namespace {
constexpr uint32_t SMB_SIZE = 10 * 4096;
constexpr size_t ARRAYSIZE = 1024;
const std::string SMB_NAME = "shared_memory_block_test";
const std::string PLUGIN_NAME = "shared_memory_block";
const std::string PLUGIN_NAME_SECOND = "shared_memory_block_second";
const std::string PLUGIN_NAME_THIRD = "shared_memory_block_third";
constexpr int PAGE_SIZE = 4096;
constexpr int NUM_FOUR = 4;
constexpr int NUM_FIVE = 5;
void* g_smbAddr = nullptr;
int g_smbFd = 0;

int InitShareMemory()
{
    int fd = syscall(SYS_memfd_create, SMB_NAME.c_str(), 0);
    if (fd < 0) {
        HILOG_BASE_WARN(LOG_CORE, "CreateBlock FAIL SYS_memfd_create");
        return -1;
    }

    int check = ftruncate(fd, SMB_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_BASE_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr = mmap(nullptr, SMB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr == (reinterpret_cast<void *>(-1))) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_BASE_ERROR(LOG_CORE, "CreateBlock g_smbAddr mmap ERR : %s", buf);
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

class SharedMemoryBlockTest : public testing::Test {
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

/**
 * @tc.name: share memory
 * @tc.desc: read lock.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, ReadLock, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: get name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, GetName, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    shareMemoryBlock.GetName();

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: get size.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, GetSize, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    shareMemoryBlock.GetSize();

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: get file descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, GetfileDescriptor, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    shareMemoryBlock.GetfileDescriptor();

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory type test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, DROP_NONE, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    shareMemoryBlock.SetReusePolicy(ShareMemoryBlock::ReusePolicy::DROP_NONE);

    int8_t data[ARRAYSIZE];
    for (int i = 0; i < NUM_FIVE; i++) {
        *((uint32_t*)data) = i;
        shareMemoryBlock.PutRaw(data, ARRAYSIZE);
    }
    int8_t* p = shareMemoryBlock.GetFreeMemory(ARRAYSIZE);
    ASSERT_TRUE(p == nullptr);

    do {
        p = const_cast<int8_t*>(shareMemoryBlock.GetDataPoint());
    } while (shareMemoryBlock.Next() && shareMemoryBlock.GetDataSize() > 0);

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory type test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, DROP_OLD, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    shareMemoryBlock.SetReusePolicy(ShareMemoryBlock::ReusePolicy::DROP_OLD);

    int8_t data[ARRAYSIZE];
    for (int i = 0; i < NUM_FIVE; i++) {
        *((uint32_t*)data) = i;
        shareMemoryBlock.PutRaw(data, ARRAYSIZE);
    }
    int8_t* p = shareMemoryBlock.GetFreeMemory(ARRAYSIZE);
    ASSERT_TRUE(p != nullptr);

    do {
        p = const_cast<int8_t*>(shareMemoryBlock.GetDataPoint());
    } while (shareMemoryBlock.Next() && shareMemoryBlock.GetDataSize() > 0);

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: put protobuf.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, PutMessage, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());
    ASSERT_TRUE(shareMemoryBlock.GetDataSize() == 0);

    NotifyResultResponse response;
    response.set_status(ResponseStatus::OK);
    ASSERT_TRUE(shareMemoryBlock.PutMessage(response, "test"));
    EXPECT_EQ(shareMemoryBlock.GetDataSize(), response.ByteSizeLong());
    response.ParseFromArray(shareMemoryBlock.GetDataPoint(), shareMemoryBlock.GetDataSize());
    ASSERT_TRUE(response.status() == ResponseStatus::OK);

    // 调用next移动指针，取值正常
    shareMemoryBlock.Next();
    NotifyResultResponse response2;
    response2.set_status(ResponseStatus::OK);
    ASSERT_TRUE(shareMemoryBlock.PutMessage(response2, "test"));
    EXPECT_EQ(shareMemoryBlock.GetDataSize(), response2.ByteSizeLong());
    response2.ParseFromArray(shareMemoryBlock.GetDataPoint(), shareMemoryBlock.GetDataSize());
    EXPECT_TRUE(response2.status() == ResponseStatus::OK);

    // 调用next，设置空message
    shareMemoryBlock.Next();
    NotifyResultRequest request;
    ASSERT_TRUE(shareMemoryBlock.PutMessage(request, "test"));
    EXPECT_EQ(shareMemoryBlock.GetDataSize(), request.ByteSizeLong());

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory PutMessage abnormal test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, PutMessageAbnormal, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());
    ASSERT_TRUE(shareMemoryBlock.GetDataSize() == 0);

    NotifyResultResponse response;
    response.set_status(ResponseStatus::OK);
    ASSERT_TRUE(shareMemoryBlock.PutMessage(response, "test"));
    EXPECT_EQ(shareMemoryBlock.GetDataSize(), response.ByteSizeLong());
    response.ParseFromArray(shareMemoryBlock.GetDataPoint(), shareMemoryBlock.GetDataSize());
    ASSERT_TRUE(response.status() == ResponseStatus::OK);

    // 不调用next无法移动指针，取值出错
    NotifyResultResponse response2;
    response2.set_status(ResponseStatus::ERR);
    ASSERT_TRUE(shareMemoryBlock.PutMessage(response2, "test"));
    EXPECT_EQ(shareMemoryBlock.GetDataSize(), response2.ByteSizeLong());
    EXPECT_EQ(shareMemoryBlock.GetDataSize(), response.ByteSizeLong());
    response2.ParseFromArray(shareMemoryBlock.GetDataPoint(), shareMemoryBlock.GetDataSize());
    EXPECT_FALSE(response2.status() == ResponseStatus::ERR);
    EXPECT_TRUE(response2.status() == ResponseStatus::OK);

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory PutRaw abnormal test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, PutRawAbnormal, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    ASSERT_FALSE(shareMemoryBlock.PutRaw(nullptr, ARRAYSIZE));
    ASSERT_NE(shareMemoryBlock.GetFreeMemory(ARRAYSIZE), nullptr);

    int8_t data[ARRAYSIZE];
    ASSERT_FALSE(shareMemoryBlock.PutRaw(data, 0));
    ASSERT_NE(shareMemoryBlock.GetFreeMemory(0), nullptr);

    ASSERT_FALSE(shareMemoryBlock.PutRaw(data, PAGE_SIZE + 1));
    ASSERT_EQ(shareMemoryBlock.GetFreeMemory(PAGE_SIZE + 1), nullptr);

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

bool function(const int8_t data[], uint32_t size)
{
    auto pluginData = std::make_shared<ProfilerPluginData>();
    const int len = 6;
    return pluginData->ParseFromArray(reinterpret_cast<const char*>(data), len);
}

bool functionErr(const int8_t data[], uint32_t size)
{
    auto pluginData = std::make_shared<ProfilerPluginData>();
    return pluginData->ParseFromArray(reinterpret_cast<const char*>(data), PAGE_SIZE);
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory TakeData test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, TakeData, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    // 不匹配的空message
    NotifyResultRequest request;
    ASSERT_TRUE(shareMemoryBlock.PutMessage(request, "test"));
    ASSERT_TRUE(shareMemoryBlock.GetDataSize() == 0);
    EXPECT_FALSE(shareMemoryBlock.TakeData(function));

    // 不匹配的非空message
    shareMemoryBlock.Next();
    NotifyResultResponse response;
    response.set_status(ResponseStatus::OK);
    ASSERT_TRUE(shareMemoryBlock.PutMessage(response, "test"));
    EXPECT_FALSE(shareMemoryBlock.GetDataSize() == 0);
    EXPECT_FALSE(shareMemoryBlock.TakeData(function));

    // 匹配的空message
    shareMemoryBlock.Next();
    ProfilerPluginData data;
    ASSERT_TRUE(shareMemoryBlock.PutMessage(data, "test"));
    ASSERT_TRUE(shareMemoryBlock.GetDataSize() == 0);
    EXPECT_FALSE(shareMemoryBlock.TakeData(function));

    // 匹配的非空message, 但DataSize设置为大值
    shareMemoryBlock.Next();
    data.set_name("test");
    ASSERT_TRUE(shareMemoryBlock.PutMessage(data, "test"));
    EXPECT_FALSE(shareMemoryBlock.GetDataSize() == 0);
    EXPECT_FALSE(shareMemoryBlock.TakeData(functionErr));

    // 匹配的非空message,正确的DataSize
    shareMemoryBlock.Next();
    data.set_name("test");
    ASSERT_TRUE(shareMemoryBlock.PutMessage(data, "test"));
    EXPECT_FALSE(shareMemoryBlock.GetDataSize() == 0);
    EXPECT_TRUE(shareMemoryBlock.TakeData(function));

    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory PutRawTimeout
 * @tc.desc: Shared memory type test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, PutRawTimeout, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());
    shareMemoryBlock.SetReusePolicy(ShareMemoryBlock::ReusePolicy::DROP_OLD);
    int8_t data[ARRAYSIZE];
    for (int i = 0; i < NUM_FIVE; i++) {
        *((uint32_t*)data) = i;
        shareMemoryBlock.PutRawTimeout(data, ARRAYSIZE);
    }
    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}
/**
 * @tc.name: share memory PutWithPayloadTimeout
 * @tc.desc: Shared memory type test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, PutWithPayloadTimeout, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());
    shareMemoryBlock.SetReusePolicy(ShareMemoryBlock::ReusePolicy::DROP_OLD);
    int8_t data[ARRAYSIZE];
    int8_t header[ARRAYSIZE];
    for (int i = 0; i < NUM_FIVE; i++) {
        *((uint32_t*)data) = i;
        *((uint32_t*)header) = i + 1;
        shareMemoryBlock.PutWithPayloadTimeout(header, ARRAYSIZE, data, ARRAYSIZE);
    }
    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory PutWithPayloadSync
 * @tc.desc: Shared memory type test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, PutWithPayloadSync, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", 8192);
    ASSERT_TRUE(shareMemoryBlock.Valid());
    shareMemoryBlock.SetReusePolicy(ShareMemoryBlock::ReusePolicy::DROP_OLD);
    int8_t data[ARRAYSIZE];
    int8_t header[ARRAYSIZE];
    const int size = 2;
    for (int i = 0; i < size; i++) {
        *((uint32_t*)data) = i;
        *((uint32_t*)header) = i + 1;
        shareMemoryBlock.PutWithPayloadSync(header, ARRAYSIZE, data, ARRAYSIZE, nullptr);
    }
    ASSERT_TRUE(shareMemoryBlock.ReleaseBlock());
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory GetMemory test with no write data.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, GetMemory, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    // There are only 12 bytes left in shared memory.
    const int expectedSize = 12;
    int usedSize = PAGE_SIZE - sizeof(ShareMemoryBlock::BlockHeader) - NUM_FOUR - expectedSize;
    shareMemoryBlock.UseMemory(usedSize);
    shareMemoryBlock.ResetPos();

    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    const int fieldSize = 10;
    auto ret = shareMemoryBlock.GetMemory(fieldSize, &memory, &offset);
    EXPECT_FALSE(ret);
    EXPECT_EQ(memory, nullptr);
    EXPECT_EQ(offset, 0);

    memory = nullptr;
    offset = 0;
    ret = shareMemoryBlock.GetMemory(NUM_FOUR, &memory, &offset);
    EXPECT_TRUE(ret);
    EXPECT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FOUR);
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory GetMemory test with write data.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, GetMemoryAndWriteData, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    // There are only 20 bytes left in shared memory.
    const int expectedSize = 20;
    int usedSize = PAGE_SIZE - sizeof(ShareMemoryBlock::BlockHeader) - NUM_FOUR - expectedSize;
    shareMemoryBlock.UseMemory(usedSize);
    shareMemoryBlock.ResetPos();
    // False data, return true is in order to offset the rp.
    auto ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        return true;
    });
    EXPECT_TRUE(ret);

    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    const int len = 6;
    ret = shareMemoryBlock.GetMemory(len, &memory, &offset);
    EXPECT_TRUE(ret);
    ASSERT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FOUR);
    const uint8_t data[] = {0x0A, 0x04, 0x31, 0x32, 0x33, 0x34};
    ret = memcpy_s(memory, len, data, len);
    EXPECT_EQ(ret, EOK);
    shareMemoryBlock.UseMemory(len);
    shareMemoryBlock.ResetPos();

    // offset rp to be equal to wp.
    ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        return true;
    });
    EXPECT_TRUE(ret);

    // rp == wp, in order to move wp until wp is less than rp.
    memory = nullptr;
    offset = 0;
    const int fieldSize1 = 100;
    ret = shareMemoryBlock.GetMemory(fieldSize1, &memory, &offset);
    EXPECT_TRUE(ret);
    EXPECT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FOUR);
    shareMemoryBlock.UseMemory(fieldSize1);
    shareMemoryBlock.ResetPos();

    // rp > wp
    memory = nullptr;
    offset = 0;
    const int fieldSize2 = 1000;
    ret = shareMemoryBlock.GetMemory(fieldSize2, &memory, &offset);
    EXPECT_TRUE(ret);
    EXPECT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FOUR);
}

/**
 * @tc.name: share memory
 * @tc.desc: Shared memory TakeDataOptimize test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, TakeDataOptimize, TestSize.Level1)
{
    ShareMemoryBlock shareMemoryBlock("testname", PAGE_SIZE);
    ASSERT_TRUE(shareMemoryBlock.Valid());

    // There are only 21 bytes left in shared memory.
    const int expectedSize = 21;
    int usedSize = PAGE_SIZE - sizeof(ShareMemoryBlock::BlockHeader) - NUM_FOUR - expectedSize;
    shareMemoryBlock.UseMemory(usedSize);
    shareMemoryBlock.ResetPos();

    ProfilerPluginData pluginData;
    // False data, Parsing failed, return true is in order to offset the rp
    auto ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        int retval = pluginData.ParseFromArray(reinterpret_cast<const char*>(data), size);
        EXPECT_FALSE(retval);
        return true;
    });
    EXPECT_TRUE(ret);
    EXPECT_STREQ(pluginData.name().c_str(), "");

    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    const int len = 7;
    ret = shareMemoryBlock.GetMemory(len, &memory, &offset);
    EXPECT_TRUE(ret);
    ASSERT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FOUR);
    const uint8_t data[] = {0x0A, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35};
    ret = memcpy_s(memory, len, data, len);
    EXPECT_EQ(ret, EOK);
    shareMemoryBlock.UseMemory(len);
    shareMemoryBlock.ResetPos();

    // func return false
    ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        return false;
    });
    EXPECT_FALSE(ret);

    // rp < wp
    ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        int retval = pluginData.ParseFromArray(reinterpret_cast<const char*>(data), size);
        EXPECT_TRUE(retval);
        return true;
    });
    EXPECT_TRUE(ret);
    EXPECT_STREQ(pluginData.name().c_str(), "12345");

    memory = nullptr;
    offset = 0;
    ret = shareMemoryBlock.GetMemory(1, &memory, &offset);
    EXPECT_TRUE(ret);
    ASSERT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FOUR);
    const uint8_t data1[] = {0x0A};
    const int fieldSize = 2;
    ret = memcpy_s(memory, 1, data1, 1);
    EXPECT_EQ(ret, EOK);
    ret = shareMemoryBlock.Seek(NUM_FIVE);
    EXPECT_TRUE(ret);
    ret = shareMemoryBlock.GetMemory(fieldSize, &memory, &offset);
    EXPECT_TRUE(ret);
    ASSERT_NE(memory, nullptr);
    EXPECT_EQ(offset, NUM_FIVE);
    const uint8_t data2[] = {0x01, 0x37};
    ret = memcpy_s(memory, fieldSize, data2, fieldSize);
    EXPECT_EQ(ret, EOK);

    // wp < rp && wp == 0
    ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        return true;
    });
    EXPECT_FALSE(ret);
    shareMemoryBlock.UseMemory(NUM_FOUR - 1);
    shareMemoryBlock.ResetPos();

    // wp < rp
    ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        int retval = pluginData.ParseFromArray(reinterpret_cast<const char*>(data), size);
        EXPECT_TRUE(retval);
        return true;
    });
    EXPECT_TRUE(ret);
    EXPECT_STREQ(pluginData.name().c_str(), "7");

    // rp == wp
    ret = shareMemoryBlock.TakeDataOptimize([&](const int8_t data[], uint32_t size) -> bool {
        return true;
    });
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, WriteaNormalTest, TestSize.Level0)
{
    auto write = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, SMB_SIZE, g_smbFd);
    EXPECT_NE(write->Valid(), false);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};

    EXPECT_TRUE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer1), sizeof(buffer1)));
    EXPECT_TRUE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer2), sizeof(buffer2)));
    EXPECT_TRUE(CheckBuffer(buffer2, sizeof(buffer2)));
    EXPECT_TRUE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer3), sizeof(buffer3)));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));

    EXPECT_FALSE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer3), 0));
    EXPECT_FALSE(write->PutRaw(nullptr, 0));
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: Write failure process.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, WriteaFalseTest, TestSize.Level0)
{
    auto write = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, SMB_SIZE, 0);
    EXPECT_NE(write->Valid(), true);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};

    EXPECT_FALSE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer1), sizeof(buffer1)));
    EXPECT_FALSE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_FALSE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer2), sizeof(buffer2)));
    EXPECT_FALSE(CheckBuffer(buffer2, sizeof(buffer2)));
    EXPECT_FALSE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer3), sizeof(buffer3)));
    EXPECT_FALSE(CheckBuffer(buffer3, sizeof(buffer3)));

    EXPECT_FALSE(write->PutRaw(reinterpret_cast<const int8_t*>(buffer3), 0));
    EXPECT_FALSE(write->PutRaw(nullptr, 0));
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: test Write with two shared memory block.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, WriteTwoSmbTest, TestSize.Level0)
{
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    auto write1 = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, SMB_SIZE, g_smbFd);
    auto write2 = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME_SECOND, SMB_SIZE, InitShareMemory());
    EXPECT_NE(write1->Valid(), false);
    EXPECT_NE(write2->Valid(), false);
    EXPECT_TRUE(write1->PutRaw(reinterpret_cast<const int8_t*>(buffer1), sizeof(buffer1)));
    EXPECT_FALSE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write2->PutRaw(reinterpret_cast<const int8_t*>(buffer2), sizeof(buffer2)));
    EXPECT_TRUE(CheckBuffer(buffer2, sizeof(buffer2)));
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: test WriteTimeout with three shared memory block.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, WriteTimeoutMultiSmbTest, TestSize.Level0)
{
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};
    auto write1 = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, SMB_SIZE, g_smbFd);
    auto write2 = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME_SECOND, SMB_SIZE, InitShareMemory());
    auto write3 = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME_THIRD, SMB_SIZE, InitShareMemory());
    EXPECT_NE(write1->Valid(), false);
    EXPECT_NE(write2->Valid(), false);
    EXPECT_NE(write3->Valid(), false);
    EXPECT_FALSE(write1->PutRawTimeout(nullptr, sizeof(buffer1)));
    EXPECT_FALSE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_FALSE(write2->PutRawTimeout(reinterpret_cast<const int8_t*>(buffer2), 0));
    EXPECT_FALSE(CheckBuffer(buffer2, sizeof(buffer2)));
    EXPECT_TRUE(write3->PutRawTimeout(reinterpret_cast<const int8_t*>(buffer3), sizeof(buffer3)));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: test PutWithPayloadTimeout with three shared memory block.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, WritePutWithPayloadTimeout, TestSize.Level0)
{
    auto write = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, SMB_SIZE, g_smbFd);
    EXPECT_NE(write->Valid(), false);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t buffer4[] = {0xCC, 0xDD, 0xBB, 0xEE};

    EXPECT_FALSE(write->PutWithPayloadTimeout(nullptr, 0, nullptr, 0));
    EXPECT_TRUE(write->PutWithPayloadTimeout(reinterpret_cast<const int8_t*>(buffer1), sizeof(buffer1),
        reinterpret_cast<const int8_t*>(buffer2), sizeof(buffer2)));
    EXPECT_TRUE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->PutWithPayloadTimeout(reinterpret_cast<const int8_t*>(buffer3), sizeof(buffer3),
        reinterpret_cast<const int8_t*>(buffer4), sizeof(buffer4)));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));
    EXPECT_TRUE(write->Valid());
    EXPECT_FALSE(write->PutWithPayloadTimeout(reinterpret_cast<const int8_t*>(buffer3), 0, nullptr, 0));
    EXPECT_FALSE(write->PutWithPayloadTimeout(nullptr, 0, nullptr, 0));
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: SharedMemoryBlockTest failure test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, SharedMemoryWriterFalseTest, TestSize.Level0)
{
    auto write = std::make_shared<ShareMemoryBlock>("", SMB_SIZE, 0);
    EXPECT_EQ(write->Valid(), false);

    write = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, 0, 0);
    EXPECT_EQ(write->Valid(), false);

    write = std::make_shared<ShareMemoryBlock>("", 0, 0);
    EXPECT_EQ(write->Valid(), false);

    write = std::make_shared<ShareMemoryBlock>("", SMB_SIZE, 0);
    EXPECT_EQ(write->Valid(), false);

    write = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, 0, 0);
    EXPECT_EQ(write->Valid(), false);

    write = std::make_shared<ShareMemoryBlock>("", 0, 0);
    EXPECT_EQ(write->Valid(), false);
}

/**
 * @tc.name: SharedMemoryBlockTest
 * @tc.desc: Write data to shared memory with blocked mode.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryBlockTest, WriterSyncTest, TestSize.Level0)
{
    auto write = std::make_shared<ShareMemoryBlock>(PLUGIN_NAME, SMB_SIZE, g_smbFd);
    EXPECT_NE(write->Valid(), false);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t buffer4[] = {0xCC, 0xDD, 0xBB, 0xEE};

    EXPECT_FALSE(write->PutWithPayloadSync(nullptr, 0, nullptr, 0, nullptr));
    EXPECT_TRUE(write->PutWithPayloadSync(reinterpret_cast<const int8_t*>(buffer1), sizeof(buffer1),
        reinterpret_cast<const int8_t*>(buffer2), sizeof(buffer2), nullptr));
    EXPECT_TRUE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->PutWithPayloadSync(reinterpret_cast<const int8_t*>(buffer3), sizeof(buffer3),
        reinterpret_cast<const int8_t*>(buffer4), sizeof(buffer4), nullptr));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));
    EXPECT_TRUE(write->Valid());
    EXPECT_FALSE(write->PutWithPayloadSync(reinterpret_cast<const int8_t*>(buffer3), 0, nullptr, 0, nullptr));
    EXPECT_FALSE(write->PutWithPayloadTimeout(nullptr, 0, nullptr, 0));
}
} // namespace
