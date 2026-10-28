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
struct HrtimerCancelData {
    uint64_t hrtimer {};
};
__attribute__((noinline)) void ParseHrtimerCancelData(HrtimerCancelData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseHrtimerCancel(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    HrtimerCancelData eventData {};
    ParseHrtimerCancelData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_cancel_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_hrtimer(eventData.hrtimer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_cancel, ParseHrtimerCancel<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_cancel, ParseHrtimerCancel<ProtoEncoder::FtraceEvent>);

struct HrtimerExpireEntryData {
    uint64_t hrtimer {};
    int64_t now {};
    uint64_t function {};
};
__attribute__((noinline)) void ParseHrtimerExpireEntryData(
    HrtimerExpireEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.now = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseHrtimerExpireEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    HrtimerExpireEntryData eventData {};
    ParseHrtimerExpireEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_expire_entry_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_now(eventData.now);
    msg->set_function(eventData.function);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_expire_entry, ParseHrtimerExpireEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_expire_entry, ParseHrtimerExpireEntry<ProtoEncoder::FtraceEvent>);

struct HrtimerExpireExitData {
    uint64_t hrtimer {};
};
__attribute__((noinline)) void ParseHrtimerExpireExitData(HrtimerExpireExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseHrtimerExpireExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    HrtimerExpireExitData eventData {};
    ParseHrtimerExpireExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_expire_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_hrtimer(eventData.hrtimer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_expire_exit, ParseHrtimerExpireExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_expire_exit, ParseHrtimerExpireExit<ProtoEncoder::FtraceEvent>);

struct HrtimerInitData {
    uint64_t hrtimer {};
    int32_t clockid {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseHrtimerInitData(HrtimerInitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.clockid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseHrtimerInit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    HrtimerInitData eventData {};
    ParseHrtimerInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_init_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_clockid(eventData.clockid);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_init, ParseHrtimerInit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_init, ParseHrtimerInit<ProtoEncoder::FtraceEvent>);

struct HrtimerStartData {
    uint64_t hrtimer {};
    uint64_t function {};
    int64_t expires {};
    int64_t softexpires {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseHrtimerStartData(HrtimerStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.expires = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
    eventData.softexpires = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseHrtimerStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    HrtimerStartData eventData {};
    ParseHrtimerStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_function(eventData.function);
    msg->set_expires(eventData.expires);
    msg->set_softexpires(eventData.softexpires);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_start, ParseHrtimerStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_start, ParseHrtimerStart<ProtoEncoder::FtraceEvent>);

struct ItimerExpireData {
    int32_t which {};
    int32_t pid {};
    uint64_t now {};
};
__attribute__((noinline)) void ParseItimerExpireData(ItimerExpireData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.which = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.now = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseItimerExpire(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    ItimerExpireData eventData {};
    ParseItimerExpireData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_itimer_expire_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_which(eventData.which);
    msg->set_pid(eventData.pid);
    msg->set_now(eventData.now);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(itimer_expire, ParseItimerExpire<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(itimer_expire, ParseItimerExpire<ProtoEncoder::FtraceEvent>);

struct ItimerStateData {
    int32_t which {};
    uint64_t expires {};
    uint64_t value_sec {};
    uint64_t value_nsec {};
    uint64_t interval_sec {};
    uint64_t interval_nsec {};
};
__attribute__((noinline)) void ParseItimerStateData(ItimerStateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.which = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.expires = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.value_sec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.value_nsec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.interval_sec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.interval_nsec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseItimerState(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    ItimerStateData eventData {};
    ParseItimerStateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_itimer_state_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_which(eventData.which);
    msg->set_expires(eventData.expires);
    msg->set_value_sec(eventData.value_sec);
    msg->set_value_nsec(eventData.value_nsec);
    msg->set_interval_sec(eventData.interval_sec);
    msg->set_interval_nsec(eventData.interval_nsec);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(itimer_state, ParseItimerState<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(itimer_state, ParseItimerState<ProtoEncoder::FtraceEvent>);

struct TimerCancelData {
    uint64_t timer {};
};
__attribute__((noinline)) void ParseTimerCancelData(TimerCancelData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseTimerCancel(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    TimerCancelData eventData {};
    ParseTimerCancelData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_cancel_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_timer(eventData.timer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_cancel, ParseTimerCancel<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_cancel, ParseTimerCancel<ProtoEncoder::FtraceEvent>);

struct TimerExpireEntryData {
    uint64_t timer {};
    uint64_t now {};
    uint64_t function {};
    uint64_t baseclk {};
};
__attribute__((noinline)) void ParseTimerExpireEntryData(TimerExpireEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.now = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.baseclk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseTimerExpireEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    TimerExpireEntryData eventData {};
    ParseTimerExpireEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_expire_entry_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_timer(eventData.timer);
    msg->set_now(eventData.now);
    msg->set_function(eventData.function);
    msg->set_baseclk(eventData.baseclk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_expire_entry, ParseTimerExpireEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_expire_entry, ParseTimerExpireEntry<ProtoEncoder::FtraceEvent>);

struct TimerExpireExitData {
    uint64_t timer {};
};
__attribute__((noinline)) void ParseTimerExpireExitData(TimerExpireExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseTimerExpireExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    TimerExpireExitData eventData {};
    ParseTimerExpireExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_expire_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_timer(eventData.timer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_expire_exit, ParseTimerExpireExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_expire_exit, ParseTimerExpireExit<ProtoEncoder::FtraceEvent>);

struct TimerInitData {
    uint64_t timer {};
};
__attribute__((noinline)) void ParseTimerInitData(TimerInitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseTimerInit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    TimerInitData eventData {};
    ParseTimerInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_init_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_timer(eventData.timer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_init, ParseTimerInit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_init, ParseTimerInit<ProtoEncoder::FtraceEvent>);

struct TimerStartData {
    uint64_t timer {};
    uint64_t function {};
    uint64_t expires {};
    uint64_t now {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseTimerStartData(TimerStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.expires = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.now = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseTimerStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    TimerStartData eventData {};
    ParseTimerStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_timer(eventData.timer);
    msg->set_function(eventData.function);
    msg->set_expires(eventData.expires);
    msg->set_now(eventData.now);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_start, ParseTimerStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_start, ParseTimerStart<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
