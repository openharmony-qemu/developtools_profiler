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
#include "hook_common.h"
#include <sys/mman.h>
#include <sys/syscall.h>
#include <cstring>

using namespace testing::ext;
using namespace std;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr uint32_t SMB_SIZE_EXTRA = 10 * 4096;
const std::string SMB_NAME_EXTRA = "share_memory_extra_test";
}

/*
 * @tc.name: PutWithExtraDataTimeout_AllParts
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout with all extra data parts.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_AllParts, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};
    int8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    int8_t secondExtra[4] = {19, 20, 21, 22};

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra));
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataTimeout_NoPayload
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout without payload.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_NoPayload, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    int8_t secondExtra[4] = {19, 20, 21, 22};

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), nullptr, 0,
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra));
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataTimeout_NoFirstExtra
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout without first extra.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_NoFirstExtra, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};
    int8_t secondExtra[4] = {19, 20, 21, 22};

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), payload, sizeof(payload),
        nullptr, 0, secondExtra, sizeof(secondExtra));
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataTimeout_NoSecondExtra
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout without second extra.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_NoSecondExtra, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};
    int8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), nullptr, 0);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataTimeout_OnlyHeader
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout with only header.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_OnlyHeader, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataTimeout_InvalidBlock
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout with invalid block.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_InvalidBlock, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>("", SMB_SIZE_EXTRA);
    EXPECT_FALSE(shareMemoryBlock_->Valid());

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0);
    EXPECT_FALSE(result);
}

/*
 * @tc.name: PutWithExtraDataTimeout_ExceedsMemorySize
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout with data exceeding memory size.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_ExceedsMemorySize, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    size_t hugeSize = SMB_SIZE_EXTRA + 1;
    std::vector<int8_t> hugeData(hugeSize, 0);

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        hugeData.data(), hugeSize, nullptr, 0, nullptr, 0, nullptr, 0);
    EXPECT_FALSE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_AllParts
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync with all extra data parts.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_AllParts, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};
    int8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    int8_t secondExtra[4] = {19, 20, 21, 22};

    auto callback = []() { return false; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra), callback);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_NoPayload
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync without payload.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_NoPayload, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};
    int8_t secondExtra[4] = {19, 20, 21, 22};

    auto callback = []() { return false; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), nullptr, 0,
        firstExtra, sizeof(firstExtra), secondExtra, sizeof(secondExtra), callback);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_NoFirstExtra
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync without first extra.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_NoFirstExtra, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};
    int8_t secondExtra[4] = {19, 20, 21, 22};

    auto callback = []() { return false; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), payload, sizeof(payload),
        nullptr, 0, secondExtra, sizeof(secondExtra), callback);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_NoSecondExtra
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync without second extra.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_NoSecondExtra, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};
    int8_t firstExtra[6] = {13, 14, 15, 16, 17, 18};

    auto callback = []() { return false; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), payload, sizeof(payload),
        firstExtra, sizeof(firstExtra), nullptr, 0, callback);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_OnlyHeader
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync with only header.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_OnlyHeader, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    auto callback = []() { return false; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0, callback);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_InvalidBlock
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync with invalid block.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_InvalidBlock, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>("", SMB_SIZE_EXTRA);
    EXPECT_FALSE(shareMemoryBlock_->Valid());

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    auto callback = []() { return false; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), nullptr, 0, nullptr, 0, nullptr, 0, callback);
    EXPECT_FALSE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_NullCallback
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync with null callback.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_NullCallback, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t payload[4] = {9, 10, 11, 12};

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        header, sizeof(header), payload, sizeof(payload),
        nullptr, 0, nullptr, 0, nullptr);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_CallbackReturnsTrue
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync with callback returning true.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_CallbackReturnsTrue, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    size_t hugeSize = SMB_SIZE_EXTRA - sizeof(ShareMemoryBlock::BlockHeader) + 1;
    std::vector<int8_t> hugeData(hugeSize, 0);

    auto callback = []() { return true; };

    bool result = shareMemoryBlock_->PutWithExtraDataSync(
        hugeData.data(), hugeSize, nullptr, 0, nullptr, 0, nullptr, 0, callback);
    EXPECT_FALSE(result);
}

/*
 * @tc.name: PutWithExtraDataSync_MultipleWrites
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataSync with multiple writes.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataSync_MultipleWrites, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int8_t header2[8] = {11, 12, 13, 14, 15, 16, 17, 18};
    int8_t firstExtra[4] = {19, 20, 21, 22};

    auto callback = []() { return false; };

    bool result1 = shareMemoryBlock_->PutWithExtraDataSync(
        header1, sizeof(header1), nullptr, 0,
        firstExtra, sizeof(firstExtra), nullptr, 0, callback);
    EXPECT_TRUE(result1);

    bool result2 = shareMemoryBlock_->PutWithExtraDataSync(
        header2, sizeof(header2), nullptr, 0,
        firstExtra, sizeof(firstExtra), nullptr, 0, callback);
    EXPECT_TRUE(result2);
}

/*
 * @tc.name: PutWithExtraDataTimeout_DynamicTagFormat
 * @tc.desc: test ShareMemoryBlock::PutWithExtraDataTimeout with dynamic tag format.
 * @tc.type: FUNC
 */
HWTEST_F(ShareMemoryExceptionTest, PutWithExtraDataTimeout_DynamicTagFormat, TestSize.Level0)
{
    shareMemoryBlock_ = std::make_shared<ShareMemoryBlock>(SMB_NAME_EXTRA, SMB_SIZE_EXTRA);
    if (!shareMemoryBlock_->Valid()) {
        return;
    }

    int8_t header[sizeof(BaseStackRawData)] = {0};
    BaseStackRawData* baseData = reinterpret_cast<BaseStackRawData*>(header);
    baseData->type = MMAP_FILE_PAGE_MSG;
    baseData->tagId = TAG_ID_PENDING;
    baseData->pid = 1234;
    baseData->tid = 5678;

    std::string tagName = "libtest_dynamic.so";
    int8_t firstExtra[32] = {0};
    memcpy_s(firstExtra, sizeof(firstExtra), tagName.c_str(), tagName.size() + 1);

    uint32_t originSize = sizeof(BaseStackRawData);
    int8_t secondExtra[sizeof(uint32_t)] = {0};
    memcpy_s(secondExtra, sizeof(uint32_t), &originSize, sizeof(uint32_t));

    bool result = shareMemoryBlock_->PutWithExtraDataTimeout(
        header, sizeof(header), nullptr, 0,
        firstExtra, tagName.size() + 1, secondExtra, sizeof(uint32_t));
    EXPECT_TRUE(result);
}

} // namespace OHOS::Developtools::NativeDaemon