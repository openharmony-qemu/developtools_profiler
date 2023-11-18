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

#ifndef TRACE_DATA_CACHE_BASE_H
#define TRACE_DATA_CACHE_BASE_H

#include <array>
#include <deque>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "trace_stdtype.h"
namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
class TraceDataCacheBase {
public:
    TraceDataCacheBase() = default;
    TraceDataCacheBase(const TraceDataCacheBase&) = delete;
    TraceDataCacheBase& operator=(const TraceDataCacheBase&) = delete;
    virtual ~TraceDataCacheBase() = default;

public:
    GpuCounter gpuCounter_;
    GpuCounterObject gpuCounterObject_;
    SliceObject sliceObject_;
    SliceData sliceData_;
    MetaData metaData_;
    uint64_t traceStartTime_ = std::numeric_limits<uint64_t>::max();
    uint64_t traceEndTime_ = 0;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_DATA_CACHE_BASE_H
