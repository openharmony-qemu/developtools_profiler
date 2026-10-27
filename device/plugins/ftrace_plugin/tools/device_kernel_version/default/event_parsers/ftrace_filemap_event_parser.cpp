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
struct FileCheckAndAdvanceWbErrData {
    uint64_t file {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint32_t old {};
    uint32_t seq_new {};
};
__attribute__((noinline)) void ParseFileCheckAndAdvanceWbErrData(FileCheckAndAdvanceWbErrData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.file = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.old = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seq_new = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseFileCheckAndAdvanceWbErr(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FileCheckAndAdvanceWbErrData eventData {};
    ParseFileCheckAndAdvanceWbErrData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_file_check_and_advance_wb_err_format();
    msg->set_file(eventData.file);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_old(eventData.old);
    msg->set_seq_new(eventData.seq_new);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(file_check_and_advance_wb_err, ParseFileCheckAndAdvanceWbErr<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(file_check_and_advance_wb_err, ParseFileCheckAndAdvanceWbErr<ProtoEncoder::FtraceEvent>);

struct FilemapSetWbErrData {
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint32_t errseq {};
};
__attribute__((noinline)) void ParseFilemapSetWbErrData(FilemapSetWbErrData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.errseq = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseFilemapSetWbErr(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FilemapSetWbErrData eventData {};
    ParseFilemapSetWbErrData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_filemap_set_wb_err_format();
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_errseq(eventData.errseq);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(filemap_set_wb_err, ParseFilemapSetWbErr<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(filemap_set_wb_err, ParseFilemapSetWbErr<ProtoEncoder::FtraceEvent>);

struct MmFilemapAddToPageCacheData {
    uint64_t pfn {};
    uint64_t i_ino {};
    uint64_t index {};
    uint64_t s_dev {};
};
__attribute__((noinline)) void ParseMmFilemapAddToPageCacheData(MmFilemapAddToPageCacheData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmFilemapAddToPageCache(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmFilemapAddToPageCacheData eventData {};
    ParseMmFilemapAddToPageCacheData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_filemap_add_to_page_cache_format();
    msg->set_pfn(eventData.pfn);
    msg->set_i_ino(eventData.i_ino);
    msg->set_index(eventData.index);
    msg->set_s_dev(eventData.s_dev);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_filemap_add_to_page_cache, ParseMmFilemapAddToPageCache<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_filemap_add_to_page_cache, ParseMmFilemapAddToPageCache<ProtoEncoder::FtraceEvent>);

struct MmFilemapDeleteFromPageCacheData {
    uint64_t pfn {};
    uint64_t i_ino {};
    uint64_t index {};
    uint64_t s_dev {};
};
__attribute__((noinline)) void ParseMmFilemapDeleteFromPageCacheData(MmFilemapDeleteFromPageCacheData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmFilemapDeleteFromPageCache(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmFilemapDeleteFromPageCacheData eventData {};
    ParseMmFilemapDeleteFromPageCacheData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_filemap_delete_from_page_cache_format();
    msg->set_pfn(eventData.pfn);
    msg->set_i_ino(eventData.i_ino);
    msg->set_index(eventData.index);
    msg->set_s_dev(eventData.s_dev);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_filemap_delete_from_page_cache, ParseMmFilemapDeleteFromPageCache<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_filemap_delete_from_page_cache, ParseMmFilemapDeleteFromPageCache<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
