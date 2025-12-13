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
import time
import sys
sys.path.append("..")
from tools.utils import *
import threading
import sqlite3
import datetime
import os
import stat
from hypium import UiDriver, BY
from string import Template
uiconn = UiDriver.connect()
uiconn.find_component(BY.text("11"))

def check_gpu_statistics(lines, gpu_type):
    for line in lines:
        if ("type" in line) and (gpu_type in line) and ("trace" not in line):
            return True
    return False

def check_gpu_non_statistics(lines, gpu_type):
    for line in lines:
        if ("trace_type" in line) and (gpu_type in line):
            return True
    return False

def check_tag_statistics(lines, gpu_type):
    for line in lines:
        if ("tag_name" in line) and (gpu_type in line):
            return True
    return False

def check_tag_non_statistics(lines, gpu_type):
    for line in lines:
        if ("tag_name" in line) and (gpu_type in line):
            return True
    return False


def construct_command(statistics, dwarf, process_name):
    file_content = Template('request_id: 1                       \n'
                            'session_config {                    \n'
                            ' buffers {                          \n'
                            '  pages: 16384                      \n'
                            ' }                                  \n'
                            '}                                   \n'
                            'plugin_configs {                    \n'
                            ' plugin_name: "nativehook"          \n'
                            ' sample_interval: 5000              \n'
                            ' config_data {                      \n'
                            '  save_file: false                  \n'
                            '  smb_pages: 16384                  \n'
                            '  max_stack_depth: 8                \n'
                            '  process_name: "${s1}"             \n'
                            '  string_compressed: true           \n'
                            '  fp_unwind: ${s2}                  \n'
                            '  blocked: false                    \n'
                            '  callframe_compress: true          \n'
                            '  record_accurately: true           \n'
                            '  offline_symbolization: false      \n'
                            '  statistics_interval: ${s3}        \n'
                            '  startup_mode: true                \n'
                            '  js_stack_report: 1                \n'
                            '  max_js_stack_depth: 10            \n'
                            '  memtrace_enable: true             \n'
                            '  restrace_tag: "RES_GPU_VK"        \n'
                            '  restrace_tag: "RES_GPU_GLES_IMAGE"\n'
                            '  restrace_tag: "RES_GPU_GLES_BUFFER"\n'
                            '  restrace_tag: "RES_GPU_CL_IMAGE"  \n'
                            '  restrace_tag: "RES_GPU_CL_BUFFER" \n'
                            ' }                                  \n'
                            '}                                   \n')
    if statistics:
        statistics = 10
    vmfile = file_content.safe_substitute(s1=process_name, s2=(not dwarf), s3=statistics)
    #写入文件
    write_str_file("./../inputfiles/nativehook/config.txt", vmfile)

    subprocess.check_output(f"hdc file send ./../inputfiles/nativehook/config.txt /data/local/tmp/",
                            text=True, encoding="utf-8")
    

def check_result(gpu_type, statistics, process_name, tagname=""):

    task_thread = None
    subprocess.check_output(r"hdc shell killall " + process_name)
    task_thread = threading.Thread(target=task_template, args=(True,))
    time.sleep(3)
    task_thread.start()
    time.sleep(3)
    subprocess.check_output(
        f"hdc shell aa start -a {process_name}.MainAbility -b {process_name}",
        text=True, encoding="utf-8")
    task_thread.join()

    subprocess.check_output(
        r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    subprocess.check_output(
        r"hdc shell chmod 777 /data/local/tmp/hookDecoder", text=True, encoding="utf-8")
    subprocess.check_output(
        "hdc shell " + '"/data/local/tmp/hookDecoder -f /data/local/tmp/test.htrace > ./data/local/tmp/result.txt"'
        , text=True, encoding="utf-8")
    subprocess.check_output(
            r"hdc file recv /data/local/tmp/result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    check_result = False
    with open(r'.\..\outputfiles\result.txt', 'r') as file:
        lines = file.readlines()
        if (tagname == ""):
            if statistics:
                check_result = check_gpu_statistics(lines, gpu_type)
            else:
                check_result = check_gpu_non_statistics(lines, gpu_type)
        else:
            if statistics:
                check_result = check_tag_statistics(lines, gpu_type)
            else:
                check_result = check_tag_non_statistics(lines, gpu_type)
    return check_result


def check_hook_gpu_vk(statistics=False, dwarf=False):
    delete_old_files()
    process_name = "com.ohos.weather"
    construct_command(statistics, dwarf, process_name)
    return check_result("GPU_VK", statistics, process_name)

def check_hook_gpu_gles(statistics=False, dwarf=False):
    delete_old_files()
    process_name = "com.ohos.camera"
    construct_command(statistics, dwarf, process_name)
    return check_result("GPU_GLES", statistics, process_name)

def check_hook_gpu_cl(statistics=False, dwarf=False):
    delete_old_files()
    process_name = "com.ohos.camera"
    construct_command(statistics, dwarf, process_name)
    return check_result("GPU_CL", statistics, process_name)


def check_tagname(tagName, statistics=0):
    delete_old_files()
    process_name = ""
    if (tagName == "GPU_VK"):
        process_name = "com.ohos.weather"
    else:
        process_name = "com.ohos.camera"
    construct_command(statistics, False, process_name)
    return check_result("GPU_CL", statistics, process_name, tagName)


class TestHiprofilerGpuData:
    @pytest.mark.L0
    def test_hook_vk_statistics(self):
        check_hook_gpu_vk(True)

    @pytest.mark.L0
    def test_hook_gles_statistics(self):
        check_hook_gpu_gles(True)

    @pytest.mark.L0
    def test_hook_cl_statistics(self):
        check_hook_gpu_cl(True)

    @pytest.mark.L0
    def test_hook_vk(self):
        check_hook_gpu_vk()

    @pytest.mark.L0
    def test_hook_gles(self):
        check_hook_gpu_gles()

    @pytest.mark.L0
    def test_hook_cl(self):
        check_hook_gpu_cl()

    @pytest.mark.L0
    def test_hook_vk_statistics_dwarf(self):
        check_hook_gpu_vk(True, True)

    @pytest.mark.L0
    def test_hook_gles_statistics_dwarf(self):
        check_hook_gpu_gles(True, True)

    @pytest.mark.L0
    def test_hook_cl_statistics_dwarf(self):
        check_hook_gpu_cl(True, True)

    @pytest.mark.L0
    def test_hook_vk_dwarf(self):
        check_hook_gpu_vk(False, True)

    @pytest.mark.L0
    def test_hook_gles_dwarf(self):
        check_hook_gpu_gles(False, True)

    @pytest.mark.L0
    def test_hook_cl_dwarf(self):
        check_hook_gpu_cl(False, True)

    @pytest.mark.L0
    def test_tagname_vk(self):
        test_tagname("GPU_VK")

    @pytest.mark.L0
    def test_tagname_gles(self):
        test_tagname("GPU_GLES")

    @pytest.mark.L0
    def test_tagname_cl(self):
        test_tagname("GPU_CL")

