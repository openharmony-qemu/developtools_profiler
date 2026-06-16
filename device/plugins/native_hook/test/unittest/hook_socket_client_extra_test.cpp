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

#include "hook_socket_client.h"
#include "hook_service.h"
#include "hook_common.h"
#include "service_entry.h"
#include "socket_context.h"
#include "unix_socket_client.h"
#include "logging.h"
#include "sampling.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
static ClientConfig g_ClientConfigTest = {0};
constexpr unsigned int WAIT_THREAD_TIME = 5;

class HookSocketClientExtraTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataBasic, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t payload[4] = {9, 10, 11, 12};
    
    std::string tagName = "test_tag";
    uint32_t originSize = sizeof(header) + sizeof(payload);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), payload, sizeof(payload),
        extraDataInfo, 0);
    
    EXPECT_TRUE(result || !hookClient unixSocketClient_);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataNullPayload, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "test_tag";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0);
    
    EXPECT_TRUE(result || !hookClient.unixSocketClient_);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataDynamicTag, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = getpid();
    baseData->tid = gettid();
    
    std::string tagName = "libtest_dynamic.so";
    uint32_t originSize = sizeof(BaseStackRawData);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0);
    
    EXPECT_TRUE(result || !hookClient.unixSocketClient_);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataEmptyTagName, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0);
    
    EXPECT_TRUE(result || !hookClient.unixSocketClient_);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataLargeTagName, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "libvery_long_library_name_for_testing_boundary_conditions.so";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0);
    
    EXPECT_TRUE(result || !hookClient.unixSocketClient_);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataInvalidSmbIndex, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "test_tag";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 100);
    
    EXPECT_FALSE(result);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataMultipleCalls, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t header2[8] = {11, 12, 13, 14, 15, 16, 17, 18};
    
    std::string tagName = "test_tag";
    uint32_t originSize = sizeof(header1);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result1 = hookClient.SendStackWithExtraData(header1, sizeof(header1), nullptr, 0,
        extraDataInfo, 0);
    
    bool result2 = hookClient.SendStackWithExtraData(header2, sizeof(header2), nullptr, 0,
        extraDataInfo, 0);
    
    EXPECT_TRUE(result1 || !hookClient.unixSocketClient_);
    EXPECT_TRUE(result2 || !hookClient.unixSocketClient_);
}

HWTEST_F(HookSocketClientExtraTest, SendStackWithExtraDataBlockedMode, TestSize.Level0)
{
    ClientConfig config;
    config.shareMemorySize = 1024 * 1024;
    config.isBlocked = true;
    HookSocketClient hookClient(getpid(), &config, nullptr);
    
    uint8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    std::string tagName = "test_blocked_tag";
    uint32_t originSize = sizeof(header);
    ExtraDataInfo extraDataInfo(
        reinterpret_cast<const int8_t*>(tagName.c_str()), tagName.size() + 1,
        reinterpret_cast<const int8_t*>(&originSize), sizeof(originSize));
    
    bool result = hookClient.SendStackWithExtraData(header, sizeof(header), nullptr, 0,
        extraDataInfo, 0);
    
    EXPECT_TRUE(result || !hookClient.unixSocketClient_);
}

} // namespace