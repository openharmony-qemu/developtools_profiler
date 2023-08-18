#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os
import re
import logging


class Common:
    this_file = os.path.basename(__file__)
    logging.basicConfig(
        format="%(asctime)s %(levelname)s %(message)s", level=logging.INFO
    )
    logger = logging.getLogger(this_file)


def enumerate_files(top_dir):
    files = []
    stack = [top_dir]
    while len(stack) > 0:
        current = stack.pop()
        for entry in os.listdir(current):
            if entry.startswith("."):
                continue
            path = os.path.join(current, entry)
            if os.path.isfile(path):
                files.append(path)  # append to result list.
            if os.path.isdir(path):
                stack.append(path)  # continue to do depth first search.
    files.sort()
    return files


class FtraceEvent(object):
    def __init__(self, format_path, event_category, event_name):
        self.format_path = format_path
        self.event_category = event_category
        self.event_name = event_name

    def __str__(self):
        return '{{ P: "{}", C: "{}", N: {} }}'.format(
            self.format_path, self.event_category, self.event_name
        )

    def __repr__(self):
        return self.__str__()


class ProtoType(object):
    INVALID, INTEGER, STRING, BYTES, ARRAY = 0, 1, 2, 3, 4

    def __init__(self, tid, size=0, signed=False):
        self.tid = tid  # type id
        self.size = size
        self.signed = signed

    def __str__(self):
        return "{{ T: {}, SZ: {}, SN: {} }}".format(self.tid, self.size, self.signed)

    def __repr__(self):
        return self.__str__()

    def to_string(self):
        if self.tid != ProtoType.INVALID:
            if self.tid == ProtoType.STRING:
                return "string"
            elif self.tid == ProtoType.BYTES:
                return "string"
            elif self.tid == ProtoType.ARRAY:
                return "repeated uint64"
            elif self.tid == ProtoType.INTEGER:
                t = ""
                if not self.signed:
                    t = "{}{}".format(t, "u")
                if self.size <= 4:
                    t = "{}{}".format(t, "int32")
                elif self.size <= 8:
                    t = "{}{}".format(t, "int64")
                return t


class FtraceEventField(object):
    def __init__(self, event, field, name, offset, size, signed):
        self.event = event
        self.field = field
        self.name = name
        self.offset = offset
        self.size = size
        self.signed = signed

    def __str__(self):
        return '{{ E: {}, F: "{}", N: "{}", O: {}, SZ: {}, SN: {} }}'.format(
            self.event, self.field, self.name, self.offset, self.size, self.signed
        )

    def __repr__(self):
        return self.__str__()

    def to_proto_type_special_case(self):
        if self.event == ("ftrace", "print") and self.name == "buf":
            # ftrace/print: char buf[];
            return ProtoType(ProtoType.STRING)
        if self.size == 0:
            Common.logger.fatal("zero size field {}!".format(self))
        return ProtoType.INVALID

    def is_string_type(self):
        return (
            re.match(r"char\s+\w+\[\d*\]", self.field)
            or re.match(r"char\s*\*\s*\w+", self.field)
            or re.findall(r"char\s*\*\s*", self.field)
            or re.findall(r"char\[\]", self.field)
            or re.findall(r"char\s*\w+\[.*\]", self.field)
            or re.match(r"__u8\s+\w+\[\d+\]", self.field)
            or re.match(r"__u8\s+\w+\[.*]", self.field)
            or re.match(r"u32\s+\w+\[\d+\]", self.field)
        )

    def is_array_type(self):
        return re.match(r"unsigned long\s+\w+\[\d*\]", self.field)

    def is_uintptr_type(self):
        return (
            self.field.startswith("ino_t ")
            or self.field.startswith("i_ino ")
            or self.field.startswith("dev_t ")
            or self.field.startswith("loff_t ")
            or self.field.startswith("sector_t ")
            or self.field.startswith("ext4_fsblk_t ")
            or "long" in self.field
            or "size_t" in self.field
            or "intptr_t" in self.field
        )

    def to_proto_type(self):
        t = self.to_proto_type_special_case()
        if t is not ProtoType.INVALID:
            return t

        if self.is_string_type():
            return ProtoType(ProtoType.STRING)
        if self.is_array_type():
            return ProtoType(ProtoType.ARRAY)
        elif self.is_uintptr_type():
            return ProtoType(ProtoType.INTEGER, 8, False)
        elif 0 < self.size <= 4:
            if self.size in [1, 2, 4]:
                if "*" in self.field:
                    return ProtoType(ProtoType.INTEGER, 8, False)
                return ProtoType(ProtoType.INTEGER, 4, self.signed)
        elif 4 < self.size <= 8:
            if self.size in [8]:
                return ProtoType(ProtoType.INTEGER, 8, self.signed)
        else:
            Common.logger.fatal("can not convert {} to proto type.".format(str(self)))


