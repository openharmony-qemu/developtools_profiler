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

#include "binder_filter.h"
#include "htrace_cpu_detail_parser.h"
#include "htrace_event_parser.h"
#include "trace_plugin_result.pb.h"
#include "trace_plugin_result.pbreader.h"
#include "trace_streamer_selector.h"
#include "ts_common.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class HtraceBinderEventTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};

/**
 * @tc.name: BinderSenderfilterNeedReply
 * @tc.desc: Binary formate binder event test, The binder event needs reply
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderfilterNeedReply, TestSize.Level1)
{
    TS_LOGI("test10-1");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_); 0x01
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 7);
}

/**
 * @tc.name: BinderSenderfilterNeedReplyAndReceive
 * @tc.desc: Binary formate binder event test, The binder event needs reply and received reply
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderfilterNeedReplyAndReceive, TestSize.Level1)
{
    TS_LOGI("test10-2");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_); 0x01
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 7);

    ts1 = 200;
    uint32_t pid1 = 1;
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(0);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(pid1);
    std::string appName2 = "app2";
    ftraceEvent2->set_comm(appName2);
    BinderTransactionReceivedFormat* binderReceivedEvent = new BinderTransactionReceivedFormat();
    binderReceivedEvent->set_debug_id(transactionId1);
    ftraceEvent2->set_allocated_binder_transaction_received_format(binderReceivedEvent);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 2);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[1] == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 11);
}

/**
 * @tc.name: BinderSenderfilterNeedReplyAndReceiveWithAlloc
 * @tc.desc: Binary formate BinderTransactionAllocBuf event test, The binder event needs reply and received reply
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderfilterNeedReplyAndReceiveWithAlloc, TestSize.Level1)
{
    TS_LOGI("test10-3");
    int64_t ts1 = 100;
    std::string appName = "app1";
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 7);

    ts1 = 150;
    uint64_t dataSize = 100;
    uint64_t offsetSize = 200;
    BinderTransactionAllocBufFormat* binderAllocEvent = new BinderTransactionAllocBufFormat();
    binderAllocEvent->set_data_size(dataSize);
    binderAllocEvent->set_offsets_size(offsetSize);

    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(0);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(tid1);
    ftraceEvent2->set_comm(appName);
    ftraceEvent2->set_allocated_binder_transaction_alloc_buf_format(binderAllocEvent);

    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 9);

    ts1 = 200;
    uint32_t pid1 = 1;
    TracePluginResult tracePacket3;
    FtraceCpuDetailMsg* ftraceCpuDetail3 = tracePacket3.add_ftrace_cpu_detail();
    ftraceCpuDetail3->set_cpu(0);
    ftraceCpuDetail3->set_overwrite(0);
    auto ftraceEvent3 = ftraceCpuDetail3->add_event();

    ftraceEvent3->set_timestamp(ts1);
    ftraceEvent3->set_tgid(pid1);
    std::string appName2 = "app2";
    ftraceEvent3->set_comm(appName2);
    BinderTransactionReceivedFormat* binderReceivedEvent = new BinderTransactionReceivedFormat();
    binderReceivedEvent->set_debug_id(transactionId1);
    ftraceEvent3->set_allocated_binder_transaction_received_format(binderReceivedEvent);
    HtraceDataSegment dataSeg3;
    dataSeg3.clockId = TS_CLOCK_BOOTTIME;
    tracePacket3.SerializeToString(&cpuDetailStrMsg);
    dataSeg3.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView3(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg3.protoData = cpuDetailBytesView3;
    eventParser.ParseDataItem(dataSeg3, dataSeg3.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 2);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[1] == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 13);
}

/**
 * @tc.name: BinderSenderfilterNeedReplyAndReceiveNotmatch
 * @tc.desc: Binary formate BinderTransaction event test, The binder event needs reply but received not match
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderfilterNeedReplyAndReceiveNotmatch, TestSize.Level1)
{
    TS_LOGI("test10-4");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x02; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);

    ts1 = 200;
    uint32_t pid1 = 1;
    uint64_t transactionId2 = 2;
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(0);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(pid1);
    std::string appName2 = "app2";
    ftraceEvent2->set_comm(appName2);
    BinderTransactionReceivedFormat* binderReceivedEvent = new BinderTransactionReceivedFormat();
    binderReceivedEvent->set_debug_id(transactionId2);
    ftraceEvent2->set_allocated_binder_transaction_received_format(binderReceivedEvent);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
}

/**
 * @tc.name: BinderSenderfilterNoNeedReply
 * @tc.desc: Binary formate binder event test, The binder event needs no reply
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderfilterNoNeedReply, TestSize.Level1)
{
    TS_LOGI("test10-5");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);
}

/**
 * @tc.name: BinderSenderNoneedReplyAndReceivefilter
 * @tc.desc: Binary formate binder event test, other party received and no need reply。
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderNoneedReplyAndReceivefilter, TestSize.Level1)
{
    TS_LOGI("test10-6");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);

    ts1 = 200;
    uint32_t pid1 = 1;
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(0);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(pid1);
    std::string appName2 = "app2";
    ftraceEvent2->set_comm(appName2);
    BinderTransactionReceivedFormat* binderReceivedEvent = new BinderTransactionReceivedFormat();
    binderReceivedEvent->set_debug_id(transactionId1);
    ftraceEvent2->set_allocated_binder_transaction_received_format(binderReceivedEvent);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 2);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[1] == 0);
}

/**
 * @tc.name: BinderSenderNoneedReplyAndReceivefilterNotmatch
 * @tc.desc: Binary formate binder event test, other party received but not match
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderNoneedReplyAndReceivefilterNotmatch, TestSize.Level1)
{
    TS_LOGI("test10-7");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = false;
    uint32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    uint32_t code = 0;     // not importent
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);

    ts1 = 200;
    uint32_t pid1 = 1;
    uint64_t transactionId2 = 2;
    TracePluginResult tracePacket2;
    FtraceCpuDetailMsg* ftraceCpuDetail2 = tracePacket2.add_ftrace_cpu_detail();
    ftraceCpuDetail2->set_cpu(0);
    ftraceCpuDetail2->set_overwrite(0);
    auto ftraceEvent2 = ftraceCpuDetail2->add_event();

    ftraceEvent2->set_timestamp(ts1);
    ftraceEvent2->set_tgid(pid1);
    std::string appName2 = "app2";
    ftraceEvent2->set_comm(appName2);
    BinderTransactionReceivedFormat* binderReceivedEvent = new BinderTransactionReceivedFormat();
    binderReceivedEvent->set_debug_id(transactionId2);
    ftraceEvent2->set_allocated_binder_transaction_received_format(binderReceivedEvent);
    HtraceDataSegment dataSeg2;
    dataSeg2.clockId = TS_CLOCK_BOOTTIME;
    tracePacket2.SerializeToString(&cpuDetailStrMsg);
    dataSeg2.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView2(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                               cpuDetailStrMsg.size());
    dataSeg2.protoData = cpuDetailBytesView2;
    eventParser.ParseDataItem(dataSeg2, dataSeg2.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 1);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().ArgSetIdsData()[0] == 0);
}

/**
 * @tc.name: BinderSenderfilterWrongReply
 * @tc.desc: Binary formate binder event test, other party replyed wrong Info
 * @tc.type: FUNC
 */
