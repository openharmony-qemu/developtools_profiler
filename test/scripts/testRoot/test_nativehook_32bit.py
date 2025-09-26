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
import threading
import sqlite3
from string import Template
import os
from tools.utils import run_and_get_output


def get_pid_by_process_name(process_name):
    pid = None
    cmd = f"hdc shell \"pidof {process_name}\""
    try:
        pid = subprocess.check_output(cmd, shell=False, encoding="utf-8", text=True)
        pid = int(pid.strip().split()[0])
    except subprocess.CalledProcessError as e:
        print(f"Command failed: {cmd}\nError: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    return pid


def get_file_size(file_path):
    size = os.path.getsize(file_path)
    return size


def task_nativehook_dwarf():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/inputfiles/nativehook/config_nativehook_dwarf.txt -o /data/local/tmp/test_nativehook_dwarf.htrace -t 60 -s -k"')


def write_str_file(file_path, large_string):
    lines = large_string.split('\n')
    with open(file_path, 'w') as file:
        for line in lines:
            file.write(line + '\n')


#检查进程是否离线
def check_process_offline():
    count = 0
    while (count < 5):
        pid_profiler = get_pid_by_process_name("hiprofilerd")
        pid_plugin = get_pid_by_process_name("hiprofiler_plugins")
        pid_daemon = get_pid_by_process_name("native_daemon")
        assert (pid_profiler > 0)
        assert (pid_plugin > 0)
        assert (pid_daemon > 0)
        time.sleep(10)
        count = count + 1


def hap_op_func(ability_name, bundle_name):
    #打开系统设置的应用com.ohos.settings.MainAbility  com.ohos.settings
    run_and_get_output(f"hdc shell uitest uiInput keyEvent Home", shell=False, 
                                text=True, encoding="utf-8")
    time.sleep(1)
    run_and_get_output(f"hdc shell aa start -a {ability_name} -b {bundle_name}", shell=False, 
                                text=True, encoding="utf-8")
    time.sleep(2)


def hidumper_op_func(ability_name, bundle_name):
    pid_text = subprocess.run(f"hdc shell pidof '{bundle_name}'", stdout=subprocess.PIPE, text=True, check=True)
    pidinfo = pid_text.stdout
    if pidinfo.strip() != "":
        run_and_get_output(f"hdc shell kill " + pidinfo.strip(), shell=False, text=True, encoding="utf-8")
    #拉起hidumper
    run_and_get_output(f"hdc shell hidumper -h", shell=False, text=True, encoding="utf-8")


def hidumper_prepare_func(ability_name, bundle_name):
    pid_text = subprocess.run(f"hdc shell pidof '{bundle_name}'", stdout=subprocess.PIPE, text=True, check=True)
    pidinfo = pid_text.stdout
    #进程存在
    if pidinfo.strip() != "":
        run_and_get_output(f"hdc shell hidumper -lc", shell=False, text=True, encoding="utf-8")
    else:
        run_and_get_output(f"hdc shell hidumper -h", shell=False, text=True, encoding="utf-8")


def hidumper_op_nostart_func(ability_name, bundle_name):
    run_and_get_output(f"hdc shell hidumper -lc", shell=False, text=True, encoding="utf-8")
    run_and_get_output(f"hdc shell hidumper -c", shell=False, text=True, encoding="utf-8")


def nativehook_dwarf_startup(statistics_int, ability_name, bundle_name, op_func):
    pid_text = subprocess.run(f"hdc shell pidof '{bundle_name}'", stdout=subprocess.PIPE, text=True, check=True)
    pidinfo = pid_text.stdout
    if pidinfo.strip() != "":
        run_and_get_output(f"hdc shell kill " + pidinfo.strip(), shell=False, text=True, encoding="utf-8")
    #删除cppcrash
    run_and_get_output(f"hdc shell rm -f /data/log/faultlog/faultlogger/cppcrash-*", shell=False, text=True, encoding="utf-8")
    #dwarf 统计模式
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
                            '  process_name: "${s2}"             \n'
                            '  string_compressed: true           \n'
                            '  fp_unwind: false                  \n'
                            '  blocked: false                    \n'
                            '  callframe_compress: true          \n'
                            '  record_accurately: true           \n'
                            '  offline_symbolization: false      \n'
                            '  statistics_interval: ${s1}        \n'
                            '  startup_mode: true                \n'
                            '  js_stack_report: 1                \n'
                            '  max_js_stack_depth: 2             \n'
                            ' }                                  \n'
                            '}                                   \n')
    vmfile = file_content.safe_substitute(s1=statistics_int, s2=bundle_name)
    #写入文件
    write_str_file("./inputfiles/nativehook/config_nativehook_dwarf.txt", vmfile)

    run_and_get_output(f"hdc file send ./inputfiles/nativehook/config_nativehook_dwarf.txt /data/local/tmp/", shell=False,
                            text=True, encoding="utf-8")
    task_thread = threading.Thread(target=task_nativehook_dwarf, args=())
    task_thread.start()
    time.sleep(2)
    check_thread = threading.Thread(target=check_process_offline, args=())
    check_thread.start()
    op_func(ability_name, bundle_name)
    check_thread.join()
    task_thread.join()
    subprocess.run(f'hdc file recv /data/local/tmp/test_nativehook_dwarf.htrace ./outputfiles/', shell=False,
                     text=True, encoding="utf-8")
    # 检查文件大小
    file_size = get_file_size(f"./outputfiles/test_nativehook_dwarf.htrace")
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


