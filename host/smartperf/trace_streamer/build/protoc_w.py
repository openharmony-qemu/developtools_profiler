#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2023 Huawei Device Co., Ltd.
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
import subprocess
import sys
import platform

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_TOP = os.path.realpath(os.path.join(THIS_DIR, ".."))
TAIL_DIR = "."

if "developtools" in sys.argv[3]:
    TAIL_DIR = "developtools/profiler"
    PROJECT_TOP = os.path.realpath(os.path.join(THIS_DIR, ".."))

OHOS_X64_OUT = os.path.join(PROJECT_TOP, sys.argv[2])
LIBCXX_X64_OUT = os.path.join(PROJECT_TOP, sys.argv[1], "ndk/libcxx/linux_x86_64")
SUBSYS_X64_OUT = os.path.join(PROJECT_TOP, sys.argv[2], TAIL_DIR)
PROTOC_NAME = "protoc"
PROTOCREADER_PLUGIN = "protoreader_plugin"
current_os = platform.system()
if current_os == "Windows":
    PROTOC_NAME = "protoc.exe"
    PROTOCREADER_PLUGIN = "protoreader_plugin.exe"
PROTOC = os.path.join(PROJECT_TOP, sys.argv[2], TAIL_DIR, PROTOC_NAME)
OPT_PLUGIN_PROTOREADER_PATH = os.path.join(PROJECT_TOP, sys.argv[2], TAIL_DIR, PROTOCREADER_PLUGIN)
OPT_PLUGIN_PROTOREADER = "--plugin=protoc-gen-plugin=" + os.path.join(PROJECT_TOP, sys.argv[2], TAIL_DIR, PROTOCREADER_PLUGIN)
PLUGINOUT = "--plugin_out=wrapper_namespace=ProtoReader"

PARAMS = sys.argv[1:]
print("PARAMS =", PARAMS)
PARAMS_FILTER = " ".join(sys.argv[1:4])

PARAMS_SRC = " ".join(PARAMS).replace(PARAMS_FILTER, "")
PARAMS_ALL = f"{PARAMS_SRC}"

if not sys.argv[4].startswith("--plugin"):
    if os.path.isfile(OPT_PLUGIN_PROTOREADER_PATH):
        cmd=[PROTOC, OPT_PLUGIN_PROTOREADER, f"{PLUGINOUT}:{sys.argv[5]}", *PARAMS_ALL.split()]
        print("执行参数：--------------- ", cmd, " --------------------------")
        subprocess.run(cmd)
subprocess.run([PROTOC, *PARAMS_ALL.split()])
