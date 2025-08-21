/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "hook_record_factory_test.h"
#include "native_hook_config.pb.h"
#include <string>
#include <sys/time.h>
#include <vector>

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
NativeHookConfig g_hookConfig;
std::shared_ptr<HookRecordFactory> g_factory = nullptr;
class HookRecordFactoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HookRecordFactoryTest::SetUpTestCase(void) {}
void HookRecordFactoryTest::TearDownTestCase(void) {}
void HookRecordFactoryTest::SetUp()
{
    g_factory = std::make_shared<HookRecordFactory>(g_hookConfig);
}
void HookRecordFactoryTest::TearDown()
{
    g_factory = nullptr;
}

/*
 * @tc.name: GetFreeSimpRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with free simple message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetFreeSimpRecord, TestSize.Level1)
{
    int8_t addr = 1;
    int8_t data[] = {addr};
    uint32_t dataSize = sizeof(void *);
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
    EXPECT_EQ(hookRecord->GetType(), FREE_MSG_SIMP);
}

/*
 * @tc.name: GetJsRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with js message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetJsRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = JS_STACK_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(hookRecord->GetType(), JS_STACK_MSG);
    }
    delete[] data;
}

/*
 * @tc.name: GetMallocRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with malloc message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetMallocRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = MALLOC_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(hookRecord->GetType(), MALLOC_MSG);
    }
    delete[] data;
}

/*
 * @tc.name: GetNmdRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with jemalloc stats message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetNmdRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = NMD_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(hookRecord->GetType(), NMD_MSG);
    }
    delete[] data;
}

/*
 * @tc.name: GetFreeRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with free message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetFreeRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = FREE_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(hookRecord->GetType(), FREE_MSG);
    }
    delete[] data;
}

/*
 * @tc.name: ReturnMallocRecord
 * @tc.desc: test HookRecordFactory::SaveHookRecord with malloc message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ReturnMallocRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = MALLOC_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(g_factory->mallocRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE - 1);
        g_factory->SaveHookRecord(hookRecord);
        EXPECT_EQ(g_factory->mallocRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE);
    }
    delete[] data;
}

/*
 * @tc.name: ReturnMmapRecord
 * @tc.desc: test HookRecordFactory::SaveHookRecord with mmap message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ReturnMmapRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = MMAP_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(g_factory->mmapRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE - 1);
        g_factory->SaveHookRecord(hookRecord);
        EXPECT_EQ(g_factory->mmapRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE);
    }
    delete[] data;
}

/*
 * @tc.name: ReturnEndRecord
 * @tc.desc: test HookRecordFactory::SaveHookRecord with end message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ReturnEndRecord, TestSize.Level1)
{
    StackRawData rawdata;
    rawdata.type = END_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(g_factory->mallocRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE);
        EXPECT_EQ(g_factory->mmapRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE);
        g_factory->SaveHookRecord(hookRecord);
        EXPECT_EQ(g_factory->mallocRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE);
        EXPECT_EQ(g_factory->mmapRecordCache_.size(), HOOK_RECORD_CACHE_INIT_SIZE);
    }
    delete[] data;
}
}