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
    cgroup_attach_task,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_attach_task_format();
        msg->set_dst_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_destroy_root,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_destroy_root_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_ss_mask(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_freeze,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_freeze_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_mkdir,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_mkdir_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_notify_frozen,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_notify_frozen_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_val(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_notify_populated,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_notify_populated_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_val(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_release,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_release_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_remount,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_remount_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_ss_mask(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_rename,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_rename_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_rmdir,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_rmdir_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_setup_root,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_setup_root_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_ss_mask(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_transfer_tasks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_transfer_tasks_format();
        msg->set_dst_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dst_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    cgroup_unfreeze,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_cgroup_unfreeze_format();
        msg->set_root(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_level(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_path(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
