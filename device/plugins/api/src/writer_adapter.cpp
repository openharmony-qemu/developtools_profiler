/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "writer_adapter.h"

WriterAdapter::WriterAdapter(bool isProtobufSerialize)
{
    writerStruct_.write = &WriterAdapter::WriteFunc;
    writerStruct_.flush = &WriterAdapter::FlushFunc;
    writerStruct_.startReport = &WriterAdapter::StartReportFunc;
    writerStruct_.finishReport = &WriterAdapter::FinishReportFunc;
    writerStruct_.isProtobufSerialize = isProtobufSerialize;
}

WriterAdapter::~WriterAdapter() {}

void WriterAdapter::SetWriter(const WriterPtr& writer)
{
    writer_ = writer;
}

WriterPtr WriterAdapter::GetWriter()
{
    return writer_;
}

const WriterStruct* WriterAdapter::GetStruct()
{
    return &writerStruct_;
}

long WriterAdapter::WriteFunc(WriterStruct* writer, const void* data, size_t size)
{
    static_assert(offsetof(WriterAdapter, writerStruct_) == 0, "unexpected alignment of writerStruct_!");
    WriterAdapter* writerAdaptor = reinterpret_cast<WriterAdapter*>(writer); // 转成 WriterAdapter*
    if (writerAdaptor && writerAdaptor->writer_) {
        return writerAdaptor->writer_->Write(data, size);
    }
    return 0;
}

bool WriterAdapter::FlushFunc(WriterStruct* writer)
{
    WriterAdapter* writerAdaptor = reinterpret_cast<WriterAdapter*>(writer);
    if (writerAdaptor && writerAdaptor->writer_) {
        return writerAdaptor->writer_->Flush();
    }
    return false;
}

RandomWriteCtx* WriterAdapter::StartReportFunc(WriterStruct* writer)
{
    WriterAdapter* writerAdaptor = reinterpret_cast<WriterAdapter*>(writer);
    if (writerAdaptor && writerAdaptor->writer_) {
        return writerAdaptor->writer_->StartReport();
    }
    return nullptr;
}

void WriterAdapter::FinishReportFunc(WriterStruct* writer, int32_t size)
{
    WriterAdapter* writerAdaptor = reinterpret_cast<WriterAdapter*>(writer);
    if (writerAdaptor && writerAdaptor->writer_) {
        writerAdaptor->writer_->FinishReport(size);
    }
}