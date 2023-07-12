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
#include "sdk_plugin_data_parser.h"
#include <string>
#include "sdk/ts_sdk_api.h"

namespace SysTuning {
namespace TraceStreamer {
extern "C" {
void sdk_plugin_init_table_name()
{
    SDK_SetTableName("counter_table", "gpu_counter_object", "slice_table", "slice_object_table");
}
int32_t sdk_plugin_data_parser(const uint8_t* data, int32_t len)
{
    std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(len);
    std::copy(data, data + len, buf.get());
    MockDataArr mockDataArr;
    mockDataArr.ParseFromArray(buf.get(), len);
    int32_t size = mockDataArr.mockdata_size();
    if (size > 1) {
        for (auto m = 0; m < size; m++) {
            auto mockData = mockDataArr.mockdata().at(m);
            sdk_plugin_parser(data, len, mockData);
        }
    } else {
        MockData mockData;
        mockData.ParseFromArray(buf.get(), len);
        sdk_plugin_parser(data, len, mockData);
    }
    return 0;
}

int32_t sdk_plugin_parser(const uint8_t* data, int32_t len, MockData mockData)
{
    // parser counterObject
    for (auto i = 0; i < mockData.counterobj_size(); i++) {
        int32_t counterId = mockData.counterobj(i).id();
        std::string counterName = mockData.counterobj(i).name();
        SDK_AppendCounterObject(counterId, counterName.c_str());
    }

    // parsercounterInfo
    for (auto i = 0; i < mockData.counterinfo_size(); i++) {
        CounterInfo counterInfo;
        counterInfo = mockData.counterinfo(i);
        SDK_AppendCounter(counterInfo.key(), counterInfo.ts(), counterInfo.value());
    }

    // parserSliceObj
    for (auto i = 0; i < mockData.sliceobj_size(); i++) {
        int32_t sliceId = mockData.sliceobj(i).id();
        std::string sliceName = mockData.sliceobj(i).name();
        SDK_AppendSliceObject(sliceId, sliceName.c_str());
    }

    // parserSliceInfo
    for (auto i = 0; i < mockData.sliceinfo_size(); i++) {
        int32_t sliceKey = mockData.sliceinfo(i).id();
        int32_t sliceValue = mockData.sliceinfo(i).value();
        uint64_t startTime = mockData.sliceinfo(i).start_time();
        uint64_t endTime = mockData.sliceinfo(i).end_time();
        SDK_AppendSlice(sliceKey, startTime, endTime, sliceValue);
    }
    return 0;
}
}
} // namespace TraceStreamer
} // namespace SysTuning