HWTEST_F(HtraceBinderEventTest, BinderSenderfilterWrongReply, TestSize.Level1)
{
    TS_LOGI("test10-8");
    std::string appName = "app1";
    int64_t ts1 = 100;
    uint32_t tid1 = 1;
    uint64_t transactionId1 = 1;
    int32_t destNode1 = 1;
    int32_t destTgid1 = 2;
    int32_t destTid1 = 3;
    bool isReply = true;
    uint32_t flags = 0x01; // if need reply  bool needReply = !isReply && !(flags & noReturnMsgFlag_)
    uint32_t code = 0;     // not important
    BinderTransactionFormat* binderEvent = new BinderTransactionFormat();
    binderEvent->set_to_proc(destTgid1);
    binderEvent->set_target_node(destNode1);
    binderEvent->set_to_thread(destTid1);
    binderEvent->set_debug_id(transactionId1);
    binderEvent->set_reply(static_cast<int32_t>(isReply));
    binderEvent->set_code(code);
    binderEvent->set_flags(flags);

    TracePluginResult tracePacket;
    FtraceCpuDetailMsg* ftraceCpuDetail = tracePacket.add_ftrace_cpu_detail();
    ftraceCpuDetail->set_cpu(0);
    ftraceCpuDetail->set_overwrite(0);
    auto ftraceEvent = ftraceCpuDetail->add_event();

    ftraceEvent->set_timestamp(ts1);
    ftraceEvent->set_tgid(tid1);
    ftraceEvent->set_comm(appName);
    ftraceEvent->set_allocated_binder_transaction_format(binderEvent);

    HtraceDataSegment dataSeg;
    dataSeg.clockId = TS_CLOCK_BOOTTIME;
    std::string cpuDetailStrMsg = "";
    tracePacket.SerializeToString(&cpuDetailStrMsg);
    dataSeg.seg = std::make_shared<std::string>(cpuDetailStrMsg);
    ProtoReader::BytesView cpuDetailBytesView(reinterpret_cast<const uint8_t*>(cpuDetailStrMsg.data()),
                                              cpuDetailStrMsg.size());
    dataSeg.protoData = cpuDetailBytesView;

    HtraceEventParser eventParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    eventParser.ParseDataItem(dataSeg, dataSeg.clockId);
    eventParser.FilterAllEvents();
    EXPECT_TRUE(stream_.traceDataCache_->GetConstInternalSlicesData().Size() == 0);
    EXPECT_TRUE(stream_.traceDataCache_->GetConstArgSetData().Size() == 0);
}
} // namespace TraceStreamer
} // namespace SysTuning
