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

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "hook_record.h"

namespace OHOS::Developtools::NativeDaemon {

uint16_t HookRecord::GetType()
{
    return (IsValid()) ? rawStack_->stackContext->type : UNKNOWN;
}

void HookRecord::Reset()
{
    if (rawStack_ != nullptr) {
        rawStack_->Reset();
        rawStack_ = nullptr;
    }
}

uint64_t HookRecord::GetAddr()
{
    return (IsValid()) ? reinterpret_cast<uint64_t>(rawStack_->stackContext->addr) : 0;
}

void HookRecord::SetAddr(uint64_t addr)
{
    if (IsValid()) {
        rawStack_->stackContext->addr = reinterpret_cast<void*>(addr);
    }
}

bool HookRecord::IsValid()
{
    return (rawStack_ && rawStack_->stackContext);
}

template <typename T>
void HookRecord::SetEventFrame(T* event, SerializeInfo& hookInfo)
{
    if (!(event && IsValid())) {
        PROFILER_LOG_ERROR(LOG_CORE, "hookRecord SetEventFrame get nullptr");
        return;
    }
    // ignore the first two frame if dwarf unwind
    size_t idx = hookInfo.config->fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    event->set_pid(rawStack_->stackContext->pid);
    event->set_tid(rawStack_->stackContext->tid);
    event->set_addr(GetAddr());

    if (hookInfo.config->callframe_compress() && hookInfo.stackMapId != 0) {
        event->set_thread_name_id(rawStack_->stackContext->tid);
        event->set_stack_id(hookInfo.stackMapId);
        return;
    }
    for (; idx < (hookInfo.callFrames)->size(); ++idx) {
        auto frame = event->add_frame_info();
        SetFrameInfo(*frame, (*(hookInfo.callFrames))[idx], hookInfo.config);
    }
    event->set_thread_name_id(rawStack_->stackContext->tid);
}

template <typename T>
void HookRecord::SetFrameInfo(T& frame, CallFrame& callFrame, NativeHookConfig* config)
{
    if (config == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "hookRecord SetFrameInfo invalid config");
        return;
    }
    frame.set_ip(callFrame.ip_);
    if (config->offline_symbolization()) {
        // when js mixes offline symbols, the js call stack is reported according to the online symbolization
        if (callFrame.isJsFrame_ && callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0) {
            frame.set_sp(callFrame.sp_);
            frame.set_offset(callFrame.offset_);
            frame.set_symbol_offset(callFrame.symbolOffset_);
            frame.set_symbol_name_id(callFrame.symbolNameId_);
            frame.set_file_path_id(callFrame.filePathId_);
        }
        return;
    }
    frame.set_sp(callFrame.sp_);
    if (!(callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0)) {
        frame.set_symbol_name(std::string(callFrame.symbolName_));
        frame.set_file_path(std::string(callFrame.filePath_));
    }
    frame.set_offset(callFrame.offset_);
    frame.set_symbol_offset(callFrame.symbolOffset_);
    if (callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0) {
        frame.set_symbol_name_id(callFrame.symbolNameId_);
        frame.set_file_path_id(callFrame.filePathId_);
    }
}

template <typename T>
void HookRecord::SetSize(T* event)
{
    if (event == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "hookRecord SetSize invalid event");
        return;
    }
    auto size = static_cast<uint64_t>(rawStack_->stackContext->mallocSize);
#ifdef USE_JEMALLOC
    if (GetType() != MEMORY_USING_MSG) {
        size = static_cast<uint64_t>(ComputeAlign(size));
    }
#endif
    event->set_size(size);
}

void FreeRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto freeEvent = protoData->mutable_free_event();
        HookRecord::SetEventFrame(freeEvent, hookInfo);
        }, stackData);
}

void MallocRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto allocEvent = protoData->mutable_alloc_event();
        HookRecord::SetSize(allocEvent);
        HookRecord::SetEventFrame(allocEvent, hookInfo);
        }, stackData);
}

void MmapRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto mmapEvent = protoData->mutable_mmap_event();
        if (hookInfo.tagName != "") {
            mmapEvent->set_type(hookInfo.tagName);
        }
        HookRecord::SetSize(mmapEvent);
        HookRecord::SetEventFrame(mmapEvent, hookInfo);
        }, stackData);
}

void MmapFilePageRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto mmapEvent = protoData->mutable_mmap_event();
        if (hookInfo.tagName != "") {
            mmapEvent->set_type(MMAP_FILE_PAGE_PREFIX + hookInfo.tagName);
        }
        HookRecord::SetSize(mmapEvent);
        HookRecord::SetEventFrame(mmapEvent, hookInfo);
        }, stackData);
}

void MunmapRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto munmapEvent = protoData->mutable_munmap_event();
        HookRecord::SetSize(munmapEvent);
        HookRecord::SetEventFrame(munmapEvent, hookInfo);
        }, stackData);
}

void PrSetVmaRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this](auto protoData) {
        auto tagEvent = protoData->mutable_tag_event();
        std::string tagName(reinterpret_cast<char*>(rawStack_->data));
        tagEvent->set_addr(reinterpret_cast<uint64_t>(rawStack_->stackContext->addr));
        tagEvent->set_size(rawStack_->stackContext->mallocSize);
        tagEvent->set_tag(PR_SET_VMA_PREFIX + tagName);
        tagEvent->set_pid(rawStack_->stackContext->pid);
        }, stackData);
}

void MemoryUsingRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto memUsingEvent = protoData->mutable_mmap_event();
        if (hookInfo.tagName != "") {
            memUsingEvent->set_type(hookInfo.tagName);
        }
        HookRecord::SetSize(memUsingEvent);
        HookRecord::SetEventFrame(memUsingEvent, hookInfo);
        }, stackData);
}

void MemoryUnusingRecord::SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo)
{
    std::visit([this, &hookInfo](auto protoData) {
        auto munmapEvent = protoData->mutable_munmap_event();
        HookRecord::SetSize(munmapEvent);
        HookRecord::SetEventFrame(munmapEvent, hookInfo);
        }, stackData);
}
}