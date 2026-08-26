/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <google/protobuf/text_format.h>
#define private public
#include "hook_record.h"
#undef private
#include "hook_common.h"
#include "native_hook_config.pb.h"
#include "plugin_module_api.h"
#include "native_hook_result_standard.pb.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr uint32_t TEST_PID = 1234;
constexpr uint32_t TEST_TID = 5678;
constexpr uint64_t TEST_ADDR = 0x10000000;
constexpr uint32_t TEST_SIZE = 1024;
constexpr uint16_t TEST_TYPE = MALLOC_MSG;
constexpr uint32_t TEST_NODE_TYPE = 1;
constexpr uint64_t TEST_NODE_ID = 100;
const std::string TEST_TAG_NAME = "test_tag";
constexpr size_t ENCODER_BUFFER_SIZE = 4096;
static uint8_t g_encoderBuffer[ENCODER_BUFFER_SIZE];
static uint32_t g_encoderOffset = 0;

static bool GetEncoderMemory(RandomWriteCtx*, uint32_t size, uint8_t** memory, uint32_t* offset)
{
    if (memory == nullptr || offset == nullptr || g_encoderOffset + size > ENCODER_BUFFER_SIZE) {
        return false;
    }
    *memory = g_encoderBuffer + g_encoderOffset;
    *offset = g_encoderOffset;
    g_encoderOffset += size;
    return true;
}

static bool SeekEncoderMemory(RandomWriteCtx*, uint32_t offset)
{
    if (offset > ENCODER_BUFFER_SIZE) {
        return false;
    }
    g_encoderOffset = offset;
    return true;
}

static RandomWriteCtx MakeEncoderWriteCtx()
{
    g_encoderOffset = 0;
    return {GetEncoderMemory, SeekEncoderMemory};
}

static RawStackPtr MakeRawStack(uint16_t type, uint64_t addr, uint32_t size)
{
    auto raw = RawStackPtr(new RawStack(), [](RawStack* p) {
        if (p != nullptr) {
            delete p->stackContext;
            p->stackContext = nullptr;
            delete p;
        }
    });

    raw->stackContext = new BaseStackRawData();
    raw->stackContext->type = type;
    raw->stackContext->addr = reinterpret_cast<void*>(addr);
    raw->stackContext->mallocSize = size;
    raw->stackContext->pid = TEST_PID;
    raw->stackContext->tid = TEST_TID;
    raw->stackSize = 1;
    raw->reportFlag = true;
    return raw;
}

static std::vector<CallFrame> MakeCallFrames()
{
    std::vector<CallFrame> frames;
    CallFrame frame(1);
    frame.ip_ = 0x1000;
    frame.sp_ = 0x2000;
    frame.symbolName_ = "test_function";
    frame.filePath_ = "/path/to/file.cpp";
    frame.offset_ = 0x100;
    frame.symbolOffset_ = 0x50;
    frame.symbolNameId_ = 1;
    frame.filePathId_ = 1;
    frames.push_back(frame);
    return frames;
}

static std::vector<CallFrame> MakeJsCallFrames()
{
    std::vector<CallFrame> frames = MakeCallFrames();
    frames[0].isJsFrame_ = true;
    frames[0].symbolNameId_ = 11;
    frames[0].filePathId_ = 12;
    return frames;
}

static SerializeInfo MakeSerializeInfo(std::vector<CallFrame>& frames, NativeHookConfig& config)
{
    SerializeInfo hookInfo;
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &config;
    return hookInfo;
}

template <typename Record>
static void ExpectEncoderSerializeSucceeds(Record&& record, SerializeInfo& hookInfo)
{
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    OHOS::Developtools::Profiler::ProtoEncoder::NativeHookData encoderEvent(&ctx);
    record.SerializeData(&encoderEvent, hookInfo);
    EXPECT_GT(encoderEvent.Finish(), 0);
}

class HookRecordExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    NativeHookConfig hookConfig_;
};

void HookRecordExtTest::SetUpTestCase(void)
{
}

void HookRecordExtTest::TearDownTestCase(void)
{
}

void HookRecordExtTest::SetUp()
{
    hookConfig_.set_fp_unwind(false);
    hookConfig_.set_callframe_compress(false);
    hookConfig_.set_offline_symbolization(false);
}

void HookRecordExtTest::TearDown()
{
}

