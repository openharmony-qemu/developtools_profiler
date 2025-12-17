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
uiconn = UiDriver.connect()
uiconn.find_component(BY.text("11"))

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
FILE_SIZE_INDEX = 4
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
KILL_PROCESS_TIME = 10
SAMPLE_SMALL = 512
SAMPLE_LARGE = 51200
FILTER_SMALL = 256
FILTER_LARGE = 10000
CLICK_TWICE = 2
CLICK_THREETIMES = 3
STATISTICS_INTERVAL = 10
MATCH_INTERVAL = 10


def task_multiple_template(extend=False):
    if extend:
        run_and_get_output(
            r"hdc shell hiprofiler_cmd -c /data/local/tmp/config_multipleprocess.txt -o /data/local/tmp/test.htrace -t 50 -s -k")
    else:
        run_and_get_output(
            r"hdc shell hiprofiler_cmd -c /data/local/tmp/config_multipleprocess.txt -o /data/local/tmp/test.htrace -t 30 -s -k")


def get_target_stack(result):
    malloc_release_stack = [0, 0]
    small_malloc_stack = [0, 0]
    for row in result:
        if 'Add(napi_env__*, napi_callback_info__*)' in row[1]:
            small_malloc_stack[0] = row[0]
            malloc_release_stack[0] = row[0]
        if 'js_depth_released6' in row[1]:
            malloc_release_stack[1] = row[0]
        if 'js_depth_small7' in row[1]:
            small_malloc_stack[1] = row[0]
    return malloc_release_stack, small_malloc_stack


def get_target_so(result, target_so_name):
    file_id = 0
    for row in result:
        if target_so_name in row[1]:
            file_id = row[0]
    return file_id


def check_library_result(statistics, startup, offline, sample_interval,
                         dwarf, filtersize, depth, touchtimes, malloc_match_interval):
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


def check_result(statistics, startup, offline, sample_interval, dwarf,
                 js_report, filtersize, depth, touchtimes, malloc_match_interval,
                 target_so_name=""):
    conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM data_dict')
    result = cursor.fetchall()
    malloc_release_stack, small_malloc_stack = get_target_stack(result)
    if target_so_name != "":
        target_so_id = get_target_so(result, target_so_name)
        cursor.execute('SELECT DISTINCT callchain_id FROM native_hook_frame WHERE callchain_id NOT IN\
                        ( SELECT callchain_id FROM native_hook_frame WHERE file_id = ' + str(target_so_name) + ' )')
        result = cursor.fetchall()
        if len(result) != 0:
            print(f"Target SO name check failed - Expected: 0 callchain IDs, Actual: {len(result)} callchain IDs")
            return False
        return True
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
    if js_report:
        callstack_ids_destroyed = list(set(callstack_ids_destroyed) & set(callstack_ids_native))
        callstack_ids_exists = list(set(callstack_ids_exists) & set(callstack_ids_native))
    else:
        callstack_ids_destroyed = callstack_ids_native
        callstack_ids_exists = callstack_ids_native
    if depth == DEPTH_FIVE and (not dwarf) and js_report:
        if len(callstack_ids_destroyed) != 0:
            print(f"Depth five check failed for destroyed callstacks - Expected: 0, Actual: {len(callstack_ids_destroyed)}")
            return False
        if len(callstack_ids_exists) != 0:
            print(f"Depth five check failed for existing callstacks - Expected: 0, Actual: {len(callstack_ids_exists)}")
            return False
        return True
    if len(callstack_ids_destroyed) == 0 and (malloc_match_interval != 0):
        print(f"Callstack destroyed check failed - Expected: >0 destroyed callstack IDs, Actual: {len(callstack_ids_destroyed)} when malloc_match_interval={malloc_match_interval}")
        return False
    if (sample_interval < FILTER_THRESH and filtersize < FILTER_THRESH) and len(callstack_ids_exists) == 0:
        print(f"Callstack exists check failed - Expected: >0 existing callstack IDs, Actual: {len(callstack_ids_exists)} when sample_interval={sample_interval} < {FILTER_THRESH} and filtersize={filtersize} < {FILTER_THRESH}")
        return False
    if (sample_interval >= FILTER_THRESH or filtersize >= FILTER_THRESH) and len(callstack_ids_exists) != 0 and js_report:
        print(f"JS report check failed - Expected: 0 existing callstack IDs, Actual: {len(callstack_ids_exists)} when sample_interval={sample_interval} >= {FILTER_THRESH} or filtersize={filtersize} >= {FILTER_THRESH}")
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
                print(f"Malloc match interval check failed - Expected: 0 destroyed allocations, Actual: {times_destroyed} when malloc_match_interval={malloc_match_interval}")
                return False
        elif times_destroyed != (touchtimes * MALLOC_TIMES) and (not ((sample_interval >= FILTER_THRESH) or (filtersize >= FILTER_THRESH))):
            expected_times = touchtimes * MALLOC_TIMES
            print(f"Times destroyed check failed - Expected: {expected_times}, Actual: {times_destroyed} when touchtimes={touchtimes}, MALLOC_TIMES={MALLOC_TIMES}")
            return False
        for row in result:
            if row[ADDR_INDEX] in malloc_addrs and row[ALLOC_INDEX] == (DESTROY_SIZE / MALLOC_TIMES) and row[TYPE_INDEX] == "FreeEvent":
                times_destroyed -= 1
                malloc_addrs.remove(row[ADDR_INDEX])
        
        check_destroyed = (times_destroyed == 0)
        check_exists = (times_exists == (touchtimes * MALLOC_TIMES))
    if (sample_interval >= FILTER_THRESH) or (filtersize >= FILTER_THRESH):
        check_exists = True
    if malloc_match_interval > 0:
        check_destroyed = True
    cursor.close()
    conn.close()
    return check_destroyed and check_exists


