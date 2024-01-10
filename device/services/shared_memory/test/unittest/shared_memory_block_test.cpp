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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "plugin_service_types.pb.h"
#include "share_memory_block.h"

using namespace testing::ext;

namespace {
constexpr size_t ARRAYSIZE = 1024;
constexpr int PAGE_SIZE = 4096;
constexpr int NUM_FOUR = 4;
constexpr int NUM_FIVE = 5;

class SharedMemoryBlockTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

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
} // namespace
