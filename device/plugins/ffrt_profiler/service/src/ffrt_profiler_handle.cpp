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

#include "ffrt_profiler_handle.h"

namespace {
constexpr uint8_t FLUSH_INTERVAL = 10;
}

namespace OHOS::Developtools::Profiler {
FfrtProfilerHandle::FfrtProfilerHandle(uint32_t bufferSize, bool isProtobufSerialize)
    : isProtobufSerialize_(isProtobufSerialize), bufferSize_(bufferSize)
{
    if (isProtobufSerialize_) {
        buffer_ = std::make_unique<uint8_t[]>(bufferSize);
    }
}

FfrtProfilerHandle::~FfrtProfilerHandle()
{
    std::visit([&](auto& protoData) {
        FlushData(protoData);
        }, protoData_);
}

void FfrtProfilerHandle::SetWriter(const std::shared_ptr<Writer>& writer)
{
    writer_ = writer;
    protoData_ = ::FfrtProfilerResult();
}

void FfrtProfilerHandle::SetWriter(const WriterStructPtr& writer)
{
    writerStruct_ = writer;
    auto ctx = writerStruct_->startReport(writerStruct_);
    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    protoData_ = ProtoEncoder::FfrtProfilerResult(ctx);
}

void FfrtProfilerHandle::SerializeData(const int8_t data[], uint32_t size)
{
    std::visit([&](auto& protoData) {
        if (size < sizeof(FfrtResultBase)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s the size=%d is too small", __FUNCTION__, size);
            return;
        }
        SerializeDataImpl(protoData, data, size);
        FlushCheck(protoData);
        }, protoData_);
}

template <typename T>
void FfrtProfilerHandle::SerializeDataImpl(T& protoData, const int8_t data[], uint32_t size)
{
    FfrtResultBase* baseData = reinterpret_cast<FfrtResultBase*>(const_cast<int8_t*>(data));
    if (baseData->type == static_cast<int32_t>(EventType::INVALID)) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s type is invalid", __FUNCTION__);
        return;
    }
    ReportCommonData(protoData, baseData);
    if (baseData->type == TRACE_DATA) {
        SerializeTraceData(protoData, data, size);
    } else {
        SerializeRawData(protoData, data, size);
    }
}

template <typename T>
void FfrtProfilerHandle::ReportCommonData(T& protoData, FfrtResultBase* base)
{
    if (base == nullptr || strlen(base->threadName) == 0) {
        return;
    }
    FlushData(protoData);
    auto baseDataProto = protoData.add_ffrt_event();
    SerializeBaseData(baseDataProto, reinterpret_cast<const int8_t*>(base));

    if (reportProcessName_) {
        baseDataProto->set_process_name(processName_.c_str(), processName_.size());
        reportProcessName_ = false;
    }

    baseDataProto->set_thread_name(base->threadName, strlen(base->threadName));
    FlushData(protoData);
}

template <typename T>
void FfrtProfilerHandle::SerializeBaseData(T& baseDataProto, const int8_t* data)
{
    FfrtResultBase* baseData = reinterpret_cast<FfrtResultBase*>(const_cast<int8_t*>(data));

    baseDataProto->set_tv_sec(baseData->ts.tv_sec);
    baseDataProto->set_tv_nsec(baseData->ts.tv_nsec);
    baseDataProto->set_pid(pid_);
    baseDataProto->set_tid(baseData->tid);
}

template <typename T>
void FfrtProfilerHandle::SerializeTraceData(T& protoData, const int8_t data[], uint32_t size)
{
    FfrtTraceEvent* traceData = reinterpret_cast<FfrtTraceEvent*>(const_cast<int8_t*>(data));
    auto ffrtEvent = protoData.add_ffrt_event();
    SerializeBaseData(ffrtEvent, data);

    auto trace = ffrtEvent->mutable_trace();
    trace->set_cpu(traceData->cpu);
    trace->set_trace_type(static_cast<const void*>(&(traceData->traceType)), 1);
    trace->set_cookie(traceData->cookie);

    size_t traceDataSize = sizeof(FfrtTraceEvent);
    if (size > traceDataSize) {
        trace->set_label(data + traceDataSize, strlen(reinterpret_cast<const char*>(data + traceDataSize)));
    }
}

template <typename T>
void FfrtProfilerHandle::SerializeRawData(T& protoData, const int8_t data[], uint32_t size)
{
    FfrtResultBase* baseData = reinterpret_cast<FfrtResultBase*>(const_cast<int8_t*>(data));
    auto ffrtEvent = protoData.add_ffrt_event();
    SerializeBaseData(ffrtEvent, data);

    size_t baseDataSize = sizeof(FfrtResultBase);
    if (size > baseDataSize) {
        auto rawEvent = ffrtEvent->mutable_raw();
        rawEvent->set_type(baseData->type);
        rawEvent->set_payload(data + baseDataSize, size - baseDataSize);
    }
}

template <typename T>
void FfrtProfilerHandle::FlushCheck(T& protoData)
{
    if ((++flushCount_ & FLUSH_INTERVAL) != 0) {
        return;
    }
    FlushData(protoData);
}

void FfrtProfilerHandle::FlushData(::FfrtProfilerResult& data)
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
        std::get<::FfrtProfilerResult>(protoData_).clear_ffrt_event();
    }
}

void FfrtProfilerHandle::FlushData(ProtoEncoder::FfrtProfilerResult& data)
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
    protoData_ = ProtoEncoder::FfrtProfilerResult(ctx);
}
}