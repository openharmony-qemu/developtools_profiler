#!/usr/bin/env python
# -*- coding: utf-8 -*-
#   Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

def get_type_def(file):
    offset = file.tell()
    file.seek(0, 2)
    file_size = file.tell()
    file.seek(offset, 0)
    startline = ""
    type_def_lines = []
    while file.tell() < file_size:
        startline = file.readline()
        index = startline.find("struct sys")
        if index == 0:
            type_def_lines.append(startline)
            while True:
                line = file.readline()
                line = line.strip()
                type_def_lines.append(line)
                if line == "};":
                    break
            break
    return type_def_lines


def get_args_name(type_def_lines):
    startline = type_def_lines[0]
    splitted_startline = startline.split(' ')
    type_name = splitted_startline[1]
    return type_name[4:-2]


def get_converter_name(type_def_lines):
    startline = type_def_lines[0]
    splitted_startline = startline.split(' ')
    type_name = splitted_startline[1]
    splitted_type_name = type_name.split('_')
    splitted_type_name = splitted_type_name[1:-1]
    converter_name = ""
    for item in splitted_type_name:
        converter_name = "{}{}".format(converter_name, item.capitalize()
    converter_name = "{}Converter".format(converter_name)
    return converter_name


BAD_MEMBER = "bad nr"
VOID_MEMBER = "void"


def get_argmember_name(type_def_lines, nr_member):
    max_nr_members = len(type_def_lines) - 2
    max_nr_members_limit = 4
    if max_nr_members > max_nr_members_limit:
        max_nr_members = max_nr_members_limit
    if nr_member < 1:
        return BAD_MEMBER
    if max_nr_members < nr_member and nr_member <= max_nr_members_limit:
        return VOID_MEMBER
    if max_nr_members_limit < nr_member:
        return BAD_MEMBER
    member_line = type_def_lines[nr_member]
    splitted_member_line = member_line.split(' ')
    member_name = splitted_member_line[-1]
    member_name = member_name[:-1]
    return member_name

converter_table = []


def generate_converter(type_def_lines):
    fn_name = get_converter_name(type_def_lines)
    fn_decl = "static int {}(__u64* args, const struct fstrace_start_event_t* start_event)\n".format(fn_name)
    max_nr_args = 4
    expressions = ""
    for nr in range(max_nr_args):
        expression_line = ""
        arg_name = get_args_name(type_def_lines)
        member_name = get_argmember_name(type_def_lines, nr + 1)
        if member_name == BAD_MEMBER:
            break
        if member_name == VOID_MEMBER:
            expression_line = "    args[{}] = 0xFFFFFFFFFFFFFFFF;\n".format(str(nr))
        else:
            expression_line = "    args[{}] = (__u64) start_event->{}.{};\n".format(str(nr), arg_name, member_name)
        expressions.join(expression_line)
    converter = "{}{\n{}    return 0;\n}\n\n".format(fn_decl, expressions)
    converter_table.append(("&" + fn_name))
    return converter

CONVERTER_FILE_HEAD = '''
#include "type_headers.h"


'''


def output_converter_code(fstrace_types_file, converters_file):
    nr_converters = 1
    with open(converters_file, 'w') as outf:
        outf.write(CONVERTER_FILE_HEAD)
        with open(fstrace_types_file, 'r') as inf:
            while True:
                type_def_lines = get_type_def(inf)
                if len(type_def_lines) == 0:
                    break
                converter = generate_converter(type_def_lines)
                outf.write(converter)
                nr_converters = nr_converters + 1
        converter_table_code = (
            "using ConverterType = int (*) (__u64*, const struct fstrace_start_event_t *);\n")
        converter_table_code = "{}ConverterType g_argsConverterTable[{}] = {\n    nullptr".format(converter_table_code, str(nr_converters))
        for fn_ptr in converter_table:
            converter_table_code = "{},\n    {}".format(converter_table_code, fn_ptr)
        converter_table_code = "%s\n};\n" % (converter_table_code)
        outf.write(converter_table_code)

if __name__ == '__main__':
    output_converter_code("../include/fstrace_types.h", "../src/fstrace_args_converter.cpp")
