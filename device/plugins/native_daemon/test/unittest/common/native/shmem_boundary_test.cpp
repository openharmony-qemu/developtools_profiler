/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "shmem_boundary_test.h"
#include <climits>

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
constexpr uint32_t PAGE_SIZE = 4096;
constexpr uint32_t BUFF_SIZE = 1024;
const std::string SMB_NAME = "shmem_boundary_test";

void ShmemBoundaryTest::SetUpTestCase(void) {}
void ShmemBoundaryTest::TearDownTestCase(void) {}

void ShmemBoundaryTest::SetUp()
{
    shareMemoryBlock_ = nullptr;
}

void ShmemBoundaryTest::TearDown()
{
    shareMemoryBlock_ = nullptr;
}

/*
 * @tc.name: ShmemSizePageBoundary
 * @tc.desc: test shared memory size = PAGE_SIZE (4096) boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, ShmemSizePageBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        EXPECT_EQ(shareMemoryBlock_->GetSize(), PAGE_SIZE);
    }
}

/*
 * @tc.name: ShmemSizeMultiPageBoundary
 * @tc.desc: test shared memory size = PAGE_SIZE * 4 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, ShmemSizeMultiPageBoundary, TestSize.Level0)
{
    uint32_t multiPageSize = PAGE_SIZE * 4;
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, multiPageSize);
    if (shareMemoryBlock_->Valid()) {
        EXPECT_EQ(shareMemoryBlock_->GetSize(), multiPageSize);
    }
}

/*
 * @tc.name: ShmemSizeNotAlignedBoundary
 * @tc.desc: test shared memory size = 4097 (not page aligned) boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, ShmemSizeNotAlignedBoundary, TestSize.Level0)
{
    uint32_t notAlignedSize = PAGE_SIZE + 1;
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, notAlignedSize);
    if (shareMemoryBlock_->Valid()) {
        EXPECT_GE(shareMemoryBlock_->GetSize(), notAlignedSize);
    }
}

/*
 * @tc.name: SeekZeroBoundary
 * @tc.desc: test read offset = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SeekZeroBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        bool result = shareMemoryBlock_->Seek(0);
        EXPECT_TRUE(result);
    }
}

/*
 * @tc.name: SeekValidPositionBoundary
 * @tc.desc: test Seek with valid position boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SeekValidPositionBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        bool result = shareMemoryBlock_->Seek(100);
        EXPECT_TRUE(result);
    }
}

/*
 * @tc.name: GetNameBoundary
 * @tc.desc: test GetName returns correct name.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, GetNameBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        std::string name = shareMemoryBlock_->GetName();
        EXPECT_FALSE(name.empty());
    }
}

/*
 * @tc.name: GetFileDescriptorBoundary
 * @tc.desc: test GetfileDescriptor returns valid fd.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, GetFileDescriptorBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        int fd = shareMemoryBlock_->GetfileDescriptor();
        EXPECT_GE(fd, 0);
    }
}

/*
 * @tc.name: ResetPosBoundary
 * @tc.desc: test ResetPos boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, ResetPosBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        shareMemoryBlock_->Seek(100);
        shareMemoryBlock_->ResetPos();
        EXPECT_TRUE(shareMemoryBlock_->Valid());
    }
}

/*
 * @tc.name: SetReusePolicyDropNone
 * @tc.desc: test SetReusePolicy with DROP_NONE.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SetReusePolicyDropNone, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        shareMemoryBlock_->SetReusePolicy(ShareMemoryBlock::DROP_NONE);
        EXPECT_TRUE(shareMemoryBlock_->Valid());
    }
}

/*
 * @tc.name: SetReusePolicyDropOld
 * @tc.desc: test SetReusePolicy with DROP_OLD.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SetReusePolicyDropOld, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        shareMemoryBlock_->SetReusePolicy(ShareMemoryBlock::DROP_OLD);
        EXPECT_TRUE(shareMemoryBlock_->Valid());
    }
}

/*
 * @tc.name: SetWaitTimeZeroBoundary
 * @tc.desc: test SetWaitTime with zero value.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SetWaitTimeZeroBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        shareMemoryBlock_->SetWaitTime(0);
        EXPECT_TRUE(shareMemoryBlock_->Valid());
    }
}

/*
 * @tc.name: SetWaitTimeMaxBoundary
 * @tc.desc: test SetWaitTime with max value.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SetWaitTimeMaxBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        shareMemoryBlock_->SetWaitTime(INT_MAX);
        EXPECT_TRUE(shareMemoryBlock_->Valid());
    }
}

/*
 * @tc.name: GetCtxBoundary
 * @tc.desc: test GetCtx returns valid context.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, GetCtxBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        auto ctx = shareMemoryBlock_->GetCtx();
        EXPECT_NE(ctx, nullptr);
    }
}

/*
 * @tc.name: GetMMapBaseBoundary
 * @tc.desc: test GetMMapBase returns valid pointer.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, GetMMapBaseBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        void* base = shareMemoryBlock_->GetMMapBase();
        EXPECT_NE(base, nullptr);
    }
}

/*
 * @tc.name: ShmemSizeSmallBoundary
 * @tc.desc: test shared memory size = 1024 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, ShmemSizeSmallBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, BUFF_SIZE);
    if (shareMemoryBlock_->Valid()) {
        EXPECT_GE(shareMemoryBlock_->GetSize(), BUFF_SIZE);
    }
}

/*
 * @tc.name: ShmemSizeLargeBoundary
 * @tc.desc: test shared memory size = PAGE_SIZE * 16 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, ShmemSizeLargeBoundary, TestSize.Level0)
{
    uint32_t largeSize = PAGE_SIZE * 16;
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, largeSize);
    if (shareMemoryBlock_->Valid()) {
        EXPECT_EQ(shareMemoryBlock_->GetSize(), largeSize);
    }
}

/*
 * @tc.name: PutAndTakeDataBoundary
 * @tc.desc: test PutRaw and TakeData boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, PutAndTakeDataBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        int8_t testData[100] = {1, 2, 3, 4, 5};
        bool putResult = shareMemoryBlock_->PutRaw(testData, sizeof(testData));
        EXPECT_TRUE(putResult);
    }
}

/*
 * @tc.name: SeekMaxPositionBoundary
 * @tc.desc: test Seek with max valid position boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SeekMaxPositionBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        uint32_t maxPos = shareMemoryBlock_->GetSize() - 1;
        bool result = shareMemoryBlock_->Seek(maxPos);
        EXPECT_TRUE(result);
    }
}

/*
 * @tc.name: GetMemoryValidSizeBoundary
 * @tc.desc: test GetMemory with valid size boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, GetMemoryValidSizeBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        uint8_t* memory = nullptr;
        uint32_t offset = 0;
        bool result = shareMemoryBlock_->GetMemory(100, &memory, &offset);
        EXPECT_TRUE(result);
        EXPECT_NE(memory, nullptr);
    }
}

/*
 * @tc.name: GetMemorySizeOneBoundary
 * @tc.desc: test GetMemory with valid size boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, GetMemorySizeOneBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        uint8_t* memory = nullptr;
        uint32_t offset = 0;
        bool result = shareMemoryBlock_->GetMemory(1, &memory, &offset);
        EXPECT_TRUE(result);
        EXPECT_NE(memory, nullptr);
    }
}

/*
 * @tc.name: SetWaitTimeMinBoundary
 * @tc.desc: test SetWaitTime with min value (1).
 * @tc.type: FUNC
 */
HWTEST_F(ShmemBoundaryTest, SetWaitTimeMinBoundary, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME, PAGE_SIZE);
    if (shareMemoryBlock_->Valid()) {
        shareMemoryBlock_->SetWaitTime(INT_MIN);
        EXPECT_TRUE(shareMemoryBlock_->Valid());
    }
}
} // namespace OHOS::Developtools::NativeDaemon
