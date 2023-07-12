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

#ifndef SDK_DATA_PARSER_H
#define SDK_DATA_PARSER_H

#include <functional>
#include <mutex>
#include <clock_filter.h>
#include "file.h"
#include "htrace_plugin_time_parser.h"
#include "table/table_base.h"
#include "trace_streamer/trace_streamer_selector.h"

namespace SysTuning {
namespace TraceStreamer {
enum Third_Party_Wasm_Id {
    DATA_TYPE_MOCK_PLUGIN = 0,
    DATA_TYPE_CLOCK = 100,
};
class SDKDataParser : public HtracePluginTimeParser {
public:
    using TraceRangeCallbackFunction = std::function<void(const std::string)>;
    using QueryResultCallbackFunction = std::function<void(const std::string /* result */, int32_t, int32_t)>;
    SDKDataParser(TraceDataCache* dataCache);
    ~SDKDataParser(){};

    // third_party
    int32_t CreateTableByJson();
    int32_t SetTableName(const char* counterTableName,
                         const char* counterObjectTableName,
                         const char* sliceTableName,
                         const char* sliceObjectName);
    int32_t GetJsonConfig(QueryResultCallbackFunction queryResultCallbackFunction);
    int32_t GetPluginName(std::string pluginName);
    int32_t ParseDataOver(TraceRangeCallbackFunction traceRangeCallbackFunction);
    int32_t ParserData(const uint8_t* data, int32_t len, int32_t componentId);
    int32_t AppendCounterObject(int32_t counterId, const char* columnName);
    int32_t AppendCounter(int32_t counterId, uint64_t ts, int32_t value);
    int32_t AppendSliceObject(int32_t sliceId, const char* columnName);
    int32_t AppendSlice(int32_t sliceId, uint64_t ts, uint64_t endTs, int32_t value);

private:
    int32_t CreateCounterObjectTable(const std::string& tableName);
    int32_t CreateCounterTable(const std::string& tableName);
    int32_t CreateSliceObjectTable(const std::string& tableName);
    int32_t CreateSliceTable(const std::string& tableName);
    int32_t ParserClock(const uint8_t* data, int32_t len);
    int32_t UpdateJson();

public:
    std::string counterTableName_ = "counter_table";
    std::string counterObjectTableName_ = "gpu_counter_object";
    std::string sliceTableName_ = "slice_table";
    std::string sliceObjectName_ = "slice_object_table";
    std::string jsonConfig_ =
        "{\"tableConfig\":{\"showType\":[{\"tableName\":\"counter_table\",\"inner\":{\"tableName\":\"gpu_counter_"
        "object\","
        "\"columns\":[{\"column\":\"counter_name\",\"type\":\"STRING\",\"displayName\":\"\",\"showType\":[0]},{"
        "\"column\":"
        "\"counter_id\",\"type\":\"INTEGER\",\"displayName\":\"\",\"showType\":[0]}]},\"columns\":[{\"column\":\"ts\","
        "\"type\":\"INTEGER\",\"displayName\":\"TimeStamp\",\"showType\":[1,3]},{\"column\":\"counter_id\",\"type\":"
        "\"INTEGER\",\"displayName\":\"MonitorValue\",\"showType\":[1,3]},{\"column\":\"value\",\"type\":\"INTEGER\","
        "\"displayName\":\"Value\",\"showType\":[1,3]}]},{\"tableName\":\"slice_table\",\"inner\":{\"tableName\":"
        "\"slice_"
        "object_table\",\"columns\":[{\"column\":\"slice_name\",\"type\":\"STRING\",\"displayName\":\"\",\"showType\":["
        "0]},"
        "{\"column\":\"slice_id\",\"type\":\"INTEGER\",\"displayName\":\"\",\"showType\":[0]}]},\"columns\":[{"
        "\"column\":"
        "\"start_ts\",\"type\":\"INTEGER\",\"displayName\":\"startts\",\"showType\":[2,3]},{\"column\":\"end_ts\","
        "\"type\":"
        "\"INTEGER\",\"displayName\":\"endts\",\"showType\":[2,3]},{\"column\":\"slice_id\",\"type\":\"INTEGER\","
        "\"displayName\":\"slice_id\",\"showType\":[2,3]},{\"column\":\"value\",\"type\":\"INTEGER\",\"displayName\":"
        "\"Value\",\"showType\":[2,3]}]}]},\"settingConfig\":{\"name\":\"mailG77\",\"configuration\":{\"version\":{"
        "\"type\":\"number\",\"default\":\"1\",\"description\":\"gatordversion\"},\"counters\":{\"type\":\"string\","
        "\"enum\":[\"ARM_Mali-TTRx_JS1_ACTIVE\",\"ARM_Mali-TTRx_JS0_ACTIVE\",\"ARM_Mali-TTRx_GPU_ACTIVE\",\"ARM_Mali-"
        "TTRx_FRAG_ACTIVE\"]},\"stop_gator\":{\"type\":\"boolean\",\"default\":\"true\",\"description\":\"stop_gator\"}"
        "}}}";

private:
    TraceDataCache* traceDataCache_ = nullptr;
    std::unique_ptr<ClockFilter> clockFilter_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // SDK_DATA_PARSER_H
