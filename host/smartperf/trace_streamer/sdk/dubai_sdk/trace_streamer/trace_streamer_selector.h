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

#ifndef TRACE_STREAMER_SELECTOR_H
#define TRACE_STREAMER_SELECTOR_H
#include <functional>
#include <memory>
#include "sdk/sdk_data_parser.h"
#include "trace_data/trace_data_cache.h"

namespace SysTuning {
namespace TraceStreamer {
class SDKDataParser;
enum TraceFileType { TRACE_FILETYPE_UN_KNOW };
class TraceStreamerSelector {
public:
    TraceStreamerSelector();
    ~TraceStreamerSelector();
    static bool ParseTraceDataSegment(std::unique_ptr<uint8_t[]> data, size_t size);
    void EnableMetaTable(bool enabled);
    static void SetCleanMode(bool cleanMode);
    int32_t ExportDatabase(const std::string& outputName) const;
    int32_t SearchData();
    int32_t OperateDatabase(const std::string& sql);
    int32_t SearchDatabase(const std::string& sql, TraceDataDB::ResultCallBack resultCallBack);
    int32_t SearchDatabase(const std::string& sql, uint8_t* out, int32_t outLen);
    MetaData* GetMetaData();
    static void WaitForParserEnd();
    void Clear();
    void SetDataType(TraceFileType type);
    void SetCancel(bool cancel);
    std::unique_ptr<SDKDataParser> sdkDataParser_ = {};

private:
    void InitFilter();
    std::unique_ptr<TraceDataCache> traceDataCache_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_STREAMER_SELECTOR_H