def nativehook_dwarf_no_startup(prepare_op_func, statistics_int, ability_name, bundle_name, op_func):
    prepare_op_func(ability_name, bundle_name)
    #删除cppcrash
    run_and_get_output(f"hdc shell rm -f /data/log/faultlog/faultlogger/cppcrash-*", shell=False, text=True, encoding="utf-8")
    #dwarf
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
                            '  process_name: "${s2}"             \n'
                            '  string_compressed: true           \n'
                            '  fp_unwind: false                  \n'
                            '  blocked: false                    \n'
                            '  callframe_compress: true          \n'
                            '  record_accurately: true           \n'
                            '  offline_symbolization: false      \n'
                            '  statistics_interval: ${s1}        \n'
                            '  startup_mode: false               \n'
                            '  js_stack_report: 1                \n'
                            '  max_js_stack_depth: 2             \n'
                            ' }                                  \n'
                            '}                                   \n')
    vmfile = file_content.safe_substitute(s1=statistics_int, s2=bundle_name)
    #写入文件
    write_str_file("./inputfiles/nativehook/config_nativehook_dwarf.txt", vmfile)

    run_and_get_output(f"hdc file send ./inputfiles/nativehook/config_nativehook_dwarf.txt /data/local/tmp/", shell=False,
                            text=True, encoding="utf-8")
    task_thread = threading.Thread(target=task_nativehook_dwarf, args=())
    task_thread.start()
    time.sleep(2)
    check_thread = threading.Thread(target=check_process_offline, args=())
    check_thread.start()
    op_func(ability_name, bundle_name)
    check_thread.join()
    task_thread.join()
    subprocess.run(f'hdc file recv /data/local/tmp/test_nativehook_dwarf.htrace ./outputfiles/', shell=False,
                     text=True, encoding="utf-8")
    # 检查文件大小
    file_size = get_file_size(f"./outputfiles/test_nativehook_dwarf.htrace")
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


