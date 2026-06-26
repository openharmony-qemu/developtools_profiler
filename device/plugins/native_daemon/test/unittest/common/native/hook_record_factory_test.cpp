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
        EXPECT_TRUE(tagId >= INITIAL_MEM_TAG_ID);
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
 * @tc.name: ProcessDynamicTagInfo_MmapFilePageMsg
 * @tc.desc: test ProcessDynamicTagInfo with MMAP_FILE_PAGE_MSG type.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_MmapFilePageMsg, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 50;
    std::string tagName = "test_mmap_file.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = 100;
    baseData->tid = 101;
    baseData->addr = reinterpret_cast<void*>(0x1000);
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t callbackTagId = 0;
    std::string callbackTagName;
    auto callback = [&callbackTagId, &callbackTagName](uint32_t tagId, const std::string& name) {
        callbackTagId = tagId;
        callbackTagName = name;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MMAP_FILE_PAGE_MSG);
    EXPECT_GE(callbackTagId, INITIAL_MEM_TAG_ID);
    EXPECT_LT(callbackTagId, TAG_ID_PENDING);
    EXPECT_EQ(callbackTagName, tagName);
}

/*
 * @tc.name: ProcessDynamicTagInfo_MemoryUsingMsg
 * @tc.desc: test ProcessDynamicTagInfo with MEMORY_USING_MSG type.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_MemoryUsingMsg, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 30;
    std::string tagName = "memory_using_test.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MEMORY_USING_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = 200;
    baseData->tid = 201;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MEMORY_USING_MSG);
    EXPECT_EQ(callbackCount, 1);
}

/*
 * @tc.name: ProcessDynamicTagInfo_MemoryUnUsingMsg
 * @tc.desc: test ProcessDynamicTagInfo with MEMORY_UNUSING_MSG type.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_MemoryUnUsingMsg, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 40;
    std::string tagName = "memory_unusing_test.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MEMORY_UNUSING_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = 300;
    baseData->tid = 301;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MEMORY_UNUSING_MSG);
}

/*
 * @tc.name: ProcessDynamicTagInfo_NonDynamicTagType
 * @tc.desc: test ProcessDynamicTagInfo with non-dynamic tag message type.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_NonDynamicTagType, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 20;
    std::string tagName = "non_dynamic.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MALLOC_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(callbackCount, 0);
}

/*
 * @tc.name: ProcessDynamicTagInfo_NonPendingTagId
 * @tc.desc: test ProcessDynamicTagInfo with non-pending tagId.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_NonPendingTagId, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 20;
    std::string tagName = "existing_tag.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = INITIAL_MEM_TAG_ID + 10;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(callbackCount, 0);
}

/*
 * @tc.name: ProcessDynamicTagInfo_InvalidOriginDataSize
 * @tc.desc: test ProcessDynamicTagInfo with invalid originDataSize.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_InvalidOriginDataSize, TestSize.Level0)
{
    uint32_t totalSize = sizeof(BaseStackRawData) + 30 + sizeof(uint32_t);
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = 10;
    
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    EXPECT_EQ(callbackCount, 0);
}

/*
 * @tc.name: ProcessDynamicTagInfo_OriginDataSizeTooLarge
 * @tc.desc: test ProcessDynamicTagInfo with originDataSize too large.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_OriginDataSizeTooLarge, TestSize.Level0)
{
    uint32_t totalSize = sizeof(BaseStackRawData) + 30 + sizeof(uint32_t);
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = totalSize + 100;
    
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    EXPECT_EQ(callbackCount, 0);
}

/*
 * @tc.name: ProcessDynamicTagInfo_EmptyTagName
 * @tc.desc: test ProcessDynamicTagInfo with empty tagName.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_EmptyTagName, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 20;
    uint32_t tagNameSize = 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    buffer[originDataSize] = '\0';
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true, callback);
    
    EXPECT_EQ(callbackCount, 0);
}

/*
 * @tc.name: ProcessDynamicTagInfo_DuplicateTagName
 * @tc.desc: test ProcessDynamicTagInfo with duplicate tagName.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_DuplicateTagName, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 20;
    std::string tagName = "duplicate_tag.so";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t firstTagId = 0;
    uint32_t secondTagId = 0;
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    uint32_t size1 = totalSize;
    std::shared_ptr<HookRecord> hookRecord1 = g_factory->GetHookRecord(
        buffer.data(), size1, true, callback);
    firstTagId = hookRecord1->GetRawStack()->stackContext->tagId;
    
    uint32_t size2 = totalSize;
    std::shared_ptr<HookRecord> hookRecord2 = g_factory->GetHookRecord(
        buffer.data(), size2, true, callback);
    secondTagId = hookRecord2->GetRawStack()->stackContext->tagId;
    
    ASSERT_NE(hookRecord1, nullptr);
    ASSERT_NE(hookRecord2, nullptr);
    EXPECT_EQ(firstTagId, secondTagId);
    EXPECT_EQ(callbackCount, 1);
}

/*
 * @tc.name: ProcessDynamicTagInfo_LongTagName
 * @tc.desc: test ProcessDynamicTagInfo with long tagName.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_LongTagName, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData) + 20;
    std::string tagName(256, 'a');
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MMAP_FILE_PAGE_MSG);
}

/*
 * @tc.name: ProcessDynamicTagInfo_MinimumValidSize
 * @tc.desc: test ProcessDynamicTagInfo with minimum valid size.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_MinimumValidSize, TestSize.Level0)
{
    uint32_t originDataSize = sizeof(BaseStackRawData);
    std::string tagName = "a";
    uint32_t tagNameSize = tagName.size() + 1;
    uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
    
    std::vector<int8_t> buffer(totalSize, 0);
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    
    memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
    uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
    *sizePtr = originDataSize;
    
    uint32_t size = totalSize;
    std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
        buffer.data(), size, true);
    
    ASSERT_NE(hookRecord, nullptr);
    EXPECT_EQ(hookRecord->GetType(), MMAP_FILE_PAGE_MSG);
}

/*
 * @tc.name: ProcessDynamicTagInfo_AllThreeTypes
 * @tc.desc: test ProcessDynamicTagInfo with all three valid types in sequence.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFactoryTest, ProcessDynamicTagInfo_AllThreeTypes, TestSize.Level0)
{
    const std::vector<uint16_t> types = {MMAP_FILE_PAGE_MSG, MEMORY_USING_MSG, MEMORY_UNUSING_MSG};
    uint32_t callbackCount = 0;
    auto callback = [&callbackCount](uint32_t tagId, const std::string& name) {
        callbackCount++;
    };
    
    for (uint16_t type : types) {
        uint32_t originDataSize = sizeof(BaseStackRawData) + 20;
        std::string tagName = "test_type_" + std::to_string(type) + ".so";
        uint32_t tagNameSize = tagName.size() + 1;
        uint32_t totalSize = originDataSize + tagNameSize + sizeof(uint32_t);
        
        std::vector<int8_t> buffer(totalSize, 0);
        BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data());
        baseData->type = type;
        baseData->tagId = TAG_ID_PENDING;
        
        memcpy_s(buffer.data() + originDataSize, tagNameSize, tagName.c_str(), tagNameSize);
        uint32_t* sizePtr = reinterpret_cast<uint32_t*>(buffer.data() + totalSize - sizeof(uint32_t));
        *sizePtr = originDataSize;
        
        uint32_t size = totalSize;
        std::shared_ptr<HookRecord> hookRecord = g_factory->GetHookRecord(
            buffer.data(), size, true, callback);
        
        ASSERT_NE(hookRecord, nullptr);
        EXPECT_EQ(hookRecord->GetType(), type);
    }
    
    EXPECT_EQ(callbackCount, 3);
}

} // namespace OHOS::Developtools::NativeDaemon
