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
    kfree,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kfree_format();
        msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    kmalloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kmalloc_format();
        msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    kmalloc_node,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kmalloc_node_format();
        msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_node(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    kmem_cache_alloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kmem_cache_alloc_format();
        msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    kmem_cache_alloc_node,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kmem_cache_alloc_node_format();
        msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_node(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    kmem_cache_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kmem_cache_free_format();
        msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_page_alloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_page_alloc_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_page_alloc_extfrag,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_page_alloc_extfrag_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_alloc_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_fallback_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_alloc_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_fallback_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_change_ownership(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_page_alloc_zone_locked,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_page_alloc_zone_locked_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_page_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_page_free_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_page_free_batched,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_page_free_batched_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mm_page_pcpu_drain,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mm_page_pcpu_drain_format();
        msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rss_stat,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rss_stat_format();
        msg->set_mm_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_curr(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_member(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
