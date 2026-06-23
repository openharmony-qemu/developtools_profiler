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
void ParseHrtimerCancel(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerCancelData eventData {};
    ParseHrtimerCancelData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_cancel_format();
    msg->set_hrtimer(eventData.hrtimer);
}
void ParseHrtimerCancelEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerCancelData eventData {};
    ParseHrtimerCancelData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_cancel_format();
    msg->set_hrtimer(eventData.hrtimer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_cancel, ParseHrtimerCancel);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_cancel, ParseHrtimerCancelEncoder);

struct HrtimerExpireEntryData {
    uint64_t hrtimer {};
    int64_t now {};
    uint64_t function {};
};
__attribute__((noinline)) void ParseHrtimerExpireEntryData(HrtimerExpireEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.now = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseHrtimerExpireEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerExpireEntryData eventData {};
    ParseHrtimerExpireEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_expire_entry_format();
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_now(eventData.now);
    msg->set_function(eventData.function);
}
void ParseHrtimerExpireEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerExpireEntryData eventData {};
    ParseHrtimerExpireEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_expire_entry_format();
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_now(eventData.now);
    msg->set_function(eventData.function);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_expire_entry, ParseHrtimerExpireEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_expire_entry, ParseHrtimerExpireEntryEncoder);

struct HrtimerExpireExitData {
    uint64_t hrtimer {};
};
__attribute__((noinline)) void ParseHrtimerExpireExitData(HrtimerExpireExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.hrtimer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseHrtimerExpireExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerExpireExitData eventData {};
    ParseHrtimerExpireExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_expire_exit_format();
    msg->set_hrtimer(eventData.hrtimer);
}
void ParseHrtimerExpireExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerExpireExitData eventData {};
    ParseHrtimerExpireExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_expire_exit_format();
    msg->set_hrtimer(eventData.hrtimer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_expire_exit, ParseHrtimerExpireExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_expire_exit, ParseHrtimerExpireExitEncoder);

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
void ParseHrtimerInit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerInitData eventData {};
    ParseHrtimerInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_init_format();
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_clockid(eventData.clockid);
    msg->set_mode(eventData.mode);
}
void ParseHrtimerInitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerInitData eventData {};
    ParseHrtimerInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_init_format();
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_clockid(eventData.clockid);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_init, ParseHrtimerInit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_init, ParseHrtimerInitEncoder);

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
void ParseHrtimerStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerStartData eventData {};
    ParseHrtimerStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_start_format();
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_function(eventData.function);
    msg->set_expires(eventData.expires);
    msg->set_softexpires(eventData.softexpires);
    msg->set_mode(eventData.mode);
}
void ParseHrtimerStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HrtimerStartData eventData {};
    ParseHrtimerStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hrtimer_start_format();
    msg->set_hrtimer(eventData.hrtimer);
    msg->set_function(eventData.function);
    msg->set_expires(eventData.expires);
    msg->set_softexpires(eventData.softexpires);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hrtimer_start, ParseHrtimerStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hrtimer_start, ParseHrtimerStartEncoder);

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
void ParseItimerExpire(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ItimerExpireData eventData {};
    ParseItimerExpireData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_itimer_expire_format();
    msg->set_which(eventData.which);
    msg->set_pid(eventData.pid);
    msg->set_now(eventData.now);
}
void ParseItimerExpireEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ItimerExpireData eventData {};
    ParseItimerExpireData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_itimer_expire_format();
    msg->set_which(eventData.which);
    msg->set_pid(eventData.pid);
    msg->set_now(eventData.now);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(itimer_expire, ParseItimerExpire);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(itimer_expire, ParseItimerExpireEncoder);

