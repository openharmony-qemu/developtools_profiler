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

#include "trace_data_cache_reader.h"
#include "log.h"
namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
TraceDataCacheReader::~TraceDataCacheReader() {}

const GpuCounter& TraceDataCacheReader::GetConstGpuCounterData() const
{
    return gpuCounter_;
}
const GpuCounterObject& TraceDataCacheReader::GetConstGpuCounterObjectData() const
{
    return gpuCounterObject_;
}
const SliceObject& TraceDataCacheReader::GetConstSliceObjectData() const
{
    return sliceObject_;
}
const SliceData& TraceDataCacheReader::GetConstSliceData() const
{
    return sliceData_;
}
const MetaData& TraceDataCacheReader::GetConstMetaData() const
{
    return metaData_;
}
} // namespace TraceStreamer
} // namespace SysTuning
