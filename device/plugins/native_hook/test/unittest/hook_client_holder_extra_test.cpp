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

#include <gmock/gmock.h>
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

class MockHookSocketClientExtra : public HookSocketClient {
public:
    explicit MockHookSocketClientExtra(void (*disableHookCallback)() = nullptr) : HookSocketClient(
        g_hookPid.load(), &g_clientConfig, &g_targetLibRange, &g_sharedMemCount, disableHookCallback) {};
    
    MOCK_METHOD5(SendStackWithPayload, bool(const void* data, size_t size, const void* payload,
        size_t payloadSize, int smbIndex));
    
    MOCK_METHOD5(SendStackWithExtraData, bool(const void* data, size_t size, const void* payload,
        size_t payloadSize, ExtraDataInfo& extraDataInfo, int smbIndex));
};

class HookClientHolderExtraTest : public ::testing::Test {
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
    void SetUp() override
    {
        g_hookReady = true;
        g_clientConfig.filterSize = 1;
        g_clientConfig.shareMemorySize = 1024 * 1024;
    }
    void TearDown() override
    {
        g_clientConfig.targetSoName = "";
        g_targetLibRange.clear();
        g_hookReady = false;
    }
};

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataBasic, TestSize.Level0)
{
    g_hookPid.store(1234);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataNullPayload, TestSize.Level0)
{
    g_hookPid.store(2345);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataDynamicTag, TestSize.Level0)
{
    g_hookPid.store(3456);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataEmptyTagName, TestSize.Level0)
{
    g_hookPid.store(4567);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataLargeTagName, TestSize.Level0)
{
    g_hookPid.store(5678);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataInvalidSmbIndex, TestSize.Level0)
{
    g_hookPid.store(6789);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataNullClient, TestSize.Level0)
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataMultipleCalls, TestSize.Level0)
{
    g_hookPid.store(7890);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataWithPayload, TestSize.Level0)
{
    g_hookPid.store(8901);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataMemoryUsingMsg, TestSize.Level0)
{
    g_hookPid.store(9012);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

HWTEST_F(HookClientHolderExtraTest, SendStackWithExtraDataMemoryUnusingMsg, TestSize.Level0)
{
    g_hookPid.store(9013);
    
    std::shared_ptr<MockHookSocketClientExtra> mockClient = std::make_shared<MockHookSocketClientExtra>();
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

} // namespace