struct ItimerStateData {
    int32_t which {};
    uint64_t expires {};
    uint64_t value_sec {};
    uint64_t value_usec {};
    uint64_t interval_sec {};
    uint64_t interval_usec {};
};
__attribute__((noinline)) void ParseItimerStateData(ItimerStateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.which = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.expires = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.value_sec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.value_usec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.interval_sec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.interval_usec = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseItimerState(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ItimerStateData eventData {};
    ParseItimerStateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_itimer_state_format();
    msg->set_which(eventData.which);
    msg->set_expires(eventData.expires);
    msg->set_value_sec(eventData.value_sec);
    msg->set_value_usec(eventData.value_usec);
    msg->set_interval_sec(eventData.interval_sec);
    msg->set_interval_usec(eventData.interval_usec);
}
void ParseItimerStateEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ItimerStateData eventData {};
    ParseItimerStateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_itimer_state_format();
    msg->set_which(eventData.which);
    msg->set_expires(eventData.expires);
    msg->set_value_sec(eventData.value_sec);
    msg->set_value_usec(eventData.value_usec);
    msg->set_interval_sec(eventData.interval_sec);
    msg->set_interval_usec(eventData.interval_usec);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(itimer_state, ParseItimerState);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(itimer_state, ParseItimerStateEncoder);

struct TimerCancelData {
    uint64_t timer {};
};
__attribute__((noinline)) void ParseTimerCancelData(TimerCancelData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseTimerCancel(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerCancelData eventData {};
    ParseTimerCancelData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_cancel_format();
    msg->set_timer(eventData.timer);
}
void ParseTimerCancelEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerCancelData eventData {};
    ParseTimerCancelData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_cancel_format();
    msg->set_timer(eventData.timer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_cancel, ParseTimerCancel);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_cancel, ParseTimerCancelEncoder);

struct TimerExpireEntryData {
    uint64_t timer {};
    uint64_t now {};
    uint64_t function {};
};
__attribute__((noinline)) void ParseTimerExpireEntryData(TimerExpireEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.now = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.function = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseTimerExpireEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerExpireEntryData eventData {};
    ParseTimerExpireEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_expire_entry_format();
    msg->set_timer(eventData.timer);
    msg->set_now(eventData.now);
    msg->set_function(eventData.function);
}
void ParseTimerExpireEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerExpireEntryData eventData {};
    ParseTimerExpireEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_expire_entry_format();
    msg->set_timer(eventData.timer);
    msg->set_now(eventData.now);
    msg->set_function(eventData.function);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_expire_entry, ParseTimerExpireEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_expire_entry, ParseTimerExpireEntryEncoder);

struct TimerExpireExitData {
    uint64_t timer {};
};
__attribute__((noinline)) void ParseTimerExpireExitData(TimerExpireExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseTimerExpireExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerExpireExitData eventData {};
    ParseTimerExpireExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_expire_exit_format();
    msg->set_timer(eventData.timer);
}
void ParseTimerExpireExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerExpireExitData eventData {};
    ParseTimerExpireExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_expire_exit_format();
    msg->set_timer(eventData.timer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_expire_exit, ParseTimerExpireExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_expire_exit, ParseTimerExpireExitEncoder);

struct TimerInitData {
    uint64_t timer {};
};
__attribute__((noinline)) void ParseTimerInitData(TimerInitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.timer = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseTimerInit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerInitData eventData {};
    ParseTimerInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_init_format();
    msg->set_timer(eventData.timer);
}
void ParseTimerInitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerInitData eventData {};
    ParseTimerInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_init_format();
    msg->set_timer(eventData.timer);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_init, ParseTimerInit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_init, ParseTimerInitEncoder);

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
void ParseTimerStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerStartData eventData {};
    ParseTimerStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_start_format();
    msg->set_timer(eventData.timer);
    msg->set_function(eventData.function);
    msg->set_expires(eventData.expires);
    msg->set_now(eventData.now);
    msg->set_flags(eventData.flags);
}
void ParseTimerStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimerStartData eventData {};
    ParseTimerStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_timer_start_format();
    msg->set_timer(eventData.timer);
    msg->set_function(eventData.function);
    msg->set_expires(eventData.expires);
    msg->set_now(eventData.now);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(timer_start, ParseTimerStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(timer_start, ParseTimerStartEncoder);
} // namespace
FTRACE_NS_END
