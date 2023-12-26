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
#include <cinttypes>

#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    v4l2_dqbuf,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_v4l2_dqbuf_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.v4l2_dqbuf_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "v4l2_dqbuf: minor = %d, index = %u, type = %s, bytesused = %u, flags = %s, field = %s, timestamp = "
            "%" PRIu64
            ", timecode = { type = %s, flags = %s, frames = %u, seconds = %u, minutes = %u, hours = %u, userbits = { "
            "%u %u %u %u } }, sequence = %u",
            msg.minor(), msg.index(),
            __print_symbolic(msg.type(), {1, "VIDEO_CAPTURE"}, {2, "VIDEO_OUTPUT"}, {3, "VIDEO_OVERLAY"},
                {4, "VBI_CAPTURE"}, {5, "VBI_OUTPUT"}, {6, "SLICED_VBI_CAPTURE"}, {7, "SLICED_VBI_OUTPUT"},
                {8, "VIDEO_OUTPUT_OVERLAY"}, {9, "VIDEO_CAPTURE_MPLANE"}, {10, "VIDEO_OUTPUT_MPLANE"},
                {11, "SDR_CAPTURE"}, {12, "SDR_OUTPUT"}, {13, "META_CAPTURE"}, {128, "PRIVATE"}),
            msg.bytesused(),
            __print_flags(msg.flags(), "|", {0x00000001, "MAPPED"}, {0x00000002, "QUEUED"}, {0x00000004, "DONE"},
                {0x00000008, "KEYFRAME"}, {0x00000010, "PFRAME"}, {0x00000020, "BFRAME"}, {0x00000040, "ERROR"},
                {0x00000100, "TIMECODE"}, {0x00000400, "PREPARED"}, {0x00000800, "NO_CACHE_INVALIDATE"},
                {0x00001000, "NO_CACHE_CLEAN"}, {0x0000e000, "TIMESTAMP_MASK"}, {0x00000000, "TIMESTAMP_UNKNOWN"},
                {0x00002000, "TIMESTAMP_MONOTONIC"}, {0x00004000, "TIMESTAMP_COPY"}, {0x00100000, "LAST"}),
            __print_symbolic(msg.field(), {0, "ANY"}, {1, "NONE"}, {2, "TOP"}, {3, "BOTTOM"}, {4, "INTERLACED"},
                {5, "SEQ_TB"}, {6, "SEQ_BT"}, {7, "ALTERNATE"}, {8, "INTERLACED_TB"}, {9, "INTERLACED_BT"}),
            msg.timestamp(),
            __print_symbolic(msg.timecode_type(), {1, "24FPS"}, {2, "25FPS"}, {3, "30FPS"}, {4, "50FPS"}, {5, "60FPS"}),
            __print_flags(msg.timecode_flags(), "|", {0x0001, "DROPFRAME"}, {0x0002, "COLORFRAME"},
                {0x0000, "USERBITS_USERDEFINED"}, {0x0008, "USERBITS_8BITCHARS"}),
            msg.timecode_frames(), msg.timecode_seconds(), msg.timecode_minutes(), msg.timecode_hours(),
            msg.timecode_userbits0(), msg.timecode_userbits1(), msg.timecode_userbits2(), msg.timecode_userbits3(),
            msg.sequence());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(v4l2_dqbuf) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    v4l2_qbuf,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_v4l2_qbuf_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.v4l2_qbuf_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "v4l2_qbuf: minor = %d, index = %u, type = %s, bytesused = %u, flags = %s, field = %s, timestamp = %" PRIu64
            ", timecode = { type = %s, flags = %s, frames = %u, seconds = %u, minutes = %u, hours = %u, userbits = { "
            "%u %u %u %u } }, sequence = %u",
            msg.minor(), msg.index(),
            __print_symbolic(msg.type(), {1, "VIDEO_CAPTURE"}, {2, "VIDEO_OUTPUT"}, {3, "VIDEO_OVERLAY"},
                {4, "VBI_CAPTURE"}, {5, "VBI_OUTPUT"}, {6, "SLICED_VBI_CAPTURE"}, {7, "SLICED_VBI_OUTPUT"},
                {8, "VIDEO_OUTPUT_OVERLAY"}, {9, "VIDEO_CAPTURE_MPLANE"}, {10, "VIDEO_OUTPUT_MPLANE"},
                {11, "SDR_CAPTURE"}, {12, "SDR_OUTPUT"}, {13, "META_CAPTURE"}, {128, "PRIVATE"}),
            msg.bytesused(),
            __print_flags(msg.flags(), "|", {0x00000001, "MAPPED"}, {0x00000002, "QUEUED"}, {0x00000004, "DONE"},
                {0x00000008, "KEYFRAME"}, {0x00000010, "PFRAME"}, {0x00000020, "BFRAME"}, {0x00000040, "ERROR"},
                {0x00000100, "TIMECODE"}, {0x00000400, "PREPARED"}, {0x00000800, "NO_CACHE_INVALIDATE"},
                {0x00001000, "NO_CACHE_CLEAN"}, {0x0000e000, "TIMESTAMP_MASK"}, {0x00000000, "TIMESTAMP_UNKNOWN"},
                {0x00002000, "TIMESTAMP_MONOTONIC"}, {0x00004000, "TIMESTAMP_COPY"}, {0x00100000, "LAST"}),
            __print_symbolic(msg.field(), {0, "ANY"}, {1, "NONE"}, {2, "TOP"}, {3, "BOTTOM"}, {4, "INTERLACED"},
                {5, "SEQ_TB"}, {6, "SEQ_BT"}, {7, "ALTERNATE"}, {8, "INTERLACED_TB"}, {9, "INTERLACED_BT"}),
            msg.timestamp(),
            __print_symbolic(msg.timecode_type(), {1, "24FPS"}, {2, "25FPS"}, {3, "30FPS"}, {4, "50FPS"}, {5, "60FPS"}),
            __print_flags(msg.timecode_flags(), "|", {0x0001, "DROPFRAME"}, {0x0002, "COLORFRAME"},
                {0x0000, "USERBITS_USERDEFINED"}, {0x0008, "USERBITS_8BITCHARS"}),
            msg.timecode_frames(), msg.timecode_seconds(), msg.timecode_minutes(), msg.timecode_hours(),
            msg.timecode_userbits0(), msg.timecode_userbits1(), msg.timecode_userbits2(), msg.timecode_userbits3(),
            msg.sequence());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(v4l2_qbuf) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    vb2_v4l2_buf_done,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_vb2_v4l2_buf_done_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.vb2_v4l2_buf_done_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "vb2_v4l2_buf_done: minor=%d flags = %s, field = %s, timestamp = %" PRIu64
            ", timecode = { type = %s, flags = %s, frames = %u, seconds = %u, minutes = %u, hours = %u, userbits = { "
            "%u %u %u %u } }, sequence = %u",
            msg.minor(),
            __print_flags(msg.flags(), "|", {0x00000001, "MAPPED"}, {0x00000002, "QUEUED"}, {0x00000004, "DONE"},
                {0x00000008, "KEYFRAME"}, {0x00000010, "PFRAME"}, {0x00000020, "BFRAME"}, {0x00000040, "ERROR"},
                {0x00000100, "TIMECODE"}, {0x00000400, "PREPARED"}, {0x00000800, "NO_CACHE_INVALIDATE"},
                {0x00001000, "NO_CACHE_CLEAN"}, {0x0000e000, "TIMESTAMP_MASK"}, {0x00000000, "TIMESTAMP_UNKNOWN"},
                {0x00002000, "TIMESTAMP_MONOTONIC"}, {0x00004000, "TIMESTAMP_COPY"}, {0x00100000, "LAST"}),
            __print_symbolic(msg.field(), {0, "ANY"}, {1, "NONE"}, {2, "TOP"}, {3, "BOTTOM"}, {4, "INTERLACED"},
                {5, "SEQ_TB"}, {6, "SEQ_BT"}, {7, "ALTERNATE"}, {8, "INTERLACED_TB"}, {9, "INTERLACED_BT"}),
            msg.timestamp(),
            __print_symbolic(msg.timecode_type(), {1, "24FPS"}, {2, "25FPS"}, {3, "30FPS"}, {4, "50FPS"}, {5, "60FPS"}),
            __print_flags(msg.timecode_flags(), "|", {0x0001, "DROPFRAME"}, {0x0002, "COLORFRAME"},
                {0x0000, "USERBITS_USERDEFINED"}, {0x0008, "USERBITS_8BITCHARS"}),
            msg.timecode_frames(), msg.timecode_seconds(), msg.timecode_minutes(), msg.timecode_hours(),
            msg.timecode_userbits0(), msg.timecode_userbits1(), msg.timecode_userbits2(), msg.timecode_userbits3(),
            msg.sequence());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(vb2_v4l2_buf_done) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    vb2_v4l2_buf_queue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_vb2_v4l2_buf_queue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.vb2_v4l2_buf_queue_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "vb2_v4l2_buf_queue: minor=%d flags = %s, field = %s, timestamp = %" PRIu64
            ", timecode = { type = %s, flags = %s, frames = %u, seconds = %u, minutes = %u, hours = %u, userbits = { "
            "%u %u %u %u } }, sequence = %u",
            msg.minor(),
            __print_flags(msg.flags(), "|", {0x00000001, "MAPPED"}, {0x00000002, "QUEUED"}, {0x00000004, "DONE"},
                {0x00000008, "KEYFRAME"}, {0x00000010, "PFRAME"}, {0x00000020, "BFRAME"}, {0x00000040, "ERROR"},
                {0x00000100, "TIMECODE"}, {0x00000400, "PREPARED"}, {0x00000800, "NO_CACHE_INVALIDATE"},
                {0x00001000, "NO_CACHE_CLEAN"}, {0x0000e000, "TIMESTAMP_MASK"}, {0x00000000, "TIMESTAMP_UNKNOWN"},
                {0x00002000, "TIMESTAMP_MONOTONIC"}, {0x00004000, "TIMESTAMP_COPY"}, {0x00100000, "LAST"}),
            __print_symbolic(msg.field(), {0, "ANY"}, {1, "NONE"}, {2, "TOP"}, {3, "BOTTOM"}, {4, "INTERLACED"},
                {5, "SEQ_TB"}, {6, "SEQ_BT"}, {7, "ALTERNATE"}, {8, "INTERLACED_TB"}, {9, "INTERLACED_BT"}),
            msg.timestamp(),
            __print_symbolic(msg.timecode_type(), {1, "24FPS"}, {2, "25FPS"}, {3, "30FPS"}, {4, "50FPS"}, {5, "60FPS"}),
            __print_flags(msg.timecode_flags(), "|", {0x0001, "DROPFRAME"}, {0x0002, "COLORFRAME"},
                {0x0000, "USERBITS_USERDEFINED"}, {0x0008, "USERBITS_8BITCHARS"}),
            msg.timecode_frames(), msg.timecode_seconds(), msg.timecode_minutes(), msg.timecode_hours(),
            msg.timecode_userbits0(), msg.timecode_userbits1(), msg.timecode_userbits2(), msg.timecode_userbits3(),
            msg.sequence());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(vb2_v4l2_buf_queue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    vb2_v4l2_dqbuf,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_vb2_v4l2_dqbuf_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.vb2_v4l2_dqbuf_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "vb2_v4l2_dqbuf: minor=%d flags = %s, field = %s, timestamp = %" PRIu64
            ", timecode = { type = %s, flags = %s, frames = %u, seconds = %u, minutes = %u, hours = %u, userbits = { "
            "%u %u %u %u } }, sequence = %u",
            msg.minor(),
            __print_flags(msg.flags(), "|", {0x00000001, "MAPPED"}, {0x00000002, "QUEUED"}, {0x00000004, "DONE"},
                {0x00000008, "KEYFRAME"}, {0x00000010, "PFRAME"}, {0x00000020, "BFRAME"}, {0x00000040, "ERROR"},
                {0x00000100, "TIMECODE"}, {0x00000400, "PREPARED"}, {0x00000800, "NO_CACHE_INVALIDATE"},
                {0x00001000, "NO_CACHE_CLEAN"}, {0x0000e000, "TIMESTAMP_MASK"}, {0x00000000, "TIMESTAMP_UNKNOWN"},
                {0x00002000, "TIMESTAMP_MONOTONIC"}, {0x00004000, "TIMESTAMP_COPY"}, {0x00100000, "LAST"}),
            __print_symbolic(msg.field(), {0, "ANY"}, {1, "NONE"}, {2, "TOP"}, {3, "BOTTOM"}, {4, "INTERLACED"},
                {5, "SEQ_TB"}, {6, "SEQ_BT"}, {7, "ALTERNATE"}, {8, "INTERLACED_TB"}, {9, "INTERLACED_BT"}),
            msg.timestamp(),
            __print_symbolic(msg.timecode_type(), {1, "24FPS"}, {2, "25FPS"}, {3, "30FPS"}, {4, "50FPS"}, {5, "60FPS"}),
            __print_flags(msg.timecode_flags(), "|", {0x0001, "DROPFRAME"}, {0x0002, "COLORFRAME"},
                {0x0000, "USERBITS_USERDEFINED"}, {0x0008, "USERBITS_8BITCHARS"}),
            msg.timecode_frames(), msg.timecode_seconds(), msg.timecode_minutes(), msg.timecode_hours(),
            msg.timecode_userbits0(), msg.timecode_userbits1(), msg.timecode_userbits2(), msg.timecode_userbits3(),
            msg.sequence());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(vb2_v4l2_dqbuf) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    vb2_v4l2_qbuf,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_vb2_v4l2_qbuf_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.vb2_v4l2_qbuf_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "vb2_v4l2_qbuf: minor=%d flags = %s, field = %s, timestamp = %" PRIu64
            ", timecode = { type = %s, flags = %s, frames = %u, seconds = %u, minutes = %u, hours = %u, userbits = { "
            "%u %u %u %u } }, sequence = %u",
            msg.minor(),
            __print_flags(msg.flags(), "|", {0x00000001, "MAPPED"}, {0x00000002, "QUEUED"}, {0x00000004, "DONE"},
                {0x00000008, "KEYFRAME"}, {0x00000010, "PFRAME"}, {0x00000020, "BFRAME"}, {0x00000040, "ERROR"},
                {0x00000100, "TIMECODE"}, {0x00000400, "PREPARED"}, {0x00000800, "NO_CACHE_INVALIDATE"},
                {0x00001000, "NO_CACHE_CLEAN"}, {0x0000e000, "TIMESTAMP_MASK"}, {0x00000000, "TIMESTAMP_UNKNOWN"},
                {0x00002000, "TIMESTAMP_MONOTONIC"}, {0x00004000, "TIMESTAMP_COPY"}, {0x00100000, "LAST"}),
            __print_symbolic(msg.field(), {0, "ANY"}, {1, "NONE"}, {2, "TOP"}, {3, "BOTTOM"}, {4, "INTERLACED"},
                {5, "SEQ_TB"}, {6, "SEQ_BT"}, {7, "ALTERNATE"}, {8, "INTERLACED_TB"}, {9, "INTERLACED_BT"}),
            msg.timestamp(),
            __print_symbolic(msg.timecode_type(), {1, "24FPS"}, {2, "25FPS"}, {3, "30FPS"}, {4, "50FPS"}, {5, "60FPS"}),
            __print_flags(msg.timecode_flags(), "|", {0x0001, "DROPFRAME"}, {0x0002, "COLORFRAME"},
                {0x0000, "USERBITS_USERDEFINED"}, {0x0008, "USERBITS_8BITCHARS"}),
            msg.timecode_frames(), msg.timecode_seconds(), msg.timecode_minutes(), msg.timecode_hours(),
            msg.timecode_userbits0(), msg.timecode_userbits1(), msg.timecode_userbits2(), msg.timecode_userbits3(),
            msg.sequence());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(vb2_v4l2_qbuf) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
