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
#include "irq_filter.h"
#include "slice_filter.h"
#include "stat_filter.h"
#include "ts_common.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class IrqFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.argsFilter_ = std::make_unique<ArgsFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.irqFilter_ = std::make_unique<IrqFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.sliceFilter_ = std::make_unique<SliceFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.statFilter_ = std::make_unique<StatFilter>(&traceDataCache_, &streamFilters_);
        streamFilters_.statFilter_ = std::make_unique<StatFilter>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    TraceStreamerFilters streamFilters_;
    TraceDataCache traceDataCache_;
};

/**
 * @tc.name: IrqHandlerEntryTest
 * @tc.desc: IrqHandlerEntry Normal TEST
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, IrqHandlerEntryTest, TestSize.Level1)
{
    TS_LOGI("test22-1");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    DataIndex nameId1 = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, nameId1); // IrqHandlerEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
}

/**
 * @tc.name: IrqHandlerEntryTestNotMatch
 * @tc.desc: Test two interrupts, only start, no end
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, IrqHandlerEntryTestNotMatch, TestSize.Level1)
{
    TS_LOGI("test22-2");
    int64_t ts1 = 120;
    uint32_t cpu1 = 1;
    DataIndex nameId1 = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, nameId1); // IrqHandlerEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    ts1 = 110;
    uint32_t irqRet = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, irqRet); // IrqHandlerEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 2);
    // TRACE_EVENT_IRQ_HANDLER_ENTRY STAT_EVENT_DATA_LOST
    auto eventCount =
        traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_DATA_LOST);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: IrqHandlerExitTestEmpty
 * @tc.desc:Interrupt only ends, not starts
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, IrqHandlerExitTestEmpty, TestSize.Level1)
{
    TS_LOGI("test22-3");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t ret = 1;                                               // 1 for handled, else for unhandled
    streamFilters_.irqFilter_->IrqHandlerExit(ts1, cpu1, ret, ret); // IrqHandlerExit
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 0);
    // TRACE_EVENT_IRQ_HANDLER_EXIT STAT_EVENT_NOTMATCH
    auto eventCount = traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_NOTMATCH);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: IrqHandlerEnterAndExitTest
 * @tc.desc: Interrupt normal start and end
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, IrqHandlerEnterAndExitTest, TestSize.Level1)
{
    TS_LOGI("test22-4");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    DataIndex nameId1 = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, nameId1); // IrqHandlerEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    uint32_t irqRet = 1;                                                  // 1 for handled, else for unhandled
    streamFilters_.irqFilter_->IrqHandlerExit(ts1, cpu1, irqRet, irqRet); // IrqHandlerExit
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 2);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[0] ==
                static_cast<int64_t>(streamFilters_.irqFilter_->irqHandled_));
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().names_[0] == streamFilters_.irqFilter_->irqRet_);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().argset_[0] == 0);
}

/**
 * @tc.name: IrqHandlerDoubleEnterAndExitTest
 * @tc.desc: Interrupt normal test, 2 interrupts and exits
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, IrqHandlerDoubleEnterAndExitTest, TestSize.Level1)
{
    TS_LOGI("test22-5");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    DataIndex nameId1 = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, nameId1); // IrqHandlerEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    uint32_t ret = 1; // 1 for handled, else for unhandled
    cpu1 = 2;
    ts1 = 150;
    streamFilters_.irqFilter_->IrqHandlerExit(ts1, cpu1, ret, ret); // IrqHandlerExit
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_NOTMATCH) == 1);
    cpu1 = 1;
    ts1 = 200;
    streamFilters_.irqFilter_->IrqHandlerExit(ts1, cpu1, ret, ret); // IrqHandlerExit
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().ArgSetIdsData()[0] == 0);
}

/**
 * @tc.name: IrqHandlerTripleEnterAndExitTest
 * @tc.desc: Interrupt normal test, 3 interrupts and exits
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, IrqHandlerTripleEnterAndExitTest, TestSize.Level1)
{
    TS_LOGI("test22-6");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    DataIndex nameId1 = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, nameId1); // IrqHandlerEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    uint32_t ret = 1; // 1 for handled, else for unhandled
    ts1 = 150;
    streamFilters_.irqFilter_->IrqHandlerExit(ts1, cpu1, ret, ret); // IrqHandlerExit
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    // check args
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().ArgSetIdsData()[0] == 0);

    ts1 = 200;
    cpu1 = 1;
    nameId1 = 1;
    streamFilters_.irqFilter_->IrqHandlerEntry(ts1, cpu1, nameId1);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 2);
    ret = 1; // 1 for handled, else for unhandled
    ts1 = 250;
    streamFilters_.irqFilter_->IrqHandlerExit(ts1, cpu1, ret, ret);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 2);
    // check args
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().ArgSetIdsData()[1] == 1);
}

/**
 * @tc.name: SoftIrqEntryTest
 * @tc.desc: Soft interrupt normal test
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqEntryTest, TestSize.Level1)
{
    TS_LOGI("test22-7");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
}

/**
 * @tc.name: SoftIrqEntryNotMatch
 * @tc.desc: The soft interrupts do not match. The two interrupts have only the beginning and no end
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqEntryNotMatch, TestSize.Level1)
{
    TS_LOGI("test22-8");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec); // SoftIrqEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    ts1 = 150;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec); // SoftIrqEntry
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 2);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_DATA_LOST) == 1);
}

/**
 * @tc.name: SoftIrqExitEmptyTest
 * @tc.desc: The soft interrupt only ends without starting
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqExitEmptyTest, TestSize.Level1)
{
    TS_LOGI("test22-9");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 0);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
}

/**
 * @tc.name: SoftIrqTest
 * @tc.desc: The soft interrupt normal test
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqTest, TestSize.Level1)
{
    TS_LOGI("test22-10");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    ts1 = 150;
    uint32_t irqRet = 1;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, irqRet);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().Size() == 2);
    printf("%ld\n", traceDataCache_.GetConstArgSetData().values_[0]);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().values_[0] ==
                static_cast<int64_t>(streamFilters_.irqFilter_->irqActionNameIds_[irqRet]));
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().names_[0] == streamFilters_.irqFilter_->irqRet_);
    EXPECT_TRUE(traceDataCache_.GetConstArgSetData().argset_[0] == 0);
}

/**
 * @tc.name: SoftIrqTestWithIrqEntryAndExit
 * @tc.desc: The soft interrupt normal test
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqTestWithIrqEntryAndExit, TestSize.Level1)
{
    TS_LOGI("test22-11");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    ts1 = 150;
    cpu1 = 2;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
}

/**
 * @tc.name: SoftIrqTestOneEntryTwoNotMatchExit
 * @tc.desc: The soft interrupt test with onece entry and twice Not Match exit
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqTestOneEntryTwoNotMatchExit, TestSize.Level1)
{
    TS_LOGI("test22-12");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    ts1 = 150;
    cpu1 = 2;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
    ts1 = 200;
    cpu1 = 3;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 2);
}

/**
 * @tc.name: SoftIrqTestWithSingleNotMatchExit
 * @tc.desc: The soft interrupt test with single not Match Exit
 * @tc.type: FUNC
 */
HWTEST_F(IrqFilterTest, SoftIrqTestWithSingleNotMatchExit, TestSize.Level1)
{
    TS_LOGI("test22-13");
    int64_t ts1 = 100;
    uint32_t cpu1 = 1;
    uint32_t vec = 1;
    streamFilters_.irqFilter_->SoftIrqEntry(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    ts1 = 150;
    cpu1 = 2;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
    ts1 = 200;
    cpu1 = 1;
    streamFilters_.irqFilter_->SoftIrqExit(ts1, cpu1, vec);
    EXPECT_TRUE(traceDataCache_.GetConstIrqData().Size() == 1);
    EXPECT_TRUE(traceDataCache_.GetConstStatAndInfo().GetValue(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST) == 1);
}
} // namespace TraceStreamer
} // namespace SysTuning
