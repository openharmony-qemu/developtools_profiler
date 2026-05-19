#include "sub_event_parser.h"

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;

template <typename T>
void tracing_mark_write_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(tracing_mark_write,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_tracing_mark_write_format();
        tracing_mark_write_func(msg, data, size, format);
    });

REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(tracing_mark_write,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_tracing_mark_write_format();
        tracing_mark_write_func(msg, data, size, format);
    });
}
FTRACE_NS_END