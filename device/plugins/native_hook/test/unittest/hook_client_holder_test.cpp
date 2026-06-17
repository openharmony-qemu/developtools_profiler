/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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
#include <memory>
#include "hook_client_holder.h"
#include "hook_socket_client_mock.h"
#include "rawdata_builder.h"
#include "hook_common.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr unsigned int WAIT_THREAD_TIME = 3;
constexpr size_t FIRST_RANGE_INDEX = 0;
class HookClientHolderTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr);
        ohos_malloc_hook_on_start(nullptr);
    }
    static void TearDownTestCase()
    {
        ohos_malloc_hook_on_end();
        sleep(WAIT_THREAD_TIME);
    }
    void SetUp() override {}
    void TearDown() override
    {
        g_clientConfig.targetSoName = "";
        g_targetLibRange.clear();
    }
};

/**
 * @tc.name: ValidSharedPtrTest
 * @tc.desc: Test valid shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, ValidSharedPtrTest, TestSize.Level0)
{
    g_clientConfig.shareMemorySize = 1024 * 1024;
    auto client = std::make_shared<HookSocketClient>(getpid(), &g_clientConfig, nullptr);

    HookClientHolder holder(client);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_NE(holder.Get(), nullptr);
}

/**
 * @tc.name: NullSharedPtrTest
 * @tc.desc: Test null shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, NullSharedPtrTest, TestSize.Level0)
{
    std::shared_ptr<HookSocketClient> nullClient;

    HookClientHolder holder(nullClient);
    EXPECT_FALSE(holder.IsValid());
    EXPECT_EQ(holder.Get(), nullptr);
}

/**
 * @tc.name: SharedPtrLifetimeTest
 * @tc.desc: Test holder keeps shared_ptr alive
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SharedPtrLifetimeTest, TestSize.Level0)
{
    g_clientConfig.shareMemorySize = 1024 * 1024;
    std::shared_ptr<HookSocketClient> client = std::make_shared<HookSocketClient>(
        getpid(), &g_clientConfig, nullptr);
    HookClientHolder holder(client);

    EXPECT_TRUE(holder.IsValid());
    EXPECT_NE(holder.Get(), nullptr);
}

/**
 * @tc.name: MultipleHoldersTest
 * @tc.desc: Test multiple holders can access the same client
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, MultipleHoldersTest, TestSize.Level0)
{
    g_clientConfig.shareMemorySize = 1024 * 1024;
    auto client = std::make_shared<HookSocketClient>(getpid(), &g_clientConfig, nullptr);

    HookClientHolder holder1(client);
    HookClientHolder holder2(client);

    EXPECT_TRUE(holder1.IsValid());
    EXPECT_TRUE(holder2.IsValid());
    EXPECT_EQ(holder1.Get(), holder2.Get());
}

/**
 * @tc.name: UpdateThreadNameTest
 * @tc.desc: Test holders can access the UpdateThreadName test
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, UpdateThreadNameTest, TestSize.Level0)
{
    std::shared_ptr<HookSocketClient> hookClient = std::make_shared<HookSocketClient>(
        g_hookPid.load(), &g_clientConfig, &g_targetLibRange, &g_sharedMemCount);
    HookClientHolder holder1(hookClient);
    EXPECT_TRUE(holder1.IsValid());
    EXPECT_FALSE(holder1.UpdateThreadName());
    
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_)).WillOnce(::testing::Return(true));
    HookClientHolder holder2(g_hookClient);
    EXPECT_TRUE(holder2.IsValid());
    EXPECT_TRUE(holder2.UpdateThreadName());
}

/**
 * @tc.name: SendMmapFileRawDataBaseTest
 * @tc.desc: Test holders can access the SendMmapFileRawData base test
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataBaseTest, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_hookPid.store(1234);
    // addr=0x1000, mallocSize=0x2000
    RawDataBuilder builder;
    StackRawData rawdata = builder.SetAddr(reinterpret_cast<void*>(0x1000))
           .SetSize(0x2000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "test.txt";
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce([&rawdata] (const void* data, size_t size, const void* payload, size_t payloadSize, int smbIndex) {
            if (data) {
                const NameData* nd = static_cast<const NameData*>(data);
                EXPECT_EQ(nd->addr, rawdata.addr);
                EXPECT_EQ(nd->pid, 1234);
                EXPECT_EQ(nd->mallocSize, 0x2000);
                EXPECT_EQ(nd->mmapArgs.offset, 0);
                EXPECT_EQ(nd->type, MMAP_FILE_TYPE);
                EXPECT_STREQ(nd->name, "test.txt");
                EXPECT_EQ(nd->mmapArgs.flags, 0);
            }
            return true;
        });

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, 0, 0, filePath, rawdata);
}

/**
 * @tc.name: SendMmapFileRawDataTest_ProtExec
 * @tc.desc: prot with PROT_EXEC flag
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_ProtExec, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_hookPid.store(5678);

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x2000))
           .SetSize(0x4000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "exec_test.txt";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;

    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce([&rawdata] (const void* data, size_t size, const void* payload, size_t payloadSize, int smbIndex) {
            if (data) {
                const NameData* nd = static_cast<const NameData*>(data);
                EXPECT_EQ(nd->addr, rawdata.addr);
                EXPECT_EQ(nd->pid, 5678);
                EXPECT_EQ(nd->mmapArgs.flags & PROT_EXEC, PROT_EXEC);
            }
            return true;
        });

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ | PROT_EXEC, 0, 0, filePath, rawdata);
}

/**
 * @tc.name: SendMmapFileRawDataTest_MapFixed_TargetSo
 * @tc.desc: Flags include MAP_FIXED + target SO (triggers target range update)
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_TargetSo, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.targetSoName = "libtarget.so";
    g_hookPid.store(9012);
    g_targetLibRange.clear();

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x3000)) // addr=0x3000
           .SetSize(0x5000)                          // mallocSize=0x5000
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "/usr/lib/libtarget.so";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;

    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1);

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, MAP_FIXED, 0, filePath, rawdata);

    uint64_t expectedStart = reinterpret_cast<uint64_t>(rawdata.addr);
    uint64_t expectedEnd = expectedStart + rawdata.mallocSize;
    ASSERT_EQ(g_targetLibRange.size(), 1);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].first, expectedStart);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].second, expectedEnd);
}

/**
 * @tc.name: SendMmapFileRawDataTest_MapFixed_TargetSoList
 * @tc.desc: Flags include MAP_FIXED + target SO list.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_TargetSoList, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.targetSoName = "libfirst.so, libtarget.so";
    g_hookPid.store(9013);
    g_targetLibRange.clear();

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x3000))
           .SetSize(0x5000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "/usr/lib/libtarget.so";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1);

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, MAP_FIXED, 0, filePath, rawdata);

    uint64_t expectedStart = reinterpret_cast<uint64_t>(rawdata.addr);
    uint64_t expectedEnd = expectedStart + rawdata.mallocSize;
    ASSERT_EQ(g_targetLibRange.size(), 1);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].first, expectedStart);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].second, expectedEnd);
}

/**
 * @tc.name: SendMmapFileRawDataTest_MapFixed_ExpandStart
 * @tc.desc: Flags include MAP_FIXED + target SO (expand range: smaller start address)
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_ExpandStart, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.targetSoName = "libtarget.so";
    g_hookPid.store(1111);
    // 初始range：start=0x4000, end=0x8000
    g_targetLibRange = {{0x4000, 0x8000}};

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x2000)) // 新start更小
           .SetSize(0x3000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "/usr/lib/libtarget.so";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1);

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, MAP_FIXED, 0, filePath, rawdata);

    // 验证range更新：start=0x2000, end=0x8000
    ASSERT_EQ(g_targetLibRange.size(), 1);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].first, 0x2000);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].second, 0x8000);
}

/**
 * @tc.name: SendMmapFileRawDataTest_MapFixed_ExpandEnd
 * @tc.desc: Flags include MAP_FIXED + target SO (expand range: larger end address)
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_ExpandEnd, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.targetSoName = "libtarget.so";
    g_hookPid.store(2222);
    // start=0x3000, end=0x5000
    g_targetLibRange = {{0x3000, 0x5000}};

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x3000))
           .SetSize(0x4000) // end=0x3000+0x4000=0x7000
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "/usr/lib/libtarget.so";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1);

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, MAP_FIXED, 0, filePath, rawdata);

    // start=0x3000, end=0x7000
    ASSERT_EQ(g_targetLibRange.size(), 1);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].first, 0x3000);
    EXPECT_EQ(g_targetLibRange[FIRST_RANGE_INDEX].second, 0x7000);
}

/**
 * @tc.name: SendMmapFileRawDataTest_FilePathTooLong
 * @tc.desc: File path exceeds maximum length (string copy failed)
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_FilePathTooLong, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_hookPid.store(3333);

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x1000))
           .SetSize(0x2000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    char longFilePath[MAX_HOOK_PATH + 3];
    memset_s(longFilePath, sizeof(longFilePath), 'a', MAX_HOOK_PATH + 2);
    longFilePath[MAX_HOOK_PATH + 2] = '\0';

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;

    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(0);

    HookClientHolder holder(g_hookClient);
    holder.SendMmapFileRawData(PROT_READ, 0, 0, longFilePath, rawdata);
}

/**
 * @tc.name: SendMmapFileRawDataTest_MapFixed_LdMusl
 * @tc.desc: MAP_FIXED + ld-musl.so + responseLibraryMode enabled
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_LdMusl, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.responseLibraryMode = true;
    g_clientConfig.filterLibList.clear();
    g_filterStaLibRange.clear();
    g_hookPid.store(5555);

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x5000))
           .SetSize(0x1000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "/lib/ld-musl.so.1";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;

    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize, int smbIndex) {
            if (data) {
                const NameData* nd = static_cast<const NameData*>(data);
                EXPECT_EQ(nd->mmapArgs.flags & MAP_FIXED, MAP_FIXED);
            }
            return true;
        });

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, MAP_FIXED, 0, filePath, rawdata);
    ASSERT_EQ(g_filterStaLibRange.size(), 1);
    EXPECT_EQ(g_filterStaLibRange[0].first, 0x5000);
    EXPECT_EQ(g_filterStaLibRange[0].second, 0x6000);
}

/**
 * @tc.name: SendMmapFileRawDataTest_MapFixed_CustomFilterLib
 * @tc.desc: MAP_FIXED + custom filter lib + responseLibraryMode enabled
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_CustomFilterLib, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.responseLibraryMode = true;
    g_clientConfig.filterLibList = {"libcustom_filter.so"};
    g_filterStaLibRange.clear();
    g_hookPid.store(5656);

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(reinterpret_cast<void*>(0x7000))
           .SetSize(0x2000)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "/system/lib64/libcustom_filter.so";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;

    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1);

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, MAP_FIXED, 0, filePath, rawdata);

    ASSERT_EQ(g_filterStaLibRange.size(), 1);
    EXPECT_EQ(g_filterStaLibRange[0].first, 0x7000);
    EXPECT_EQ(g_filterStaLibRange[0].second, 0x9000);
}

/**
 * @tc.name: SendMmapFileRawDataTest_AddrNull
 * @tc.desc: addr is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_AddrNull, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_hookPid.store(6666);

    StackRawData rawdata = RawDataBuilder()
           .SetAddr(nullptr)
           .SetSize(0x0)
           .SetProcessInfo()
           .SetTimestamp()
           .Build();
    const char* filePath = "null_addr.txt";

    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;

    EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize, int smbIndex) {
            if (data) {
                const NameData* nd = static_cast<const NameData*>(data);
                EXPECT_EQ(nd->addr, nullptr);
                EXPECT_EQ(nd->mallocSize, 0);
            }
            return true;
        });

    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    holder.SendMmapFileRawData(PROT_READ, 0, 0, filePath, rawdata);
}

/**
 * @tc.name: SendStackWithExtraDataBasic
 * @tc.desc: Test SendStackWithExtraData basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataBasic, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(1234);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    
    std::string tagName = "test_tag";
    uint32_t originSize = sizeof(header) + sizeof(payload);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            EXPECT_NE(data, nullptr);
            EXPECT_EQ(size, 8u);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), payload, sizeof(payload),
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataNullPayload
 * @tc.desc: Test SendStackWithExtraData with null payload
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataNullPayload, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(2345);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "test_null_payload";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            EXPECT_EQ(payloadSize, 0u);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataDynamicTag
 * @tc.desc: Test SendStackWithExtraData with dynamic tag
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataDynamicTag, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(3456);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = g_hookPid.load();
    
    std::string tagName = "libtest_dynamic.so";
    uint32_t originSize = sizeof(BaseStackRawData);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([&tagName](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            EXPECT_EQ(info.firstExtraSize, tagName.size() + 1);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataEmptyTagName
 * @tc.desc: Test SendStackWithExtraData with empty tag name
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataEmptyTagName, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(4567);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            EXPECT_EQ(info.firstExtraSize, 1u);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataLargeTagName
 * @tc.desc: Test SendStackWithExtraData with large tag name
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataLargeTagName, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(5678);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "libvery_long_library_name_for_testing_boundary_conditions_v2.so";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([&tagName](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            EXPECT_EQ(info.firstExtraSize, tagName.size() + 1);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataInvalidSmbIndex
 * @tc.desc: Test SendStackWithExtraData with invalid smb index
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataInvalidSmbIndex, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(6789);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "test_invalid_index";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .Times(0);
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_FALSE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 100));
}

/**
 * @tc.name: SendStackWithExtraDataNullClient
 * @tc.desc: Test SendStackWithExtraData with null client
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataNullClient, TestSize.Level0)
{
    std::shared_ptr<HookSocketClient> nullClient;
    HookClientHolder holder(nullClient);
    
    EXPECT_FALSE(holder.IsValid());
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "test_null_client";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_FALSE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataMultipleCalls
 * @tc.desc: Test SendStackWithExtraData with multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataMultipleCalls, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(7890);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t header2[8] = {11, 12, 13, 14, 15, 16, 17, 18};
    
    std::string tagName = "test_multiple";
    uint32_t originSize = sizeof(header1);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillRepeatedly([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header1, sizeof(header1), nullptr, 0,
        extraDataInfo, 0));
    EXPECT_TRUE(holder.SendStackWithExtraData(header2, sizeof(header2), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataWithPayload
 * @tc.desc: Test SendStackWithExtraData with payload
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataWithPayload, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(8901);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[16] = {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    
    std::string tagName = "test_with_payload";
    uint32_t originSize = sizeof(header) + sizeof(payload);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            EXPECT_EQ(payloadSize, 16u);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), payload, sizeof(payload),
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataMemoryUsingMsg
 * @tc.desc: Test SendStackWithExtraData with MEMORY_USING_MSG
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataMemoryUsingMsg, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(9012);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MEMORY_USING_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = g_hookPid.load();
    
    std::string tagName = "memory_using_tag";
    uint32_t originSize = sizeof(BaseStackRawData);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            const BaseStackRawData* bd = static_cast<const BaseStackRawData*>(data);
            EXPECT_EQ(bd->type, MEMORY_USING_MSG);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}

/**
 * @tc.name: SendStackWithExtraDataMemoryUnusingMsg
 * @tc.desc: Test SendStackWithExtraData with MEMORY_UNUSING_MSG
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendStackWithExtraDataMemoryUnusingMsg, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.shareMemorySize = 1024 * 1024;
    g_hookPid.store(9013);
    
    std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
    g_hookClient = mockClient;
    
    uint8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MEMORY_UNUSING_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = g_hookPid.load();
    
    std::string tagName = "memory_unusing_tag";
    uint32_t originSize = sizeof(BaseStackRawData);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    EXPECT_CALL(*mockClient, SendStackWithExtraData(::testing::_, ::testing::_, ::testing::_,
        ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const void* data, size_t size, const void* payload, size_t payloadSize,
            ExtraDataInfo& info, int smbIndex) {
            const BaseStackRawData* bd = static_cast<const BaseStackRawData*>(data);
            EXPECT_EQ(bd->type, MEMORY_UNUSING_MSG);
            return true;
        });
    
    HookClientHolder holder(g_hookClient);
    EXPECT_TRUE(holder.IsValid());
    EXPECT_TRUE(holder.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0));
}
}
