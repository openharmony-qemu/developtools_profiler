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

import sys

MIN_DECL_LIST_LEN = 2


def handle_macro(decl_str):
    '''
    # Function:
    #   The declaration string may contain macro,
    #   This function expands it. it's defined as follow:
    #       #ifdef CONFIG_CPU_BIG_ENDIAN
    #       #define arg_u32p(name)	u32, name##_hi, u32, name##_lo
    #       #else
    #       #define arg_u32p(name)	u32, name##_lo, u32, name##_hi
    #       #endif
    # find the macro
    '''
    start = decl_str.find('arg_u32p')
    if start == -1:
        return decl_str
    macro_str = decl_str[start:]
    end = macro_str.find(')')
    end += 1
    macro_str = macro_str[:end]
    macro_str_len = len(macro_str)
    macro_list = macro_str.split('(')
    var_name = macro_list[1]
    var_name = var_name[:-1]
    var_name = var_name.strip()
    expansion = "u64, {}".format(var_name)
    left = decl_str[:start]
    right = decl_str[start + macro_str_len:]
    decl_str = "{}{}{}".format(left, expansion, right)
    return decl_str


def get_decl_str(inf):
    '''
    # Function:
    #   Get the declaration string of system call
    # Arguments:
    #   @inf: input file
    # Return:
    #   Declaration string of system call, e.g.
    #   do_sys_openat2(open, int, dfd, const char __user*, filename, struct open_how *, how)
    '''
    decl_str = inf.readline()
    if decl_str == "":
        print("empty line read from input file")
        return ""
    decl_str = decl_str.strip()
    if decl_str[0] == '"':
        while True:
            line = inf.readline()
            line = line.strip()
            decl_str = "{}{}".format(decl_str, line)
            if (decl_str[-1] == '"'):
                break
        decl_str = decl_str[1:-1]
    return handle_macro(decl_str)


def get_decl_list(decl_str):
    '''
    # Function:
    #   Split declaration string into list
    # Arguments:
    #   @decl_str: system call declaration string, e.g.
    #       do_sys_openat2(open, int, dfd, const char __user*, filename, struct open_how *, how)
    # Return:
    #   Declaration list, e.g.
    #   ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    '''
    decl_list = decl_str.split('(')
    if len(decl_list) < 2:
        print("failed to split %s with '('" % decl_str)
        return []
    probe_entry = decl_list[0]
    probe_entry = probe_entry.strip()
    args_str = decl_list[1]
    args_str = args_str.strip()
    # drop the tailing ')'
    args_str = args_str[:-1]
    args_list = args_str.split(',')
    args_list = [args_item.strip() for args_item in args_list]
    decl_list = []
    decl_list.append(probe_entry)
    decl_list = decl_list + args_list
    return decl_list


def get_args_list(decl_list):
    '''
    # Function:
    #   Truncate the prefix and system call entry name from the declaration list
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   Arguments list of system call, e.g.
    #       ['open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to get arguments list from %s" % decl_list)
        return []
    return decl_list[MIN_DECL_LIST_LEN - 1:]


def gen_kprobe_sec(decl_list):
    '''
    # Function:
    #   Generate the kprobe prog type declaration, e.g. SEC("kprobe/__arm64_sys_open")
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   kprobe prog type declaration, e.g. SEC("kprobe/do_sys_openat2")
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to generate kprobe section from %s" % decl_list)
        return ""
    probe_str = '\nSEC("kprobe/'
    syscall_fn_index = 0
    syscall = decl_list[syscall_fn_index]
    probe_str = '{}{}")\n'.format(probe_str, syscall)
    return probe_str


