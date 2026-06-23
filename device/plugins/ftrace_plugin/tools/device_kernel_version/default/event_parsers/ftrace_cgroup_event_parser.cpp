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
struct CgroupAttachTaskData {
    int32_t dst_root {};
    int32_t dst_id {};
    int32_t dst_level {};
    int32_t pid {};
    std::string dst_path {};
    std::string comm {};
};
__attribute__((noinline)) void ParseCgroupAttachTaskData(CgroupAttachTaskData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dst_root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupAttachTask(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupAttachTaskData eventData {};
    ParseCgroupAttachTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_attach_task_format();
    msg->set_dst_root(eventData.dst_root);
    msg->set_dst_id(eventData.dst_id);
    msg->set_dst_level(eventData.dst_level);
    msg->set_pid(eventData.pid);
    msg->set_dst_path(eventData.dst_path);
    msg->set_comm(eventData.comm);
}
void ParseCgroupAttachTaskEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupAttachTaskData eventData {};
    ParseCgroupAttachTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_attach_task_format();
    msg->set_dst_root(eventData.dst_root);
    msg->set_dst_id(eventData.dst_id);
    msg->set_dst_level(eventData.dst_level);
    msg->set_pid(eventData.pid);
    msg->set_dst_path(eventData.dst_path);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_attach_task, ParseCgroupAttachTask);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_attach_task, ParseCgroupAttachTaskEncoder);

struct CgroupDestroyRootData {
    int32_t root {};
    uint32_t ss_mask {};
    std::string name {};
};
__attribute__((noinline)) void ParseCgroupDestroyRootData(CgroupDestroyRootData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ss_mask = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupDestroyRoot(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupDestroyRootData eventData {};
    ParseCgroupDestroyRootData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_destroy_root_format();
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
void ParseCgroupDestroyRootEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupDestroyRootData eventData {};
    ParseCgroupDestroyRootData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_destroy_root_format();
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_destroy_root, ParseCgroupDestroyRoot);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_destroy_root, ParseCgroupDestroyRootEncoder);

struct CgroupFreezeData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
};
__attribute__((noinline)) void ParseCgroupFreezeData(CgroupFreezeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupFreeze(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupFreezeData eventData {};
    ParseCgroupFreezeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_freeze_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
void ParseCgroupFreezeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupFreezeData eventData {};
    ParseCgroupFreezeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_freeze_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_freeze, ParseCgroupFreeze);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_freeze, ParseCgroupFreezeEncoder);

struct CgroupMkdirData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
};
__attribute__((noinline)) void ParseCgroupMkdirData(CgroupMkdirData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupMkdir(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupMkdirData eventData {};
    ParseCgroupMkdirData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_mkdir_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
void ParseCgroupMkdirEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupMkdirData eventData {};
    ParseCgroupMkdirData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_mkdir_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_mkdir, ParseCgroupMkdir);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_mkdir, ParseCgroupMkdirEncoder);

struct CgroupNotifyFrozenData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
    int32_t val {};
};
__attribute__((noinline)) void ParseCgroupNotifyFrozenData(CgroupNotifyFrozenData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.val = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseCgroupNotifyFrozen(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupNotifyFrozenData eventData {};
    ParseCgroupNotifyFrozenData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_notify_frozen_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
    msg->set_val(eventData.val);
}
void ParseCgroupNotifyFrozenEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupNotifyFrozenData eventData {};
    ParseCgroupNotifyFrozenData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_notify_frozen_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
    msg->set_val(eventData.val);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_notify_frozen, ParseCgroupNotifyFrozen);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_notify_frozen, ParseCgroupNotifyFrozenEncoder);

struct CgroupNotifyPopulatedData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
    int32_t val {};
};
__attribute__((noinline)) void ParseCgroupNotifyPopulatedData(CgroupNotifyPopulatedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.val = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseCgroupNotifyPopulated(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupNotifyPopulatedData eventData {};
    ParseCgroupNotifyPopulatedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_notify_populated_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
    msg->set_val(eventData.val);
}
void ParseCgroupNotifyPopulatedEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupNotifyPopulatedData eventData {};
    ParseCgroupNotifyPopulatedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_notify_populated_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
    msg->set_val(eventData.val);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_notify_populated, ParseCgroupNotifyPopulated);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_notify_populated, ParseCgroupNotifyPopulatedEncoder);

