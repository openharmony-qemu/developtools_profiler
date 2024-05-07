/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "network_profiler_handle.h"
#include "network_profiler_common.h"

namespace {
constexpr uint8_t FLUSH_INTERVAL = 10;
std::once_flag g_onceProcessName;
}

namespace OHOS::Developtools::Profiler {
NetworkProfilerHandle::NetworkProfilerHandle(clockid_t pluginDataClockId, uint32_t bufferSize, bool isProtobufSerialize)
    : isProtobufSerialize_(isProtobufSerialize), pluginDataClockId_(pluginDataClockId), bufferSize_(bufferSize)
{
    if (isProtobufSerialize_) {
        buffer_ = std::make_unique<uint8_t[]>(bufferSize);
    }
}

NetworkProfilerHandle::~NetworkProfilerHandle()
{
    std::visit([&](auto& protoData) {
        FlushData(protoData);
        }, protoData_);
}

void NetworkProfilerHandle::SetWriter(const std::shared_ptr<Writer>& writer)
{
    writer_ = writer;
    protoData_ = ::NetworkProfilerResult();
}

void NetworkProfilerHandle::SetWriter(const WriterStructPtr& writer)
{
    writerStruct_ = writer;
    auto ctx = writerStruct_->startReport(writerStruct_);
    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    protoData_ = ProtoEncoder::NetworkProfilerResult(ctx);
}

void NetworkProfilerHandle::SerializeData(const int8_t data[], uint32_t size)
{
    std::visit([&](auto& protoData) {
        if (size < sizeof(NetworkEvent)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s the size=%d is too small", __FUNCTION__, size);
            return;
        }
        SerializeDataImpl(protoData, data, size);
        FlushCheck(protoData);
        }, protoData_);
}

template <typename T>
void NetworkProfilerHandle::SerializeDataImpl(T& protoData, const int8_t data[], uint32_t size)
{
    if (size < sizeof(NetworkEvent)) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s the size=%d is too small", __FUNCTION__, size);
        return;
    }
    auto networkEvent = protoData.add_network_event();
    NetworkEvent* event = reinterpret_cast<NetworkEvent*>(const_cast<int8_t*>(data));
    if (event->type == static_cast<int32_t>(NetworkEventType::INVALID)) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s type is invalid", __FUNCTION__);
        return;
    }
    networkEvent->set_tv_sec(event->ts.tv_sec);
    networkEvent->set_tv_nsec(event->ts.tv_nsec);
    networkEvent->set_pid(pid_);
    networkEvent->set_tid(event->tid);
    networkEvent->set_type(event->type);

    std::call_once(g_onceProcessName, [&]() {
        networkEvent->set_process_name(processName_.c_str(), processName_.size());
    });

    size_t threadNameSize = strlen(event->threadName);
    if (threadNameSize > 0) {
        networkEvent->set_thread_name(event->threadName, threadNameSize);
    }
    size_t baseDataSize = sizeof(NetworkEvent);
    if (size > baseDataSize) {
        networkEvent->set_payload(data + baseDataSize, size - baseDataSize);
    }
}

template <typename T>
void NetworkProfilerHandle::FlushCheck(T& protoData)
{
    if ((++flushCount_ & FLUSH_INTERVAL) != 0) {
        return;
    }
    FlushData(protoData);
}

void NetworkProfilerHandle::FlushData(::NetworkProfilerResult& data)
{
    size_t length = data.ByteSizeLong();
    if (length < bufferSize_) {
        data.SerializeToArray(buffer_.get(), length);
        if (buffer_.get() == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "Flush src is nullptr");
            return;
        }

        if (writer_ == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "Flush writer_ is nullptr");
            return;
        }
        writer_->Write(buffer_.get(), length);
        writer_->Flush();
        std::get<::NetworkProfilerResult>(protoData_).clear_network_event();
    }
}

void NetworkProfilerHandle::FlushData(ProtoEncoder::NetworkProfilerResult& data)
{
    if (data.Size() == 0) {
        return;
    }

    int messageLen = data.Finish();

    RandomWriteCtx* ctx = nullptr;
    writerStruct_->finishReport(writerStruct_, messageLen);
    writerStruct_->flush(writerStruct_);
    ctx = writerStruct_->startReport(writerStruct_);
    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    protoData_ = ProtoEncoder::NetworkProfilerResult(ctx);
}
}