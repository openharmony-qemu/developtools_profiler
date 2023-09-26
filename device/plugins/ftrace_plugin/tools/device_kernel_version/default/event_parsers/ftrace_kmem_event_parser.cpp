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
using namespace OHOS::Developtools::Profiler;
template <typename T> void kfree_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kfree,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kfree_format();
        if (msg != nullptr) {
            kfree_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kfree,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kfree_format();
        if (msg != nullptr) {
            kfree_func(msg, data, size, format);
        }
    });

template <typename T> void kmalloc_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmalloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmalloc_format();
        if (msg != nullptr) {
            kmalloc_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmalloc,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmalloc_format();
        if (msg != nullptr) {
            kmalloc_func(msg, data, size, format);
        }
    });

template <typename T> void kmalloc_node_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_node(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmalloc_node,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmalloc_node_format();
        if (msg != nullptr) {
            kmalloc_node_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmalloc_node,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmalloc_node_format();
        if (msg != nullptr) {
            kmalloc_node_func(msg, data, size, format);
        }
    });

template <typename T> void kmem_cache_alloc_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmem_cache_alloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmem_cache_alloc_format();
        if (msg != nullptr) {
            kmem_cache_alloc_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmem_cache_alloc,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmem_cache_alloc_format();
        if (msg != nullptr) {
            kmem_cache_alloc_func(msg, data, size, format);
        }
    });

template <typename T> void kmem_cache_alloc_node_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_req(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_bytes_alloc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_node(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmem_cache_alloc_node,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmem_cache_alloc_node_format();
        if (msg != nullptr) {
            kmem_cache_alloc_node_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmem_cache_alloc_node,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmem_cache_alloc_node_format();
        if (msg != nullptr) {
            kmem_cache_alloc_node_func(msg, data, size, format);
        }
    });

template <typename T> void kmem_cache_free_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_call_site(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmem_cache_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmem_cache_free_format();
        if (msg != nullptr) {
            kmem_cache_free_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmem_cache_free,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kmem_cache_free_format();
        if (msg != nullptr) {
            kmem_cache_free_func(msg, data, size, format);
        }
    });

template <typename T> void mm_page_alloc_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_alloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_alloc_format();
        if (msg != nullptr) {
            mm_page_alloc_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_alloc,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_alloc_format();
        if (msg != nullptr) {
            mm_page_alloc_func(msg, data, size, format);
        }
    });

template <typename T> void mm_page_alloc_extfrag_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_alloc_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_fallback_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_alloc_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_fallback_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_change_ownership(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_alloc_extfrag,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_alloc_extfrag_format();
        if (msg != nullptr) {
            mm_page_alloc_extfrag_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_alloc_extfrag,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_alloc_extfrag_format();
        if (msg != nullptr) {
            mm_page_alloc_extfrag_func(msg, data, size, format);
        }
    });

template <typename T>
void mm_page_alloc_zone_locked_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_alloc_zone_locked,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_alloc_zone_locked_format();
        if (msg != nullptr) {
            mm_page_alloc_zone_locked_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_alloc_zone_locked,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_alloc_zone_locked_format();
        if (msg != nullptr) {
            mm_page_alloc_zone_locked_func(msg, data, size, format);
        }
    });

template <typename T> void mm_page_free_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_free_format();
        if (msg != nullptr) {
            mm_page_free_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_free,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_free_format();
        if (msg != nullptr) {
            mm_page_free_func(msg, data, size, format);
        }
    });

template <typename T> void mm_page_free_batched_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_free_batched,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_free_batched_format();
        if (msg != nullptr) {
            mm_page_free_batched_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_free_batched,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_free_batched_format();
        if (msg != nullptr) {
            mm_page_free_batched_func(msg, data, size, format);
        }
    });

template <typename T> void mm_page_pcpu_drain_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_migratetype(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_pcpu_drain,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_pcpu_drain_format();
        if (msg != nullptr) {
            mm_page_pcpu_drain_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_pcpu_drain,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_page_pcpu_drain_format();
        if (msg != nullptr) {
            mm_page_pcpu_drain_func(msg, data, size, format);
        }
    });

template <typename T> void rss_stat_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_mm_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_curr(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_member(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rss_stat,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_rss_stat_format();
        if (msg != nullptr) {
            rss_stat_func(msg, data, size, format);
        }
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rss_stat,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_rss_stat_format();
        if (msg != nullptr) {
            rss_stat_func(msg, data, size, format);
        }
    });
} // namespace
FTRACE_NS_END