def gen_kretprobe_sec(decl_list):
    '''
    # Function:
    #   Generate the kretprobe prog type declaration, e.g. SEC("kretprobe/__arm64_sys_open")
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   kretprobe prog type declaration, e.g. SEC("kretprobe/do_sys_openat2")
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to generate kretprobe section from %s" % decl_list)
        return ""
    probe_str = '\nSEC("kretprobe/'
    syscall_fn_index = 0
    syscall = decl_list[syscall_fn_index]
    probe_str = '{}{}")\n'.format(probe_str, syscall)
    return probe_str


def gen_kprobe_decl(decl_list):
    '''
    # Function:
    #   Generate the kprobe prog declaration, e.g.
    #   int BPF_KPROBE(do_sys_openat2_entry, int dfd, const char* filename, struct open_how* how)
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   kprobe prog declaration, e.g.
    #   int BPF_KPROBE(do_sys_openat2_entry, int dfd, const char __user* filename, struct open_how *how)
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to generate kprobe prog declaration from %s" % decl_list)
        return ""
    prog_decl = "int BPF_KPROBE("
    syscall_fn_index = 0
    prog_name = "{}_entry".format(decl_list[syscall_fn_index])
    prog_decl = "{}{}".format(prog_decl, prog_name)
    index = MIN_DECL_LIST_LEN + 1
    count = 0
    max_nr_args = 5
    while index < len(decl_list):
        if count >= max_nr_args:
            break
        prog_decl = '%s, %s %s' % (prog_decl, decl_list[index - 1], decl_list[index])
        index += 2
        count += 1
    prog_decl = "{})".format(prog_decl)
    return prog_decl


def gen_kretprobe_decl(decl_list):
    '''
    # Function:
    #   Generate the kretprobe prog declaration, e.g.
    #   int BPF_KRETPROBE(__arm64_sys_open_exit, long long ret)
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   kretprobe prog declaration, e.g.
    #   int BPF_KRETPROBE(do_sys_openat2_exit, long long ret)
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to generate kretprobe prog declaration from %s" % decl_list)
        return ""
    prog_decl = "int BPF_KRETPROBE("
    syscall_fn_index = 0
    prog_name = "{}_exit".format(decl_list[syscall_fn_index])
    prog_decl = "{}{}, int64_t retval)".format(prog_decl, prog_name)
    return prog_decl

HEAD_BPF_PROG_CODE = (
'/************************** fstrace BPF progs BEGIN *****************************/')

TAIL_BPF_PROG_CODE = (
'/*************************** fstrace BPF progs END ******************************/\n')

HEAD_COMMON_KPROBE_CODE = R'''
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
    struct start_event_t start_event = {};
    __builtin_memset(&start_event, 0, sizeof(start_event));
    struct fstrace_start_event_t *fs_se = &start_event.fs_se;
    u64 pid_tgid = bpf_get_current_pid_tgid();
    // get timestamp of the start of system call
    fs_se->stime = bpf_ktime_get_ns();
    // get argument of the system call
'''

TAIL_COMMON_KPROBE_CODE = R'''
    // store the start event with pid as key
    int err = (int) bpf_map_update_elem(&start_event_map, &pid_tgid, &start_event, BPF_ANY);
    if (err != 0) {
        BPFLOGE(BPF_TRUE, "failed to store fstrace start event");
        return -1;
    }
    return 0;
}
'''

HEAD_COMMON_KRETPROBE_CODE = R'''
{
    if (check_current_pid(-1, -1) != 0) {
        // not any one of target processes, skip it
        return 0;
    }
'''

TAIL_COMMON_KRETPROBE_CODE = '''
}
'''

HIEBPF_TYPES_HEAD = R'''
#ifndef FSTRACE_TYPES_H
#define FSTRACE_TYPES_H

#include "hiebpf_macros.h"

'''

HIEBPF_TYPES_TAIL = '#endif'


def get_args_type(decl_list):
    '''
    # Function:
    #   Get the arguments struct type, e.g. struct sys_pidfd_open_args_t
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   arguments struct type, e.g. struct sys_open_args_t
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to get arguments type from %s" % decl_list)
        return ""
    args_type_index = 1
    fn_name = decl_list[args_type_index]
    args_type_str = "struct sys_{}_args_t".format(fn_name)
    return args_type_str


