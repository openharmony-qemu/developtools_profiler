/*
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
#ifndef SUB_EVENT_PARSER_H
#define SUB_EVENT_PARSER_H
#include <cstdint>
#include <functional>
#include <map>
#include "ftrace_field_parser.h"
#include "trace_plugin_result.pb.h"

FTRACE_NS_BEGIN
class SubEventParserRegisterar;
class SubEventParser {
public:
    static inline SubEventParser& GetInstance()
    {
        static SubEventParser instance;
        return instance;
    }

    bool IsSupport(const std::string& eventName) const;
    bool SetupEvent(const EventFormat& format);

    using ParseFunction = std::function<void(FtraceEvent&, uint8_t[], size_t, const EventFormat&)>;
    struct ParseEventCtx {
        EventFormat format;
        ParseFunction func;
    };

    inline ParseEventCtx* GetParseEventCtx(uint32_t eventId)
    {
        if (eventId == schedSwitchEventID) {
            return schedSwitchCtx;
        } else if (eventId == schedWakingEventID) {
            return schedWakingCtx;
        } else if (eventId == schedWakupEventID) {
            return schedWakingCtx;
        }

        auto it = idToFunctions_.find(eventId);
        if (it != idToFunctions_.end()) {
            return it->second;
        }
        return nullptr;
    }

    inline void ParseEvent(FtraceEvent& event,
                           uint8_t data[],
                           size_t size,
                           const ParseEventCtx* parseEventCtx) const
    {
        parseEventCtx->func(event, data, size, parseEventCtx->format);
    }

protected:
    friend class SubEventParserRegisterar;
    void RegisterParseFunction(const std::string& name, ParseFunction&& func);
    void UnregisterParseFunction(const std::string& name);

private:
    SubEventParser();
    ~SubEventParser();
    DISALLOW_COPY_AND_MOVE(SubEventParser);
    std::unordered_map<std::string, ParseEventCtx> nameToFunctions_;
    std::unordered_map<uint32_t, ParseEventCtx*> idToFunctions_;

    uint32_t schedSwitchEventID = (uint32_t)-1;
    uint32_t schedWakingEventID = (uint32_t)-1;
    uint32_t schedWakupEventID = (uint32_t)-1;
    ParseEventCtx* schedSwitchCtx = nullptr;
    ParseEventCtx* schedWakingCtx = nullptr;
    ParseEventCtx* schedWakupCtx = nullptr;
};

class SubEventParserRegisterar {
public:
    SubEventParserRegisterar(const std::string& name, SubEventParser::ParseFunction&& func);
    ~SubEventParserRegisterar();

private:
    DISALLOW_COPY_AND_MOVE(SubEventParserRegisterar);
    std::string name_;
};
FTRACE_NS_END

#define REGISTER_FTRACE_EVENT_PARSE_FUNCTION(name, func) \
    static FTRACE_NS::SubEventParserRegisterar g_eventRegisterar##name(#name, func)

#endif // SUB_EVENT_PARSER_H
