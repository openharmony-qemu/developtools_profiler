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
#include "logging.h"
#include "trace_plugin_result.pb.h"
#include "trace_plugin_result.pbencoder.h"

FTRACE_NS_BEGIN

template <class T> // T: FtraceEvent
class SubEventParser {
public:
    using ParseFunction = std::function<void(T&, uint8_t[], size_t, const EventFormat&)>;
    struct ParseEventCtx {
        EventFormat format;
        ParseFunction func;
    };

    SubEventParser()
    {
        PROFILER_LOG_INFO(LOG_CORE, "SubEventParser create!");
    }

    ~SubEventParser()
    {
        PROFILER_LOG_INFO(LOG_CORE, "SubEventParser destroy!");
    }

    static SubEventParser<T>& GetInstance()
    {
        static SubEventParser<T> instance;
        return instance;
    }

    inline bool IsSupport(const std::string& eventName) const
    {
        return nameToFunctions_.count(eventName) > 0;
    }

    bool SetupEvent(const EventFormat& format)
    {
        auto it = nameToFunctions_.find(format.eventName);
        if (it == nameToFunctions_.end()) {
            PROFILER_LOG_INFO(LOG_CORE, "SetupEvent: event(%s) is not supported", format.eventName.c_str());
            return false;
        }

        it->second.format = format;
        idToParseCtx_[format.eventId] = it->second;

        if (format.eventName == "sched_switch") {
            schedSwitchCtx = &it->second;
            schedSwitchEventID = format.eventId;
        } else if (format.eventName == "sched_waking") {
            schedWakingCtx = &it->second;
            schedWakingEventID = format.eventId;
        } else {
            idToFunctions_[format.eventId] = &idToParseCtx_[format.eventId];
        }
        return true;
    }

    inline ParseEventCtx* GetParseEventCtx(uint32_t eventId)
    {
        if (eventId == schedSwitchEventID) {
            return schedSwitchCtx;
        } else if (eventId == schedWakingEventID) {
            return schedWakingCtx;
        }

        auto it = idToFunctions_.find(eventId);
        if (it != idToFunctions_.end()) {
            return it->second;
        }

        return nullptr;
    }

    inline void ParseEvent(T& event,
                           uint8_t data[],
                           size_t size,
                           const ParseEventCtx* parseEventCtx) const
    {
        parseEventCtx->func(event, data, size, parseEventCtx->format);
    }

protected:
    friend class SubEventParserRegisterar;
    friend class SubEventParserOptimizeRegisterar;

    void RegisterParseFunction(const std::string& name, ParseFunction&& func)
    {
        CHECK_TRUE(nameToFunctions_.count(name) == 0, NO_RETVAL,
                   "parse function for %s already registered!", name.c_str());
        nameToFunctions_[name] = {{}, func};
    }

    void UnregisterParseFunction(const std::string& name)
    {
        CHECK_TRUE(nameToFunctions_.count(name) > 0, NO_RETVAL, "parse function for %s not registered!", name.c_str());
        nameToFunctions_.erase(name);
    }

private:
    DISALLOW_COPY_AND_MOVE(SubEventParser);
    std::unordered_map<std::string, ParseEventCtx> nameToFunctions_;
    std::unordered_map<uint32_t, ParseEventCtx> idToParseCtx_;
    std::unordered_map<uint32_t, ParseEventCtx*> idToFunctions_;

    uint32_t schedSwitchEventID = (uint32_t)-1;
    uint32_t schedWakingEventID = (uint32_t)-1;
    ParseEventCtx* schedSwitchCtx = nullptr;
    ParseEventCtx* schedWakingCtx = nullptr;
};

class SubEventParserRegisterar {
public:
    SubEventParserRegisterar(const std::string& name, SubEventParser<FtraceEvent>::ParseFunction&& func)
    {
        SubEventParser<FtraceEvent>::GetInstance().RegisterParseFunction(name, std::move(func));
        name_ = name;
    }
    ~SubEventParserRegisterar()
    {
        SubEventParser<FtraceEvent>::GetInstance().UnregisterParseFunction(name_);
    }

private:
    DISALLOW_COPY_AND_MOVE(SubEventParserRegisterar);
    std::string name_;
};

class SubEventParserOptimizeRegisterar {
public:
    SubEventParserOptimizeRegisterar(const std::string& name,
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::ParseFunction&& func)
    {
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::GetInstance().RegisterParseFunction(name,
            std::move(func));
        name_ = name;
    }
    ~SubEventParserOptimizeRegisterar()
    {
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::GetInstance().UnregisterParseFunction(name_);
    }

private:
    DISALLOW_COPY_AND_MOVE(SubEventParserOptimizeRegisterar);
    std::string name_;
};
FTRACE_NS_END

// Register the protobuf parsing function
#define REGISTER_FTRACE_EVENT_PARSE_FUNCTION(name, func) \
    static FTRACE_NS::SubEventParserRegisterar g_eventRegisterar##name(#name, func)

// Register the proto_encoder parsing function
#define REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(name, func) \
    static FTRACE_NS::SubEventParserOptimizeRegisterar g_eventOptimizeRegisterar##name(#name, func)

#endif // SUB_EVENT_PARSER_H
