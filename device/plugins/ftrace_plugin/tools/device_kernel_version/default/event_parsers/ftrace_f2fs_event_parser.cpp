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

FTRACE_NS_BEGIN
namespace {
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    f2fs_sync_file_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_f2fs_sync_file_enter_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nlink(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_advise(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    f2fs_sync_file_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_f2fs_sync_file_exit_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cp_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_datasync(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    f2fs_write_begin,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_f2fs_write_begin_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    f2fs_write_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_f2fs_write_end_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_copied(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
