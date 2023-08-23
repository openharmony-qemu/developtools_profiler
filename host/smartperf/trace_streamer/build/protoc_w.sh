#!/bin/bash
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
PROJECT_TOP = os.path.realpath(os.path.join(THIS_DIR, "../../.."))
TAIL_DIR = "developtools/profiler"

if "developtools" in sys.argv[2]:
    TAIL_DIR = "."
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
OPT_OUT = "--opt_out"
OPT_PROTOREADER_OUT = "--cpp_out"
PYTHON_SHELL = os.path.join(THIS_DIR, "make_standard_proto.py")
TMP = sys.argv[2]
PROTO_OUT_DIR = os.path.join(PROJECT_TOP, os.path.dirname(TMP), sys.argv[3])

print("1 =", sys.argv[1])
print("2 =", sys.argv[2])
print("3 =", sys.argv[3])
print("4 =", sys.argv[4])
print("5 =", sys.argv[5])
print("6 =", sys.argv[6])
print("7 =", sys.argv[7])
print("8 =", sys.argv[8])
print("9 =", sys.argv[9])
print("---OPT_PLUGIN_PROTOREADER_PATH =", OPT_PLUGIN_PROTOREADER_PATH)
PARAMS = sys.argv[1:]
print("PARAMS =", PARAMS)
PARAMS_FILTER = " ".join(sys.argv[1:4])
TARFLAG = "--proto_path"
PARAMS_SOURCE_TMP = [arg for arg in PARAMS if arg.startswith(TARFLAG)][0].split()[1:]  # get source proto file path
PARAMS_SOURCES = " ".join(PARAMS_SOURCE_TMP[1:])  # filter the TARFLAG
PARAMS_PRINT = f"{PROTO_OUT_DIR} {PARAMS_SOURCES}"  # splice the python args

subprocess.run(["python3", PYTHON_SHELL, *PARAMS_PRINT.split()])

PARAMS_SRC = " ".join(PARAMS).replace(PARAMS_FILTER, "")
PARAMS_REPLACE = [param.replace(".proto", "_standard.proto") for param in PARAMS_SOURCE_TMP]
PARAMS_STANDARD = f" --proto_path {PROTO_OUT_DIR} "
PARAMS_STANDARD += " ".join([f"{PROTO_OUT_DIR}/{os.path.basename(param)}" for param in PARAMS_REPLACE])
PARAMS_ALL = f"{PARAMS_SRC} {PARAMS_STANDARD}"

print("执行参数：--------------- --------------------------")
if not sys.argv[4].startswith("--plugin"):
    print("执行参数：--------------- -------------------2-------")
    if os.path.isfile(OPT_PLUGIN_PROTOREADER_PATH):
      cmd=[PROTOC, OPT_PLUGIN_PROTOREADER, f"{PLUGINOUT}:{sys.argv[5]}", *PARAMS_ALL.split()]
      print("执行参数：--------------- ", cmd, " --------------------------")
      subprocess.run(cmd)
    #   subprocess.run(cmd, env={"LD_LIBRARY_PATH": f"{LIBCXX_X64_OUT}:{SUBSYS_X64_OUT}"})
subprocess.run([PROTOC, *PARAMS_ALL.split()])
