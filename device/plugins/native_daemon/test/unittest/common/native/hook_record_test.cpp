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

#include <gtest/gtest.h>
#include "native_hook_config.pb.h"
#include "common_types.pb.h"
#include "trace_file_reader.h"
#include "trace_file_header.h"
#include "native_hook_result_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "google/protobuf/text_format.h"
#include "hook_record_test.h"
#include "hook_record.h"
#include <string>
#include <sys/time.h>
#include <vector>
#include <sys/file.h>
#include <unistd.h>

using namespace testing::ext;
using namespace std;

using namespace OHOS::Developtools::NativeDaemon;

namespace {
BaseStackRawData baseData;
NativeHookConfig g_hookConfig;
std::shared_ptr<HookRecordFactory> g_factory = nullptr;
class HookRecordTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/*
 * @tc.name: test SetEventFrame
 * @tc.desc: test HookRecord::SetEventFrame with alloc message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSetEventFrame001, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_callframe_compress(true);
    baseData.pid = 12;
    baseData.tid = 11;
    baseData.type = MALLOC_MSG;
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x10);
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<MallocRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), MALLOC_MSG);
    std::vector<CallFrame> callFrames = {};
    std::vector<CallFrame>* callFramesptr = &callFrames;
    SerializeInfo hookInfo = {callFramesptr, 2, "malloc", &g_hookConfig};
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto allocEvent = hookData->mutable_alloc_event();
    hookRecord->SetEventFrame(allocEvent, hookInfo);
    EXPECT_EQ(allocEvent->pid(), 12);
    EXPECT_EQ(allocEvent->tid(), 11);
    EXPECT_EQ(allocEvent->addr(), 0x10);
    EXPECT_EQ(allocEvent->stack_id(), 2);
}

/*
 * @tc.name: test SetEventFrame
 * @tc.desc: test HookRecord::SetEventFrame with free message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSetEventFrame002, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_callframe_compress(true);
    baseData.pid = 20;
    baseData.tid = 21;
    baseData.type = FREE_MSG;
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x20);
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<FreeRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), FREE_MSG);
    std::vector<CallFrame> callFrames = {};
    std::vector<CallFrame>* callFramesptr = &callFrames;
    SerializeInfo hookInfo = {callFramesptr, 2, "free", &g_hookConfig};
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto freeEvent = hookData->mutable_free_event();
    hookRecord->SetEventFrame(freeEvent, hookInfo);
    EXPECT_EQ(freeEvent->pid(), 20);
    EXPECT_EQ(freeEvent->tid(), 21);
    EXPECT_EQ(freeEvent->addr(), 0x20);
    EXPECT_EQ(freeEvent->stack_id(), 2);
}

/*
 * @tc.name: test SerializeData
 * @tc.desc: test HookRecord::SerializeData with memory using message.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSerializeData, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_callframe_compress(true);
    baseData.pid = 20;
    baseData.tid = 21;
    baseData.type = MEMORY_USING_MSG;
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x20);
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<MemoryUsingRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), MEMORY_USING_MSG);
    std::vector<CallFrame> callFrames = {};
    std::vector<CallFrame>* callFramesptr = &callFrames;
    SerializeInfo hookInfo = {callFramesptr, 3, "memory_using", &g_hookConfig};
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    hookRecord->SerializeData(hookData, hookInfo);
    EXPECT_EQ(hookData->trace_alloc_event().tag_name(), "memory_using");
}

/*
 * @tc.name: test SetFrameInfo
 * @tc.desc: test HookRecord::SetFrameInfo with offline symbolization.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSetFrameInfo001, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_offline_symbolization(true);
    baseData.pid = 12;
    baseData.tid = 11;
    baseData.type = MALLOC_MSG;
    rawdata->stackContext = &baseData;
    rawdata->stackContext->addr = reinterpret_cast<void*>(0x10);
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<MallocRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), MALLOC_MSG);
    CallFrame callFrame {2, 3};
    callFrame.isJsFrame_ = true;
    callFrame.symbolNameId_ = 5;
    callFrame.filePathId_ = 6;
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto allocEvent = hookData->mutable_alloc_event();
    auto frame = allocEvent->add_frame_info();
    hookRecord->SetFrameInfo(*frame, callFrame, &g_hookConfig);
    EXPECT_EQ(frame->ip(), 2);
    EXPECT_EQ(frame->sp(), 3);
    EXPECT_EQ(frame->file_path_id(), 6);
}

/*
 * @tc.name: test SetFrameInfo
 * @tc.desc: test HookRecord::SetFrameInfo with online symbolization.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSetFrameInfo002, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_offline_symbolization(false);
    baseData.type = MMAP_MSG;
    rawdata->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<MmapRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), MMAP_MSG);
    CallFrame callFrame {15, 16};
    callFrame.isJsFrame_ = true;
    callFrame.symbolNameId_ = 5;
    callFrame.filePathId_ = 6;
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto mmapEvent = hookData->mutable_mmap_event();
    auto frame = mmapEvent->add_frame_info();
    hookRecord->SetFrameInfo(*frame, callFrame, &g_hookConfig);
    EXPECT_EQ(frame->ip(), 15);
    EXPECT_EQ(frame->sp(), 16);
    EXPECT_EQ(frame->file_path_id(), 6);
    EXPECT_EQ(frame->symbol_name_id(), 5);
}

/*
 * @tc.name: test SetFrameInfo
 * @tc.desc: test HookRecord::SetFrameInfo with Memory using record.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSetFrameInfo003, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_offline_symbolization(false);
    baseData.type = MEMORY_USING_MSG;
    rawdata->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<MemoryUsingRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), MEMORY_USING_MSG);
    CallFrame callFrame {31, 32};
    callFrame.isJsFrame_ = true;
    callFrame.symbolNameId_ = 9;
    callFrame.filePathId_ = 8;
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto traceAllocEvent = hookData->mutable_trace_alloc_event();
    auto frame = traceAllocEvent->add_frame_info();
    hookRecord->SetFrameInfo(*frame, callFrame, &g_hookConfig);
    EXPECT_EQ(frame->ip(), 31);
    EXPECT_EQ(frame->sp(), 32);
    EXPECT_EQ(frame->file_path_id(), 8);
    EXPECT_EQ(frame->symbol_name_id(), 9);
}

/*
 * @tc.name: test SetTraceType
 * @tc.desc: test HookRecord::SetTraceType.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordTest, TestSetTraceType001, TestSize.Level0)
{
    std::shared_ptr<RawStack> rawdata = std::make_shared<RawStack>();
    g_hookConfig.set_offline_symbolization(false);
    baseData.type = MMAP_MSG;
    rawdata->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<MmapRecord>(rawdata);
    EXPECT_EQ(hookRecord->GetType(), MMAP_MSG);
    BatchNativeHookData stackData;
    auto hookData = stackData.add_events();
    auto traceAllocEvent = hookData->mutable_trace_alloc_event();
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_GPU_VK);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::GPU_VK);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_GPU_GLES_IMAGE);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::GPU_GLES);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_GPU_GLES_BUFFER);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::GPU_GLES);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_GPU_CL_IMAGE);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::GPU_CL);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_GPU_CL_IMAGE);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::GPU_CL);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_FD_OPEN);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::FD);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_FD_ALL);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::FD);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::ARKTS_HEAP);
    hookRecord->SetTraceType(traceAllocEvent, TAG_RES_JS_HEAP_MASK);
    EXPECT_EQ(traceAllocEvent->trace_type(), TraceType::JS_HEAP);
}

}