class FtraceEventFormat(object):
    def __init__(self):
        self.category = ""
        self.path = ""
        self.name = ""
        self.event_id = ""
        self.common_fields = []
        self.remain_fields = []
        self.print_fmt = ""

    def __str__(self):
        return '{{ name: "{}", ID: {}, common: {}, remain: {}" }}'.format(
            self.name, self.event_id, self.common_fields, self.remain_fields
        )

    def __repr__(self):
        return self.__str__()


class FtraceEventFormatParser(object):
    def __init__(self):
        self.text = []
        self.line = None

    def _parse_name(self, type_name):
        if len(type_name) > 0:
            start_idx = type_name.rfind(" ")
            remain = type_name[start_idx + 1 :]
            if "[" not in remain:
                return remain
            return remain[: remain.rfind("[")]

    def _parse_field(self, event):
        fields = [f.strip().split(":") for f in self.line.split(";")]
        if fields[0][0] == "field" and fields[1][0] == "offset":
            type_name, offset = fields[0][1], int(fields[1][1])
            name = self._parse_name(type_name)
            if fields[2][0] == "size" and fields[3][0] == "signed":
                size, signed = int(fields[2][1]), int(fields[3][1]) == 1
                return FtraceEventField(event, type_name, name, offset, size, signed)

    def parse(self, ftrace_info):
        self.text = []
        with open(ftrace_info.format_path) as f:
            self.text = f.readlines()
        event_format = FtraceEventFormat()
        event_format.category = ftrace_info.event_category
        event_format.path = ftrace_info.format_path
        event = (ftrace_info.event_category, ftrace_info.event_name)
        cursor_line = 0
        field_list = []
        while cursor_line < len(self.text):
            self.line = self.text[cursor_line].strip()
            if self.line.startswith("name:"):
                event_format.name = self.line.split(":")[1].strip()
                cursor_line += 1
            elif self.line.startswith("ID:"):
                event_format.event_id = int(self.line.split(":")[1])
                cursor_line += 1
            elif self.line.startswith("format:"):
                field_list = event_format.common_fields
                cursor_line += 1
            elif self.line.startswith("field:"):
                field_list.append(self._parse_field(event))
                cursor_line += 1
            elif self.line.startswith("print fmt:"):
                event_format.print_fmt = self.line[len("print fmt:") :]
                cursor_line += 1
            elif len(self.line) == 0:
                field_list = event_format.remain_fields
                cursor_line += 1
            else:
                Common.logger.warning(
                    "ignore unknow line at {}:{}".format(
                        ftrace_info.format_path, cursor_line
                    )
                )
                cursor_line += 1
        return event_format


class FtraceEventCodeGenerator(object):
    def __init__(self, events_dir, allow_list):
        self.events_dir = events_dir
        self.allow_list = allow_list
        self.output_dir = None
        self.allowed_events = set()
        self.available_events = set()
        self.category_to_info = {}
        self.grouped_event_formats = {}
        self.target_event_list = []
        self.target_event_formats = []
        self.parser = FtraceEventFormatParser()
        self.text = None

    def _load_allow_list(self):
        self.text = []
        with open(self.allow_list) as f:
            self.text = [line.strip() for line in f.readlines()]
        for i in self.text:
            if i.startswith("removed") or i.startswith("#"):
                continue
            class_type = tuple(i.split("/"))  # event class and event type
            if len(class_type) == 2:
                self.allowed_events.add(class_type)

    def _get_platform_event_list(self):
        files = enumerate_files(self.events_dir)
        is_format = lambda p: os.path.basename(p) == "format"
        format_files = list(filter(is_format, files))
        event_names = set()
        for path in format_files:
            parts = path.split(os.path.sep)
            if parts[-1] == "format":
                category_type = tuple(parts[-3:-1])  # category/type
                self.available_events.add(category_type)
                event_category, event_name = parts[-3], parts[-2]
                event_info = FtraceEvent(path, event_category, event_name)
                self.category_to_info[category_type] = event_info
                event_names.add(parts[-2])
        Common.logger.info("platform events: %d", len(self.available_events))
        Common.logger.info("allowed events: %d", len(self.allowed_events))

    def _get_target_event_list(self):
        targets = list(self.allowed_events & self.available_events)
        Common.logger.info("target events: %d", len(targets))
        targets.sort()
        self.target_event_list = [self.category_to_info.get(c) for c in targets]
        Common.logger.info("target_event_list: %d", len(self.target_event_list))

    def _parse_ftrace_formats(self):
        for info in self.target_event_list:
            event_format = self.parser.parse(info)
            event_category = event_format.category
            if event_category not in self.grouped_event_formats:
                self.grouped_event_formats[event_category] = []
            self.grouped_event_formats[event_category].append(event_format)
            self.target_event_formats.append(event_format)

    def generate_code(self):
        print("FATAL: subclass must implements generate_code_files method!")
        os.abort()

    def generate(self, output_dir):
        self.output_dir = output_dir
        self._load_allow_list()
        self._get_platform_event_list()
        self._get_target_event_list()
        self._parse_ftrace_formats()
        self.generate_code()