def nativehook_dwarf_check_data(statistics_flag):
    run_and_get_output(r"./inputfiles/trace_streamer_db.exe ./outputfiles/test_nativehook_dwarf.htrace -e ./outputfiles/test_nativehook_dwarf.db")
    # 连接数据库文件
    conn = sqlite3.connect(r'./outputfiles/test_nativehook_dwarf.db')
    # # 创建游标对象
    cursor = conn.cursor()
    # 检查是否存在符号数据
    cursor.execute('select * from native_hook_frame , data_dict where symbol_id = data_dict.id limit 10')
    result = cursor.fetchall()
    row_count = len(result)
    assert (row_count > 0)
    column_names = [description[0] for description in cursor.description]
    for row in result:
        #检查是否存在符号
        assert (row[column_names.index('data')] is not None)
    if statistics_flag:
        #检查是否存在统计数据
        cursor.execute('select * from native_hook_statistic limit 10')
        result = cursor.fetchall()
        row_count = len(result)
        assert (row_count > 0)
        column_names = [description[0] for description in cursor.description]
        for row in result:
            #检查是否统计数据
            assert (row[column_names.index('apply_count')] > 0)
            assert (row[column_names.index('release_count')] >= 0)
            assert (row[column_names.index('apply_size')] > 0)
            assert (row[column_names.index('release_size')] >= 0)
    else:
        #非统计模式
        cursor.execute('select * from native_hook limit 10')
        result = cursor.fetchall()
        row_count = len(result)
        assert (row_count > 0)
        names = [description[0] for description in cursor.description]
        for row in result:
            assert (row[names.index('event_type')] == 'AllocEvent' or row[names.index('event_type')] == 'FreeEvent' or row[names.index('event_type')] == 'MmapEvent')
            assert (row[names.index('heap_size')] > 0)
            assert (row[names.index('all_heap_size')] >= 0)

    cursor.close()
    conn.close()
    return True


class TestHiprofilerMemoryPlugin:
    @pytest.mark.L0
    #启动模式 hap 应用
    def test_nativehook_dwarf(self):
        #获得32位还是64位   
        sys_bit = subprocess.run(f"hdc shell getconf LONG_BIT", stdout=subprocess.PIPE, text=True, check=True)
        sysinfo = sys_bit.stdout
        if sysinfo.strip() == "32":
            #非统计模式
            nativehook_dwarf_startup(0, "com.ohos.photos.MainAbility", "com.ohos.photos", hap_op_func)
            assert nativehook_dwarf_check_data(False)
        else:
            #非统计模式
            nativehook_dwarf_startup(0, "com.ohos.launcher", "com.ohos.launcher", hap_op_func)
            assert nativehook_dwarf_check_data(False)

    @pytest.mark.L0
    #启动模式 10S 统计一次 hap应用
    def test_nativehook_dwarf_statics(self):
        #获得32位还是64位   
        sys_bit = subprocess.run(f"hdc shell getconf LONG_BIT", stdout=subprocess.PIPE, text=True, check=True)
        sysinfo = sys_bit.stdout
        if sysinfo.strip() == "32":
            #统计模式
            nativehook_dwarf_startup(10, "com.ohos.photos.MainAbility", "com.ohos.photos", hap_op_func)
            assert nativehook_dwarf_check_data(True)
        else:
            #统计模式
            nativehook_dwarf_startup(10, "com.ohos.launcher", "com.ohos.launcher", hap_op_func)
            assert nativehook_dwarf_check_data(True)

    @pytest.mark.L0
    #启动模式 10S 统计一次 SA 进程 如：hidumper_service
    def test_nativehook_dwarf_native_statics(self):
        #统计模式
        nativehook_dwarf_startup(10, "", "hidumper_service", hidumper_op_func)
        assert nativehook_dwarf_check_data(True)
        #非统计模式
        nativehook_dwarf_startup(0, "", "hidumper_service", hidumper_op_func)
        assert nativehook_dwarf_check_data(False)

    #非启动模式
    def test_nativehook_dwarf_native_not_startup(self):
        nativehook_dwarf_no_startup(hidumper_prepare_func, 10, "", "hidumper_service", hidumper_op_nostart_func)
        assert nativehook_dwarf_check_data(True)
        #非统计模式
        nativehook_dwarf_no_startup(hidumper_prepare_func, 0, "", "hidumper_service", hidumper_op_nostart_func)
        assert nativehook_dwarf_check_data(False)
