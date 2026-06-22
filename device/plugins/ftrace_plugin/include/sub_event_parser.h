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
#include <cstring>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "ftrace_field_parser.h"
#include "logging.h"
#include "trace_plugin_result.pb.h"
#include "trace_plugin_result.pbencoder.h"

FTRACE_NS_BEGIN

#if defined(__has_attribute)
#if __has_attribute(retain)
#define FTRACE_PARSE_ENTRY_ATTR __attribute__((used, retain, section("ftrace_parse_entries")))
#else
#define FTRACE_PARSE_ENTRY_ATTR __attribute__((used, section("ftrace_parse_entries")))
#endif
#else
#define FTRACE_PARSE_ENTRY_ATTR __attribute__((used, section("ftrace_parse_entries")))
#endif

template <class T> // T: FtraceEvent
class SubEventParser {
public:
    using ParseFunction = void (*)(T&, uint8_t[], size_t, const EventFormat&);
    struct ParseEventCtx {
        EventFormat format;
        ParseFunction func = nullptr;
    };
    struct NamedParseFunction {
        const char* name = nullptr;
        ParseFunction func = nullptr;
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
        return FindParseFunction(eventName) != nullptr;
    }

    bool SetupEvent(const EventFormat& format)
    {
        auto func = FindParseFunction(format.eventName);
        if (func == nullptr) {
            PROFILER_LOG_INFO(LOG_CORE, "SetupEvent: event(%s) is not supported", format.eventName.c_str());
            return false;
        }

        if (format.eventId >= idToFunctions_.size()) {
            idToFunctions_.resize(format.eventId + 1);
        }
        idToFunctions_[format.eventId] = {format, func};

        if (format.eventName == "sched_switch") {
            schedSwitchEventID = format.eventId;
            schedSwitchCtx = GetMutableParseEventCtx(format.eventId);
        } else if (format.eventName == "sched_waking") {
            schedWakingEventID = format.eventId;
            schedWakingCtx = GetMutableParseEventCtx(format.eventId);
        }
        return true;
    }

