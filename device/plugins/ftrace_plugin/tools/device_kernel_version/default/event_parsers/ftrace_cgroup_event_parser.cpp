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
template <typename FtraceEventType>
void ParseCgroupAttachTask(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupAttachTaskData eventData {};
    ParseCgroupAttachTaskData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_attach_task_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dst_root(eventData.dst_root);
    msg->set_dst_id(eventData.dst_id);
    msg->set_dst_level(eventData.dst_level);
    msg->set_pid(eventData.pid);
    msg->set_dst_path(eventData.dst_path);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_attach_task, ParseCgroupAttachTask<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_attach_task, ParseCgroupAttachTask<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupDestroyRoot(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupDestroyRootData eventData {};
    ParseCgroupDestroyRootData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_destroy_root_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_destroy_root, ParseCgroupDestroyRoot<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_destroy_root, ParseCgroupDestroyRoot<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupFreeze(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupFreezeData eventData {};
    ParseCgroupFreezeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_freeze_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_freeze, ParseCgroupFreeze<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_freeze, ParseCgroupFreeze<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupMkdir(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupMkdirData eventData {};
    ParseCgroupMkdirData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_mkdir_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_mkdir, ParseCgroupMkdir<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_mkdir, ParseCgroupMkdir<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupNotifyFrozen(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupNotifyFrozenData eventData {};
    ParseCgroupNotifyFrozenData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_notify_frozen_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
    msg->set_val(eventData.val);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_notify_frozen, ParseCgroupNotifyFrozen<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_notify_frozen, ParseCgroupNotifyFrozen<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupNotifyPopulated(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupNotifyPopulatedData eventData {};
    ParseCgroupNotifyPopulatedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_notify_populated_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
    msg->set_val(eventData.val);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_notify_populated, ParseCgroupNotifyPopulated<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_notify_populated, ParseCgroupNotifyPopulated<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupRelease(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupReleaseData eventData {};
    ParseCgroupReleaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_release_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_release, ParseCgroupRelease<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_release, ParseCgroupRelease<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupRemount(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupRemountData eventData {};
    ParseCgroupRemountData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_remount_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_remount, ParseCgroupRemount<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_remount, ParseCgroupRemount<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupRename(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupRenameData eventData {};
    ParseCgroupRenameData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_rename_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_rename, ParseCgroupRename<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_rename, ParseCgroupRename<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupRmdir(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupRmdirData eventData {};
    ParseCgroupRmdirData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_rmdir_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_rmdir, ParseCgroupRmdir<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_rmdir, ParseCgroupRmdir<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupSetupRoot(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupSetupRootData eventData {};
    ParseCgroupSetupRootData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_setup_root_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_ss_mask(eventData.ss_mask);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_setup_root, ParseCgroupSetupRoot<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_setup_root, ParseCgroupSetupRoot<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupTransferTasks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupTransferTasksData eventData {};
    ParseCgroupTransferTasksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_transfer_tasks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dst_root(eventData.dst_root);
    msg->set_dst_id(eventData.dst_id);
    msg->set_dst_level(eventData.dst_level);
    msg->set_pid(eventData.pid);
    msg->set_dst_path(eventData.dst_path);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_transfer_tasks, ParseCgroupTransferTasks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_transfer_tasks, ParseCgroupTransferTasks<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseCgroupUnfreeze(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    CgroupUnfreezeData eventData {};
    ParseCgroupUnfreezeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_cgroup_unfreeze_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_root(eventData.root);
    msg->set_id(eventData.id);
    msg->set_level(eventData.level);
    msg->set_path(eventData.path);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(cgroup_unfreeze, ParseCgroupUnfreeze<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(cgroup_unfreeze, ParseCgroupUnfreeze<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
