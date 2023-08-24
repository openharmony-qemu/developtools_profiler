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

#include "trace_data_cache_writer.h"
#include "log.h"
namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
TraceDataCacheWriter::~TraceDataCacheWriter() {}

GpuCounter* TraceDataCacheWriter::GetGpuCounterData()
{
    return &gpuCounter_;
}
GpuCounterObject* TraceDataCacheWriter::GetGpuCounterObjectData()
{
    return &gpuCounterObject_;
}
SliceObject* TraceDataCacheWriter::GetSliceObjectData()
{
    return &sliceObject_;
}
SliceData* TraceDataCacheWriter::GetSliceTableData()
{
    return &sliceData_;
}
MetaData* TraceDataCacheWriter::GetMetaData()
{
    return &metaData_;
}
void TraceDataCacheWriter::MixTraceTime(uint64_t timestampMin, uint64_t timestampMax)
{
    if (timestampMin == std::numeric_limits<uint64_t>::max() || timestampMax == 0) {
        return;
    }
    if (traceStartTime_ != std::numeric_limits<uint64_t>::max()) {
        traceStartTime_ = std::max(traceStartTime_, timestampMin);
    } else {
        traceStartTime_ = timestampMin;
    }
    if (traceEndTime_) {
        traceEndTime_ = std::min(traceEndTime_, timestampMax);
    } else {
        traceEndTime_ = timestampMax;
    }
}
void TraceDataCacheWriter::Clear()
{
    gpuCounter_.Clear();
    gpuCounterObject_.Clear();
    sliceObject_.Clear();
    sliceData_.Clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