struct CgroupReleaseData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
};
__attribute__((noinline)) void ParseCgroupReleaseData(CgroupReleaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupRelease(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupReleaseData eventData {};
    ParseCgroupReleaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_release_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
void ParseCgroupReleaseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupReleaseData eventData {};
    ParseCgroupReleaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_release_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_release, ParseCgroupRelease);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_release, ParseCgroupReleaseEncoder);

struct CgroupRemountData {
    int32_t root {};
    uint32_t ss_mask {};
    std::string name {};
};
__attribute__((noinline)) void ParseCgroupRemountData(CgroupRemountData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ss_mask = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupRemount(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupRemountData eventData {};
    ParseCgroupRemountData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_remount_format();
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
void ParseCgroupRemountEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupRemountData eventData {};
    ParseCgroupRemountData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_remount_format();
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_remount, ParseCgroupRemount);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_remount, ParseCgroupRemountEncoder);

struct CgroupRenameData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
};
__attribute__((noinline)) void ParseCgroupRenameData(CgroupRenameData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupRename(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupRenameData eventData {};
    ParseCgroupRenameData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_rename_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
void ParseCgroupRenameEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupRenameData eventData {};
    ParseCgroupRenameData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_rename_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_rename, ParseCgroupRename);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_rename, ParseCgroupRenameEncoder);

struct CgroupRmdirData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
};
__attribute__((noinline)) void ParseCgroupRmdirData(CgroupRmdirData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupRmdir(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupRmdirData eventData {};
    ParseCgroupRmdirData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_rmdir_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
void ParseCgroupRmdirEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupRmdirData eventData {};
    ParseCgroupRmdirData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_rmdir_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_rmdir, ParseCgroupRmdir);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_rmdir, ParseCgroupRmdirEncoder);

struct CgroupSetupRootData {
    int32_t root {};
    uint32_t ss_mask {};
    std::string name {};
};
__attribute__((noinline)) void ParseCgroupSetupRootData(CgroupSetupRootData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ss_mask = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupSetupRoot(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupSetupRootData eventData {};
    ParseCgroupSetupRootData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_setup_root_format();
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
void ParseCgroupSetupRootEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupSetupRootData eventData {};
    ParseCgroupSetupRootData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_setup_root_format();
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_setup_root, ParseCgroupSetupRoot);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_setup_root, ParseCgroupSetupRootEncoder);

struct CgroupTransferTasksData {
    int32_t dst_root {};
    int32_t dst_id {};
    int32_t dst_level {};
    int32_t pid {};
    std::string dst_path {};
    std::string comm {};
};
__attribute__((noinline)) void ParseCgroupTransferTasksData(CgroupTransferTasksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dst_root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dst_path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupTransferTasks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupTransferTasksData eventData {};
    ParseCgroupTransferTasksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_transfer_tasks_format();
    msg->set_dst_root(eventData.dst_root);
    msg->set_dst_id(eventData.dst_id);
    msg->set_dst_level(eventData.dst_level);
    msg->set_pid(eventData.pid);
    msg->set_dst_path(eventData.dst_path);
    msg->set_comm(eventData.comm);
}
void ParseCgroupTransferTasksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupTransferTasksData eventData {};
    ParseCgroupTransferTasksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_transfer_tasks_format();
    msg->set_dst_root(eventData.dst_root);
    msg->set_dst_id(eventData.dst_id);
    msg->set_dst_level(eventData.dst_level);
    msg->set_pid(eventData.pid);
    msg->set_dst_path(eventData.dst_path);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_transfer_tasks, ParseCgroupTransferTasks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_transfer_tasks, ParseCgroupTransferTasksEncoder);

struct CgroupUnfreezeData {
    int32_t root {};
    int32_t id {};
    int32_t level {};
    std::string path {};
};
__attribute__((noinline)) void ParseCgroupUnfreezeData(CgroupUnfreezeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.root = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.level = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.path = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseCgroupUnfreeze(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupUnfreezeData eventData {};
    ParseCgroupUnfreezeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_unfreeze_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
void ParseCgroupUnfreezeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    CgroupUnfreezeData eventData {};
    ParseCgroupUnfreezeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_unfreeze_format();
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_unfreeze, ParseCgroupUnfreeze);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_unfreeze, ParseCgroupUnfreezeEncoder);
} // namespace
FTRACE_NS_END
