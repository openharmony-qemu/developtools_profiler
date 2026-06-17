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

static std::vector<int8_t> MakeRecordBuffer(uint16_t type, uint32_t extraSize = sizeof(uint64_t) * 2)
{
    std::vector<int8_t> buffer(sizeof(BaseStackRawData) + extraSize, 0);
    BaseStackRawData baseData = {};
    baseData.type = type;
    baseData.pid = 12;
    baseData.tid = 11;
    baseData.addr = reinterpret_cast<void*>(0x10);
    errno_t ret = memcpy_s(buffer.data(), buffer.size(), &baseData, sizeof(baseData));
    if (ret != EOK) {
        return {};
    }
    return buffer;
}

/*
 * @tc.name: GetFreeSimpRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with free simple message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetFreeSimpRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, GetJsRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, GetMallocRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, GetNmdRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, GetFreeRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, ReturnMallocRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, ReturnMmapRecord, TestSize.Level0)
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
HWTEST_F(HookRecordFactoryTest, ReturnEndRecord, TestSize.Level0)
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

/*
 * @tc.name: CreateStackRecord
 * @tc.desc: test HookRecordFactory::CreateStackRecord.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, CreateStackRecord001, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    BaseStackRawData baseData;
    baseData.pid = 12;
    baseData.tid = 11;
    baseData.type = MMAP_FILE_PAGE_MSG;
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x10);
    auto record = g_factory->CreateStackRecord(MMAP_FILE_PAGE_MSG, rawdata);
    EXPECT_EQ(record->GetType(), MMAP_FILE_PAGE_MSG);
    baseData.type = MALLOC_MSG;
    record = g_factory->CreateStackRecord(MALLOC_MSG, rawdata);
    EXPECT_EQ(record->GetType(), MALLOC_MSG);
}

/*
 * @tc.name: CreateStackRecord
 * @tc.desc: test HookRecordFactory::CreateStackRecord with memory using message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, CreateStackRecord002, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    BaseStackRawData baseData;
    baseData.pid = 12;
    baseData.tid = 11;
    baseData.type = MEMORY_USING_MSG;
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x10);
    auto record = g_factory->CreateStackRecord(MEMORY_USING_MSG, rawdata);
    EXPECT_EQ(record->GetType(), MEMORY_USING_MSG);
    baseData.type = FREE_MSG;
    record = g_factory->CreateStackRecord(FREE_MSG, rawdata);
    EXPECT_EQ(record->GetType(), FREE_MSG);
}

/*
 * @tc.name: CreateStackRecordCoversRemainingTypes
 * @tc.desc: test HookRecordFactory::CreateStackRecord with remaining stack message types.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, CreateStackRecordCoversRemainingTypes, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    BaseStackRawData baseData = {};
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x10);

    baseData.type = MUNMAP_MSG;
    auto record = g_factory->CreateStackRecord(MUNMAP_MSG, rawdata);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetType(), MUNMAP_MSG);

    baseData.type = MEMORY_UNUSING_MSG;
    record = g_factory->CreateStackRecord(MEMORY_UNUSING_MSG, rawdata);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetType(), MEMORY_UNUSING_MSG);

    baseData.type = MMAP_FILE_TYPE;
    record = g_factory->CreateStackRecord(MMAP_FILE_TYPE, rawdata);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetType(), MMAP_FILE_TYPE);

    EXPECT_EQ(g_factory->CreateStackRecord(UNKNOWN, rawdata), nullptr);
}
 
/*
* @tc.name: GetGwpAsanRecord
* @tc.desc: test HookRecordFactory::GetHookRecord with GWP_ASAN message.
* @tc.type: FUNC
*/
HWTEST_F(HookRecordFactoryTest, GetGwpAsanRecord, TestSize.Level0)
{
    StackRawData rawdata;
    rawdata.type = GWP_ASAN_MSG;
    uint32_t dataSize = sizeof(StackRawData);
    int8_t* data = new int8_t[sizeof(StackRawData)];
    if (memcpy_s(data, sizeof(StackRawData), &rawdata, sizeof(StackRawData)) == EOK) {
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(data, dataSize);
        EXPECT_EQ(hookRecord->GetType(), GWP_ASAN_MSG);
    }
    delete[] data;
}

