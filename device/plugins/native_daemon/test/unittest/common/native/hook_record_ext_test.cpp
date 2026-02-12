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

static RawStackPtr MakeRawStack(uint16_t type, uint64_t addr, uint32_t size) {
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

static std::vector<CallFrame> MakeCallFrames() {
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

class HookRecordExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    NativeHookConfig hookConfig_;
};

void HookRecordExtTest::SetUpTestCase(void) {
}

void HookRecordExtTest::TearDownTestCase(void) {
}

void HookRecordExtTest::SetUp() {
    hookConfig_.set_fp_unwind(false);
    hookConfig_.set_callframe_compress(false);
    hookConfig_.set_offline_symbolization(false);
}

void HookRecordExtTest::TearDown() {
}

/*
@tc.name: HookRecordBasicTest
@tc.desc: test basic methods of HookRecord class.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordBasicTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, HookRecordInvalidTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, FreeRecordSimpTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, HookRecordSetEventFrameTest, TestSize.Level0) {
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
@tc.name: HookRecordSetFrameInfoTest
@tc.desc: test HookRecord::SetFrameInfo method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetFrameInfoTest, TestSize.Level0) {
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
@tc.name: HookRecordSetTraceTypeTest
@tc.desc: test HookRecord::SetTraceType method with different tag names.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, HookRecordSetTraceTypeTest, TestSize.Level0) {
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
@tc.name: FreeRecordSerializeDataTest
@tc.desc: test FreeRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, FreeRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, MallocRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, MmapRecordSerializeDataTest, TestSize.Level0) {
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
@tc.name: MmapFilePageRecordSerializeDataTest
@tc.desc: test MmapFilePageRecord::SerializeData method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, MmapFilePageRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, MunmapRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, PrSetVmaRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, MemoryUsingRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, MemoryUnusingRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, ArktsFreeRecordSerializeDataTest, TestSize.Level0) {
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
HWTEST_F(HookRecordExtTest, ArktsMallocRecordSerializeDataTest, TestSize.Level0) {
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

/*
@tc.name: RawStackResetTest
@tc.desc: test RawStack::Reset method.
@tc.type: FUNC
*/
HWTEST_F(HookRecordExtTest, RawStackResetTest, TestSize.Level0) {
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
} // namespace
