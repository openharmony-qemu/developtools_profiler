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
#include "metrics.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class BytraceParser;
class HtraceParser;
class RawTraceParser;
class TraceStreamerSelector {
public:
    TraceStreamerSelector();
    ~TraceStreamerSelector();
    bool ParseTraceDataSegment(std::unique_ptr<uint8_t[]> data, size_t size, bool isSplitFile, int32_t isFinish);
    void EnableMetaTable(bool enabled);
    void EnableFileSave(bool enabled);
    static void SetCleanMode(bool cleanMode);
    int32_t ExportDatabase(const std::string& outputName, TraceDataDB::ResultCallBack resultCallBack = nullptr);
    bool ReloadSymbolFiles(std::string& symbolsPath, std::vector<std::string>& symbolsPaths);
    std::vector<std::string> SearchData();
    int32_t OperateDatabase(const std::string& sql);
    int32_t SearchDatabase(const std::string& sql, TraceDataDB::ResultCallBack resultCallBack);
    int32_t SearchDatabase(const std::string& sql, uint8_t* out, int32_t outLen);
    int32_t SearchDatabase(std::string& sql, bool printf);
    std::string SearchDatabase(const std::string& sql);
    int32_t UpdateTraceRangeTime(uint8_t* data, int32_t len);
    void WaitForParserEnd();
    void Clear();
    MetaData* GetMetaData();
    void SetDataType(TraceFileType type);
    void SetCancel(bool cancel);
    bool ParserAndPrintMetrics(const std::string& metrics);
    bool ReadSqlFileAndPrintResult(const std::string& sqlOperator);
    TraceFileType DataType() const
    {
        return fileType_;
    }
    void UpdateAnimationTraceStatus(bool status);
    void UpdateTaskPoolTraceStatus(bool status);
    void UpdateAppStartTraceStatus(bool status);
    void UpdateBinderRunnableTraceStatus(bool status);
    void InitMetricsMap(std::map<std::string, std::string>& metricsMap);
    const std::string MetricsSqlQuery(const std::string& metrics);
    auto GetBytraceData()
    {
        return bytraceParser_.get();
    }
    auto GetHtraceData()
    {
        return htraceParser_.get();
    }
    auto GetFileType()
    {
        return fileType_;
    }
    auto GetTraceDataCache()
    {
        return traceDataCache_.get();
    }
    uint64_t minTs_ = INVALID_UINT64;
    uint64_t maxTs_ = INVALID_UINT64;

private:
    void InitFilter();
    void Remove_Folders(const char* dir);
    bool LoadQueryFile(const std::string& sqlOperator, std::vector<std::string>& sqlStrings);
    TraceFileType fileType_;
    std::unique_ptr<TraceStreamerFilters> streamFilters_ = {};
    std::unique_ptr<TraceDataCache> traceDataCache_ = {};
    std::unique_ptr<BytraceParser> bytraceParser_;
    std::unique_ptr<HtraceParser> htraceParser_;
    std::unique_ptr<RawTraceParser> rawTraceParser_;
    bool enableFileSeparate_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // TRACE_STREAMER_SELECTOR_H
