/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "hook_record_full_test.h"

using namespace testing::ext;

namespace {
static const int INT_NUMBER = 866;
static const int TRACE_TYPE = 2;

static const std::string SERIAL_STRING = R"(
events {
  file_path {
    id: 866
    name: "/system/bin/appspawn"
    pid: 866
  }
}

events {
  symbol_tab {
    file_path_id: 866
    text_exec_vaddr: 866
    text_exec_vaddr_file_offset: 866
    sym_entry_size: 866
    sym_table: "\\000\\000\\000"
    str_table: "\\000__libc_start_main\\000"
    pid: 866
  }
}

events {
  maps_info {
    pid: 866
    start: 866
    end: 866
    offset: 866
    file_path_id: 866
  }
}

events {
  stack_map {
    id: 1
    ip: 866
    ip: 385843504344
    ip: 385843504344
    ip: 385843504344
    ip: 385843504344
    ip: 385843504344
    ip: 385843504344
    ip: 385843504344
    ip: 385843504344
    pid: 866
  }
}

events {
  thread_name_map {
    id: 866
    name: "com.jdx.hm.mallx"
    pid: 866
  }
}

events {
  symbol_name {
    id: 866
    name: "getUpdateInterval"
    pid: 866
  }
}

events {
  frame_map {
    id: 866
    frame {
      symbol_name_id: 866
      file_path_id: 866
    }
    pid: 866
  }
}

events {
  tv_sec: 866
  tv_nsec: 866
  statistics_event {
    pid: 866
    callstack_id: 866
    apply_count: 866
    release_count: 866
    apply_size: 866
    release_size: 866
  }
}

events {
  alloc_event {
    pid: 866
    tid: 866
    addr: 866
    size: 866
    frame_info {
      ip: 866
      sp: 866
      symbol_name: "update"
      file_path: "/data/local/tmp"
      offset: 866
      symbol_offset: 866
      symbol_name_id: 866
      file_path_id: 866
    }
    thread_name_id: 866
    stack_id: 866
  }
}

events {
  free_event {
    pid: 866
    tid: 866
    addr: 866
    frame_info {
      ip: 866
      sp: 866
      symbol_name: "update"
      file_path: "/data/local/tmp"
      offset: 866
      symbol_offset: 866
      symbol_name_id: 866
      file_path_id: 866
    }
    thread_name_id: 866
    stack_id: 866
  }
}

events {
  trace_alloc_event {
    pid: 866
    tid: 866
    addr: 866
    trace_type: 2
    tag_name: "jkk"
    size: 866
    frame_info {
      ip: 866
      sp: 866
      symbol_name: "update"
      file_path: "/data/local/tmp"
      offset: 866
      symbol_offset: 866
      symbol_name_id: 866
      file_path_id: 866
    }
    thread_name_id: 866
    stack_id: 866
  }
}

events {
  trace_free_event {
    pid: 866
    tid: 866
    addr: 866
    trace_type: 2
    tag_name: "jkk"
    frame_info {
      ip: 866
      sp: 866
      symbol_name: "update"
      file_path: "/data/local/tmp"
      offset: 866
      symbol_offset: 866
      symbol_name_id: 866
      file_path_id: 866
    }
    thread_name_id: 866
    stack_id: 866
  }
}

events {
  mmap_event {
    pid: 866
    tid: 866
    addr: 866
    type: "typejjj"
    size: 866
    frame_info {
      ip: 866
      sp: 866
      symbol_name: "update"
      file_path: "/data/local/tmp"
      offset: 866
      symbol_offset: 866
      symbol_name_id: 866
      file_path_id: 866
    }
    thread_name_id: 866
    stack_id: 866
  }
}

events {
  munmap_event {
    pid: 866
    tid: 866
    addr: 866
    size: 866
    frame_info {
      ip: 866
      sp: 866
      symbol_name: "update"
      file_path: "/data/local/tmp"
      offset: 866
      symbol_offset: 866
      symbol_name_id: 866
      file_path_id: 866
    }
    thread_name_id: 866
    stack_id: 866
  }
}
)";

void CheckFilePath(ForStandard::NativeHookData &event)
{
    auto filePath = event.mutable_file_path();
    // 866是字符串所配置的值
    EXPECT_EQ(filePath->id(), INT_NUMBER);
    EXPECT_EQ(filePath->name(), "/system/bin/appspawn");
    EXPECT_EQ(filePath->pid(), INT_NUMBER);
}

void CheckSymbolTab(ForStandard::NativeHookData &event)
{
    auto symbolTab = event.mutable_symbol_tab();
    EXPECT_EQ(symbolTab->file_path_id(), INT_NUMBER);
    EXPECT_EQ(symbolTab->text_exec_vaddr(), INT_NUMBER);
    EXPECT_EQ(symbolTab->text_exec_vaddr_file_offset(), INT_NUMBER);
    EXPECT_EQ(symbolTab->sym_entry_size(), INT_NUMBER);
    EXPECT_EQ(symbolTab->sym_table(), "\\000\\000\\000");
    EXPECT_EQ(symbolTab->str_table(), "\\000__libc_start_main\\000");
    EXPECT_EQ(symbolTab->pid(), INT_NUMBER);
}

