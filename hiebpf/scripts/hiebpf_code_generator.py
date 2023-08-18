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

import os
import sys
import getopt
import fstrace_code_generator as fstrace

FSTRACE_PROGS_FILE = "../include/fstrace_progs.h"
FSTRACE_TYPES_FILE = "../include/fstrace_types.h"
FSTRACE_TARGETS_FILE = "../data/fstrace_target_funcs.txt"
OVERWRITE = False
LICENSE_HEAD = '''/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

'''

if __name__ == '__main__':
    opt_list = [
        'progs-out-file=',
        'fstrace-out-file=',
        'fstrace-targets-file=',
        'overwrite=',
    ]
    opt_names = [
        '--progs-out-file',
        '--fstrace-out-file',
        '--fstrace-targets-file',
        '--overwrite',
    ]
    opts, args = getopt.getopt(sys.argv[1:], '', opt_list)
    for opt, val in opts:
        if opt == opt_names[0]:
            FSTRACE_PROGS_FILE = val
        if opt == opt_names[1]:
            FSTRACE_TYPES_FILE = val
        if opt == opt_names[2]:
            FSTRACE_TARGETS_FILE = val
        if opt == opt_names[3] and val == "true":
            OVERWRITE = True

    PROGS_FILE_EXISTS = os.path.exists(FSTRACE_PROGS_FILE)
    TYPES_FILE_EXISTS = os.path.exists(FSTRACE_TYPES_FILE)
    FILE_EXISTS = PROGS_FILE_EXISTS or TYPES_FILE_EXISTS
    if OVERWRITE or not FILE_EXISTS:
        with open(FSTRACE_PROGS_FILE, 'w') as progs_ouf:
            HEAD_LINE = '/* This line is generated to truncate the original file */\n'
            progs_ouf.write(LICENSE_HEAD)
            progs_ouf.write(HEAD_LINE)
        with open(FSTRACE_TYPES_FILE, 'w') as fstrace_ouf:
            HEAD_LINE = '/* This line is generated to truncate the original file */\n'
            fstrace_ouf.write(LICENSE_HEAD)
            fstrace_ouf.write(HEAD_LINE)
        fstrace.output_fstrace_code(FSTRACE_PROGS_FILE, FSTRACE_TYPES_FILE, FSTRACE_TARGETS_FILE)