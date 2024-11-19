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

import os
import pytest
import subprocess
import sys
import threading
import time
from string import Template
sys.path.append("..")
from tools.utils import get_pid_by_process_name


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
                        '  max_stack_depth: 8               \n'
                        '  process_name: "${s2}"             \n'
                        '  string_compressed: true           \n'
                        '  fp_unwind: false                  \n'
                        '  blocked: true                     \n'
                        '  callframe_compress: true          \n'
                        '  record_accurately: true           \n'
                        '  offline_symbolization: false      \n'
                        '  statistics_interval: ${s1}        \n'
                        '  startup_mode: true                \n'
                        '  js_stack_report: 1                \n'
                        '  max_js_stack_depth: 2             \n'
                        ' }                                  \n'
                        '}                                   \n')


def get_file_size(file_path):
    size = os.path.getsize(file_path)
    return size


def task_nativehook_dwarf():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_nativehook_dwarf.txt -o /data/local/tmp/test_nativehook_dwarf.htrace -t 60 -s -k"')


def write_str_file(file_path, large_string):
    lines = large_string.split('\n')
    with os.fdopen(os.open(file_path, os.O_RDWR | os.O_CREAT), 'w') as file:
        for line in lines:
            file.write(line + '\n')


#检查进程是否离线
def check_process_offline():
    count = 0
    while (count < 5):
        pid_profiler = get_pid_by_process_name("hiprofilerd")
        pid_plugin = get_pid_by_process_name("hiprofiler_plugins")
        pid_daemon = get_pid_by_process_name("native_daemon")
        assert(pid_profiler > 0)
        assert(pid_plugin > 0)
        assert(pid_daemon > 0)
        time.sleep(10)
        count = count + 1


def nativehook_dwarf(statistics_int, ability_name, bundle_name):
    #获得该应用的进程PID
    pid_text = subprocess.run(f"hdc shell pidof '{bundle_name}'", stdout=subprocess.PIPE, text=True, check=True)
    pidinfo = pid_text.stdout
    if pidinfo.strip() != "":
        subprocess.check_output(f"hdc shell kill " + pidinfo.strip(), text=True, encoding="utf-8")
    #删除cppcrash
    subprocess.check_output(f"hdc shell rm -f /data/log/faultlog/faultlogger/cppcrash-*", text=True, encoding="utf-8")
    #dwarf 统计模式
    vmfile = file_content.safe_substitute(s1=statistics_int, s2=bundle_name)
    #写入文件
    write_str_file("./../inputfiles/config_nativehook_dwarf.txt", vmfile)

    subprocess.check_output(f"hdc file send ..\\inputfiles\\native_hook_32bit\\config_nativehook_dwarf.txt /data/local/tmp/",
                            text=True, encoding="utf-8")
    task_thread = threading.Thread(target=task_nativehook_dwarf, args=())
    task_thread.start()
    time.sleep(2)
    check_thread = threading.Thread(target=check_process_offline, args=())
    check_thread.start()
    #打开系统设置的应用com.ohos.settings.MainAbility  com.ohos.settings
    subprocess.check_output(f"hdc shell uitest uiInput keyEvent Home", 
                                text=True, encoding="utf-8")
    time.sleep(1)
    subprocess.check_output(f"hdc shell aa start -a {ability_name} -b {bundle_name}", 
                                text=True, encoding="utf-8")
    time.sleep(2)
    task_thread.join()
    subprocess.run(f'hdc file recv /data/local/tmp/test_nativehook_dwarf.htrace ../outputfiles/',
                     text=True, encoding="utf-8")
    # 检查文件大小
    file_size = get_file_size(f"..\\outputfiles\\test_nativehook_dwarf.htrace")
    assert (file_size > 1024)

    #检查是否存在crash
    output_text = subprocess.run(f'hdc shell "ls /data/log/faultlog/faultlogger"', stdout=subprocess.PIPE, text=True, check=True)
    process_info = output_text.stdout
    lines = process_info.strip().split('\n')
    check_crash = False
    for line in lines:
        if line.find("profiler") != -1 or line.find("native_daemon") != -1:
            check_crash = True
            break
    assert (check_crash == False)


class TestHiprofilerMemoryPlugin:
    # 检查32位是否正常
    @pytest.mark.L0
    def test_nativehook_dwarf(self):
        #获得32位还是64位   
        sys_bit = subprocess.run(f"hdc shell getconf LONG_BIT", stdout=subprocess.PIPE, text=True, check=True)
        sysinfo = sys_bit.stdout
        if sysinfo.strip() == "32":
            #非统计模式
            nativehook_dwarf(0, "com.ohos.settings.MainAbility", "com.ohos.settings")
            #统计模式
            nativehook_dwarf(10, "com.ohos.settings.MainAbility", "com.ohos.settings")
        else:
            #非统计模式
            nativehook_dwarf(0, "com.huawei.hmos.settings.MainAbility", "com.huawei.hmos.settings")
            #统计模式
            nativehook_dwarf(10, "com.huawei.hmos.settings.MainAbility", "com.huawei.hmos.settings")