/*
@tc.name: HookRecordBasicTest
@tc.desc: test basic methods of HookRecord class.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordBasicTest, TestSize.Level0)
{
    // Test with valid raw stack
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    
    EXPECT_TRUE(hookRecord.IsValid());
    EXPECT_EQ(hookRecord.GetType(), TEST_TYPE);
    EXPECT_EQ(hookRecord.GetAddr(), TEST_ADDR);
    
    // Test SetAddr
    uint64_t newAddr = 0x20000000;
    hookRecord.SetAddr(newAddr);
    EXPECT_EQ(hookRecord.GetAddr(), newAddr);
    
    // Test Reset
    hookRecord.Reset();
    EXPECT_FALSE(hookRecord.IsValid());
    EXPECT_EQ(hookRecord.GetType(), UNKNOWN);
    EXPECT_EQ(hookRecord.GetAddr(), 0);
}

/*
@tc.name: HookRecordInvalidTest
@tc.desc: test HookRecord class with invalid raw stack.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordInvalidTest, TestSize.Level0)
{
    // Test with null raw stack
    HookRecord hookRecord;
    
    EXPECT_FALSE(hookRecord.IsValid());
    EXPECT_EQ(hookRecord.GetType(), UNKNOWN);
    EXPECT_EQ(hookRecord.GetAddr(), 0);
    
    // Test SetAddr with invalid raw stack
    hookRecord.SetAddr(TEST_ADDR);
    EXPECT_EQ(hookRecord.GetAddr(), 0);
    
    // Test Reset with null raw stack
    hookRecord.Reset();
    EXPECT_FALSE(hookRecord.IsValid());
}

/*
@tc.name: FreeRecordSimpTest
@tc.desc: test FreeRecordSimp class.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, FreeRecordSimpTest, TestSize.Level0)
{
    // Test with valid address
    FreeRecordSimp freeRecord(TEST_ADDR);
    
    EXPECT_TRUE(freeRecord.IsValid());
    EXPECT_EQ(freeRecord.GetType(), FREE_MSG_SIMP);
    EXPECT_EQ(freeRecord.GetAddr(), TEST_ADDR);
    
    // Test SetAddr
    uint64_t newAddr = 0x20000000;
    freeRecord.SetAddr(newAddr);
    EXPECT_EQ(freeRecord.GetAddr(), newAddr);
    
    // Test Reset
    freeRecord.Reset();
    EXPECT_FALSE(freeRecord.IsValid());
    EXPECT_EQ(freeRecord.GetAddr(), 0);
    
    // Test with zero address
    FreeRecordSimp freeRecordZero(0);
    EXPECT_FALSE(freeRecordZero.IsValid());
}

/*
@tc.name: HookRecordSetEventFrameTest
@tc.desc: test HookRecord::SetEventFrame method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookConfig_.set_fp_unwind(true);
    hookInfo.config = &hookConfig_;
    
    // Test with alloc event
    NativeHookData event;
    auto allocEvent = event.mutable_alloc_event();
    hookRecord.SetEventFrame(allocEvent, hookInfo);
    
    EXPECT_EQ(allocEvent->pid(), TEST_PID);
    EXPECT_EQ(allocEvent->tid(), TEST_TID);
    EXPECT_EQ(allocEvent->addr(), TEST_ADDR);
    EXPECT_EQ(allocEvent->frame_info_size(), 1);
    
    // Test with callframe compression
    hookConfig_.set_callframe_compress(true);
    hookInfo.stackMapId = 1;
    
    NativeHookData eventCompressed;
    auto allocEventCompressed = eventCompressed.mutable_alloc_event();
    hookRecord.SetEventFrame(allocEventCompressed, hookInfo);
    
    EXPECT_EQ(allocEventCompressed->pid(), TEST_PID);
    EXPECT_EQ(allocEventCompressed->tid(), TEST_TID);
    EXPECT_EQ(allocEventCompressed->addr(), TEST_ADDR);
    EXPECT_EQ(allocEventCompressed->stack_id(), 1);
    EXPECT_EQ(allocEventCompressed->frame_info_size(), 0);
    hookConfig_.set_fp_unwind(false);
}

/*
@tc.name: HookRecordSetEventFrameSkipsDwarfFilter
@tc.desc: test SetEventFrame skips two dwarf frames when fp unwind is disabled.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameSkipsDwarfFilter, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    frames.push_back(frames[0]);
    frames.push_back(frames[0]);
    frames[2].ip_ = 0x3000;
    hookConfig_.set_fp_unwind(false);
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    NativeHookData event;
    auto allocEvent = event.mutable_alloc_event();

    hookRecord.SetEventFrame(allocEvent, hookInfo);
    ASSERT_EQ(allocEvent->frame_info_size(), 1);
    EXPECT_EQ(allocEvent->frame_info(0).ip(), 0x3000u);
    EXPECT_EQ(allocEvent->thread_name_id(), TEST_TID);
}

/*
@tc.name: HookRecordSetEventFrameRejectsInvalidInputs
@tc.desc: test SetEventFrame with null event and invalid record.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameRejectsInvalidInputs, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord validRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    validRecord.SetEventFrame<AllocEvent>(nullptr, hookInfo);
    EXPECT_TRUE(validRecord.IsValid());

    HookRecord invalidRecord;
    NativeHookData event;
    auto allocEvent = event.mutable_alloc_event();
    invalidRecord.SetEventFrame(allocEvent, hookInfo);
    EXPECT_EQ(allocEvent->pid(), 0);
    EXPECT_EQ(allocEvent->frame_info_size(), 0);
}

HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameCompressesEventKinds, TestSize.Level0)
{
    constexpr uint32_t stackMapId = 7;
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    hookConfig_.set_fp_unwind(true);
    hookConfig_.set_callframe_compress(true);
    SerializeInfo hookInfo = {&frames, stackMapId, TEST_TAG_NAME, 0, 0, &hookConfig_};
    NativeHookData event;

    auto mmapEvent = event.mutable_mmap_event();
    hookRecord.SetEventFrame(mmapEvent, hookInfo);
    EXPECT_EQ(mmapEvent->stack_id(), stackMapId);
    EXPECT_EQ(mmapEvent->frame_info_size(), 0);

    auto munmapEvent = event.mutable_munmap_event();
    hookRecord.SetEventFrame(munmapEvent, hookInfo);
    EXPECT_EQ(munmapEvent->stack_id(), stackMapId);
    EXPECT_EQ(munmapEvent->frame_info_size(), 0);

    auto traceAllocEvent = event.mutable_trace_alloc_event();
    hookRecord.SetEventFrame(traceAllocEvent, hookInfo);
    EXPECT_EQ(traceAllocEvent->stack_id(), stackMapId);
    EXPECT_EQ(traceAllocEvent->frame_info_size(), 0);

    auto traceFreeEvent = event.mutable_trace_free_event();
    hookRecord.SetEventFrame(traceFreeEvent, hookInfo);
    EXPECT_EQ(traceFreeEvent->stack_id(), stackMapId);
    EXPECT_EQ(traceFreeEvent->frame_info_size(), 0);
}

HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameKeepsFramesWhenStackIdIsZero, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    hookConfig_.set_fp_unwind(true);
    hookConfig_.set_callframe_compress(true);
    SerializeInfo hookInfo = {&frames, 0, TEST_TAG_NAME, 0, 0, &hookConfig_};
    NativeHookData event;

    auto freeEvent = event.mutable_free_event();
    hookRecord.SetEventFrame(freeEvent, hookInfo);
    EXPECT_EQ(freeEvent->stack_id(), 0u);
    EXPECT_EQ(freeEvent->frame_info_size(), 1);

    auto mmapEvent = event.mutable_mmap_event();
    hookRecord.SetEventFrame(mmapEvent, hookInfo);
    EXPECT_EQ(mmapEvent->stack_id(), 0u);
    EXPECT_EQ(mmapEvent->frame_info_size(), 1);

    auto munmapEvent = event.mutable_munmap_event();
    hookRecord.SetEventFrame(munmapEvent, hookInfo);
    EXPECT_EQ(munmapEvent->stack_id(), 0u);
    EXPECT_EQ(munmapEvent->frame_info_size(), 1);

    auto traceAllocEvent = event.mutable_trace_alloc_event();
    hookRecord.SetEventFrame(traceAllocEvent, hookInfo);
    EXPECT_EQ(traceAllocEvent->stack_id(), 0u);
    EXPECT_EQ(traceAllocEvent->frame_info_size(), 1);

    auto traceFreeEvent = event.mutable_trace_free_event();
    hookRecord.SetEventFrame(traceFreeEvent, hookInfo);
    EXPECT_EQ(traceFreeEvent->stack_id(), 0u);
    EXPECT_EQ(traceFreeEvent->frame_info_size(), 1);
}

HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameRejectsInvalidEventKinds, TestSize.Level0)
{
    HookRecord validRecord(MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE));
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    validRecord.SetEventFrame<FreeEvent>(nullptr, hookInfo);
    validRecord.SetEventFrame<MmapEvent>(nullptr, hookInfo);
    validRecord.SetEventFrame<MunmapEvent>(nullptr, hookInfo);
    validRecord.SetEventFrame<TraceAllocEvent>(nullptr, hookInfo);
    validRecord.SetEventFrame<TraceFreeEvent>(nullptr, hookInfo);
    EXPECT_TRUE(validRecord.IsValid());

    HookRecord invalidRecord;
    NativeHookData event;
    auto freeEvent = event.mutable_free_event();
    invalidRecord.SetEventFrame(freeEvent, hookInfo);
    EXPECT_EQ(freeEvent->frame_info_size(), 0);

    auto mmapEvent = event.mutable_mmap_event();
    invalidRecord.SetEventFrame(mmapEvent, hookInfo);
    EXPECT_EQ(mmapEvent->frame_info_size(), 0);

    auto traceEvent = event.mutable_trace_alloc_event();
    invalidRecord.SetEventFrame(traceEvent, hookInfo);
    EXPECT_EQ(traceEvent->frame_info_size(), 0);
}

/*
@tc.name: HookRecordSetFrameInfoTest
@tc.desc: test HookRecord::SetFrameInfo method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetFrameInfoTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    
    // Create call frame
    std::vector<CallFrame> frames = MakeCallFrames();
    CallFrame& callFrame = frames[0];
    
    // Test with normal case
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto allocEvent = hookData->mutable_alloc_event();
    auto frameInfo = allocEvent->add_frame_info();
    hookRecord.SetFrameInfo(*frameInfo, callFrame, &hookConfig_);
    
    EXPECT_EQ(frameInfo->ip(), callFrame.ip_);
    EXPECT_EQ(frameInfo->sp(), callFrame.sp_);
    EXPECT_EQ(frameInfo->offset(), callFrame.offset_);
    EXPECT_EQ(frameInfo->symbol_offset(), callFrame.symbolOffset_);
    EXPECT_EQ(frameInfo->symbol_name_id(), callFrame.symbolNameId_);
    EXPECT_EQ(frameInfo->file_path_id(), callFrame.filePathId_);
    
    // Test with offline symbolization
    hookConfig_.set_offline_symbolization(true);
    BatchNativeHookData stackDataOffline;
    auto hookDataOffline = stackDataOffline.add_events();
    auto allocEventOffline = hookDataOffline->mutable_alloc_event();
    auto frameInfoOffline = allocEventOffline->add_frame_info();
    hookRecord.SetFrameInfo(*frameInfoOffline, callFrame, &hookConfig_);
    
    EXPECT_EQ(frameInfoOffline->ip(), callFrame.ip_);
    // With offline symbolization, symbol name and file path should not be set
    // Note: Frame type doesn't have has_symbol_name() and has_file_path() methods
}

/*
@tc.name: HookRecordSetFrameInfoUsesInlineStrings
@tc.desc: test HookRecord::SetFrameInfo writes inline strings when ids are missing.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetFrameInfoUsesInlineStrings, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    CallFrame& callFrame = frames[0];
    callFrame.symbolNameId_ = 0;
    callFrame.filePathId_ = 0;
    BatchNativeHookData stackData;
    auto frameInfo = stackData.add_events()->mutable_alloc_event()->add_frame_info();

    hookRecord.SetFrameInfo(*frameInfo, callFrame, &hookConfig_);
    EXPECT_EQ(frameInfo->symbol_name(), "test_function");
    EXPECT_EQ(frameInfo->file_path(), "/path/to/file.cpp");
}

/*
@tc.name: HookRecordSetFrameInfoHandlesGuardBranches
@tc.desc: test SetFrameInfo null config and offline non-js branches.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetFrameInfoHandlesGuardBranches, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    CallFrame& callFrame = frames[0];
    BatchNativeHookData stackData;
    auto nullConfigFrame = stackData.add_events()->mutable_alloc_event()->add_frame_info();
    hookRecord.SetFrameInfo(*nullConfigFrame, callFrame, nullptr);
    EXPECT_EQ(nullConfigFrame->ip(), 0u);

    hookConfig_.set_offline_symbolization(true);
    callFrame.isJsFrame_ = false;
    auto offlineFrame = stackData.add_events()->mutable_alloc_event()->add_frame_info();
    hookRecord.SetFrameInfo(*offlineFrame, callFrame, &hookConfig_);
    EXPECT_EQ(offlineFrame->ip(), callFrame.ip_);
    EXPECT_EQ(offlineFrame->sp(), 0u);
    EXPECT_EQ(offlineFrame->symbol_name_id(), 0u);

    callFrame.isJsFrame_ = true;
    auto jsOfflineFrame = stackData.add_events()->mutable_alloc_event()->add_frame_info();
    hookRecord.SetFrameInfo(*jsOfflineFrame, callFrame, &hookConfig_);
    EXPECT_EQ(jsOfflineFrame->sp(), callFrame.sp_);
    EXPECT_EQ(jsOfflineFrame->symbol_name_id(), callFrame.symbolNameId_);
    EXPECT_EQ(jsOfflineFrame->file_path_id(), callFrame.filePathId_);
}

HWTEST_F(HookRecordExtTest, HookRecordSetFrameInfoSkipsIncompleteOfflineJsIds, TestSize.Level0)
{
    HookRecord hookRecord(MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE));
    BatchNativeHookData stackData;
    hookConfig_.set_offline_symbolization(true);

    CallFrame missingSymbol = MakeCallFrames()[0];
    missingSymbol.isJsFrame_ = true;
    missingSymbol.symbolNameId_ = 0;
    auto symbolFrame = stackData.add_events()->mutable_alloc_event()->add_frame_info();
    hookRecord.SetFrameInfo(*symbolFrame, missingSymbol, &hookConfig_);
    EXPECT_EQ(symbolFrame->sp(), 0u);
    EXPECT_EQ(symbolFrame->file_path_id(), 0u);

    CallFrame missingPath = MakeCallFrames()[0];
    missingPath.isJsFrame_ = true;
    missingPath.filePathId_ = 0;
    auto pathFrame = stackData.add_events()->mutable_alloc_event()->add_frame_info();
    hookRecord.SetFrameInfo(*pathFrame, missingPath, &hookConfig_);
    EXPECT_EQ(pathFrame->sp(), 0u);
    EXPECT_EQ(pathFrame->symbol_name_id(), 0u);
}

HWTEST_F(HookRecordExtTest, HookRecordSetFrameInfoUsesNamesWhenIdsAbsent, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    CallFrame callFrame(1);
    callFrame.ip_ = 0x2000;
    callFrame.sp_ = 0x3000;
    callFrame.symbolName_ = "NoIdSymbol";
    callFrame.filePath_ = "/system/lib/libnoid.so";
    callFrame.symbolNameId_ = 0;
    callFrame.filePathId_ = 0;
    BatchNativeHookData stackData;
    auto frameInfo = stackData.add_events()->mutable_alloc_event()->add_frame_info();

    hookRecord.SetFrameInfo(*frameInfo, callFrame, &hookConfig_);
    EXPECT_EQ(frameInfo->symbol_name(), callFrame.symbolName_);
    EXPECT_EQ(frameInfo->file_path(), callFrame.filePath_);
    EXPECT_EQ(frameInfo->symbol_name_id(), 0u);
    EXPECT_EQ(frameInfo->file_path_id(), 0u);
}

/*
@tc.name: HookRecordSetSizeAndTraceTypeHandleNull
@tc.desc: test SetSize and SetTraceType null guards keep valid state unchanged.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetSizeAndTraceTypeHandleNull, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(TEST_TYPE, TEST_ADDR, TEST_SIZE);
    HookRecord hookRecord(rawStack);
    hookRecord.SetSize<AllocEvent>(nullptr);
    HookRecord::SetTraceType<TraceAllocEvent>(nullptr, TAG_RES_GPU_VK);
    EXPECT_EQ(hookRecord.GetAddr(), TEST_ADDR);

    NativeHookData event;
    auto allocEvent = event.mutable_alloc_event();
    hookRecord.SetSize(allocEvent);
    EXPECT_EQ(allocEvent->size(), TEST_SIZE);
}

/*
@tc.name: HookRecordSetTraceTypeTest
@tc.desc: test HookRecord::SetTraceType method with different tag names.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetTraceTypeTest, TestSize.Level0)
{
    // Test with GPU VK tag
    NativeHookData event1;
    auto traceEvent1 = event1.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent1, TAG_RES_GPU_VK);
    EXPECT_EQ(traceEvent1->trace_type(), TraceType::GPU_VK);
    
    // Test with GPU GLES tag
    NativeHookData event2;
    auto traceEvent2 = event2.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent2, TAG_RES_GPU_GLES_IMAGE);
    EXPECT_EQ(traceEvent2->trace_type(), TraceType::GPU_GLES);
    
    // Test with FD tag
    NativeHookData event3;
    auto traceEvent3 = event3.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent3, TAG_RES_FD_OPEN);
    EXPECT_EQ(traceEvent3->trace_type(), TraceType::FD);
    
    // Test with THREAD tag
    NativeHookData event4;
    auto traceEvent4 = event4.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent4, TAG_RES_THREAD_PTHREAD);
    EXPECT_EQ(traceEvent4->trace_type(), TraceType::THREAD);
    
    // Test with SO tag
    NativeHookData event5;
    auto traceEvent5 = event5.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent5, "libtest.so");
    EXPECT_EQ(traceEvent5->trace_type(), TraceType::SO);
    
    // Test with OTHER tag
    NativeHookData event6;
    auto traceEvent6 = event6.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent6, "unknown_tag");
    EXPECT_EQ(traceEvent6->trace_type(), TraceType::OTHER);
}

/*
@tc.name: HookRecordSetTraceTypeCoversRemainingTags
@tc.desc: test HookRecord::SetTraceType with remaining direct tag mappings.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetTraceTypeCoversRemainingTags, TestSize.Level0)
{
    NativeHookData event;
    auto traceEvent = event.mutable_trace_alloc_event();
    HookRecord::SetTraceType(traceEvent, TAG_RES_GPU_CL_BUFFER);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::GPU_CL);
    HookRecord::SetTraceType(traceEvent, TAG_RES_GPU_GLES_BUFFER);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::GPU_GLES);
    HookRecord::SetTraceType(traceEvent, TAG_RES_GPU_CL_IMAGE);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::GPU_CL);
    HookRecord::SetTraceType(traceEvent, TAG_RES_FD_EPOLL);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::FD);
    HookRecord::SetTraceType(traceEvent, TAG_RES_FD_EVENTFD);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::FD);
    HookRecord::SetTraceType(traceEvent, TAG_RES_FD_SOCKET);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::FD);
    HookRecord::SetTraceType(traceEvent, TAG_RES_FD_PIPE);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::FD);
    HookRecord::SetTraceType(traceEvent, TAG_RES_FD_DUP);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::FD);
    HookRecord::SetTraceType(traceEvent, TAG_RES_THREAD_ALL);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::THREAD);
    HookRecord::SetTraceType(traceEvent, TAG_RES_KOTLIN_HEAP_MASK);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::KMP_HEAP);
    HookRecord::SetTraceType(traceEvent, TAG_RES_FD_ALL);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::FD);
    HookRecord::SetTraceType(traceEvent, "ashmem");
    EXPECT_EQ(traceEvent->trace_type(), TraceType::ASHMEM);
    HookRecord::SetTraceType(traceEvent, TAG_RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::ARKTS_HEAP);
    HookRecord::SetTraceType(traceEvent, TAG_RES_ARKTS_STATIC_HEAP);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::ARKTS_STATIC_HEAP);
    HookRecord::SetTraceType(traceEvent, TAG_RES_JS_HEAP_MASK);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::JS_HEAP);
    HookRecord::SetTraceType(traceEvent, TAG_RES_DMABUF_MASK);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::DMA);
    HookRecord::SetTraceType(traceEvent, TAG_RES_ARK_GLOBAL_HANDLE);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::ARK_GLOBAL_HANDLE);
    HookRecord::SetTraceType(traceEvent, TAG_RES_ARK_LOCAL_HANDLE);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::ARK_LOCAL_HANDLE);
    HookRecord::SetTraceType(traceEvent, TAG_RES_VMA_ARKWEB);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::VMA_ARKWEB);
    HookRecord::SetTraceType(traceEvent, TAG_RES_DART_HEAP);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::DART_HEAP);
    HookRecord::SetTraceType(traceEvent, TAG_RES_RN_HERMES_HEAP);
    EXPECT_EQ(traceEvent->trace_type(), TraceType::RN_HERMES_HEAP);
}

/*
@tc.name: HookRecordSerializeDataWithEncoderVariant
@tc.desc: test serialize paths with proto encoder event variant.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSerializeDataWithEncoderVariant, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE);
    std::vector<CallFrame> frames = MakeCallFrames();
    hookConfig_.set_fp_unwind(true);
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    OHOS::Developtools::Profiler::ProtoEncoder::NativeHookData encoderEvent;
    NativeHookProto stackData = &encoderEvent;

    MallocRecord(rawStack).SerializeData(stackData, hookInfo);
    MmapRecord(rawStack).SerializeData(stackData, hookInfo);
    MmapFilePageRecord(rawStack).SerializeData(stackData, hookInfo);
    MemoryUsingRecord(rawStack).SerializeData(stackData, hookInfo);
    MemoryUnusingRecord(rawStack).SerializeData(stackData, hookInfo);
    ArktsFreeRecord(rawStack).SerializeData(stackData, hookInfo);
    rawStack->stackContext->newAddr = reinterpret_cast<void*>(TEST_ADDR + TEST_SIZE);
    ArktsMallocRecord(rawStack).SerializeData(stackData, hookInfo);

    EXPECT_LT(encoderEvent.Finish(), 0);
}

HWTEST_F(HookRecordExtTest, EncoderVariantSerializesCompressedStackIds, TestSize.Level0)
{
    hookConfig_.set_callframe_compress(true);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    hookInfo.stackMapId = 77;

    ExpectEncoderSerializeSucceeds(MallocRecord(MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(FreeRecord(MakeRawStack(FREE_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MmapRecord(MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MmapFilePageRecord(MakeRawStack(MMAP_FILE_PAGE_MSG, TEST_ADDR, TEST_SIZE)),
        hookInfo);
    ExpectEncoderSerializeSucceeds(MunmapRecord(MakeRawStack(MUNMAP_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MemoryUnusingRecord(MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE)),
        hookInfo);
}

HWTEST_F(HookRecordExtTest, EncoderVariantSerializesOfflineJsFramesAndTraceTypes, TestSize.Level0)
{
    hookConfig_.set_fp_unwind(true);
    hookConfig_.set_offline_symbolization(true);
    std::vector<CallFrame> frames = MakeJsCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    hookInfo.tagName = "libencoder_trace.so";

    ExpectEncoderSerializeSucceeds(MallocRecord(MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MmapRecord(MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)), hookInfo);
    ExpectEncoderSerializeSucceeds(MemoryUnusingRecord(MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE)),
        hookInfo);
    ExpectEncoderSerializeSucceeds(ArktsFreeRecord(MakeRawStack(FREE_ARKTS, TEST_ADDR, TEST_SIZE)), hookInfo);

    auto rawStack = MakeRawStack(MALLOC_ARKTS, TEST_ADDR, TEST_SIZE);
    rawStack->stackContext->newAddr = reinterpret_cast<void*>(TEST_ADDR + TEST_SIZE);
    ExpectEncoderSerializeSucceeds(ArktsMallocRecord(rawStack), hookInfo);
}

/*
@tc.name: FreeRecordSerializeDataTest
@tc.desc: test FreeRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, FreeRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(FREE_MSG, TEST_ADDR, TEST_SIZE);
    FreeRecord freeRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    freeRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_free_event());
    EXPECT_EQ(event.free_event().pid(), TEST_PID);
    EXPECT_EQ(event.free_event().tid(), TEST_TID);
    EXPECT_EQ(event.free_event().addr(), TEST_ADDR);
}

/*
@tc.name: MallocRecordSerializeDataTest
@tc.desc: test MallocRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MallocRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE);
    MallocRecord mallocRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    mallocRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_alloc_event());
    EXPECT_EQ(event.alloc_event().pid(), TEST_PID);
    EXPECT_EQ(event.alloc_event().tid(), TEST_TID);
    EXPECT_EQ(event.alloc_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.alloc_event().size(), TEST_SIZE);
    EXPECT_EQ(event.alloc_event().node_type(), TEST_NODE_TYPE);
    EXPECT_EQ(event.alloc_event().node_id(), TEST_NODE_ID);
}

/*
@tc.name: MmapRecordSerializeDataTest
@tc.desc: test MmapRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MmapRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE);
    MmapRecord mmapRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    mmapRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_mmap_event());
    EXPECT_EQ(event.mmap_event().pid(), TEST_PID);
    EXPECT_EQ(event.mmap_event().tid(), TEST_TID);
    EXPECT_EQ(event.mmap_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.mmap_event().size(), TEST_SIZE);
    EXPECT_EQ(event.mmap_event().type(), TEST_TAG_NAME);
    EXPECT_EQ(event.mmap_event().node_type(), TEST_NODE_TYPE);
    EXPECT_EQ(event.mmap_event().node_id(), TEST_NODE_ID);
}

/*
@tc.name: MmapRecordSerializeDataWithoutOptionalFields
@tc.desc: test MmapRecord::SerializeData leaves optional fields empty when context is absent.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MmapRecordSerializeDataWithoutOptionalFields, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE);
    MmapRecord mmapRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = {&frames, 0, "", 0, 0, &hookConfig_};
    NativeHookData event;
    NativeHookProto stackData = &event;

    mmapRecord.SerializeData(stackData, hookInfo);
    EXPECT_EQ(event.mmap_event().type(), "");
    EXPECT_EQ(event.mmap_event().node_type(), 0u);
    EXPECT_EQ(event.mmap_event().node_id(), 0u);
}

/*
@tc.name: MallocAndMmapFilePageSkipOptionalFields
@tc.desc: test serialize methods leave node and type fields unset when context is absent.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MallocAndMmapFilePageSkipOptionalFields, TestSize.Level0)
{
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = {&frames, 0, "", 0, 0, &hookConfig_};

    NativeHookData mallocEvent;
    NativeHookProto mallocData = &mallocEvent;
    MallocRecord(MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(mallocData, hookInfo);
    EXPECT_EQ(mallocEvent.alloc_event().node_type(), 0u);
    EXPECT_EQ(mallocEvent.alloc_event().node_id(), 0u);

    NativeHookData mmapEvent;
    NativeHookProto mmapData = &mmapEvent;
    MmapFilePageRecord(MakeRawStack(MMAP_FILE_PAGE_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(mmapData, hookInfo);
    EXPECT_EQ(mmapEvent.mmap_event().type(), "");
    EXPECT_EQ(mmapEvent.mmap_event().node_type(), 0u);
    EXPECT_EQ(mmapEvent.mmap_event().node_id(), 0u);
}

HWTEST_F(HookRecordExtTest, SerializeDataUsesNodeIdWithoutNodeType, TestSize.Level0)
{
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = {&frames, 0, TEST_TAG_NAME, 0, TEST_NODE_ID, &hookConfig_};

    NativeHookData mallocEvent;
    NativeHookProto mallocData = &mallocEvent;
    MallocRecord(MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(mallocData, hookInfo);
    EXPECT_EQ(mallocEvent.alloc_event().node_type(), 0u);
    EXPECT_EQ(mallocEvent.alloc_event().node_id(), TEST_NODE_ID);

    NativeHookData mmapEvent;
    NativeHookProto mmapData = &mmapEvent;
    MmapRecord(MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(mmapData, hookInfo);
    EXPECT_EQ(mmapEvent.mmap_event().node_type(), 0u);
    EXPECT_EQ(mmapEvent.mmap_event().node_id(), TEST_NODE_ID);

    NativeHookData mmapFileEvent;
    NativeHookProto mmapFileData = &mmapFileEvent;
    MmapFilePageRecord(MakeRawStack(MMAP_FILE_PAGE_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(mmapFileData, hookInfo);
    EXPECT_EQ(mmapFileEvent.mmap_event().node_type(), 0u);
    EXPECT_EQ(mmapFileEvent.mmap_event().node_id(), TEST_NODE_ID);

    NativeHookData traceEvent;
    NativeHookProto traceData = &traceEvent;
    MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(traceData, hookInfo);
    EXPECT_EQ(traceEvent.trace_alloc_event().node_type(), 0u);
    EXPECT_EQ(traceEvent.trace_alloc_event().node_id(), TEST_NODE_ID);
}

/*
@tc.name: MemoryTraceRecordsSkipOptionalFields
@tc.desc: test trace alloc and free records skip optional tag and node fields.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MemoryTraceRecordsSkipOptionalFields, TestSize.Level0)
{
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = {&frames, 0, "", 0, 0, &hookConfig_};

    NativeHookData allocEvent;
    NativeHookProto allocData = &allocEvent;
    MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(allocData, hookInfo);
    EXPECT_EQ(allocEvent.trace_alloc_event().tag_name(), "");
    EXPECT_EQ(allocEvent.trace_alloc_event().node_type(), 0u);
    EXPECT_EQ(allocEvent.trace_alloc_event().node_id(), 0u);
    EXPECT_EQ(allocEvent.trace_alloc_event().trace_type(), TraceType::OTHER);

    NativeHookData freeEvent;
    NativeHookProto freeData = &freeEvent;
    MemoryUnusingRecord(MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(freeData, hookInfo);
    EXPECT_EQ(freeEvent.trace_free_event().tag_name(), "");
    EXPECT_EQ(freeEvent.trace_free_event().trace_type(), TraceType::OTHER);
}

/*
@tc.name: MemoryUsingRecordSerializeDataWithSoTag
@tc.desc: test MemoryUsingRecord::SerializeData maps shared object tags to SO trace type.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MemoryUsingRecordSerializeDataWithSoTag, TestSize.Level0)
{
    RawStackPtr rawStack = MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE);
    MemoryUsingRecord memoryUsingRecord(rawStack);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = {&frames, 0, "libsample.so", 0, 0, &hookConfig_};
    NativeHookData event;
    NativeHookProto stackData = &event;

    memoryUsingRecord.SerializeData(stackData, hookInfo);
    EXPECT_EQ(event.trace_alloc_event().trace_type(), TraceType::SO);
}

/*
@tc.name: MmapFilePageRecordSerializeDataTest
@tc.desc: test MmapFilePageRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MmapFilePageRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MMAP_FILE_PAGE_MSG, TEST_ADDR, TEST_SIZE);
    MmapFilePageRecord mmapFilePageRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    mmapFilePageRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_mmap_event());
    EXPECT_EQ(event.mmap_event().pid(), TEST_PID);
    EXPECT_EQ(event.mmap_event().tid(), TEST_TID);
    EXPECT_EQ(event.mmap_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.mmap_event().size(), TEST_SIZE);
    EXPECT_EQ(event.mmap_event().type(), MMAP_FILE_PAGE_PREFIX + TEST_TAG_NAME);
    EXPECT_EQ(event.mmap_event().node_type(), TEST_NODE_TYPE);
    EXPECT_EQ(event.mmap_event().node_id(), TEST_NODE_ID);
}

/*
@tc.name: MunmapRecordSerializeDataTest
@tc.desc: test MunmapRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MunmapRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MUNMAP_MSG, TEST_ADDR, TEST_SIZE);
    MunmapRecord munmapRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    munmapRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_munmap_event());
    EXPECT_EQ(event.munmap_event().pid(), TEST_PID);
    EXPECT_EQ(event.munmap_event().tid(), TEST_TID);
    EXPECT_EQ(event.munmap_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.munmap_event().size(), TEST_SIZE);
}

/*
@tc.name: PrSetVmaRecordSerializeDataTest
@tc.desc: test PrSetVmaRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, PrSetVmaRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(PR_SET_VMA_MSG, TEST_ADDR, TEST_SIZE);
    const char* vmaTag = "test_vma_tag";
    rawStack->data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(vmaTag));
    
    PrSetVmaRecord prSetVmaRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    prSetVmaRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_tag_event());
    EXPECT_EQ(event.tag_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.tag_event().size(), TEST_SIZE);
    EXPECT_EQ(event.tag_event().pid(), TEST_PID);
    EXPECT_EQ(event.tag_event().tag(), PR_SET_VMA_PREFIX + vmaTag);
}

/*
@tc.name: MemoryUsingRecordSerializeDataTest
@tc.desc: test MemoryUsingRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MemoryUsingRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE);
    MemoryUsingRecord memoryUsingRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    memoryUsingRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_trace_alloc_event());
    EXPECT_EQ(event.trace_alloc_event().pid(), TEST_PID);
    EXPECT_EQ(event.trace_alloc_event().tid(), TEST_TID);
    EXPECT_EQ(event.trace_alloc_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.trace_alloc_event().size(), TEST_SIZE);
    EXPECT_EQ(event.trace_alloc_event().tag_name(), TEST_TAG_NAME);
    EXPECT_EQ(event.trace_alloc_event().node_type(), TEST_NODE_TYPE);
    EXPECT_EQ(event.trace_alloc_event().node_id(), TEST_NODE_ID);
}

/*
@tc.name: MemoryUnusingRecordSerializeDataTest
@tc.desc: test MemoryUnusingRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MemoryUnusingRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE);
    MemoryUnusingRecord memoryUnusingRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    memoryUnusingRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_trace_free_event());
    EXPECT_EQ(event.trace_free_event().pid(), TEST_PID);
    EXPECT_EQ(event.trace_free_event().tid(), TEST_TID);
    EXPECT_EQ(event.trace_free_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.trace_free_event().tag_name(), TEST_TAG_NAME);
}

/*
@tc.name: ArktsFreeRecordSerializeDataTest
@tc.desc: test ArktsFreeRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, ArktsFreeRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(FREE_ARKTS, TEST_ADDR, TEST_SIZE);
    ArktsFreeRecord arktsFreeRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    arktsFreeRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_trace_free_region_event());
    EXPECT_EQ(event.trace_free_region_event().pid(), TEST_PID);
    EXPECT_EQ(event.trace_free_region_event().tid(), TEST_TID);
    EXPECT_EQ(event.trace_free_region_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.trace_free_region_event().size(), TEST_SIZE);
}

/*
@tc.name: ArktsMallocRecordSerializeDataTest
@tc.desc: test ArktsMallocRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, ArktsMallocRecordSerializeDataTest, TestSize.Level0)
{
    // Create test data
    RawStackPtr rawStack = MakeRawStack(MALLOC_ARKTS, TEST_ADDR, TEST_SIZE);
    rawStack->stackContext->newAddr = reinterpret_cast<void*>(TEST_ADDR + TEST_SIZE);
    ArktsMallocRecord arktsMallocRecord(rawStack);
    
    // Create serialize info
    SerializeInfo hookInfo;
    std::vector<CallFrame> frames = MakeCallFrames();
    hookInfo.callFrames = &frames;
    hookInfo.stackMapId = 0;
    hookInfo.tagName = TEST_TAG_NAME;
    hookInfo.nodeType = TEST_NODE_TYPE;
    hookInfo.nodeId = TEST_NODE_ID;
    hookInfo.config = &hookConfig_;
    
    // Test with NativeHookData
    NativeHookData event;
    NativeHookProto stackData = &event;
    arktsMallocRecord.SerializeData(stackData, hookInfo);
    
    EXPECT_TRUE(event.has_trace_move_event());
    EXPECT_EQ(event.trace_move_event().pid(), TEST_PID);
    EXPECT_EQ(event.trace_move_event().tid(), TEST_TID);
    EXPECT_EQ(event.trace_move_event().addr(), TEST_ADDR);
    EXPECT_EQ(event.trace_move_event().new_addr(), TEST_ADDR + TEST_SIZE);
    EXPECT_EQ(event.trace_move_event().size(), TEST_SIZE);
}

HWTEST_F(HookRecordExtTest, TraceTypeSerializesKnownAndSharedObjectTags, TestSize.Level0)
{
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    hookInfo.nodeType = 0;
    hookInfo.nodeId = 0;

    hookInfo.tagName = TAG_RES_GPU_VK;
    NativeHookData gpuEvent;
    MemoryUsingRecord memoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE));
    memoryUsingRecord.SerializeData(&gpuEvent, hookInfo);
    ASSERT_TRUE(gpuEvent.has_trace_alloc_event());
    EXPECT_EQ(gpuEvent.trace_alloc_event().trace_type(), TraceType::GPU_VK);

    hookInfo.tagName = "libtrace_type.so";
    NativeHookData soEvent;
    MemoryUnusingRecord memoryUnusingRecord(MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE));
    memoryUnusingRecord.SerializeData(&soEvent, hookInfo);
    ASSERT_TRUE(soEvent.has_trace_free_event());
    EXPECT_EQ(soEvent.trace_free_event().trace_type(), TraceType::SO);

    hookInfo.tagName = TAG_RES_FD_SOCKET;
    NativeHookData fdEvent;
    ArktsFreeRecord arktsFreeRecord(MakeRawStack(FREE_ARKTS, TEST_ADDR, TEST_SIZE));
    arktsFreeRecord.SerializeData(&fdEvent, hookInfo);
    ASSERT_TRUE(fdEvent.has_trace_free_region_event());
    EXPECT_EQ(fdEvent.trace_free_region_event().trace_type(), TraceType::FD);
}

HWTEST_F(HookRecordExtTest, SerializeDataUsesCompressedStackIds, TestSize.Level0)
{
    hookConfig_.set_callframe_compress(true);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    hookInfo.stackMapId = 88;

    NativeHookData mallocEvent;
    MallocRecord mallocRecord(MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE));
    mallocRecord.SerializeData(&mallocEvent, hookInfo);
    ASSERT_TRUE(mallocEvent.has_alloc_event());
    EXPECT_EQ(mallocEvent.alloc_event().stack_id(), hookInfo.stackMapId);
    EXPECT_EQ(mallocEvent.alloc_event().frame_info_size(), 0);

    NativeHookData freeEvent;
    FreeRecord freeRecord(MakeRawStack(FREE_MSG, TEST_ADDR, TEST_SIZE));
    freeRecord.SerializeData(&freeEvent, hookInfo);
    ASSERT_TRUE(freeEvent.has_free_event());
    EXPECT_EQ(freeEvent.free_event().stack_id(), hookInfo.stackMapId);
    EXPECT_EQ(freeEvent.free_event().frame_info_size(), 0);
}

HWTEST_F(HookRecordExtTest, SerializeDataUsesCompressedStackIdsForMappedRecords, TestSize.Level0)
{
    hookConfig_.set_callframe_compress(true);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);
    hookInfo.stackMapId = 89;

    NativeHookData mmapEvent;
    MmapRecord(MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&mmapEvent, hookInfo);
    ASSERT_TRUE(mmapEvent.has_mmap_event());
    EXPECT_EQ(mmapEvent.mmap_event().stack_id(), hookInfo.stackMapId);

    NativeHookData mmapFileEvent;
    MmapFilePageRecord(MakeRawStack(MMAP_FILE_PAGE_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&mmapFileEvent, hookInfo);
    ASSERT_TRUE(mmapFileEvent.has_mmap_event());
    EXPECT_EQ(mmapFileEvent.mmap_event().stack_id(), hookInfo.stackMapId);

    NativeHookData munmapEvent;
    MunmapRecord(MakeRawStack(MUNMAP_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&munmapEvent, hookInfo);
    ASSERT_TRUE(munmapEvent.has_munmap_event());
    EXPECT_EQ(munmapEvent.munmap_event().stack_id(), hookInfo.stackMapId);

    NativeHookData traceAllocEvent;
    MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&traceAllocEvent, hookInfo);
    ASSERT_TRUE(traceAllocEvent.has_trace_alloc_event());
    EXPECT_EQ(traceAllocEvent.trace_alloc_event().stack_id(), hookInfo.stackMapId);

    NativeHookData traceFreeEvent;
    MemoryUnusingRecord(MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&traceFreeEvent, hookInfo);
    ASSERT_TRUE(traceFreeEvent.has_trace_free_event());
    EXPECT_EQ(traceFreeEvent.trace_free_event().stack_id(), hookInfo.stackMapId);
}

HWTEST_F(HookRecordExtTest, SerializeDataReportsFramesForMappedRecords, TestSize.Level0)
{
    hookConfig_.set_fp_unwind(true);
    std::vector<CallFrame> frames = MakeCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);

    NativeHookData freeEvent;
    FreeRecord(MakeRawStack(FREE_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&freeEvent, hookInfo);
    EXPECT_EQ(freeEvent.free_event().frame_info_size(), 1);

    NativeHookData mmapEvent;
    MmapRecord(MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&mmapEvent, hookInfo);
    EXPECT_EQ(mmapEvent.mmap_event().frame_info_size(), 1);

    NativeHookData mmapFileEvent;
    MmapFilePageRecord(MakeRawStack(MMAP_FILE_PAGE_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&mmapFileEvent, hookInfo);
    EXPECT_EQ(mmapFileEvent.mmap_event().frame_info_size(), 1);

    NativeHookData munmapEvent;
    MunmapRecord(MakeRawStack(MUNMAP_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&munmapEvent, hookInfo);
    EXPECT_EQ(munmapEvent.munmap_event().frame_info_size(), 1);

    NativeHookData traceAllocEvent;
    MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&traceAllocEvent, hookInfo);
    EXPECT_EQ(traceAllocEvent.trace_alloc_event().frame_info_size(), 1);

    NativeHookData traceFreeEvent;
    MemoryUnusingRecord(MakeRawStack(MEMORY_UNUSING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&traceFreeEvent, hookInfo);
    EXPECT_EQ(traceFreeEvent.trace_free_event().frame_info_size(), 1);
}

HWTEST_F(HookRecordExtTest, SerializeDataReportsOfflineJsFrameIds, TestSize.Level0)
{
    hookConfig_.set_fp_unwind(true);
    hookConfig_.set_offline_symbolization(true);
    std::vector<CallFrame> frames = MakeJsCallFrames();
    SerializeInfo hookInfo = MakeSerializeInfo(frames, hookConfig_);

    NativeHookData allocEvent;
    MallocRecord(MakeRawStack(MALLOC_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&allocEvent, hookInfo);
    ASSERT_EQ(allocEvent.alloc_event().frame_info_size(), 1);
    EXPECT_EQ(allocEvent.alloc_event().frame_info(0).symbol_name_id(), frames[0].symbolNameId_);

    NativeHookData mmapEvent;
    MmapRecord(MakeRawStack(MMAP_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&mmapEvent, hookInfo);
    ASSERT_EQ(mmapEvent.mmap_event().frame_info_size(), 1);
    EXPECT_EQ(mmapEvent.mmap_event().frame_info(0).file_path_id(), frames[0].filePathId_);

    NativeHookData traceEvent;
    MemoryUsingRecord(MakeRawStack(MEMORY_USING_MSG, TEST_ADDR, TEST_SIZE)).SerializeData(&traceEvent, hookInfo);
    ASSERT_EQ(traceEvent.trace_alloc_event().frame_info_size(), 1);
    EXPECT_EQ(traceEvent.trace_alloc_event().frame_info(0).sp(), frames[0].sp_);
}

/*
@tc.name: RawStackResetTest
@tc.desc: test RawStack::Reset method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, RawStackResetTest, TestSize.Level0)
{
    // Create and initialize raw stack
    RawStack rawStack;
    rawStack.stackContext = new BaseStackRawData();
    rawStack.stackContext->type = TEST_TYPE;
    rawStack.stackContext->addr = reinterpret_cast<void*>(TEST_ADDR);
    rawStack.stackData = nullptr;
    rawStack.data = nullptr;
    rawStack.stackSize = 1;
    rawStack.fpDepth = 1;
    rawStack.reportFlag = true;
    rawStack.reduceStackFlag = true;
    
    // Reset raw stack
    rawStack.Reset();
    
    // Verify all fields are reset
    EXPECT_EQ(rawStack.stackContext, nullptr);
    EXPECT_EQ(rawStack.stackData, nullptr);
    EXPECT_EQ(rawStack.data, nullptr);
    EXPECT_EQ(rawStack.stackSize, 0);
    EXPECT_EQ(rawStack.fpDepth, 0);
    EXPECT_FALSE(rawStack.reportFlag);
    EXPECT_FALSE(rawStack.reduceStackFlag);
}

HWTEST_F(HookRecordExtTest, HookCommonNormalizesUniqueStackTableSize, TestSize.Level0)
{
    EXPECT_EQ(NormalizeUniqueStackTableSize(0), DEFAULT_UNIQUE_STACK_TABLE_SIZE);
    EXPECT_EQ(NormalizeUniqueStackTableSize(DEFAULT_UNIQUE_STACK_TABLE_SIZE - 1), DEFAULT_UNIQUE_STACK_TABLE_SIZE);
    EXPECT_EQ(NormalizeUniqueStackTableSize(DEFAULT_UNIQUE_STACK_TABLE_SIZE + ONE_MEGABYTE),
        DEFAULT_UNIQUE_STACK_TABLE_SIZE + ONE_MEGABYTE);
    EXPECT_EQ(NormalizeUniqueStackTableSize(MAX_UNIQUE_STACK_TABLE_SIZE + ONE_MEGABYTE),
        MAX_UNIQUE_STACK_TABLE_SIZE);
}

HWTEST_F(HookRecordExtTest, HookCommonCopiesClientConfigStringsWithBounds, TestSize.Level0)
{
    char dest[4] = {};

    EXPECT_FALSE(CopyClientConfigString(nullptr, sizeof(dest), "a"));
    EXPECT_FALSE(CopyClientConfigString(dest, 0, "a"));
    EXPECT_FALSE(CopyClientConfigString(dest, sizeof(dest), "abcd"));
    EXPECT_TRUE(CopyClientConfigString(dest, sizeof(dest), ""));
    EXPECT_STREQ(dest, "");
    EXPECT_TRUE(CopyClientConfigString(dest, sizeof(dest), "abc"));
    EXPECT_STREQ(dest, "abc");
}

HWTEST_F(HookRecordExtTest, HookCommonConvertsClientConfigTextWithBounds, TestSize.Level0)
{
    ClientConfig config;
    ClientConfigIpc ipc = {};
    config.targetSoName = "libnative.so";
    config.filterLibList = {"liba.so", ""};

    EXPECT_TRUE(ConvertClientConfigToIpc(config, ipc));
    EXPECT_STREQ(ipc.targetSoName, "libnative.so");
    EXPECT_EQ(ipc.filterLibCount, 2u);

    ClientConfig tooLongTarget;
    tooLongTarget.targetSoName.assign(CLIENT_CONFIG_TARGET_SO_NAME_SIZE, 'x');
    EXPECT_FALSE(ConvertClientConfigToIpc(tooLongTarget, ipc));

    ClientConfig tooManyFilters;
    tooManyFilters.filterLibList.resize(CLIENT_CONFIG_FILTER_LIB_COUNT + 1, "liba.so");
    EXPECT_FALSE(ConvertClientConfigToIpc(tooManyFilters, ipc));

    ClientConfig tooLongFilter;
    tooLongFilter.filterLibList.push_back(std::string(CLIENT_CONFIG_FILTER_LIB_NAME_SIZE, 'y'));
    EXPECT_FALSE(ConvertClientConfigToIpc(tooLongFilter, ipc));
}
} // namespace