void CheckMapsInfo(ForStandard::NativeHookData &event)
{
    auto mapsInfo = event.mutable_maps_info();
    EXPECT_EQ(mapsInfo->file_path_id(), INT_NUMBER);
    EXPECT_EQ(mapsInfo->pid(), INT_NUMBER);
    EXPECT_EQ(mapsInfo->start(), INT_NUMBER);
    EXPECT_EQ(mapsInfo->end(), INT_NUMBER);
    EXPECT_EQ(mapsInfo->offset(), INT_NUMBER);
}

void CheckStackMap(ForStandard::NativeHookData &event)
{
    auto stackMap = event.mutable_stack_map();
    EXPECT_EQ(stackMap->pid(), INT_NUMBER);
    EXPECT_EQ(stackMap->id(), 1);
}

void CheckThreadNameMap(ForStandard::NativeHookData &event)
{
    auto threadNameMap = event.mutable_thread_name_map();
    EXPECT_EQ(threadNameMap->pid(), INT_NUMBER);
    EXPECT_EQ(threadNameMap->id(), INT_NUMBER);
    EXPECT_EQ(threadNameMap->name(), "com.jdx.hm.mallx");
}

void CheckSymbolName(ForStandard::NativeHookData &event)
{
    auto symbolName = event.mutable_symbol_name();
    EXPECT_EQ(symbolName->pid(), INT_NUMBER);
    EXPECT_EQ(symbolName->id(), INT_NUMBER);
    EXPECT_EQ(symbolName->name(), "getUpdateInterval");
}

void CheckFrameMap(ForStandard::NativeHookData &event)
{
    auto frameMap = event.mutable_frame_map();
    auto frame = frameMap->mutable_frame();
    EXPECT_EQ(frameMap->pid(), INT_NUMBER);
    EXPECT_EQ(frameMap->id(), INT_NUMBER);
    EXPECT_EQ(frame->symbol_name_id(), INT_NUMBER);
    EXPECT_EQ(frame->file_path_id(), INT_NUMBER);
}

void CheckStatisticsEvent(ForStandard::NativeHookData &event)
{
    EXPECT_EQ(event.tv_sec(), INT_NUMBER);
    EXPECT_EQ(event.tv_nsec(), INT_NUMBER);
    auto statisticsEvent = event.mutable_statistics_event();
    EXPECT_EQ(statisticsEvent->pid(), INT_NUMBER);
    EXPECT_EQ(statisticsEvent->callstack_id(), INT_NUMBER);
    EXPECT_EQ(statisticsEvent->apply_count(), INT_NUMBER);
    EXPECT_EQ(statisticsEvent->release_count(), INT_NUMBER);
    EXPECT_EQ(statisticsEvent->apply_size(), INT_NUMBER);
    EXPECT_EQ(statisticsEvent->release_size(), INT_NUMBER);
}

void CheckFrame(ForStandard::Frame &frameInfo)
{
    EXPECT_EQ(frameInfo.ip(), INT_NUMBER);
    EXPECT_EQ(frameInfo.sp(), INT_NUMBER);
    EXPECT_EQ(frameInfo.symbol_name(), "update");
    EXPECT_EQ(frameInfo.file_path(), "/data/local/tmp");
    EXPECT_EQ(frameInfo.offset(), INT_NUMBER);
    EXPECT_EQ(frameInfo.symbol_offset(), INT_NUMBER);
    EXPECT_EQ(frameInfo.symbol_name_id(), INT_NUMBER);
    EXPECT_EQ(frameInfo.file_path_id(), INT_NUMBER);
}

void CheckAllocEvent(ForStandard::NativeHookData &event)
{
    auto allocEvent = event.mutable_alloc_event();
    EXPECT_EQ(allocEvent->pid(), INT_NUMBER);
    EXPECT_EQ(allocEvent->tid(), INT_NUMBER);
    EXPECT_EQ(allocEvent->addr(), INT_NUMBER);
    EXPECT_EQ(allocEvent->size(), INT_NUMBER);
    auto frameInfo = allocEvent->frame_info(0);
    CheckFrame(frameInfo);

    EXPECT_EQ(allocEvent->thread_name_id(), INT_NUMBER);
    EXPECT_EQ(allocEvent->stack_id(), INT_NUMBER);
}

void CheckFreeEvent(ForStandard::NativeHookData &event)
{
    auto freeEvent = event.mutable_free_event();
    EXPECT_EQ(freeEvent->pid(), INT_NUMBER);
    EXPECT_EQ(freeEvent->tid(), INT_NUMBER);
    EXPECT_EQ(freeEvent->addr(), INT_NUMBER);
    auto frameInfo = freeEvent->frame_info(0);
    CheckFrame(frameInfo);

    EXPECT_EQ(freeEvent->thread_name_id(), INT_NUMBER);
    EXPECT_EQ(freeEvent->stack_id(), INT_NUMBER);
}

