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
    std::vector<uint32_t> tagIds;
    
    for (const auto& tagName : tagNames) {
        bool isNewTagId = false;
        uint32_t tagId = g_factory->GetDynamicTagId(tagName, isNewTagId);
        EXPECT_TRUE(isNewTagId);
        tagIds.push_back(tagId);
    }
    
    for (size_t i = 0; i < tagIds.size(); ++i) {
        for (size_t j = i + 1; j < tagIds.size(); ++j) {
            EXPECT_NE(tagIds[i], tagIds[j]);
        }
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
    rawStack->stackContext->pid = 1234;
    rawStack->stackContext->tid = 5678;
    
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