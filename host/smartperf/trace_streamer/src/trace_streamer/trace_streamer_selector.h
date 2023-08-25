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
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class BytraceParser;
class HtraceParser;
class TraceStreamerSelector {
public:
    TraceStreamerSelector();
    ~TraceStreamerSelector();
    bool ParseTraceDataSegment(std::unique_ptr<uint8_t[]> data, size_t size);
    void EnableMetaTable(bool enabled);
    void EnableFileSave(bool enabled);
    static void SetCleanMode(bool cleanMode);
    int32_t ExportDatabase(const std::string& outputName, TraceDataDB::ResultCallBack resultCallBack = nullptr);
    bool ReloadSymbolFiles(std::string& symbolsPath, std::vector<std::string>& symbolsPaths);
    std::vector<std::string> SearchData();
    int32_t OperateDatabase(const std::string& sql);
    int32_t SearchDatabase(const std::string& sql, TraceDataDB::ResultCallBack resultCallBack);
    int32_t SearchDatabase(const std::string& sql, uint8_t* out, int32_t outLen);
    int32_t SearchDatabase(const std::string& sql, bool printf);
    int32_t UpdateTraceRangeTime(uint8_t* data, int32_t len);
    void WaitForParserEnd();
    void Clear();
    MetaData* GetMetaData();
    void SetDataType(TraceFileType type);
    void SetCancel(bool cancel);
    TraceFileType DataType() const
    {
        return fileType_;
    }
    void UpdateAnimationTraceStatus(bool status);
    void UpdateTaskPoolTraceStatus(bool status);
    void UpdateAppStartTraceStatus(bool status);
    void UpdateBinderRunnableTraceStatus(bool status);

private:
    void InitFilter();
    void Remove_Folders(const char* dir);
    TraceFileType fileType_;
    std::unique_ptr<TraceStreamerFilters> streamFilters_ = {};
    std::unique_ptr<TraceDataCache> traceDataCache_ = {};
    std::unique_ptr<BytraceParser> bytraceParser_;
    std::unique_ptr<HtraceParser> htraceParser_;
    bool enableFileSeparate_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_STREAMER_SELECTOR_H