void CheckTraceAllocEvent(ForStandard::NativeHookData &event)
{
    auto traceAllocEvent = event.mutable_trace_alloc_event();
    EXPECT_EQ(traceAllocEvent->pid(), INT_NUMBER);
    EXPECT_EQ(traceAllocEvent->tid(), INT_NUMBER);
    EXPECT_EQ(traceAllocEvent->addr(), INT_NUMBER);
    EXPECT_EQ(traceAllocEvent->trace_type(), TRACE_TYPE);
    EXPECT_EQ(traceAllocEvent->tag_name(), "jkk");
    EXPECT_EQ(traceAllocEvent->size(), INT_NUMBER);
    auto frameInfo = traceAllocEvent->frame_info(0);
    CheckFrame(frameInfo);

    EXPECT_EQ(traceAllocEvent->thread_name_id(), INT_NUMBER);
    EXPECT_EQ(traceAllocEvent->stack_id(), INT_NUMBER);
}

void CheckTraceFreeEvent(ForStandard::NativeHookData &event)
{
    auto traceFreeEvent = event.mutable_trace_free_event();
    EXPECT_EQ(traceFreeEvent->pid(), INT_NUMBER);
    EXPECT_EQ(traceFreeEvent->tid(), INT_NUMBER);
    EXPECT_EQ(traceFreeEvent->addr(), INT_NUMBER);
    EXPECT_EQ(traceFreeEvent->trace_type(), TRACE_TYPE);
    EXPECT_EQ(traceFreeEvent->tag_name(), "jkk");
    auto frameInfo = traceFreeEvent->frame_info(0);
    CheckFrame(frameInfo);

    EXPECT_EQ(traceFreeEvent->thread_name_id(), INT_NUMBER);
    EXPECT_EQ(traceFreeEvent->stack_id(), INT_NUMBER);
}

void CheckMmapEvent(ForStandard::NativeHookData &event)
{
    auto mmapEvent = event.mutable_mmap_event();
    EXPECT_EQ(mmapEvent->pid(), INT_NUMBER);
    EXPECT_EQ(mmapEvent->tid(), INT_NUMBER);
    EXPECT_EQ(mmapEvent->addr(), INT_NUMBER);
    EXPECT_EQ(mmapEvent->type(), "typejjj");
    EXPECT_EQ(mmapEvent->size(), INT_NUMBER);
    auto frameInfo = mmapEvent->frame_info(0);
    CheckFrame(frameInfo);

    EXPECT_EQ(mmapEvent->thread_name_id(), INT_NUMBER);
    EXPECT_EQ(mmapEvent->stack_id(), INT_NUMBER);
}

void CheckMunmapEvent(ForStandard::NativeHookData &event)
{
    auto munmapEvent = event.mutable_munmap_event();
    EXPECT_EQ(munmapEvent->pid(), INT_NUMBER);
    EXPECT_EQ(munmapEvent->tid(), INT_NUMBER);
    EXPECT_EQ(munmapEvent->addr(), INT_NUMBER);
    EXPECT_EQ(munmapEvent->size(), INT_NUMBER);
    auto frameInfo = munmapEvent->frame_info(0);
    CheckFrame(frameInfo);

    EXPECT_EQ(munmapEvent->thread_name_id(), INT_NUMBER);
    EXPECT_EQ(munmapEvent->stack_id(), INT_NUMBER);
}

/*
 * @tc.name: HookDataFullTest
 * @tc.desc: test HookService::ProtocolProc with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookRecordFullTest, HookDataFullTest, TestSize.Level0)
{
    ForStandard::BatchNativeHookData stackData;
    google::protobuf::TextFormat::ParseFromString(SERIAL_STRING, &stackData);
    EXPECT_EQ(stackData.events_size(), 14);

    auto event = stackData.events(0);
    CheckFilePath(event);

    event = stackData.events(1);
    CheckSymbolTab(event);

    event = stackData.events(2);
    CheckMapsInfo(event);

    event = stackData.events(3);
    CheckStackMap(event);

    event = stackData.events(4);
    CheckThreadNameMap(event);

    event = stackData.events(5);
    CheckSymbolName(event);

    event = stackData.events(6);
    CheckFrameMap(event);

    event = stackData.events(7);
    CheckStatisticsEvent(event);
    
    event = stackData.events(8);
    CheckAllocEvent(event);

    event = stackData.events(9);
    CheckFreeEvent(event);
    
    event = stackData.events(10);
    CheckTraceAllocEvent(event);
    
    event = stackData.events(11);
    CheckTraceFreeEvent(event);
    
    event = stackData.events(12);
    CheckMmapEvent(event);
    
    event = stackData.events(13);
    CheckMunmapEvent(event);
}
} // namespace
