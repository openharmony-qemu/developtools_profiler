/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "args_filter.h"
#include "binder_filter.h"
#include "process_filter.h"
#include "slice_filter.h"
#include "stat_filter.h"
#include "trace_plugin_result.pb.h"
#include "ts_common.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class BinderFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.processFilter_ = std::make_unique<ProcessFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.argsFilter_ = std::make_unique<ArgsFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.binderFilter_ = std::make_unique<BinderFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.statFilter_ = std::make_unique<StatFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.sliceFilter_ = std::make_unique<SliceFilter>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    TraceStreamerFilters streamFilters_;
    TraceDataCache traceDataCache_;
};

/**
 * @tc.name: BinderSenderfilterNeedReply
 * @tc.desc: Binder event needs reply to
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderfilterNeedReply, TestSize.Level1)
{
    TS_LOGI("test1-1");
    uint64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_); 0x01
    int32_t code = 0;     // not important
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags, code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 7);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().TimeStampData()[0] == ts1);
}

/**
 * @tc.name: BinderSenderfilterNeedReplyAndReceive
 * @tc.desc: Complete event, Binder event needs reply to
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderfilterNeedReplyAndReceive, TestSize.Level1)
{
    TS_LOGI("test1-2");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not importent
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags, code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 7);
    ts1 = 200;
    uint32_t pid1 = 1;
    streamFilters_.binderFilter_->ReceiveTraction(ts1, pid1, transactionId1); // receive binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 2);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[1] == 1);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 11);
    auto len = traceDataCache_.GetConstArgSetData().Size();
    for (uint64_t i = 0; i < len; i++) {
        if (traceDataCache_.GetConstArgSetData().names_[i] == streamFilters_.binderFilter_->isReplayId_) {
            EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[i] == static_cast<int64_t>(isReply));
        } else if (traceDataCache_.GetConstArgSetData().names_[i] == streamFilters_.binderFilter_->destNodeId_) {
            EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[i] == static_cast<int64_t>(destNode1));
        } else if (traceDataCache_.GetConstArgSetData().names_[i] == streamFilters_.binderFilter_->destThreadId_) {
            EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[i] == static_cast<int64_t>(pid1));
        } else if (traceDataCache_.GetConstArgSetData().names_[i] == streamFilters_.binderFilter_->callingTid_) {
            EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[i] == static_cast<int64_t>(tid1));
        } else if (traceDataCache_.GetConstArgSetData().names_[i] == streamFilters_.binderFilter_->transId_) {
            EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[i] == static_cast<int64_t>(transactionId1));
        }
    }
}

/**
 * @tc.name: BinderSenderfilterNeedReplyAndReceiveWithAlloc
 * @tc.desc: The binder test that needs to be replied and there is an allock event
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderfilterNeedReplyAndReceiveWithAlloc, TestSize.Level1)
{
    TS_LOGI("test1-3");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not importent
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags,
                                               code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 7);

    ts1 = 150;
    uint64_t dataSize = 100;
    uint64_t offsetSize = 200;
    streamFilters_.binderFilter_->TransactionAllocBuf(ts1, tid1, dataSize, offsetSize);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 9);

    ts1 = 200;
    uint32_t pid1 = 1;
    streamFilters_.binderFilter_->ReceiveTraction(ts1, pid1, transactionId1); // receive binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 2);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[1] == 1);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 13);
}

/**
 * @tc.name: BinderSenderfilterNeedReplyAndReceiveNotmatch
 * @tc.desc: The binder test that needs to be replied but not match
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderfilterNeedReplyAndReceiveNotmatch, TestSize.Level1)
{
    TS_LOGI("test1-4");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not importent
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags,
                                               code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
    ts1 = 200;
    uint32_t pid1 = 1;
    uint64_t transactionId2 = 2;
    streamFilters_.binderFilter_->ReceiveTraction(ts1, pid1, transactionId2); // receive binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
}

/**
 * @tc.name: BinderSenderfilterNoNeedReply
 * @tc.desc: The binder test that donot needs to be replied
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderfilterNoNeedReply, TestSize.Level1)
{
    TS_LOGI("test1-5");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not importent
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags, code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
}

/**
 * @tc.name: BinderSenderNoneedReplyAndReceivefilter
 * @tc.desc: Complete event, The binder test that donot needs to be replied
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderNoneedReplyAndReceivefilter, TestSize.Level1)
{
    TS_LOGI("test1-6");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not importent
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags,
                                               code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);

    ts1 = 200;
    uint32_t pid1 = 1;
    streamFilters_.binderFilter_->ReceiveTraction(ts1, pid1, transactionId1); // receive binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 2);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[1] == 0);
}

/**
 * @tc.name: BinderSenderNoneedReplyAndReceivefilterNotmatch
 * @tc.desc: Not Match, The binder test that donot needs to be replied
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderNoneedReplyAndReceivefilterNotmatch, TestSize.Level1)
{
    TS_LOGI("test1-7");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    int32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not importent
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags,
                                               code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);

    ts1 = 200;
    uint32_t pid1 = 1;
    uint64_t transactionId2 = 2;
    streamFilters_.binderFilter_->ReceiveTraction(ts1, pid1, transactionId2); // receive binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
}

/**
 * @tc.name: BinderSenderfilterWrongReply
 * @tc.desc: The binder test with wrong replie
 * @tc.type: FUNC
 */
HWTEST_F(BinderFilterTest, BinderSenderfilterWrongReply, TestSize.Level1)
{
    TS_LOGI("test1-8");
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = true;
    int32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    int32_t code = 0;     // not important
    streamFilters_.binderFilter_->SendTraction(ts1, tid1, transactionId1, destNode1, destTgid1, destTid1, isReply,
                                               flags,
                                               code); // start binder
    EXPECT_TRUE(traceDataCache_.GetConstInternalSlicesData().Size() == 0);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 0);
}
} // namespace TraceStreamer
} // namespace SysTuning
