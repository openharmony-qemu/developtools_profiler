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

#include "process_filter.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class ProcessFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        streamFilters_.processFilter_ = std::make_unique<ProcessFilter>(&traceDataCache_, &streamFilters_);
    }

    void TearDown() {}

public:
    TraceStreamerFilters streamFilters_;
    TraceDataCache traceDataCache_;
};

/**
 * @tc.name: UpdateOrCreateThread
 * @tc.desc: Test UpdateOrCreateThread interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThread, TestSize.Level1)
{
    TS_LOGI("test26-1");
    uint64_t ts = 168758662877000;
    uint32_t tid0 = 2716;
    uint32_t iTid0 = streamFilters_.processFilter_->UpdateOrCreateThread(ts, tid0);
    EXPECT_TRUE(iTid0 == 1);

    uint32_t tid1 = 2519;
    uint32_t iTid1 = streamFilters_.processFilter_->UpdateOrCreateThread(0, tid1);
    EXPECT_TRUE(iTid1 == 2);

    Thread* thread = traceDataCache_.GetThreadData(iTid0);
    EXPECT_TRUE(thread->tid_ == tid0);

    thread = traceDataCache_.GetThreadData(iTid1);
    EXPECT_TRUE(thread->tid_ == tid1);
}

/**
 * @tc.name: UpdateOrCreateProcessWithName
 * @tc.desc: Test UpdateOrCreateProcessWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateProcessWithName, TestSize.Level1)
{
    TS_LOGI("test26-2");
    uint32_t pid0 = 8629;
    std::string_view processName = "RenderThread";
    uint32_t iPid0 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid0, processName);
    EXPECT_TRUE(iPid0 == 1);

    uint32_t pid1 = 8709;
    uint32_t iPid1 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid1, processName);
    EXPECT_TRUE(iPid1 == 2);

    Process* process = traceDataCache_.GetProcessData(iPid0);
    EXPECT_TRUE(process->pid_ == pid0);

    process = traceDataCache_.GetProcessData(iPid1);
    EXPECT_TRUE(process->pid_ == pid1);
}

/**
 * @tc.name: UpdateOrCreateProcessWithNameSingleIpid
 * @tc.desc: Test whether the internal PID of the generated single process is as expected.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateProcessWithNameSingleIpid, TestSize.Level1)
{
    TS_LOGI("test26-3");
    uint32_t pid = 8629;
    std::string_view processName = "RenderThread";
    uint32_t iPid0 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid, processName);
    EXPECT_TRUE(iPid0 == 1);
}

/**
 * @tc.name: UpdateOrCreateProcessWithNameMultiIpid
 * @tc.desc: Test genarated multi ipid with UpdateOrCreateProcessWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateProcessWithNameMultiIpid, TestSize.Level1)
{
    TS_LOGI("test26-4");
    uint32_t pid0 = 8629;
    std::string_view processName = "RenderThread";
    uint32_t iPid0 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid0, processName);
    EXPECT_TRUE(iPid0 == 1);

    uint32_t pid1 = 8709;
    uint32_t iPid1 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid1, processName);
    EXPECT_TRUE(iPid1 == 2);
}

/**
 * @tc.name: UpdateOrCreateProcessWithNameSinglePid
 * @tc.desc: Test genarated single pid with UpdateOrCreateProcessWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateProcessWithNameSinglePid, TestSize.Level1)
{
    TS_LOGI("test26-5");
    uint32_t pid = 8629;
    std::string_view processName = "RenderThread";
    uint32_t iPid0 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid, processName);
    EXPECT_TRUE(iPid0 == 1);

    Process* process = traceDataCache_.GetProcessData(iPid0);
    EXPECT_TRUE(process->pid_ == pid);
}

/**
 * @tc.name: UpdateOrCreateProcessWithNameMultiPid
 * @tc.desc: est genarated multi pid with UpdateOrCreateProcessWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateProcessWithNameMultiPid, TestSize.Level1)
{
    TS_LOGI("test26-6");
    uint32_t pid0 = 8629;
    std::string_view processName = "RenderThread";
    uint32_t iPid0 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid0, processName);
    EXPECT_TRUE(iPid0 == 1);

    uint32_t pid1 = 8709;
    uint32_t iPid1 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid1, processName);
    EXPECT_TRUE(iPid1 == 2);

    uint32_t pid3 = 87091;
    uint32_t iPid2 = streamFilters_.processFilter_->UpdateOrCreateProcessWithName(87091, processName);
    EXPECT_TRUE(iPid2 == 3);

    Process* process = traceDataCache_.GetProcessData(iPid0);
    EXPECT_TRUE(process->pid_ == pid0);

    process = traceDataCache_.GetProcessData(iPid1);
    EXPECT_TRUE(process->pid_ == pid1);

    process = traceDataCache_.GetProcessData(iPid2);
    EXPECT_TRUE(process->pid_ == pid3);
}

/**
 * @tc.name: UpdateOrCreateThreadWithName
 * @tc.desc: Test UpdateOrCreateThreadWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithName, TestSize.Level1)
{
    TS_LOGI("test26-7");
    uint64_t ts = 168758662957020;
    uint32_t tid = 123;
    std::string_view threadName = "RenderThread";
    uint32_t iTid0 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts, tid, threadName);
    EXPECT_TRUE(iTid0 == 1);
    uint64_t ts2 = 168758663957020;
    uint32_t tid2 = 2519;
    std::string_view threadName2 = "RenderThread2";
    uint32_t iTid1 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts2, tid2, threadName2);
    EXPECT_TRUE(iTid1 == 2);

    Thread* thread = traceDataCache_.GetThreadData(iTid0);
    EXPECT_TRUE(thread->tid_ == tid);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName));

    thread = traceDataCache_.GetThreadData(iTid1);
    EXPECT_TRUE(thread->tid_ == tid2);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName2));
}

/**
 * @tc.name: UpdateOrCreateThreadWithNameSingleItid
 * @tc.desc: Test genarated single itid with UpdateOrCreateThreadWithName
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithNameSingleItid, TestSize.Level1)
{
    TS_LOGI("test26-8");
    uint64_t ts = 168758662957020;
    uint32_t tid = 123;
    std::string_view threadName = "RenderThread";
    uint32_t iTid0 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts, tid, threadName);
    EXPECT_TRUE(iTid0 == 1);
}

/**
 * @tc.name: UpdateOrCreateThreadWithNameGenarateTidAndItid
 * @tc.desc: Test genarated single itid and tid with UpdateOrCreateThreadWithName
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithNameGenarateTidAndItid, TestSize.Level1)
{
    TS_LOGI("test26-9");
    uint64_t ts = 168758662957020;
    uint32_t tid = 123;
    std::string_view threadName = "RenderThread2";
    uint32_t iTid0 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts, tid, threadName);
    EXPECT_TRUE(iTid0 == 1);
    Thread* thread = traceDataCache_.GetThreadData(iTid0);
    EXPECT_TRUE(thread->tid_ == tid);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName));
}

/**
 * @tc.name: UpdateOrCreateThreadWithNameDoubleItid
 * @tc.desc: Test genarate double itid with UpdateOrCreateThreadWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithNameDoubleItid, TestSize.Level1)
{
    TS_LOGI("test26-10");
    uint64_t ts = 168758662957020;
    uint32_t tid = 123;
    std::string_view threadName = "RenderThread";
    uint32_t iTid0 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts, tid, threadName);
    EXPECT_TRUE(iTid0 == 1);
    uint64_t ts2 = 168758663957020;
    uint32_t tid2 = 2519;
    std::string_view threadName2 = "RenderThread2";
    uint32_t iTid1 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts2, tid2, threadName2);
    EXPECT_TRUE(iTid1 == 2);
    auto thread = traceDataCache_.GetThreadData(iTid1);
    EXPECT_TRUE(thread->tid_ == tid2);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName2));
}

/**
 * @tc.name: UpdateOrCreateThreadWithNameTripleItid
 * @tc.desc:  Test genarate triple itid with UpdateOrCreateThreadWithName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithNameTripleItid, TestSize.Level1)
{
    TS_LOGI("test26-11");
    uint64_t ts = 168758662957020;
    uint32_t tid = 123;
    std::string_view threadName = "RenderThread";
    uint32_t iTid0 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts, tid, threadName);
    EXPECT_TRUE(iTid0 == 1);
    uint64_t ts2 = 168758663957020;
    uint32_t tid2 = 2519;
    std::string_view threadName2 = "RenderThread2";
    uint32_t iTid1 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts2, tid2, threadName2);
    EXPECT_TRUE(iTid1 == 2);
    uint64_t ts3 = 168758663957020;
    uint32_t tid3 = 25191;
    std::string_view threadName3 = "RenderThread3";
    uint32_t iTid2 = streamFilters_.processFilter_->UpdateOrCreateThreadWithName(ts3, tid3, threadName3);
    EXPECT_TRUE(iTid2 == 3);
    auto thread = traceDataCache_.GetThreadData(iTid2);
    EXPECT_TRUE(thread->tid_ == tid3);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName3));
}

/**
 * @tc.name: UpdateOrCreateThreadWithPidAndName
 * @tc.desc: Test UpdateOrCreateThreadWithPidAndName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithPidAndName, TestSize.Level1)
{
    TS_LOGI("test26-12");
    uint32_t tid = 869;
    uint32_t pid = 123;
    std::string_view threadName = "RenderThread";
    streamFilters_.processFilter_->UpdateOrCreateThreadWithPidAndName(tid, pid, threadName);
    auto itid = streamFilters_.processFilter_->GetInternalTid(tid);
    EXPECT_TRUE(itid != INVALID_ID);

    Thread* thread = traceDataCache_.GetThreadData(itid);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName));
}

/**
 * @tc.name: UpdateOrCreateThreadWithPidAndNameAbnomal
 * @tc.desc: Test UpdateOrCreateThreadWithPidAndName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithPidAndNameAbnomal, TestSize.Level1)
{
    TS_LOGI("test26-13");
    uint32_t tid = 869;
    uint32_t pid = 123;
    std::string_view threadName = "RenderThread";
    streamFilters_.processFilter_->UpdateOrCreateThreadWithPidAndName(tid, pid, threadName);
    uint32_t tid2 = 969;
    auto itid = streamFilters_.processFilter_->GetInternalTid(tid2);
    EXPECT_TRUE(itid == INVALID_ID);
}

/**
 * @tc.name: UpdateOrCreateThreadWithPidAndNameSingleItid
 * @tc.desc: Test UpdateOrCreateThreadWithPidAndName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithPidAndNameSingleItid, TestSize.Level1)
{
    TS_LOGI("test26-14");
    uint32_t tid = 869;
    uint32_t pid = 123;
    std::string_view threadName = "RenderThread";
    streamFilters_.processFilter_->UpdateOrCreateThreadWithPidAndName(tid, pid, threadName);
    auto itid = streamFilters_.processFilter_->GetInternalPid(pid);
    EXPECT_TRUE(itid != INVALID_ID);
}

/**
 * @tc.name: UpdateOrCreateThreadWithPidAndNameAbnomalPid
 * @tc.desc: Test UpdateOrCreateThreadWithPidAndName interface
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateOrCreateThreadWithPidAndNameAbnomalPid, TestSize.Level1)
{
    TS_LOGI("test26-15");
    uint32_t tid = 869;
    uint32_t pid0 = 123;
    std::string_view threadName = "RenderThread";
    streamFilters_.processFilter_->UpdateOrCreateThreadWithPidAndName(tid, pid0, threadName);
    uint32_t pid1 = 124;
    auto itid = streamFilters_.processFilter_->GetInternalPid(pid1);
    EXPECT_TRUE(itid == INVALID_ID);
}

/**
 * @tc.name: UpdateThreadWithName
 * @tc.desc: Test update thread name
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateThreadWithName, TestSize.Level1)
{
    TS_LOGI("test26-16");
    uint32_t tid = 869;
    uint64_t timeStamp = 168758662957020;
    std::string_view threadName = "RenderThread";
    streamFilters_.processFilter_->UpdateOrCreateThread(timeStamp, tid);
    streamFilters_.processFilter_->UpdateOrCreateThreadWithName(timeStamp, tid, threadName);
    auto itid = streamFilters_.processFilter_->GetInternalTid(tid);
    EXPECT_TRUE(itid != INVALID_ID);
    Thread* thread = traceDataCache_.GetThreadData(itid);
    EXPECT_TRUE(thread->nameIndex_ == traceDataCache_.GetDataIndex(threadName));
}

/**
 * @tc.name: UpdateProcessWithName
 * @tc.desc: Test update process name
 * @tc.type: FUNC
 */
HWTEST_F(ProcessFilterTest, UpdateProcessWithName, TestSize.Level1)
{
    TS_LOGI("test26-17");
    uint32_t pid = 869;
    uint64_t timeStamp = 168758662957020;
    std::string_view processName = "RenderProcess";
    auto ipid = streamFilters_.processFilter_->GetOrCreateInternalPid(timeStamp, pid);
    EXPECT_TRUE(ipid != INVALID_ID);
    streamFilters_.processFilter_->UpdateOrCreateProcessWithName(pid, processName);
    Process* process = traceDataCache_.GetProcessData(ipid);
    EXPECT_TRUE(process->cmdLine_ == processName);
}
} // namespace TraceStreamer
} // namespace SysTuning
