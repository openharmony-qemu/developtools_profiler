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

DESTROY_SIZE = 41943040
EXIST_SIZE = 40960
SLEEP_TWO = 2
SLEEP_FOUR = 4
SLEEP_FIVE = 5
SLEEP_TWENTY = 20
SYMBOL_INDEX = 4
APPLY_INDEX = 8
RELEASE_INDEX = 9
ALLOC_INDEX = 10
TYPE_INDEX = 4
MALLOC_TIMES = 10
ADDR_INDEX = 9
FILTER_THRESH = 5000
DEPTH_FIVE = 5
DEPTH_TEN = 10
DEPTH_FIFTEEN = 15
DEPTH_TWENTY = 20
DEPTH_THIRTY = 30
DEPTH_FIFTY = 50
CALLSTACKID_INDEX = 4
IPID_INDEX = 2
PID_INDEX = 2
MALLOC_THRESH = 1000
SA_CLICK_TIMES = 67
SA_WAIT_TIMES = 7
SA_STATISTICS = 300
SA_SAMPLE = 512
SAMPLE_SMALL = 512
SAMPLE_LARGE = 51200
FILTER_SMALL = 256
FILTER_LARGE = 10000
CLICK_TWICE = 2
CLICK_THREETIMES = 3
STATISTICS_INTERVAL = 10
MATCH_INTERVAL = 10


def task_template():
    subprocess.check_output("hdc shell hiprofiler_cmd -c /data/local/tmp/config.txt -o /data/local/tmp/test.htrace -t 20 -s -k")


def task_multiple_template():
    subprocess.check_output("hdc shell hiprofiler_cmd -c /data/local/tmp/config_multipleprocess.txt -o /data/local/tmp/test.htrace -t 25 -s -k")


def get_target_stack(result):
    malloc_release_stack = [0, 0]
    small_malloc_stack = [0, 0]
    for row in result:
        if 'Add(napi_env__*, napi_callback_info__*)' in row[1]:
            small_malloc_stack[0] = row[0]
            malloc_release_stack[0] = row[0]
        if 'js_depthmr6' in row[1]:
            malloc_release_stack[1] = row[0]
        if 'js_depthsm7' in row[1]:
            small_malloc_stack[1] = row[0]
    return malloc_release_stack, small_malloc_stack