/*
 * @tc.name: GetHookRecordCoversDirectTypes
 * @tc.desc: test HookRecordFactory::GetHookRecord direct record creation paths.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetHookRecordCoversDirectTypes, TestSize.Level0)
{
    const std::vector<uint16_t> types = {
        MEM_LEAK_MSG, MEMORY_TAG, THREAD_NAME_MSG, PR_SET_VMA_MSG, MALLOC_ARKTS, FREE_ARKTS
    };
    for (uint16_t type : types) {
        std::vector<int8_t> buffer = MakeRecordBuffer(type);
        ASSERT_FALSE(buffer.empty());
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
            buffer.data(), static_cast<uint32_t>(buffer.size()));
        ASSERT_NE(hookRecord, nullptr);
        EXPECT_EQ(hookRecord->GetType(), type);
    }
}

/*
 * @tc.name: GetHookRecordSetsFpDepthAndStackData
 * @tc.desc: test HookRecordFactory::GetHookRecord calculates raw stack metadata.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetHookRecordSetsFpDepthAndStackData, TestSize.Level0)
{
    NativeHookConfig fpConfig;
    fpConfig.set_fp_unwind(true);
    HookRecordFactory fpFactory(fpConfig);
    std::vector<int8_t> fpBuffer = MakeRecordBuffer(MALLOC_MSG, sizeof(uint64_t) * 3);
    ASSERT_FALSE(fpBuffer.empty());
    auto fpRecord = fpFactory.GetHookRecord(fpBuffer.data(), static_cast<uint32_t>(fpBuffer.size()), false);
    ASSERT_NE(fpRecord, nullptr);
    EXPECT_EQ(fpRecord->GetRawStack()->fpDepth, 3u);
    EXPECT_EQ(fpRecord->GetRawStack()->stackContext,
        reinterpret_cast<BaseStackRawData*>(fpBuffer.data()));

    NativeHookConfig asyncConfig;
    asyncConfig.set_fp_unwind(true);
    asyncConfig.set_async_stack_enable(true);
    HookRecordFactory asyncFactory(asyncConfig);
    auto asyncRecord = asyncFactory.GetHookRecord(fpBuffer.data(), static_cast<uint32_t>(fpBuffer.size()));
    ASSERT_NE(asyncRecord, nullptr);
    EXPECT_EQ(asyncRecord->GetRawStack()->fpDepth, 0u);

    NativeHookConfig dwarfConfig;
    HookRecordFactory dwarfFactory(dwarfConfig);
    std::vector<int8_t> dwarfBuffer = MakeRecordBuffer(MALLOC_MSG, MAX_REG_SIZE + 16);
    ASSERT_FALSE(dwarfBuffer.empty());
    auto dwarfRecord = dwarfFactory.GetHookRecord(dwarfBuffer.data(), static_cast<uint32_t>(dwarfBuffer.size()));
    ASSERT_NE(dwarfRecord, nullptr);
    EXPECT_EQ(dwarfRecord->GetRawStack()->stackSize, 16u);
    EXPECT_NE(dwarfRecord->GetRawStack()->stackData, nullptr);
}

/*
 * @tc.name: GetStacklessMunmapRecord
 * @tc.desc: test HookRecordFactory::GetHookRecord with stackless munmap message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetStacklessMunmapRecord, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_async_stack_enable(true);
    HookRecordFactory factory(hookConfig);
    std::vector<int8_t> buffer = MakeRecordBuffer(MUNMAP_MSG, 0);
    ASSERT_FALSE(buffer.empty());
    auto* rawdata = reinterpret_cast<BaseStackRawData*>(buffer.data());
    rawdata->addr = reinterpret_cast<void*>(0x1000);
    rawdata->mallocSize = 4096;
    auto hookRecord = factory.GetHookRecord(buffer.data(), static_cast<uint32_t>(buffer.size()));
    ASSERT_NE(hookRecord, nullptr);
    auto rawStack = hookRecord->GetRawStack();
    ASSERT_NE(rawStack, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MUNMAP_MSG);
    EXPECT_EQ(rawStack->fpDepth, 0);
    EXPECT_EQ(rawStack->stackSize, 0);
    EXPECT_EQ(hookRecord->GetAddr(), reinterpret_cast<uint64_t>(rawdata->addr));
}

/*
 * @tc.name: GetDynamicTagId_NewTag
 * @tc.desc: test HookRecordFactory::GetDynamicTagId with new tag name.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetDynamicTagId_NewTag, TestSize.Level0)
{
    std::string tagName = "test_new_tag";
    bool isNewTagId = false;
    uint32_t tagId = g_factory->GetDynamicTagId(tagName, isNewTagId);
    EXPECT_TRUE(isNewTagId);
    EXPECT_GE(tagId, INITIAL_MEM_TAG_ID);
    EXPECT_LT(tagId, TAG_ID_PENDING);
}

/*
 * @tc.name: GetDynamicTagId_ExistingTag
 * @tc.desc: test HookRecordFactory::GetDynamicTagId with existing tag name.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetDynamicTagId_ExistingTag, TestSize.Level0)
{
    std::string tagName = "test_existing_tag";
    bool isNewTagId1 = false;
    uint32_t firstTagId = g_factory->GetDynamicTagId(tagName, isNewTagId1);
    EXPECT_TRUE(isNewTagId1);
    
    bool isNewTagId2 = false;
    uint32_t secondTagId = g_factory->GetDynamicTagId(tagName, isNewTagId2);
    EXPECT_FALSE(isNewTagId2);
    EXPECT_EQ(firstTagId, secondTagId);
}

/*
 * @tc.name: GetDynamicTagId_MultipleTags
 * @tc.desc: test HookRecordFactory::GetDynamicTagId with multiple different tag names.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetDynamicTagId_MultipleTags, TestSize.Level0)
{
    std::vector<std::string> tagNames = {"tag1", "tag2", "tag3", "tag4"};
    for (const auto& tagName : tagNames) {
        bool isNewTagId = false;
        uint32_t tagId = g_factory->GetDynamicTagId(tagName, isNewTagId);
        EXPECT_TRUE(isNewTagId);
    }
}

/*
 * @tc.name: GetOriginStackDataSize_NonDynamicTagType
 * @tc.desc: test HookRecordFactory::GetOriginStackDataSize with non-dynamic tag message type.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetOriginStackDataSize_NonDynamicTagType, TestSize.Level0)
{
    std::vector<int8_t> buffer(sizeof(BaseStackRawData) + 100, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MALLOC_MSG;
    baseData->tagId = 100;
    
    uint32_t originSize = g_factory->GetOriginStackDataSize(buffer.data(), buffer.size());
    EXPECT_EQ(originSize, buffer.size());
}

/*
 * @tc.name: GetOriginStackDataSize_DynamicTagWithValidData
 * @tc.desc: test HookRecordFactory::GetOriginStackDataSize with valid dynamic tag data.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetOriginStackDataSize_DynamicTagWithValidData, TestSize.Level0)
{
    uint32_t realDataSize = sizeof(BaseStackRawData) + 50;
    std::string tagName = "test_file.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = realDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    memcpy_s(buffer.data() + realDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = realDataSize;
    
    uint32_t originSize = g_factory->GetOriginStackDataSize(buffer.data(), buffer.size());
    EXPECT_EQ(originSize, realDataSize);
}

/*
 * @tc.name: GetOriginStackDataSize_DynamicTagWithInvalidSize
 * @tc.desc: test HookRecordFactory::GetOriginStackDataSize with invalid realDataSize.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetOriginStackDataSize_DynamicTagWithInvalidSize, TestSize.Level0)
{
    uint32_t totalSize = sizeof(BaseStackRawData) + 20 + sizeof(uint32_t);
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MEMORY_USING_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = 0;
    
    uint32_t originSize = g_factory->GetOriginStackDataSize(buffer.data(), buffer.size());
    EXPECT_EQ(originSize, buffer.size());
}

/*
 * @tc.name: GetOriginStackDataSize_DynamicTagWithTooLargeSize
 * @tc.desc: test HookRecordFactory::GetOriginStackDataSize with too large realDataSize.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetOriginStackDataSize_DynamicTagWithTooLargeSize, TestSize.Level0)
{
    uint32_t totalSize = sizeof(BaseStackRawData) + 20 + sizeof(uint32_t);
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MEMORY_UNUSING_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = buffer.size() + 1000;
    
    uint32_t originSize = g_factory->GetOriginStackDataSize(buffer.data(), buffer.size());
    EXPECT_EQ(originSize, buffer.size());
}

/*
 * @tc.name: ProcessDynamicTagInfo_NonDynamicTagType
 * @tc.desc: test HookRecordFactory::ProcessDynamicTagInfo with non-dynamic tag message type.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_NonDynamicTagType, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(new int8_t[sizeof(BaseStackRawData) + 50]);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->tagId = 100;
    
    uint32_t size = sizeof(BaseStackRawData) + 50;
    std::string savedTagName;
    uint32_t savedTagId = 0;
    auto callback = [&savedTagName, &savedTagId](uint32_t tagId, const std::string& tagName) {
        savedTagId = tagId;
        savedTagName = tagName;
    };
    
    g_factory->ProcessDynamicTagInfo(rawStack, size, callback);
    EXPECT_EQ(savedTagId, 0u);
    EXPECT_TRUE(savedTagName.empty());
    
    delete[] reinterpret_cast<int8_t*>(rawStack->stackContext);
}

/*
 * @tc.name: ProcessDynamicTagInfo_DynamicTagValidData
 * @tc.desc: test HookRecordFactory::ProcessDynamicTagInfo with valid dynamic tag data.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_DynamicTagValidData, TestSize.Level0)
{
    uint32_t realDataSize = sizeof(BaseStackRawData) + 50;
    std::string tagName = "libtest.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = realDataSize + tagNameSize + sizeof(uint32_t);
    
    std::shared_ptr<RawStack> rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(new int8_t[totalSize]);
    rawStack->stackContext->type = MMAP_FILE_PAGE_MSG;
    rawStack->stackContext->tagId = TAG_ID_PENDING;
    
    uint8_t* baseData = reinterpret_cast<uint8_t*>(rawStack->stackContext);
    memcpy_s(baseData + realDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(baseData + totalSize - sizeof(uint32_t));
    *sizePtr = realDataSize;
    
    uint32_t size = totalSize;
    std::string savedTagName;
    uint32_t savedTagId = 0;
    bool callbackCalled = false;
    auto callback = [&savedTagName, &savedTagId, &callbackCalled](uint32_t tagId, const std::string& name) {
        savedTagId = tagId;
        savedTagName = name;
        callbackCalled = true;
    };
    
    g_factory->ProcessDynamicTagInfo(rawStack, size, callback);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(savedTagName, tagName);
    EXPECT_GE(savedTagId, INITIAL_MEM_TAG_ID);
    EXPECT_LT(savedTagId, TAG_ID_PENDING);
    EXPECT_EQ(rawStack->stackContext->tagId, savedTagId);
    EXPECT_EQ(size, realDataSize);
    
    delete[] reinterpret_cast<int8_t*>(rawStack->stackContext);
}

/*
 * @tc.name: ProcessDynamicTagInfo_DynamicTagExistingName
 * @tc.desc: test HookRecordFactory::ProcessDynamicTagInfo with existing tag name.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_DynamicTagExistingName, TestSize.Level0)
{
    std::string tagName = "libexisting.so";
    bool isNewTagId = false;
    uint32_t existingTagId = g_factory->GetDynamicTagId(tagName, isNewTagId);
    EXPECT_TRUE(isNewTagId);
    
    uint32_t realDataSize = sizeof(BaseStackRawData) + 50;
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = realDataSize + tagNameSize + sizeof(uint32_t);
    
    std::shared_ptr<RawStack> rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(new int8_t[totalSize]);
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->tagId = TAG_ID_PENDING;
    
    uint8_t* baseData = reinterpret_cast<uint8_t*>(rawStack->stackContext);
    memcpy_s(baseData + realDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(baseData + totalSize - sizeof(uint32_t));
    *sizePtr = realDataSize;
    
    uint32_t size = totalSize;
    bool callbackCalled = false;
    auto callback = [&callbackCalled](uint32_t tagId, const std::string& name) {
        callbackCalled = true;
    };
    
    g_factory->ProcessDynamicTagInfo(rawStack, size, callback);
    
    EXPECT_FALSE(callbackCalled);
    EXPECT_EQ(rawStack->stackContext->tagId, existingTagId);
    
    delete[] reinterpret_cast<int8_t*>(rawStack->stackContext);
}

/*
 * @tc.name: ProcessDynamicTagInfo_DynamicTagInvalidRealDataSize
 * @tc.desc: test HookRecordFactory::ProcessDynamicTagInfo with invalid realDataSize.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_DynamicTagInvalidRealDataSize, TestSize.Level0)
{
    uint32_t totalSize = sizeof(BaseStackRawData) + 20 + sizeof(uint32_t);
    
    std::shared_ptr<RawStack> rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(new int8_t[totalSize]);
    rawStack->stackContext->type = MEMORY_UNUSING_MSG;
    rawStack->stackContext->tagId = TAG_ID_PENDING;
    
    uint8_t* baseData = reinterpret_cast<uint8_t*>(rawStack->stackContext);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(baseData + totalSize - sizeof(uint32_t));
    *sizePtr = 0;
    
    uint32_t size = totalSize;
    auto callback = [](uint32_t tagId, const std::string& name) {};
    
    g_factory->ProcessDynamicTagInfo(rawStack, size, callback);
    
    EXPECT_EQ(rawStack->stackContext->tagId, TAG_ID_PENDING);
    EXPECT_EQ(size, totalSize);
    
    delete[] reinterpret_cast<int8_t*>(rawStack->stackContext);
}

/*
 * @tc.name: ProcessDynamicTagInfo_DynamicTagEmptyTagName
 * @tc.desc: test HookRecordFactory::ProcessDynamicTagInfo with empty tag name.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_DynamicTagEmptyTagName, TestSize.Level0)
{
    uint32_t realDataSize = sizeof(BaseStackRawData) + 50;
    uint32_t tagNameSize = 1;
    uint32_t totalSize = realDataSize + tagNameSize + sizeof(uint32_t);
    
    std::shared_ptr<RawStack> rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(new int8_t[totalSize]);
    rawStack->stackContext->type = MMAP_FILE_PAGE_MSG;
    rawStack->stackContext->tagId = TAG_ID_PENDING;
    
    uint8_t* baseData = reinterpret_cast<uint8_t*>(rawStack->stackContext);
    baseData[realDataSize] = '\0';
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(baseData + totalSize - sizeof(uint32_t));
    *sizePtr = realDataSize;
    
    uint32_t size = totalSize;
    auto callback = [](uint32_t tagId, const std::string& name) {};
    
    g_factory->ProcessDynamicTagInfo(rawStack, size, callback);
    
    EXPECT_EQ(rawStack->stackContext->tagId, TAG_ID_PENDING);
    
    delete[] reinterpret_cast<int8_t*>(rawStack->stackContext);
}

/*
 * @tc.name: ProcessDynamicTagInfo_DynamicTagWithNullCallback
 * @tc.desc: test HookRecordFactory::ProcessDynamicTagInfo with null callback.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_DynamicTagWithNullCallback, TestSize.Level0)
{
    uint32_t realDataSize = sizeof(BaseStackRawData) + 50;
    std::string tagName = "libnullcallback.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = realDataSize + tagNameSize + sizeof(uint32_t);
    
    std::shared_ptr<RawStack> rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(new int8_t[totalSize]);
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->tagId = TAG_ID_PENDING;
    
    uint8_t* baseData = reinterpret_cast<uint8_t*>(rawStack->stackContext);
    memcpy_s(baseData + realDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(baseData + totalSize - sizeof(uint32_t));
    *sizePtr = realDataSize;
    
    uint32_t size = totalSize;
    
    g_factory->ProcessDynamicTagInfo(rawStack, size, nullptr);
    
    EXPECT_GE(rawStack->stackContext->tagId, INITIAL_MEM_TAG_ID);
    EXPECT_LT(rawStack->stackContext->tagId, TAG_ID_PENDING);
    EXPECT_EQ(size, realDataSize);
    
    delete[] reinterpret_cast<int8_t*>(rawStack->stackContext);
}

/*
 * @tc.name: GetHookRecordWithDynamicTag
 * @tc.desc: test HookRecordFactory::GetHookRecord with dynamic tag data.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetHookRecordWithDynamicTag, TestSize.Level0)
{
    uint32_t realDataSize = sizeof(BaseStackRawData) + 50;
    std::string tagName = "libtest_dynamic.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = realDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = 1234;
    baseData->tid = 5678;
    
    memcpy_s(buffer.data() + realDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = realDataSize;
    
    std::string savedTagName;
    uint32_t savedTagId = 0;
    auto callback = [&savedTagName, &savedTagId](uint32_t tagId, const std::string& name) {
        savedTagId = tagId;
        savedTagName = name;
    };
    
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), buffer.size(), true, callback);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MMAP_FILE_PAGE_MSG);
    EXPECT_GE(savedTagId, INITIAL_MEM_TAG_ID);
    EXPECT_LT(savedTagId, TAG_ID_PENDING);
    EXPECT_EQ(savedTagName, tagName);
    EXPECT_EQ(hookRecord->GetRawStack()->stackContext->tagId, savedTagId);
}

/*
 * @tc.name: GetHookRecordWithNonDynamicTag
 * @tc.desc: test HookRecordFactory::GetHookRecord with non-dynamic tag data and callback.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, GetHookRecordWithNonDynamicTag, TestSize.Level0)
{
    std::vector<int8_t> buffer(sizeof(BaseStackRawData) + 50, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MALLOC_MSG;
    baseData->tagId = 100;
    
    bool callbackCalled = false;
    auto callback = [&callbackCalled](uint32_t tagId, const std::string& name) {
        callbackCalled = true;
    };
    
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), buffer.size(), false, callback);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_FALSE(callbackCalled);
}

} // namespace OHOS::Developtools::NativeDaemon
