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

#ifndef TRACE_DATA_CACHE_H
#define TRACE_DATA_CACHE_H

#include <memory>
#include "trace_data_cache_reader.h"
#include "trace_data_cache_writer.h"
#include "trace_data_db.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace TraceStdtype;
class TraceDataCache : public TraceDataCacheReader, public TraceDataCacheWriter, public TraceDataDB {
public:
    TraceDataCache();
    TraceDataCache(const TraceDataCache* dataCache) = delete;
    TraceDataCache* operator=(const TraceDataCache* dataCache) = delete;
    ~TraceDataCache() override;

    bool AnimationTraceEnabled();
    void UpdateAnimationTraceStatus(bool status);
    bool TaskPoolTraceEnabled();
    void UpdateTaskPoolTraceStatus(bool status);
    bool AppStartTraceEnabled();
    void UpdateAppStartTraceStatus(bool status);
    bool BinderRunnableTraceEnabled();
    void UpdateBinderRunnableTraceStatus(bool status);

private:
    void InitDB() override;
    bool dbInited_ = false;
    bool animationTraceEnabled_ = false;
    bool taskPoolTraceEnabled_ = false;
    bool appStartTraceEnabled_ = false;
    bool binderRunnableTraceEnabled_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_DATA_CACHE_H
