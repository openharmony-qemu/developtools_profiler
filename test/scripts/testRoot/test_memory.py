#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2024 Huawei Device Co., Ltd.
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

import pytest
import subprocess
import re
import sys
import math
sys.path.append("..")
from tools.utils import *

OUTPUT_PATH = "testModule/output"
LIB_PATH = "/system/lib64/"
BIN_PATH = "/system/bin/"
ETC_PATH = "/system/etc/"
MB_SIZE = 1024
ROM_THRESH = 9800
SIZE_INDEX = 4
FOUR_KB = 4
NO_SUCH_FILE = "No such file or directory"

file_paths = [
    LIB_PATH + "libcpudataplugin.z.so",
    LIB_PATH + "libgpudataplugin.z.so",
    LIB_PATH + "libdiskiodataplugin.z.so",
    LIB_PATH + "libhidumpplugin.z.so",
    LIB_PATH + "libhilogplugin.z.so",
    LIB_PATH + "libhiperfplugin.z.so",
    LIB_PATH + "libhisyseventplugin.z.so",
    LIB_PATH + "libmemdataplugin.z.so",
    LIB_PATH + "libftrace_plugin.z.so",
    LIB_PATH + "libnetworkplugin.z.so",
    LIB_PATH + "libprocessplugin.z.so",
    LIB_PATH + "libxpowerplugin.z.so",
    LIB_PATH + "libnative_hook.z.so",
    LIB_PATH + "libnetwork_profiler.z.so",
    BIN_PATH + "native_daemon",
    BIN_PATH + "hiprofilerd",
    BIN_PATH + "hiprofiler_cmd",
    BIN_PATH + "hiprofiler_plugins",
    BIN_PATH + "ps",
    BIN_PATH + "timestamps",
    ETC_PATH + "init/hiprofiler_daemon.cfg",
    ETC_PATH + "init/hiprofiler_plugins.cfg",
    ETC_PATH + "init/hiprofilerd.cfg",
    ETC_PATH + "param/hiprofiler.para",
    ETC_PATH + "param/hiprofiler.para.dac",
    "/system/framework/hidebug.abc",
    LIB_PATH + "chipset-sdk/libffrt_profiler.z.so",
    LIB_PATH + "libhidebug.so",
    LIB_PATH + "libhidebug_ani.so",
    LIB_PATH + "libhidebug_native.z.so",
    LIB_PATH + "libshared_memory.z.so",
    LIB_PATH + "module/libhidebug.z.so",
    LIB_PATH + "ndk/libohhidebug.so",
    LIB_PATH + "platformsdk/libcj_hidebug_ffi.z.so",
    LIB_PATH + "platformsdk/libnative_daemon_client.z.so"
]


def check_rom(output):
    result = output.split()[SIZE_INDEX]
    multi = False
    if (result[-1] == 'M'):
        multi = True
    if (result[-1] == 'M' or result[-1] == 'K'):
        result = float(result[:-1])
    else:
        return float(FOUR_KB)
    if multi:
        result *= MB_SIZE
    return result


def get_size_result(size):
    ceil_size = math.ceil(size)
    return ((ceil_size + 3) // 4) * 4


class TestHiprofilerRom:
    # 校验rom总值
    @pytest.mark.L0
    def test_rom(self):
        rom_total_size = 0
        for file_path in file_paths:
            get_rom_info = subprocess.check_output(f"hdc shell ls -lh " + file_path, text=True, encoding="utf-8")
            if NO_SUCH_FILE in get_rom_info:
                continue
            get_rom_size = check_rom(get_rom_info)
            if get_rom_size % FOUR_KB != 0:
                get_rom_size = get_size_result(get_rom_size)
            rom_total_size += get_rom_size

        assert (rom_total_size < ROM_THRESH)