def get_arg_variable_name(decl_list):
    '''
    # Function
    #   Get the arguments struct type instance name, e.g. pidfd_open_args
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   the arguments struct type instance name, e.g. open_args
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to get arguments variable name from %s" % decl_list)
        return ""
    args_type_index = 1
    return "{}_args".format(decl_list[args_type_index])


def get_arg_member_index(decl_list):
    '''
    # Function:
    #   Get the member name list of arguments type
    # Arguments:
    #    @decl_list: Declaration list, e.g.
    #       ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    #   member name list of arguments type, e.g. ['pid', 'flags']
    '''
    member_index_list = []
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to get arguments member index list from %s" % decl_list)
        return member_index_list
    index = MIN_DECL_LIST_LEN + 1
    count = 0
    max_nr_args = 5
    while index < len(decl_list):
        if count >= max_nr_args:
            break
        member_index_list.append(index)
        index += 2
        count += 1
    return member_index_list


def gen_kprobe_code(decl_list):
    '''
    # Function:
    #   Generate system call specific code of BPF kprobe prog
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #      ['do_sys_openat2', 'open', 'int', 'dfd', 'const char __user*', 'filename', 'struct open_how *', 'how']
    # Return:
    # system call specific code string of BPF kprobe prog
    '''
    indent = "    "
    args_type = get_args_type(decl_list)
    if args_type == "":
        return ""
    args_var_name = get_arg_variable_name(decl_list)
    if args_var_name == "":
        return ""
    args_type_index = 1
    event_type = "SYS_{}".format(decl_list[args_type_index].upper())
    code_str = "    fs_se->type = {};\n".format(event_type)
    var_decl_str = "{}{}* args = &fs_se->{};\n".format(indent, args_type, args_var_name)
    member_index_list = get_arg_member_index(decl_list)
    assignment_code_str = ""
    count = 0
    max_nr_args = 5
    for index in member_index_list:
        if count >= max_nr_args:
            break
        member_name = decl_list[index]
        assignment_str = "{}args->{} = {};\n".format(indent, member_name, member_name)
        if member_name == "filename":
            assignment_str = "{}{}emit_strtrace_event(fs_se->stime, fs_se->type, filename, FSTRACE);\n".format(
                assignment_str, indent)
        assignment_code_str = "{}{}".format(assignment_code_str, assignment_str)
        count += 1
    code_str = "{}{}{}".format(code_str, var_decl_str, assignment_code_str)
    return code_str


def gen_kretprobe_code(decl_list):
    '''
    # Function:
    #   Generate system call specific code of BPF kretprobe prog
    # Arguments:
    #   @decl_list: Declaration list, e.g.
    #      ['yes', '__arm64_sys_pidfd_open', 'pidfd_open', 'pid_t', 'pid', 'unsigned int', 'flags']
    # Return:
    # system call specific code string of BPF kretprobe prog
    '''
    if len(decl_list) < MIN_DECL_LIST_LEN:
        print("failed to get system call type from %s" % decl_list)
        return ""
    fn_name = decl_list[2]
    tracer = "FSTRACE"
    return "    return emit_event(ctx, retval, {});".format(tracer)


def underscore_int_types(arg_type):
    # underscore u8
    arg_type = arg_type.strip()
    arg_type_list = arg_type.split(' ')
    for index , arg_item in enumerate(arg_type_list):
        if arg_item == "u8":
            arg_type_list[index] = "__u8"
            break
        if arg_item == "u16":
            arg_type_list[index] = "__u16"
            break
        if arg_item == "u32":
            arg_type_list[index] = "__u32"
            break
        if arg_item == "u64":
            arg_type_list[index] = "__u64"
            break
        if arg_item == "s8":
            arg_type_list[index] = "__s8"
            break
        if arg_item == "s16":
            arg_type_list[index] = "__s16"
            break
        if arg_item == "s32":
            arg_type_list[index] = "__s32"
            break
        if arg_item == "s64":
            arg_type_list[index] = "__s64"
            break
    arg_type = arg_type_list[0]
    for index, arg_item in enumerate(arg_type_list):
        if index == 0:
            continue
        arg_type = "%s %s" % (arg_type, arg_type_list[index])
    return arg_type


def gen_struct_str(args_list):
    '''
    # Function:
    #   Generate arguments type struct, e.g.
    #   struct sys_open_args_t {
    #       const char __user* filename;
    #       int flags;
    #       umode_t mode;
    #   }
    # Arguments:
    #   @args_list: arguments list of system call, e.g.
    #       ['do_sys_openat2', 'int', 'dfd', 'unsigned int', 'flags']
    # Return:
    #   arguments type struct, e.g.
    #   struct sys_open_args_t {
    #       const char __user* filename;
    #       int flags;
    #       umode_t mode;
    #   }
    '''
    length = len(args_list)
    if length < 1:
        return ""
    result = "struct "
    index = 0
    while index < length:
        if index == 0:
            fn_name = args_list[index]
            fn_name = "sys_%s_args_t {\n" % (fn_name)
            result = "{}{}".format(result, fn_name)
            index = index + 1
        else:
            arg_type = underscore_int_types(args_list[index])
            index = index + 1
            if index >= length:
                return ""
            arg_name = args_list[index]
            index = index + 1
            result = "{}    {} {};\n".format(result, arg_type, arg_name)
    result = "%s};\n" % (result)
    return result


def output_fstrace_code(fstrace_progs_file, fstrace_types_file, fstrace_targets_file):
    with open(fstrace_progs_file, 'a') as progs_output_file:
        progs_output_file.write(HEAD_BPF_PROG_CODE)
        with open(fstrace_types_file, 'a') as args_output_file:
            args_output_file.write(HIEBPF_TYPES_HEAD)
            with open(fstrace_targets_file, 'r') as inf:
                type_defs_set = set()
                fn_names_set = set()
                start_event_def = (
                    "struct fstrace_start_event_t {\n"
                    "    __u32 type;\n"
                    "    __u64 stime;\n"
                    "    union {\n")
                start_event_def_end = "    };\n};\n\n"
                arg_types_enum_def = "enum FSTraceEventType:__u32 {\n"
                arg_types_enum_def_end = "};\n"
                count = 1
                while True:
                    # get declaration list, arguments list and arguments definition string
                    decl_str = get_decl_str(inf)
                    if decl_str == "":
                        break
                    decl_list = get_decl_list(decl_str)
                    args_list = get_args_list(decl_list)
                    type_def_str = gen_struct_str(args_list)
                    fn_name = args_list[0]
                    # save arguments type definitions
                    if type_def_str not in type_defs_set:
                        type_defs_set.add(type_def_str)
                        type_def_lines = type_def_str.split('\n')
                        max_type_def_lines = 7
                        type_def_lines_reduced = type_def_lines
                        if len(type_def_lines) > max_type_def_lines:
                            type_def_lines_reduced = type_def_lines[:(max_type_def_lines - 1)]
                            type_def_lines_reduced.append("};\n")
                        type_def_str_reduced = ""
                        for type_def_line in type_def_lines_reduced:
                            type_def_str_reduced = "{}{}\n".format(type_def_str_reduced, type_def_line)
                        args_output_file.write(type_def_str_reduced)
                        start_event_def  = "{}        struct sys_{}_args_t {}_args;\n".format(start_event_def,
                                                                                              fn_name, fn_name)
                        if count == 1:
                            arg_type = "SYS_{} = 1".format(fn_name.upper())
                            count = 0
                        else:
                            arg_type = "SYS_{}".format(fn_name.upper())
                        arg_types_enum_def = "{}    {},\n".format(arg_types_enum_def, arg_type)

                    # generate kprobe prog
                    kprobe_sec = gen_kprobe_sec(decl_list)
                    krpobe_decl = gen_kprobe_decl(decl_list)
                    kprobe_code = gen_kprobe_code(decl_list)

                    # save kprobe prog
                    progs_output_file.write(kprobe_sec)
                    progs_output_file.write(krpobe_decl)
                    progs_output_file.write(HEAD_COMMON_KPROBE_CODE)
                    progs_output_file.write(kprobe_code)
                    progs_output_file.write(TAIL_COMMON_KPROBE_CODE)

                    # generate kretprobe prog
                    kretprobe_sec = gen_kretprobe_sec(decl_list)
                    kretprobe_decl = gen_kretprobe_decl(decl_list)
                    kretprobe_code = gen_kretprobe_code(decl_list)

                    # save kretprobe prog
                    progs_output_file.write(kretprobe_sec)
                    progs_output_file.write(kretprobe_decl)
                    progs_output_file.write(HEAD_COMMON_KRETPROBE_CODE)
                    progs_output_file.write(kretprobe_code)
                    progs_output_file.write(TAIL_COMMON_KRETPROBE_CODE)
            start_event_def = "{}{}".format(start_event_def, start_event_def_end)
            arg_types_enum_def = "{}{}".format(arg_types_enum_def, arg_types_enum_def_end)
            args_output_file.write(start_event_def)
            args_output_file.write(arg_types_enum_def)
            args_output_file.write(HIEBPF_TYPES_TAIL)
        progs_output_file.write(TAIL_BPF_PROG_CODE)
        progs_output_file.write("\n\n")
    return