def check_nativehook_result(statistics, startup, offline, sample_interval, dwarf, js_report, filtersize, depth, touchtimes, malloc_match_interval=0, response_library=False,
                            callframe_compress=True, string_compress=True, target_so_name=""):
    try:
        run_and_get_output(r"del .\..\inputfiles\nativehook\config.txt", text=True, encoding="utf-8")
        run_and_get_output(r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
        run_and_get_output(r"del .\..\inputfiles\layout.json", text=True, encoding="utf-8")
        run_and_get_output(r"del .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
        run_and_get_output("hdc shell rm /data/local/tmp/test.htrace")
        run_and_get_output("hdc shell rm /data/log/faultlog/faultlogger/*")
    except Exception as e:
        print(f"An error occurred: {e}")
        pass

    with open(r".\..\inputfiles\nativehook\config_template.txt", 'r') as file:
        content = file.read()
    run_and_get_output("hdc shell power-shell setmode 602")
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

    if not js_report:
        modified_content = modified_content.replace('js_stack_report: 1', 'js_stack_report: 0')
        modified_content = modified_content.replace('max_js_stack_depth: 20', 'max_js_stack_depth: 0')

    if response_library:
        modified_content = modified_content.replace('response_library_mode: false', 'response_library_mode: true')

    if not callframe_compress:
        modified_content = modified_content.replace('callframe_compress: true', 'callframe_compress: false')

    if not string_compress:
        modified_content = modified_content.replace('string_compress: true', 'string_compress: false')
    if target_so_name != "":
        modified_content = modified_content.replace('target_so_name: ""', 'target_so_name: "' + target_so_name + '"')

    write_config_file(modified_content)

(r"hdc file send .\..\inputfiles\nativehook\config.txt /data/local/tmp/", text=True, encoding="utf-8")    run_and_get_output

    task_thread = None
    if (dwarf or startup):
        task_thread = threading.Thread(target=task_template, args=(True,))
    else:
        task_thread = threading.Thread(target=task_template, args=())
    task_thread.start()
    time.sleep(SLEEP_TWO)
    if (startup):
        run_and_get_output("hdc shell killall com.example.insight_test_stage")
        run_and_get_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
        time.sleep(SLEEP_FOUR)
        if (dwarf):
            time.sleep(SLEEP_FOUR)
        touch_button("模板测试")
        time.sleep(1)
        run_and_get_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
        time.sleep(1)
        touch_button("Allocations_Js_Depth")

    i = 0
    while i < touchtimes:
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        i += 1
    task_thread.join()

    run_and_get_output(
        r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    run_and_get_output(
        r".\..\inputfiles\trace_streamer_nativehook.exe"
        r" .\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")

    if response_library:
        return check_library_result(statistics, startup, offline, sample_interval, dwarf,
                                    filtersize, depth, touchtimes, malloc_match_interval)
    return check_result(statistics, startup, offline, sample_interval, dwarf, js_report,
                        filtersize, depth, touchtimes, malloc_match_interval, target_so_name)


def check_nativehook_multipleprocess(statistics, startup, offline, sample_interval, dwarf,
                                     filtersize, depth, touchtimes, malloc_match_interval=0, response_library=False):
    run_and_get_output(
        r"del .\..\inputfiles\nativehook\config_multipleprocess.txt", text=True, encoding="utf-8")
    run_and_get_output(
        r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
    run_and_get_output(
        r"del .\..\inputfiles\layout.json", text=True, encoding="utf-8")
    run_and_get_output(
        r"del .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
    run_and_get_output(
        r"hdc shell rm /data/local/tmp/test.htrace")

    with open(r".\..\inputfiles\nativehook\config_multipleprocess_template.txt", 'r') as file:
        content = file.read()
    run_and_get_output(
        "hdc shell power-shell setmode 602")
    sceneboard = get_pid("com.ohos.launcher")
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

    run_and_get_output(
        r"hdc file send .\..\inputfiles\nativehook\config_multipleprocess.txt /data/local/tmp/", text=True, encoding="utf-8")
    task_thread = None
    if (dwarf or startup):
        task_thread = threading.Thread(target=task_multiple_template, args=(True,))
    else:
        task_thread = threading.Thread(target=task_multiple_template, args=())
    task_thread.start()
    time.sleep(SLEEP_TWO)
    if (startup):
        run_and_get_output(
            "hdc shell killall com.example.insight_test_stage")
        run_and_get_output(
            "hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
        time.sleep(SLEEP_FOUR)
        touch_button("模板测试")
        time.sleep(1)
        run_and_get_output(
            "hdc shell uitest uiInput drag 100 800 100 100 1000")
        time.sleep(1)
        touch_button("Allocations_Js_Depth")

    i = 0
    while i < touchtimes:
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        i += 1
    task_thread.join()
    run_and_get_output(
        r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    run_and_get_output(
        r".\..\inputfiles\trace_streamer_nativehook.exe "
        r".\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
    
    first_process = False
    if response_library:
        first_process = check_library_result(statistics, startup, offline, sample_interval, dwarf,
                                             filtersize, depth, touchtimes, malloc_match_interval)
    else:
        first_process = check_result(statistics, startup, offline, sample_interval, dwarf, True,
                                     filtersize, depth, touchtimes, malloc_match_interval)
    
    conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM process')
    result = cursor.fetchall()
    ipid = 0
    sceneboard = get_pid("com.ohos.launcher")
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
    run_and_get_output(
        "hdc shell ls -lh /data/log/reliability/resource_leak/memory_leak/ > /data/local/tmp/leak.txt")
    run_and_get_output(
        r"hdc file recv /data/local/tmp/leak.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    with open(r'.\..\outputfiles\leak.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if process in line and ("smaps" not in line) and ("sample" not in line):
                return line.split()[len(line.split()) - 1]
    return ""

def get_nmd_file(process):
    run_and_get_output(
        r"hdc shell ls -lh /data/log/reliability/resource_leak/memory_leak/ > /data/local/tmp/leak.txt")
    run_and_get_output(
        r"hdc file recv /data/local/tmp/leak.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    with open(r'.\..\outputfiles\leak.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if process in line and ("smaps" in line) and ("sample" not in line):
                return line.split()[len(line.split()) - 1]
    return ""


def check_file_size(output):
    result = output.split()[FILE_SIZE_INDEX]
    multi = False
    if (int(result[0]) > 2):
        return True
    else:
        return False


def check_sa_result(kill_process=False, check_dump_catch=False, multithread=False):
    try:
        run_and_get_output(
            r"hdc shell rm /data/local/tmp/test.htrace")
        run_and_get_output(
            r"hdc shell rm /data/log/reliability/resource_leak/memory_leak/*")
        run_and_get_output(
            r"del .\..\outputfiles\nativehook.db ", text=True, encoding="utf-8")
        run_and_get_output(
            r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
    except Exception as e:
        print(f"An error occurred: {e}")
        pass
    run_and_get_output("hdc target mount")
    if not multithread:
        run_and_get_output(
            r"hdc file send .\..\inputfiles\process_resource_limit.json /system/variant/phone/base/etc/efficiency_manager", text=True, encoding="utf-8")
    else:
        run_and_get_output(
            r"hdc file send .\..\inputfiles\process_resource_limit_multi.json /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc shell mv /data/local/tmp/process_resource_limit_multi.json /data/local/tmp/process_resource_limit.json", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc shell cp -f /data/local/tmp/process_resource_limit.json /system/variant/phone/base/etc/efficiency_manager", text=True, encoding="utf-8")
    run_and_get_output("hdc shell reboot", text=True, encoding="utf-8")
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
    run_and_get_output(
        "hdc shell uitest uiInput drag 100 500 100 100 1000")
    time.sleep(SLEEP_FIVE)
    run_and_get_output(
        "hdc shell uitest uiInput drag 100 500 100 100 1000")
    time.sleep(SLEEP_FIVE)
    run_and_get_output(
        "hdc shell uitest uiInput drag 100 500 100 100 1000")

    run_and_get_output(
        "hdc shell power-shell setmode 602")
    
    run_and_get_output(
        "hdc shell killall com.example.insight_test_stage")
    run_and_get_output(
        "hdc shell param set hiview.memleak.test enable")
    run_and_get_output(
        "hdc shell killall hiview")
    run_and_get_output(
        "hdc shell uitest uiInput click 100 200")
    run_and_get_output(
        "hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
    time.sleep(SLEEP_FOUR)
    touch_button("模板测试")
    time.sleep(1)
    run_and_get_output(
        "hdc shell uitest uiInput drag 100 800 100 100 1000")
    time.sleep(1)
    touch_button("Allocations_Js_Depth")
    i = 0
    dump_catch_result = False
    process_hilog = None
    daemonpid = 0
    wait_time = 0
    while i < SA_CLICK_TIMES:
        daemonpid = get_pid("native_daemon")
        if ((kill_process or check_dump_catch) and int(daemonpid) > 0):
            wait_time += 1
        if (wait_time == KILL_PROCESS_TIME):
            if (kill_process):
                run_and_get_output(
                    "hdc shell killall com.example.insight_test_stage")
                time.sleep(SLEEP_TWENTY)
                break
            if check_dump_catch:
                pid = get_pid("native_daemon")
                run_and_get_output("hdc shell echo " + str(pid) + " > /dev/frz/Frozen/procs")
                process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep Hiprofiler > /data/local/tmp/sahilog.txt'])
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        i += 1
    if (check_dump_catch):
        process_hilog.terminate()
        run_and_get_output(
            r"hdc file recv /data/local/tmp/sahilog.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        with open(r'.\..\outputfiles\sahilog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "DumpCatch" in line:
                    dump_catch_result = True
        return dump_catch_result
    filename = get_profiler_test_trace("com.example.insight_test_stage")
    nmdfile = get_nmd_file("com.example.insight_test_stage")
    if nmdfile == "":
        return False
    run_and_get_output(
        r"hdc shell cp /data/log/reliability/resource_leak/memory_leak/" + nmdfile + " /data/local/tmp/nmd.txt")
    run_and_get_output(
        r"hdc file recv /data/local/tmp/nmd.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    nmd_result = False
    with open(r'.\..\outputfiles\nmd.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if "End jemalloc ohos statistics" in line:
                nmd_result = True
    if not nmd_result:
        return False
    
    if (multithread):
        sceneboard_file = get_profiler_test_trace("com.ohos.launcher")
        run_and_get_output(
            r"hdc shell cp /data/log/reliability/resource_leak/memory_leak/" +
            sceneboard_file + r" /data/local/tmp/test.htrace")
        run_and_get_output(
            r"hdc shell ls -lh /data/local/tmp/ > /data/local/tmp/tmp.txt")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/tmp.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        result = False
        with open(r'.\..\outputfiles\tmp.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "test.htrace" in line:
                    result = (line.split()[SIZE_INDEX][-1] == 'M')
        if not result:
            return False

    run_and_get_output(
        r"hdc shell cp /data/log/reliability/resource_leak/memory_leak/" + filename +
        r" /data/local/tmp/test.htrace")
    run_and_get_output(
        r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ",
        text=True, encoding="utf-8")
    run_and_get_output(
        r".\..\inputfiles\trace_streamer_nativehook.exe "
        r".\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")

    return check_result(SA_STATISTICS, False, True, SA_SAMPLE, False, 0, 0, DEPTH_TWENTY, 0, 0)


class TestNativehook:
    @pytest.mark.L0
    def test_sa(self):
        assert check_sa_result()

    @pytest.mark.L0
    def test_startup_statistics_sample(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_complete_data(self):
        assert check_nativehook_result(0, False, False, 0, False, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_dwarf(self):
        assert check_nativehook_result(0, False, False, 0, True, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_dwarf_stringcompress(self):
        assert check_nativehook_result(0, False, False, 0, True, False, 0, DEPTH_TEN, CLICK_TWICE, 0, False, False, True)

    @pytest.mark.L0
    def test_dwarf_stringcompress_callframecompress(self):
        assert check_nativehook_result(0, False, False, 0, True, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, False, True, True)

    @pytest.mark.L0
    def test_dwarf_offline(self):
        assert check_nativehook_result(0, False, True, 0, True, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_match(self):
        assert check_nativehook_result(0, False, False, 0, False, False, 0, DEPTH_TEN, CLICK_TWICE, 10, False, False, False)


    @pytest.mark.L0
    def test_jsreport(self):
        assert check_nativehook_result(0, False, False, 0, False, True, 0, DEPTH_TEN, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_dwarf_jsreport(self):
        assert check_nativehook_result(0, False, False, 0, True, True, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_filter(self):
        assert check_nativehook_result(0, False, False, 0, False, False, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_dwarf_filter(self):
        assert check_nativehook_result(0, False, False, 0, True, False, FILTER_LARGE, DEPTH_TEN, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_dwarf_startup(self):
        assert check_nativehook_result(0, True, False, 0, True, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_startup(self):
        assert check_nativehook_result(0, True, False, 0, False, False, 0, DEPTH_TEN, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_response_library(self):
        assert check_nativehook_result(0, False, False, 0, False, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True, False, False)

    @pytest.mark.L0
    def test_dwarf_response_library(self):
        assert check_nativehook_result(0, False, False, 0, True, False, 0, DEPTH_TEN, 1, 0, True, False, False)

    @pytest.mark.L0
    def test_startup_response_library(self):
        assert check_nativehook_result(0, True, False, 0, False, False, 0, DEPTH_TEN, CLICK_TWICE, 0, True, False, False)

    @pytest.mark.L0
    def test_sample(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_complete_data(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_dwarf_stringcompress(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, False, 0, DEPTH_TEN, CLICK_TWICE, 0, False, False, True)

    @pytest.mark.L0
    def test_statistics_dwarf_stringcompress_callframecompress(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, False, True, True)

    @pytest.mark.L0
    def test_statistics_dwarf_offline(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, 0, True, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_jsreport(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, True, 0, DEPTH_TEN, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_dwarf_jsreport(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, True, 0, DEPTH_TEN, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, False, FILTER_LARGE, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_dwarf_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, False, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_dwarf_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, 0, True, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, 0, False, False, 0, DEPTH_TEN, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, False, 0, DEPTH_TEN, CLICK_TWICE, 0, True, False, False)

    @pytest.mark.L0
    def test_statistics_dwarf_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True, False, False)

    @pytest.mark.L0
    def test_statistics_startup_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, 0, False, False, 0, DEPTH_TEN, 1, 0, True, False, False)

    @pytest.mark.L0
    def test_statistics_sample(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, False, 0, DEPTH_TEN, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_no_dataqueue(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, False, 0, DEPTH_TEN, CLICK_THREETIMES)
    
    @pytest.mark.L0
    def test_dwarf_depth_five(self):
        assert check_nativehook_result(0, False, False, 0, True, False, 0, DEPTH_FIVE, 1, 0, False, False, False)

    @pytest.mark.L0
    def test_depth_five(self):
        assert check_nativehook_result(0, False, False, 0, False, False, 0, DEPTH_FIVE, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_startup_depth_five(self):
        assert check_nativehook_result(0, True, False, 0, False, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_offline_depth_five(self):
        assert check_nativehook_result(0, False, True, 0, False, False, 0, DEPTH_FIVE, 1, 0, False, False, False)
    
    @pytest.mark.L0
    def test_filter_depth_five(self):
        assert check_nativehook_result(0, False, False, 0, False, False, FILTER_SMALL, DEPTH_FIVE, CLICK_TWICE, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_depth_five(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False)

    @pytest.mark.L0
    def test_statistics_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libentry.so")

    @pytest.mark.L1
    def test_js_sample(self):
        assert check_nativehook_result(0, False, True, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_statistics_dwarf(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L1
    def test_js_match(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, 1, MATCH_INTERVAL)

    @pytest.mark.L1
    def test_js_startup(self):
        assert check_nativehook_result(0, True, True, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_statistics_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True)

    @pytest.mark.L1
    def test_js_statistics_dwarf_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, FILTER_SMALL, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_statistics_dwarf_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, CLICK_TWICE)
    
    @pytest.mark.L1
    def test_js_statistics_dwarf_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L1
    def test_js_statistics_dwarf_filtersize(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_dwarf_match(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, True, FILTER_SMALL, DEPTH_TEN, 1, False)

    @pytest.mark.L1
    def test_js_statistics_dwarf_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L1
    def test_js_response_library(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L1
    def test_js_match_response_library(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, 1, MATCH_INTERVAL, True)

    @pytest.mark.L1
    def test_js_statistics_startup_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_statistics_startup_non_statistics(self):
        assert check_nativehook_result(0, True, True, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_startup_online(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_startup_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, 1, MATCH_INTERVAL, False)

    @pytest.mark.L1
    def test_js_statistics_startup_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L1
    def test_js_statistics_startup_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE, 0, True)

    @pytest.mark.L1
    def test_js_statistics_startup_response_library_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True)

    @pytest.mark.L1
    def test_js_statistics_startup_response_library_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE, 0, True)

    @pytest.mark.L1
    def test_js_statistics_startup_sample_interval_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_startup_sample_interval(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)
    
    @pytest.mark.L1
    def test_js_startup_sample_interval_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE, MATCH_INTERVAL, False)

    @pytest.mark.L1
    def test_js_statistics_sample_interval_filter_size(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_sample_interval_filter_size(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_startup_filter_match(self):
        assert check_nativehook_result(0, True, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, CLICK_TWICE, MATCH_INTERVAL, False)

    @pytest.mark.L1
    def test_js_statistics_online_filtersize(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_statistics_online_sample_interval(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, FILTER_SMALL, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_statistics_online_sample_interval_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_statistics_online_response_library_filter(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L1
    def test_statistics_js_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, FILTER_SMALL, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L1
    def test_js_online_match(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, True, FILTER_SMALL, MATCH_INTERVAL, 1, DEPTH_TEN, False)

    @pytest.mark.L1
    def test_statistics_js_online_response_library(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, FILTER_SMALL, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L1
    def test_js_statistics_response_library_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, False, SAMPLE_SMALL, False, True, FILTER_SMALL, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L1
    def test_js_response_library_startup(self):
        assert check_nativehook_result(0, True, True, SAMPLE_SMALL, False, True, FILTER_SMALL, DEPTH_TEN, 1, 0, True)

    @pytest.mark.L1
    def test_js_online_match_filter(self):
        assert check_nativehook_result(0, False, False, 0, False, True, FILTER_LARGE, DEPTH_TEN, 1, MATCH_INTERVAL, False)

    @pytest.mark.L1
    def test_js_startup_online_match_filter(self):
        assert check_nativehook_result(0, True, False, 0, False, True, FILTER_LARGE, DEPTH_TEN, 1, MATCH_INTERVAL, False)

    @pytest.mark.L1
    def test_js_online_match_filter_sample_interval(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, 1, MATCH_INTERVAL, False)

    @pytest.mark.L1
    def test_js_online_filter(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_TEN, 1)

    @pytest.mark.L1
    def test_js_statistics_no_dataqueue(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_THREETIMES)

    @pytest.mark.L1
    def test_js_statistics_no_dataqueue_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_js_statistics_no_dataqueue_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, 0, DEPTH_TEN, CLICK_TWICE)
    
    @pytest.mark.L1
    def test_js_statistics_no_dataqueue_dwarf(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L1
    def test_sa_killprocess(self):
        assert check_sa_result(True)

    @pytest.mark.L1
    def test_sa_multi(self):
        assert check_sa_result(False, False, True)

    @pytest.mark.L1
    def test_nonstatistics_target_so(self):
        assert check_nativehook_result(0, False, False, 0, False, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libentry.so")

    @pytest.mark.L1
    def test_statistics_dwarf_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libc++_shared.so")
                            
    @pytest.mark.L1
    def test_statistics_js_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, False, True, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libentry.so")

    @pytest.mark.L1
    def test_statistics_dwarf_js_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, 0, True, True, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libc++_shared.so")
    
    @pytest.mark.L1
    def test_statistics_sample_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libentry.so")
    
    @pytest.mark.L1
    def test_sample_dwarf_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libc++_shared.so")

    @pytest.mark.L1
    def test_nonstatistics_dwarf_target_so(self):
        assert check_nativehook_result(0, False, False, 0, True, False, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libentry.so")

    @pytest.mark.L1
    def test_nonstatistics_js_target_so(self):
        assert check_nativehook_result(0, False, False, 0, False, True, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libc++_shared.so")

    @pytest.mark.L1
    def test_sample_js_target_so(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, 0, DEPTH_FIVE, CLICK_THREETIMES, 0, False, False, False,
                                       "libentry.so")        

    @pytest.mark.L2
    def test_js_statistics_depth_five_dwarf(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_FIVE, 1)

    @pytest.mark.L2
    def test_js_statistics_depth_five(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, True, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_depth_five_startup(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, True, True, SAMPLE_SMALL, False, True, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_depth_five_online(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, 0, DEPTH_FIVE, CLICK_TWICE)

    @pytest.mark.L2
    def test_js_statistics_depth_five_filtersize(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, True, FILTER_LARGE, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_depth_five(self):
        assert check_nativehook_result(0, False, False, SAMPLE_SMALL, False, True, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_depth_fifteen(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_FIFTEEN, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_depth_twenty(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_TWENTY, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_depth_thirty(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_THIRTY, CLICK_TWICE)

    @pytest.mark.L2
    def test_js_statistics_depth_fifty(self):
        assert check_nativehook_result(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, True, 0, DEPTH_FIFTY, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_js_statistics_usermode_nondebug_app_startup(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, 0, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L2
    def test_multipleprocess_statistics_online(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, 0, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L2
    def test_multipleprocess_statistics(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, 0, False, 0, DEPTH_TEN, 1)

    @pytest.mark.L2
    def test_multipleprocess_statistics_offline(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_TWENTY, CLICK_TWICE)

    @pytest.mark.L2
    def test_multipleprocess_statistics_dwarf(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, True, 0, DEPTH_THIRTY, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_multipleprocess_statistics_offline_sample(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_THIRTY, CLICK_TWICE)

    @pytest.mark.L2
    def test_multipleprocess_dwarf(self):
        assert check_nativehook_multipleprocess(0, False, True, SAMPLE_SMALL, False, 0, DEPTH_THIRTY, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_multipleprocess_statistics_dwarf_response_library(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_TWICE, 0, True)

    @pytest.mark.L2
    def test_multipleprocess_response_library(self):
        assert check_nativehook_multipleprocess(0, False, True, SAMPLE_SMALL, False, 0, DEPTH_TEN, CLICK_THREETIMES, 0, True)

    @pytest.mark.L2
    def test_multipleprocess_statistics_nodataqueue_dwarf(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_TEN, CLICK_TWICE)

    @pytest.mark.L2
    def test_multipleprocess_statistics_depth_five_dwarf(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, True, 0, DEPTH_FIVE, CLICK_TWICE)

    @pytest.mark.L2
    def test_multipleprocess_statistics_depth_five(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, 0, DEPTH_FIVE, CLICK_THREETIMES)

    @pytest.mark.L2
    def test_multipleprocess_statistics_depth_five_filter(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_FIVE, 1)

    @pytest.mark.L2
    def test_multipleprocess_statistics_depth_five_online(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, False, SAMPLE_SMALL, False, 0, DEPTH_FIVE, 1)

    @pytest.mark.L2
    def test_multipleprocess_statistics_depth_five_filtersize(self):
        assert check_nativehook_multipleprocess(STATISTICS_INTERVAL, False, True, SAMPLE_SMALL, False, FILTER_LARGE, DEPTH_FIVE, 1)

    @pytest.mark.L2
    def test_multipleprocess_depth_five(self):
        assert check_nativehook_multipleprocess(0, False, True, SAMPLE_SMALL, False, 0, DEPTH_FIVE, 1)

    @pytest.mark.L2
    def test_sa_dumpcatch(self):
        assert check_sa_result(False, True)

    @pytest.mark.L2
    def test_appfreeze(self):
        run_and_get_output(f"hdc shell ls -lh /data/log/faultlog/faultlogger/ > /data/local/tmp/faultlog.txt")
        run_and_get_output(r"hdc file recv /data/local/tmp/faultlog.txt .\..\outputfiles\ ",
                           text=True, encoding="utf-8")
        check = True
        with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "com.ohos.launcher" in line and ("syswarning" not in line):
                    check = False
                if "com.example.insight_test_stage" in line:
                    check = False
        assert check == True

    @pytest.mark.L2
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