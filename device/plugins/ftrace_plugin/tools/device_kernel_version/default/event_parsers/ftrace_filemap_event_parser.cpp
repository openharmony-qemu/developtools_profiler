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
    file_check_and_advance_wb_err,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_file_check_and_advance_wb_err_format();
        msg->set_file(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_old(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_seq_new(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    filemap_set_wb_err,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_filemap_set_wb_err_format();
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_errseq(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_filemap_add_to_page_cache,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_filemap_add_to_page_cache_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_filemap_delete_from_page_cache,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_filemap_delete_from_page_cache_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
