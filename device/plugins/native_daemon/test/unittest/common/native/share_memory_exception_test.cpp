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

#include "share_memory_exception_test.h"
#include <sys/mman.h>
#include <sys/syscall.h>

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
constexpr uint32_t SMB_SIZE = 4096;
const std::string SMB_NAME = "share_memory_exception_test";

void ShareMemoryExceptionTest::SetUpTestCase(void) {}
void ShareMemoryExceptionTest::TearDownTestCase(void) {}

void ShareMemoryExceptionTest::SetUp()
{
    shareMemoryBlock_ = nullptr;
}

void ShareMemoryExceptionTest::TearDown()
{
    shareMemoryBlock_ = nullptr;
}

/*
 * @tc.name: CreateBlockWithEmptyName
 * @tc.desc: test ShareMemoryBlock constructor with empty name.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, CreateBlockWithEmptyName, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>("", SMB_SIZE);
    EXPECT_FALSE(shareMemoryBlock_->Valid());
}

/*
 * @tc.name: CreateBlockWithZeroSize
 * @tc.desc: test ShareMemoryBlock constructor with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, CreateBlockWithZeroSize, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, 0);
    EXPECT_FALSE(shareMemoryBlock_->Valid());
}

/*
 * @tc.name: CreateBlockWithInvalidFd
 * @tc.desc: test ShareMemoryBlock constructor with invalid file descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, CreateBlockWithInvalidFd, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE, -1);
    EXPECT_FALSE(shareMemoryBlock_->Valid());
}

/*
 * @tc.name: PutRawWithNullData
 * @tc.desc: test PutRaw with null data pointer.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutRawWithNullData, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        bool result = shareMemoryBlock_->PutRaw(nullptr, 100);
        EXPECT_FALSE(result);
    }
}

/*
 * @tc.name: PutRawWithZeroSize
 * @tc.desc: test PutRaw with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutRawWithZeroSize, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        int8_t data[10] = {0};
        bool result = shareMemoryBlock_->PutRaw(data, 0);
        EXPECT_FALSE(result);
    }
}

/*
 * @tc.name: PutRawWithOversizeData
 * @tc.desc: test PutRaw with data size larger than buffer.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutRawWithOversizeData, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        std::vector<int8_t> largeData(SMB_SIZE * 2, 0);
        bool result = shareMemoryBlock_->PutRaw(largeData.data(), largeData.size());
        EXPECT_FALSE(result);
    }
}

/*
 * @tc.name: TakeDataWithNullHandler
 * @tc.desc: test TakeData with null handler function.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, TakeDataWithNullHandler, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        ShareMemoryBlock::DataHandler nullHandler = nullptr;
        bool result = shareMemoryBlock_->TakeData(nullHandler);
        EXPECT_FALSE(result);
    }
}

/*
 * @tc.name: TakeDataFromEmptyBuffer
 * @tc.desc: test TakeData when buffer is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, TakeDataFromEmptyBuffer, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        bool handlerCalled = false;
        auto handler = [&handlerCalled](const int8_t* data, uint32_t size) -> bool {
            handlerCalled = true;
            return true;
        };
        shareMemoryBlock_->TakeData(handler);
        EXPECT_FALSE(handlerCalled);
    }
}

/*
 * @tc.name: GetMemoryWithZeroSize
 * @tc.desc: test GetMemory with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, GetMemoryWithZeroSize, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        uint8_t* memory = nullptr;
        uint32_t offset = 0;
        bool result = shareMemoryBlock_->GetMemory(0, &memory, &offset);
        EXPECT_FALSE(result);
    }
}

/*
 * @tc.name: GetMemoryWithOversizeRequest
 * @tc.desc: test GetMemory with size larger than buffer.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, GetMemoryWithOversizeRequest, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        uint8_t* memory = nullptr;
        uint32_t offset = 0;
        bool result = shareMemoryBlock_->GetMemory(SMB_SIZE * 2, &memory, &offset);
        EXPECT_FALSE(result);
    }
}

/*
 * @tc.name: SeekWithInvalidPosition
 * @tc.desc: test Seek with position beyond buffer size.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, SeekWithInvalidPosition, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, SMB_SIZE);
    if (shareMemoryBlock_->Valid()) {
        bool result = shareMemoryBlock_->Seek(SMB_SIZE * 2);
        EXPECT_FALSE(result);
    }
}
} // namespace OHOS::Developtools::NativeDaemon
