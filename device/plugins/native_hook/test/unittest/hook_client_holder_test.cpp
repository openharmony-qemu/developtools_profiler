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
    void TearDown() override {}
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
        g_hookPid.load(), &g_clientConfig, &g_targetedRange, &g_sharedMemCount);
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
 * @tc.desc: Flags include MAP_FIXED + target SO (triggers targetedRange update)
 * @tc.type: FUNC
 */
HWTEST_F(HookClientHolderTest, SendMmapFileRawDataTest_MapFixed_TargetSo, TestSize.Level0)
{
    g_hookReady = true;
    g_clientConfig.filterSize = 1;
    g_clientConfig.targetSoName = "libtarget.so";
    g_hookPid.store(9012);
    g_targetedRange.store({0, 0});

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

    Range updatedRange = g_targetedRange.load();
    uint64_t expectedStart = reinterpret_cast<uint64_t>(rawdata.addr);
    uint64_t expectedEnd = expectedStart + rawdata.mallocSize;
    EXPECT_EQ(updatedRange.start, expectedStart);
    EXPECT_EQ(updatedRange.end, expectedEnd);
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
    g_targetedRange.store({0x4000, 0x8000});

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
    Range updatedRange = g_targetedRange.load();
    EXPECT_EQ(updatedRange.start, 0x2000);
    EXPECT_EQ(updatedRange.end, 0x8000);
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
    g_targetedRange.store({0x3000, 0x5000});

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
    Range updatedRange = g_targetedRange.load();
    EXPECT_EQ(updatedRange.start, 0x3000);
    EXPECT_EQ(updatedRange.end, 0x7000);
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
}
