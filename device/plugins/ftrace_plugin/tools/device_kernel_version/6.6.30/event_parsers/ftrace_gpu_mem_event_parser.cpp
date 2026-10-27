/* THIS FILE IS GENERATE BY ftrace_cpp_generator.py, PLEASE DON'T EDIT IT!
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "sub_event_parser.h"

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct GpuMemTotalData {
    uint32_t gpu_id {};
    uint32_t pid {};
    uint64_t size {};
};
__attribute__((noinline)) void ParseGpuMemTotalData(GpuMemTotalData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.gpu_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseGpuMemTotal(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    GpuMemTotalData eventData {};
    ParseGpuMemTotalData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_gpu_mem_total_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_gpu_id(eventData.gpu_id);
    msg->set_pid(eventData.pid);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(gpu_mem_total, ParseGpuMemTotal<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(gpu_mem_total, ParseGpuMemTotal<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
