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
#ifndef FTRACE_EVENT_PROCESSOR_H
#define FTRACE_EVENT_PROCESSOR_H
#include <cstdint>
#include <functional>
#include <map>
#include "ftrace_field_processor.h"
#include "trace_plugin_result.pb.h"
#include "trace_streamer_config.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace TraceCfg;
class FtraceEventProcessor {
public:
    static FtraceEventProcessor& GetInstance();
    bool SetupEvent(const EventFormat& format);
    bool IsSupported(uint32_t eventId) const;
    bool IsSupported(const std::string& eventName) const;
    bool HandleEvent(FtraceEvent& event, uint8_t data[], size_t size, const EventFormat& format) const;
    const std::string& GetEventNameById(uint32_t eventId);
    using HandleFunction = std::function<void(FtraceEvent&, uint8_t[], size_t, const EventFormat&)>;

private:
    FtraceEventProcessor();
    ~FtraceEventProcessor();
    std::map<uint32_t, HandleFunction> eventIdToFunctions_;
    std::map<uint32_t, std::string> eventIdToNames_;
    std::map<std::string, HandleFunction> eventNameToFunctions_;
    TraceStreamerConfig config_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // FTRACE_EVENT_PROCESSOR_H