    inline ParseEventCtx* GetParseEventCtx(uint32_t eventId)
    {
        if (eventId == schedSwitchEventID) {
            return GetMutableParseEventCtx(eventId);
        } else if (eventId == schedWakingEventID) {
            return GetMutableParseEventCtx(eventId);
        }

        return GetMutableParseEventCtx(eventId);
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
    friend class SubEventParserPairRegisterar;

    void RegisterParseFunction(const char* name, ParseFunction func)
    {
        CHECK_NOTNULL(name, NO_RETVAL, "parse function name is null!");
        CHECK_NOTNULL(func, NO_RETVAL, "parse function for %s is null!", name);
        CHECK_TRUE(FindParseFunction(name) == nullptr, NO_RETVAL, "parse function for %s already registered!", name);
        registeredFunctions_.push_back({name, func});
    }

    void UnregisterParseFunction(const char* name)
    {
        CHECK_NOTNULL(name, NO_RETVAL, "parse function name is null!");
        for (auto it = registeredFunctions_.begin(); it != registeredFunctions_.end(); ++it) {
            if (std::strcmp(it->name, name) == 0) {
                registeredFunctions_.erase(it);
                return;
            }
        }
        CHECK_TRUE(false, NO_RETVAL, "parse function for %s not registered!", name);
    }

private:
    ParseFunction FindParseFunction(const std::string& name) const;
    ParseFunction FindParseFunction(const char* name) const;
    ParseEventCtx* GetMutableParseEventCtx(uint32_t eventId)
    {
        if (eventId < idToFunctions_.size() && idToFunctions_[eventId].func != nullptr) {
            return &idToFunctions_[eventId];
        }
        return nullptr;
    }

    DISALLOW_COPY_AND_MOVE(SubEventParser);
    std::vector<NamedParseFunction> registeredFunctions_;
    std::vector<ParseEventCtx> idToFunctions_;

    uint32_t schedSwitchEventID = (uint32_t)-1;
    uint32_t schedWakingEventID = (uint32_t)-1;
    ParseEventCtx* schedSwitchCtx = nullptr;
    ParseEventCtx* schedWakingCtx = nullptr;
};

using FtraceEventParseFunction = SubEventParser<FtraceEvent>::ParseFunction;
using FtraceEventParseEncoderFunction =
    SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::ParseFunction;

struct FtraceParseEntry {
    const char* name = nullptr;
    FtraceEventParseFunction func = nullptr;
    FtraceEventParseEncoderFunction encoderFunc = nullptr;
};

extern "C" {
extern const FtraceParseEntry __start_ftrace_parse_entries[] __attribute__((weak));
extern const FtraceParseEntry __stop_ftrace_parse_entries[] __attribute__((weak));
}

template <class T>
typename SubEventParser<T>::ParseFunction SubEventParser<T>::FindParseFunction(const std::string& name) const
{
    return FindParseFunction(name.c_str());
}

template <class T>
typename SubEventParser<T>::ParseFunction SubEventParser<T>::FindParseFunction(const char* name) const
{
    for (const auto& item : registeredFunctions_) {
        if (std::strcmp(item.name, name) == 0) {
            return item.func;
        }
    }
    if (__start_ftrace_parse_entries == nullptr || __stop_ftrace_parse_entries == nullptr) {
        return nullptr;
    }
    for (auto entry = __start_ftrace_parse_entries; entry < __stop_ftrace_parse_entries; ++entry) {
        if (std::strcmp(entry->name, name) != 0) {
            continue;
        }
        if constexpr (std::is_same_v<T, FtraceEvent>) {
            return entry->func;
        } else {
            return entry->encoderFunc;
        }
    }
    return nullptr;
}

class SubEventParserRegisterar {
public:
    __attribute__((noinline))
    SubEventParserRegisterar(const char* name, SubEventParser<FtraceEvent>::ParseFunction func)
    {
        name_ = name;
        SubEventParser<FtraceEvent>::GetInstance().RegisterParseFunction(name_, func);
    }
    __attribute__((noinline))
    ~SubEventParserRegisterar()
    {
        SubEventParser<FtraceEvent>::GetInstance().UnregisterParseFunction(name_);
    }

private:
    DISALLOW_COPY_AND_MOVE(SubEventParserRegisterar);
    const char* name_ = nullptr;
};

class SubEventParserOptimizeRegisterar {
public:
    __attribute__((noinline))
    SubEventParserOptimizeRegisterar(const char* name,
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::ParseFunction func)
    {
        name_ = name;
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::GetInstance().RegisterParseFunction(name_,
            func);
    }
    __attribute__((noinline))
    ~SubEventParserOptimizeRegisterar()
    {
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::GetInstance().UnregisterParseFunction(name_);
    }

private:
    DISALLOW_COPY_AND_MOVE(SubEventParserOptimizeRegisterar);
    const char* name_ = nullptr;
};

class SubEventParserPairRegisterar {
public:
    __attribute__((noinline))
    SubEventParserPairRegisterar(const char* name, SubEventParser<FtraceEvent>::ParseFunction func,
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::ParseFunction encoderFunc)
    {
        name_ = name;
        SubEventParser<FtraceEvent>::GetInstance().RegisterParseFunction(name_, func);
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::GetInstance().RegisterParseFunction(name_,
            encoderFunc);
    }
    __attribute__((noinline))
    ~SubEventParserPairRegisterar()
    {
        SubEventParser<Developtools::Profiler::ProtoEncoder::FtraceEvent>::GetInstance().UnregisterParseFunction(name_);
        SubEventParser<FtraceEvent>::GetInstance().UnregisterParseFunction(name_);
    }

private:
    DISALLOW_COPY_AND_MOVE(SubEventParserPairRegisterar);
    const char* name_ = nullptr;
};
FTRACE_NS_END

// Register the protobuf parsing function
#define REGISTER_FTRACE_EVENT_PARSE_FUNCTION(name, func) \
    static constexpr FTRACE_NS::FtraceEventParseFunction g_eventParseFunc##name = func

// Register the proto_encoder parsing function
#define REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(name, func) \
    FTRACE_PARSE_ENTRY_ATTR static const FTRACE_NS::FtraceParseEntry g_eventParseEntry##name = {#name, \
        g_eventParseFunc##name, func}

#endif // SUB_EVENT_PARSER_H
