/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include "buffer_writer.h"
#include "command_poller.h"
#include "common_types.pb.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "share_memory_allocator.h"

#include <algorithm>
#include <cinttypes>
#include <thread>
#include <unistd.h>

using namespace OHOS::Developtools::Profiler;

BufferWriter::BufferWriter(std::string name,
                           std::string version,
                           uint32_t size,
                           int smbFd,
                           int eventFd,
                           uint32_t pluginId)
    : pluginName_(name), pluginVersion_(version)
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:%s %d [%d] [%d]", __func__, name.c_str(), size, smbFd, eventFd);
    shareMemoryBlock_ = ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote(name, size, smbFd);
    if (shareMemoryBlock_ == nullptr) {
        PROFILER_LOG_DEBUG(LOG_CORE, "%s:create shareMemoryBlock_ failed!", __func__);
    }
    eventNotifier_ = EventNotifier::CreateWithFd(eventFd);
    pluginId_ = pluginId;
    lastFlushTime_ = std::chrono::steady_clock::now();
    if (shareMemoryBlock_ != nullptr) {
        writeCtx_ = reinterpret_cast<RandomWriteCtx*>(shareMemoryBlock_->GetCtx());
        profilerPluginData_ = ProtoEncoder::ProfilerPluginData(writeCtx_);
    }
}

BufferWriter::~BufferWriter()
{
    PROFILER_LOG_DEBUG(LOG_CORE, "%s:destroy eventfd = %d!", __func__, eventNotifier_ ? eventNotifier_->GetFd() : -1);
    eventNotifier_ = nullptr;
    ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote(pluginName_);
}

void BufferWriter::Report() const
{
    PROFILER_LOG_DEBUG(LOG_CORE, "%s:stats B: %" PRIu64 ", P: %d, W:%" PRIu64 ", F: %d", __func__,
        bytesCount_.load(), bytesPending_.load(), writeCount_.load(), flushCount_.load());
}

void BufferWriter::DoStats(long bytes)
{
    ++writeCount_;
    bytesCount_ += static_cast<uint64_t>(bytes);
    bytesPending_ += static_cast<uint32_t>(bytes);
}

long BufferWriter::Write(const void* data, size_t size)
{
    if (shareMemoryBlock_ == nullptr || data == nullptr || size == 0) {
        return false;
    }
    ProfilerPluginData pluginData;
    pluginData.set_name(pluginName_);
    pluginData.set_version(pluginVersion_);
    pluginData.set_status(0);
    pluginData.set_data(data, size);

    struct timespec ts = { 0, 0 };
    clock_gettime(clockId_, &ts);

    pluginData.set_clock_id(static_cast<ProfilerPluginData_ClockId>(clockId_));
    pluginData.set_tv_sec(ts.tv_sec);
    pluginData.set_tv_nsec(ts.tv_nsec);

    DoStats(pluginData.ByteSizeLong());
    return shareMemoryBlock_->PutMessage(pluginData, pluginName_);
}

RandomWriteCtx* BufferWriter::StartReport()
{
    if (shareMemoryBlock_ == nullptr || writeCtx_ == nullptr) {
        return nullptr;
    }

    shareMemoryBlock_->ResetPos();
    profilerPluginData_.Reset(writeCtx_);
    profilerPluginData_.set_name(pluginName_);
    profilerPluginData_.set_version(pluginVersion_);
    profilerPluginData_.set_status(0);
    return profilerPluginData_.startAdd_data();
}

void BufferWriter::FinishReport(int32_t size)
{
    if (shareMemoryBlock_ == nullptr) {
        return;
    }

    profilerPluginData_.finishAdd_data(size);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    profilerPluginData_.set_clock_id(ProfilerPluginData::CLOCKID_REALTIME);
    profilerPluginData_.set_tv_sec(ts.tv_sec);
    profilerPluginData_.set_tv_nsec(ts.tv_nsec);
    int32_t len = profilerPluginData_.Finish();
    shareMemoryBlock_->UseMemory(len);
    DoStats(len);
}

bool BufferWriter::WriteMessage(const google::protobuf::Message& pmsg, const std::string& pluginName)
{
    if (shareMemoryBlock_ == nullptr) {
        return false;
    }
    DoStats(pmsg.ByteSizeLong());
    return shareMemoryBlock_->PutMessage(pmsg, pluginName);
}

bool BufferWriter::Flush()
{
    ++flushCount_;
    if (eventNotifier_ == nullptr) {
        return false;
    }
    eventNotifier_->Post(flushCount_.load());
    lastFlushTime_ = std::chrono::steady_clock::now();
    bytesPending_ = 0;
    return true;
}

void BufferWriter::UseMemory(int32_t size)
{
    if (shareMemoryBlock_ == nullptr) {
        return;
    }

    shareMemoryBlock_->UseMemory(size);
    DoStats(size);
}

void BufferWriter::ResetPos()
{
    if (shareMemoryBlock_ == nullptr) {
        return;
    }

    shareMemoryBlock_->ResetPos();
}