def check_library_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval):
    conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM data_dict')
    result = cursor.fetchall()
    callstack_destroyed = []
    callstack_exists = []
    symbol_destroy = 0
    symbol_exist = 0
    for row in result:
        if "createAndReleaseHeap" in row[1]:
            symbol_destroy = row[0]
        if "createMemory" in row[1]:
            symbol_exist = row[0]
    
    cursor.execute('SELECT * FROM native_hook_frame')
    result = cursor.fetchall()
    for row in result:
        if row[SYMBOL_INDEX] == symbol_destroy:
            callstack_destroyed.append(row[1])
        if row[SYMBOL_INDEX] == symbol_exist:
            callstack_exists.append(row[1])
    check_destroyed = False
    check_exists = False
    if statistics > 0:
        cursor.execute('SELECT * FROM native_hook_statistic')
        result = cursor.fetchall()
        if touchtimes != 0:
            for row in result:
                for callstackid in callstack_destroyed:
                    if row[1] == callstackid:
                        if row[APPLY_INDEX] == DESTROY_SIZE * touchtimes and row[RELEASE_INDEX] == DESTROY_SIZE * touchtimes:
                            check_destroyed = True
                for callstackid in callstack_exists:
                    if row[1] == callstackid:
                        if row[APPLY_INDEX] == EXIST_SIZE * touchtimes and row[RELEASE_INDEX] == 0:
                            check_exists = True
        else:
            for row in result:
                for callstackid in callstack_destroyed:
                    if row[1] == callstackid:
                        if (row[APPLY_INDEX] % DESTROY_SIZE == 0) and row[RELEASE_INDEX] == row[APPLY_INDEX]:
                            check_destroyed = True
                            check_exists = True
    else:
        cursor.execute('SELECT * FROM native_hook')
        result = cursor.fetchall()
        times_destroyed = 0
        times_exists = 0
        malloc_addrs = []
        for row in result:
            for callstackid in callstack_destroyed:
                if row[1] == callstackid and row[ALLOC_INDEX] == (DESTROY_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "AllocEvent":
                    times_destroyed += 1
                    malloc_addrs.append(row[ADDR_INDEX])
            for callstackid in callstack_exists:
                if row[1] == callstackid and row[ALLOC_INDEX] == (EXIST_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "AllocEvent":
                    times_exists += 1
        if malloc_match_interval != 0:
            if times_destroyed != 0:
                return False
        elif times_destroyed != (touchtimes * MALLOC_TIMES):
            return False
        for row in result:
            if row[ADDR_INDEX] in malloc_addrs and row[ALLOC_INDEX] == (DESTROY_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "FreeEvent":
                times_destroyed -= 1
                malloc_addrs.remove(row[ADDR_INDEX])
        
        check_destroyed = (times_destroyed == 0)
        check_exists = (times_exists == (touchtimes * MALLOC_TIMES))
    if (sample_interval > FILTER_THRESH) or (filtersize > FILTER_THRESH):
        check_exists = True
    if malloc_match_interval > 0:
        check_destroyed = True
    cursor.close()
    conn.close()
    return check_destroyed and check_exists


def check_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval):
    conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM data_dict')
    result = cursor.fetchall()
    malloc_release_stack, small_malloc_stack = get_target_stack(result)
    cursor.execute('SELECT * FROM native_hook_frame')
    result = cursor.fetchall()
    callstack_ids_destroyed = []
    callstack_ids_exists = []
    callstack_ids_native = []
    for row in result:
        if row[CALLSTACKID_INDEX] == malloc_release_stack[1]:
            callstack_ids_destroyed.append(row[1])
        if row[CALLSTACKID_INDEX] == small_malloc_stack[1]:
            callstack_ids_exists.append(row[1])
        if row[CALLSTACKID_INDEX] == malloc_release_stack[0]:
            callstack_ids_native.append(row[1])
    callstack_ids_destroyed = list(set(callstack_ids_destroyed) & set(callstack_ids_native))
    callstack_ids_exists = list(set(callstack_ids_exists) & set(callstack_ids_native))
    if depth == ADDR_INDEX and (not dwarf):
        if len(callstack_ids_destroyed) != 0:
            return False
        if len(callstack_ids_exists) != 0:
            return False
        return True
    if len(callstack_ids_destroyed) == 0 and (malloc_match_interval != 0):
        return False
    if (sample_interval < FILTER_THRESH and filtersize < FILTER_THRESH) and len(callstack_ids_exists) == 0:
        return False
    if (sample_interval >= FILTER_THRESH and filtersize >= FILTER_THRESH) and len(callstack_ids_exists) != 0:
        return False
    check_destroyed = False
    check_exists = False
    if statistics > 0:
        cursor.execute('SELECT * FROM native_hook_statistic')
        result = cursor.fetchall()
        if touchtimes != 0:
            for row in result:
                for callstackid in callstack_ids_destroyed:
                    if row[1] == callstackid:
                        if row[APPLY_INDEX] == DESTROY_SIZE * touchtimes and row[RELEASE_INDEX] == DESTROY_SIZE * touchtimes:
                            check_destroyed = True
                for callstackid in callstack_ids_exists:
                    if row[1] == callstackid:
                        if row[APPLY_INDEX] == EXIST_SIZE * touchtimes and row[RELEASE_INDEX] == 0:
                            check_exists = True
        else:
            for row in result:
                for callstackid in callstack_ids_destroyed:
                    if row[1] == callstackid:
                        if (row[APPLY_INDEX] % DESTROY_SIZE == 0) and row[RELEASE_INDEX] == row[APPLY_INDEX]:
                            check_destroyed = True
                            check_exists = True
    else:
        cursor.execute('SELECT * FROM native_hook')
        result = cursor.fetchall()
        times_destroyed = 0
        times_exists = 0
        malloc_addrs = []
        for row in result:
            for callstackid in callstack_ids_destroyed:
                if row[1] == callstackid and row[ALLOC_INDEX] == (DESTROY_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "AllocEvent":
                    times_destroyed += 1
                    malloc_addrs.append(row[ADDR_INDEX])
            for callstackid in callstack_ids_exists:
                if row[1] == callstackid and row[ALLOC_INDEX] == (EXIST_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "AllocEvent":
                    times_exists += 1
        if malloc_match_interval != 0:
            if times_destroyed != 0:
                return False
        elif times_destroyed != (touchtimes * MALLOC_TIMES):
            return False
        for row in result:
            if row[ADDR_INDEX] in malloc_addrs and row[ALLOC_INDEX] == (DESTROY_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "FreeEvent":
                times_destroyed -= 1
                malloc_addrs.remove(row[ADDR_INDEX])
        
        check_destroyed = (times_destroyed == 0)
        check_exists = (times_exists == (touchtimes * MALLOC_TIMES))
    if (sample_interval > FILTER_THRESH) or (filtersize > FILTER_THRESH):
        check_exists = True
    if malloc_match_interval > 0:
        check_destroyed = True
    cursor.close()
    conn.close()
    return check_destroyed and check_exists


def check_nativehook_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval=0, response_library=False):
    try:
        subprocess.check_output(r"del .\..\inputfiles\nativehook\config.txt", text=True, encoding="utf-8")
        subprocess.check_output(r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
        subprocess.check_output(r"del .\..\inputfiles\layout.json", text=True, encoding="utf-8")
        subprocess.check_output(r"del .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
        subprocess.check_output("hdc shell rm /data/local/tmp/test.htrace")
    except Exception as e:
        print(f"An error occurred: {e}")
        pass

    with open(r".\..\inputfiles\nativehook\config_template.txt", 'r') as file:
        content = file.read()
    subprocess.check_output("hdc shell power-shell setmode 602")
    modified_content = content.replace('sample_interval: 256', 'sample_interval: ' + str(sample_interval))
    if malloc_match_interval == 0:
        modified_content = modified_content.replace('statistics_interval: 10', 'statistics_interval: ' + str(statistics))
    else:
        modified_content = modified_content.replace('statistics_interval: 10', 'statistics_interval: ' + str(statistics) + '\n' + 
                                                    "    malloc_free_matching_interval: " + str(malloc_match_interval))
    modified_content = modified_content.replace('filter_size: 500', 'filter_size: ' + str(filtersize))
    modified_content = modified_content.replace('max_js_stack_depth: 20', 'max_js_stack_depth: ' + str(depth))

    if not offline:
        modified_content = modified_content.replace('offline_symbolization: true', 'offline_symbolization: false')

    if not startup:
        modified_content = modified_content.replace('startup_mode: true', 'startup_mode: false')

    if dwarf:
        modified_content = modified_content.replace('fp_unwind: true', 'fp_unwind: false')

    if response_library:
        modified_content = modified_content.replace('response_library_mode: false', 'response_library_mode: true')

    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    mode = stat.S_IWUSR | stat.S_IRUSR
    with os.fdopen(os.open(r".\..\inputfiles\nativehook\config.txt", flags, mode), 'w') as file:
        file.write(modified_content)

    subprocess.check_output(r"hdc file send .\..\inputfiles\nativehook\config.txt /data/local/tmp/", text=True, encoding="utf-8")

    task_thread = threading.Thread(target=task_template, args=())
    task_thread.start()
    time.sleep(SLEEP_TWO)
    if (startup):
        subprocess.check_output("hdc shell killall com.example.insight_test_stage")
        subprocess.check_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
        time.sleep(SLEEP_FOUR)
        touch_button("模板测试")
        time.sleep(1)
        subprocess.check_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
        time.sleep(1)
        touch_button("Allocations_Js_Depth")

    i = 0
    while i < touchtimes:
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        i += 1
    task_thread.join()

    subprocess.check_output(r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    subprocess.check_output(r".\..\inputfiles\trace_streamer_nativehook.exe .\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")

    if response_library:
        return check_library_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval)
    return check_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval)


def check_nativehook_multipleprocess(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval=0, response_library=False):
    subprocess.check_output(r"del .\..\inputfiles\nativehook\config_multipleprocess.txt", text=True, encoding="utf-8")
    subprocess.check_output(r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
    subprocess.check_output(r"del .\..\inputfiles\layout.json", text=True, encoding="utf-8")
    subprocess.check_output(r"del .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
    subprocess.check_output("hdc shell rm /data/local/tmp/test.htrace")

    with open(r".\..\inputfiles\nativehook\config_multipleprocess_template.txt", 'r') as file:
        content = file.read()
    subprocess.check_output("hdc shell power-shell setmode 602")
    sceneboard = get_pid("com.ohos.sceneboard")
    modified_content = content.replace('sample_interval: 256', 'sample_interval: ' + str(sample_interval))
    if malloc_match_interval == 0:
        modified_content = modified_content.replace('statistics_interval: 10', 'statistics_interval: ' + str(statistics))
    else:
        modified_content = modified_content.replace('statistics_interval: 10', 'statistics_interval: ' + str(statistics) + '\n' + 
                                                    "    malloc_free_matching_interval: " + str(malloc_match_interval))
    modified_content = modified_content.replace('filter_size: 500', 'filter_size: ' + str(filtersize))
    modified_content = modified_content.replace('max_js_stack_depth: 20', 'max_js_stack_depth: ' + str(depth))
    modified_content = modified_content.replace('expand_pids: 0', 'expand_pids: ' + str(sceneboard))
    if not offline:
        modified_content = modified_content.replace('offline_symbolization: true', 'offline_symbolization: false')

    if not startup:
        modified_content = modified_content.replace('startup_mode: true', 'startup_mode: false')

    if dwarf:
        modified_content = modified_content.replace('fp_unwind: true', 'fp_unwind: false')

    if response_library:
        modified_content = modified_content.replace('response_library_mode: false', 'response_library_mode: true')
    
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    mode = stat.S_IWUSR | stat.S_IRUSR
    with os.fdopen(os.open(r".\..\inputfiles\nativehook\config_multipleprocess.txt", flags, mode), 'w') as file:
        file.write(modified_content)

    subprocess.check_output(r"hdc file send .\..\inputfiles\nativehook\config_multipleprocess.txt /data/local/tmp/", text=True, encoding="utf-8")

    task_thread = threading.Thread(target=task_multiple_template, args=())
    task_thread.start()
    time.sleep(SLEEP_TWO)
    if (startup):
        subprocess.check_output("hdc shell killall com.example.insight_test_stage")
        subprocess.check_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
        time.sleep(SLEEP_FOUR)
        touch_button("模板测试")
        time.sleep(1)
        subprocess.check_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
        time.sleep(1)
        touch_button("Allocations_Js_Depth")

    i = 0
    while i < touchtimes:
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        i += 1
    task_thread.join()
    subprocess.check_output(r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    subprocess.check_output(r".\..\inputfiles\trace_streamer_nativehook.exe .\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
    
    first_process = False
    if response_library:
        first_process = check_library_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval)
    else:
        first_process = check_result(statistics, startup, offline, sample_interval, dwarf, filtersize, depth, touchtimes, malloc_match_interval)
    
    conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM process')
    result = cursor.fetchall()
    ipid = 0
    sceneboard = get_pid("com.ohos.sceneboard")
    for row in result:
        if row[PID_INDEX] == int(sceneboard):
            ipid = row[1]
    if ipid == 0:
        return False
    second_process = False
    if statistics > 0:
        cursor.execute('SELECT * FROM native_hook_statistic')
        result = cursor.fetchall()
        for row in result:
            if row[IPID_INDEX] == ipid and row[APPLY_INDEX] >= MALLOC_THRESH:
                second_process = True
    else:
        cursor.execute('SELECT * FROM native_hook')
        result = cursor.fetchall()
        for row in result:
            if row[IPID_INDEX] == ipid and row[ALLOC_INDEX] >= MALLOC_THRESH:
                second_process = True
    cursor.close()
    conn.close()
    return first_process and second_process


def get_profiler_test_trace(process):
    subprocess.check_output("hdc shell ls -lh /data/log/reliability/resource_leak/memory_leak/ > /data/local/tmp/leak.txt")
    subprocess.check_output(r"hdc file recv /data/local/tmp/leak.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    with open(r'.\..\outputfiles\leak.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if process in line and ("smaps" not in line) and ("sample" not in line):
                return line.split()[len(line.split()) - 1]
    return ""


def check_sa_result():
    subprocess.check_output(r"hdc shell rm /data/local/tmp/test.htrace")
    subprocess.check_output("hdc shell rm /data/log/reliability/resource_leak/memory_leak/*")
    subprocess.check_output(r"del .\..\outputfiles\nativehook.db ", text=True, encoding="utf-8")
    subprocess.check_output(r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
    subprocess.check_output("hdc target mount")
    subprocess.check_output(f"hdc file send .\..\inputfiles\process_resource_limit.json /system/variant/phone/base/etc/efficiency_manager", text=True, encoding="utf-8")
    subprocess.check_output("hdc shell reboot", text=True, encoding="utf-8")
    time.sleep(SLEEP_TWENTY)
    j = 0
    while j < SA_WAIT_TIMES:
        output = subprocess.check_output(r"hdc list targets", text=True, encoding="utf-8")
        if output == '[Empty]\n\n':
            time.sleep(SLEEP_FIVE)
            j += 1
        else:
            break
    
    #解除锁屏
    subprocess.check_output("hdc shell uitest uiInput drag 100 500 100 100 1000")
    time.sleep(SLEEP_FIVE)
    subprocess.check_output("hdc shell uitest uiInput drag 100 500 100 100 1000")
    time.sleep(SLEEP_FIVE)
    subprocess.check_output("hdc shell uitest uiInput drag 100 500 100 100 1000")

    subprocess.check_output("hdc shell power-shell setmode 602")
    
    subprocess.check_output("hdc shell killall com.example.insight_test_stage")
    subprocess.check_output("hdc shell param set hiview.memleak.test enable")
    subprocess.check_output("hdc shell killall hiview")
    subprocess.check_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
    time.sleep(SLEEP_FOUR)
    touch_button("模板测试")
    time.sleep(1)
    subprocess.check_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
    time.sleep(1)
    touch_button("Allocations_Js_Depth")
    i = 0
    while i < SA_CLICK_TIMES:
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        i += 1
    
    filename = get_profiler_test_trace("com.example.insight_test_stage")

    subprocess.check_output("hdc shell cp /data/log/reliability/resource_leak/memory_leak/" + filename + " /data/local/tmp/test.htrace")
    subprocess.check_output(r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    subprocess.check_output(r".\..\inputfiles\trace_streamer_nativehook.exe .\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")

    return check_result(SA_STATISTICS, False, True, SA_SAMPLE, False, 0, DEPTH_TWENTY, 0)


class TestNativehook:
    @pytest.mark.L0
    def test_nativehook_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_nativehook_nonstatistics_sample(self):
        assert check_nativehook_result(0, False, True, SAMPLE_LARGE, False, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_nativehook_statistics_dwarf(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_nativehook_nonstatistics_match(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, 0, DEPTH_TEN, 1, MATCH_INTERVAL)

    @pytest.mark.L0
    def test_nativehook_startup_nonstatistics(self):
        assert check_nativehook_result(0, True, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_nativehook_statistics_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True)

    @pytest.mark.L0
    def test_dwarf_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, FILTER_SMALL, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_dwarf_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_TWICE)
    
    @pytest.mark.L0
    def test_dwarf_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_LARGE, True, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_dwarf_filtersize(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_dwarf_nonstatistics(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, 0, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_dwarf_nonstatistics_match(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, 0, FILTER_SMALL, 1, DEPTH_TEN, False)

    @pytest.mark.L0
    def test_dwarf_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L0
    def test_nonstatistics_response_library(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, 0, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L0
    def test_nonstatistics_match_response_library(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, 0, DEPTH_TEN, 1, MATCH_INTERVAL, True)

    @pytest.mark.L0
    def test_startup_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_startup_non_statistics(self):
        assert check_nativehook_result(0, True, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_startup_online(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_startup_nonstatistics_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE, MATCH_INTERVAL, False)

    @pytest.mark.L0
    def test_startup_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_LARGE, False, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_startup_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE, 0, True)

    @pytest.mark.L0
    def test_startup_response_library_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_LARGE, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True)

    @pytest.mark.L0
    def test_startup_response_library_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE, 0, True)

    @pytest.mark.L0
    def test_startup_sample_interval_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_LARGE, False, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_startup_sample_interval_nonstatistics(self):
        assert check_nativehook_result(0, True, False, SAMPLE_LARGE, False, 0, DEPTH_TEN, CLICK_TWICE)
    
    @pytest.mark.L0
    def test_startup_sample_interval_nonstatistics_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_LARGE, False, 0, DEPTH_TEN, CLICK_TWICE, MATCH_INTERVAL, False)

    @pytest.mark.L0
    def test_startup_filter_nonstatistics_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, CLICK_THREETIMES, MATCH_INTERVAL, False)

    @pytest.mark.L0
    def test_startup_filter_nonstatistics_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE, MATCH_INTERVAL, False)

    @pytest.mark.L0
    def test_online_filtersize(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_online_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_LARGE, False, FILTER_SMALL, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_online_sample_interval_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_LARGE, False, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_online_response_library_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L0
    def test_online_nonstatistics(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, FILTER_SMALL, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_online_nonstatistics_match(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, FILTER_SMALL, MATCH_INTERVAL, 1, DEPTH_TEN, False)

    @pytest.mark.L0
    def test_online_nonstatistics_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, FILTER_SMALL, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L0
    def test_response_library_startup_statistics(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, FILTER_SMALL, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L0
    def test_response_library_startup_nonstatistics(self):
        assert check_nativehook_result(0, True, True, SAMPLE_SMALL, False, FILTER_SMALL, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L0
    def test_online_nonstatistics_match_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, 1, MATCH_INTERVAL, False)

    @pytest.mark.L0
    def test_online_nonstatistics_filter(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_no_dataqueue(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_no_dataqueue_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_no_dataqueue_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_TWICE)
    
    @pytest.mark.L0
    def test_no_dataqueue_dwarf(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_depth_five_dwarf(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_FIVE, 1)

    @pytest.mark.L0
    def test_depth_five(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_depth_five_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_depth_five_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, 0, DEPTH_FIVE, CLICK_TWICE)

    @pytest.mark.L0
    def test_depth_five_filtersize(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_depth_five_nonstatistics(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_depth_fifteen(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_FIFTEEN, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_depth_twenty(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_TWENTY, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_depth_thirty(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_THIRTY, CLICK_TWICE)

    @pytest.mark.L0
    def test_depth_fifty(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_FIFTY, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_nativehook_sa(self):
        assert check_sa_result()

    @pytest.mark.L0
    def test_usermode_nondebug_app_startup(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, 0, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_multipleprocess_online(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, 0, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_multipleprocess_statistics(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, 0, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L0
    def test_multipleprocess_offline(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_TWENTY, CLICK_TWICE)

    @pytest.mark.L0
    def test_multipleprocess_dwarf(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_THIRTY, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_multipleprocess_offline_sample(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_LARGE, False, 0, DEPTH_THIRTY, CLICK_TWICE)

    @pytest.mark.L0
    def test_multipleprocess_dwarf_nonstatistics(self):
        assert check_nativehook_multipleprocess(0, False, True, SAMPLE_SMALL, False, 0, DEPTH_THIRTY, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_multipleprocess_dwarf_response_library(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_TWICE, 0, True)

    @pytest.mark.L0
    def test_multipleprocess_nonstatics_response_library(self):
        assert check_nativehook_multipleprocess(0, False, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True)

    @pytest.mark.L0
    def test_multipleprocess_nodataqueue_dwarf(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L0
    def test_multipleprocess_depth_five_dwarf(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_FIVE, CLICK_TWICE)

    @pytest.mark.L0
    def test_multipleprocess_depth_five(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L0
    def test_multipleprocess_depth_five_filter(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_FIVE, 1)

    @pytest.mark.L0
    def test_multipleprocess_depth_five_online(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, 0, DEPTH_FIVE, 1)

    @pytest.mark.L0
    def test_multipleprocess_depth_five_filtersize(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_FIVE, 1)

    @pytest.mark.L0
    def test_multipleprocess_depth_five_nonstatistics(self):
        assert check_nativehook_multipleprocess(0, False, True, SAMPLE_SMALL, False, 0, DEPTH_FIVE, 1)

    @pytest.mark.L0
    def test_appfreeze(self):
        subprocess.check_output(f"hdc shell ls -lh /data/log/faultlog/faultlogger/ > /data/local/tmp/faultlog.txt")
        subprocess.check_output(r"hdc file recv /data/local/tmp/faultlog.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check = True
        with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "com.ohos.sceneboard" in line and ("syswarning" not in line):
                    check = False
                if "com.example.insight_test_stage" in line:
                    check = False
        assert check == True

    @pytest.mark.L0
    def test_nocrash(self):
        check = True
        with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "hiprofilerd" in line:
                    check = False
                if "hiprofiler_plugins" in line:
                    check = False
                if "native_daemon" in line:
                    check = False
        assert check == True