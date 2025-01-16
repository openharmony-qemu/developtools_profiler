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
sys.path.append("..")
from tools.utils import *

OUTPUT_PATH = "testModule/output"
LIB_PATH = "/system/lib64"
MB_SIZE = 1024
ROM_THRESH = 9000
SIZE_INDEX = 4


def check_rom(output):
    result = output.split()[SIZE_INDEX]
    multi = False
    if (result[-1] == 'M'):
        multi = True
    result = float(result[:-1])
    if multi:
        result *= MB_SIZE
    return result
    

class TestHiprofilerRom:
    @pytest.mark.L0
    def test_rom(self):
        # 校验命令行输出
        rom_cpu = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libcpudataplugin*", text=True, encoding="utf-8")
        rom_cpu = check_rom(rom_cpu)

        rom_gpu = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libgpudataplugin*", text=True, encoding="utf-8")
        rom_gpu = check_rom(rom_gpu)

        rom_disk = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libdiskiodataplugin*", text=True, encoding="utf-8")
        rom_disk = check_rom(rom_disk)

        rom_ftrace = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libftrace_plugin*", text=True, encoding="utf-8")
        rom_ftrace = check_rom(rom_ftrace)

        rom_hidump = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libhidumpplugin*", text=True, encoding="utf-8")
        rom_hidump = check_rom(rom_hidump)

        rom_hilog = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libhilogplugin*", text=True, encoding="utf-8")
        rom_hilog = check_rom(rom_hilog)

        rom_hiperf = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libhiperfplugin*", text=True, encoding="utf-8")
        rom_hiperf = check_rom(rom_hiperf)

        rom_hisys = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libhisyseventplugin*", text=True, encoding="utf-8")
        rom_hisys = check_rom(rom_hisys)

        rom_memory = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libmemdataplugin*", text=True, encoding="utf-8")
        rom_memory = check_rom(rom_memory)

        rom_network = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libnetworkplugin*", text=True, encoding="utf-8")
        rom_network = check_rom(rom_network)

        rom_process = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libprocessplugin*", text=True, encoding="utf-8")
        rom_process = check_rom(rom_process)

        rom_xpower = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libxpowerplugin*", text=True, encoding="utf-8")
        rom_xpower = check_rom(rom_xpower)

        rom_hook = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libnative_hook*", text=True, encoding="utf-8")
        rom_hook = check_rom(rom_hook)

        rom_netprofiler = subprocess.check_output(f"hdc shell ls -lh /system/lib64/libnetwork_profiler*", text=True, encoding="utf-8")
        rom_netprofiler = check_rom(rom_netprofiler)

        rom_daemon = subprocess.check_output(f"hdc shell ls -lh /system/bin/native_daemon*", text=True, encoding="utf-8")
        rom_daemon = check_rom(rom_daemon)

        rom_hiprofilerd = subprocess.check_output(f"hdc shell ls -lh /system/bin/hiprofilerd*", text=True, encoding="utf-8")
        rom_hiprofilerd = check_rom(rom_hiprofilerd)

        rom_cmd = subprocess.check_output(f"hdc shell ls -lh /system/bin/hiprofiler_cmd*", text=True, encoding="utf-8")
        rom_cmd = check_rom(rom_cmd)

        rom_plugins = subprocess.check_output(f"hdc shell ls -lh /system/bin/hiprofiler_plugins*", text=True, encoding="utf-8")
        rom_plugins = check_rom(rom_plugins)

        assert (rom_cpu + rom_gpu + rom_disk + rom_ftrace + rom_hidump + rom_hilog + rom_hiperf + rom_hisys + rom_memory + rom_network + rom_process + rom_xpower + rom_hook + rom_daemon + rom_hiprofilerd + rom_cmd + rom_plugins + rom_netprofiler < ROM_THRESH)