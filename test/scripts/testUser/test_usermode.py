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

LIB_PATH = "/system/lib"
NETWORK_PROFILER_RESULT = 2
SLEEP_TWO = 2
SLEEP_THREE = 3
SLEEP_FOUR = 4
CONFIG_NETWORK_PROFILER = 10
CONFIG_NETWORK_PROFILER_NONDEBUG = 12
CONFIG_SCENEBOARD = 7
SIZE_INDEX = 4


def task(index):
    indexstr = str(index)
    run_and_get_output("hdc shell hiprofiler_cmd -c /data/local/tmp/config" + indexstr + ".txt -o /data/local/tmp/test" + indexstr + ".htrace -t 20 -s -k")


def malloctest():
    run_and_get_output("hdc shell chmod 777 /data/local/tmp/malloctest")
    run_and_get_output("hdc shell ./data/local/tmp/malloctest 10 1024 1000000 > /data/local/tmp/malloctest.txt")


class TestHiprofilerUserMode:
    @pytest.mark.L0
    def test_usermode_nativehook_debug_app(self):
        run_and_get_output(r"hdc file send .\..\inputfiles\nativehook\config1.txt /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output("hdc shell power-shell setmode 602")
        run_and_get_output("hdc shell killall com.example.insight_test_stage")
        run_and_get_output("hdc shell rm /data/local/tmp/test1.htrace")
        task_thread = threading.Thread(target=task, args=(1, ))
        task_thread.start()
        time.sleep(SLEEP_TWO)
        run_and_get_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
        time.sleep(SLEEP_FOUR)
        touch_button("模板测试")
        time.sleep(SLEEP_TWO)
        run_and_get_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
        time.sleep(1)
        touch_button("Allocations_Js_Depth")
        touch_button("malloc-release(depth 6)")
        touch_button("small-malloc(depth 7)")
        task_thread.join()
        run_and_get_output(f"hdc file recv /data/local/tmp/test1.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output("hdc shell ls -lh /data/local/tmp/ > /data/local/tmp/tmp.txt")
        run_and_get_output(f"hdc file recv /data/local/tmp/tmp.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        result = False
        with open(r'.\..\outputfiles\tmp.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "test1.htrace" in line:
                    result = (line.split()[SIZE_INDEX][-1] == 'M')
        assert result

    @pytest.mark.L0
    def test_usermode_nondebug_app(self):
        # 校验命令行输出
        run_and_get_output("hdc shell rm /data/local/tmp/test7.htrace")
        run_and_get_output("hdc shell rm /data/local/tmp/tmp.txt")
        run_and_get_output(r"hdc file send .\..\inputfiles\nativehook\config7.txt /data/local/tmp/", text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=(CONFIG_SCENEBOARD, ))
        task_thread.start()
        time.sleep(SLEEP_TWO)
        run_and_get_output("hdc shell uitest uinput drag 100 800 100 100 1000")
        time.sleep(1)
        run_and_get_output("hdc shell uitest uinput drag 100 100 800 100 1000")
        task_thread.join()
        run_and_get_output(f"hdc file recv /data/local/tmp/test7.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output("hdc shell ls -lh /data/local/tmp/ > /data/local/tmp/tmp.txt")
        run_and_get_output(f"hdc file recv /data/local/tmp/tmp.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        result = False
        with open(r'.\..\outputfiles\tmp.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "test7.htrace" in line:
                    result = (line.split()[SIZE_INDEX][:-1] == "1.0")
        assert result

    @pytest.mark.L0
    def test_usermode_network_profiler_debugapp(self):
        run_and_get_output(r"hdc file send .\..\inputfiles\network_profiler\config10.txt /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output("hdc shell killall com.example.myapplication523")
        task_thread = threading.Thread(target=task, args=(CONFIG_NETWORK_PROFILER, ))
        time.sleep(SLEEP_TWO)
        task_thread.start()
        time.sleep(SLEEP_TWO)
        run_and_get_output("hdc shell aa start -a EntryAbility -b com.example.myapplication523")
        time.sleep(SLEEP_THREE)
        touch_button("http_request")
        task_thread.join()
  
        run_and_get_output("hdc shell chmod 777 /data/local/tmp/hookDecoder")
        run_and_get_output("hdc shell ./data/local/tmp/hookDecoder -f /data/local/tmp/test10.htrace > /data/local/tmp/test10_result.txt")
        run_and_get_output(f"hdc file recv /data/local/tmp/test10.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(f"hdc file recv /data/local/tmp/test10_result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        count = 0
        with open(r'.\..\outputfiles\test10_result.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "tv_nsec" in line:
                    count += 1
        assert count == NETWORK_PROFILER_RESULT
        #包括文件头

    @pytest.mark.L0
    def test_usermode_network_profiler_nondebugapp(self):
        run_and_get_output("hdc shell rm /data/local/tmp/test12.htrace")
        run_and_get_output(r"hdc file send .\..\inputfiles\network_profiler\config12.txt /data/local/tmp/", text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=(CONFIG_NETWORK_PROFILER_NONDEBUG, ))
        time.sleep(SLEEP_TWO)
        task_thread.start()
        time.sleep(SLEEP_THREE)
        run_and_get_output("hdc shell aa start -a EntryAbility -b com.tencent.mtthm")
        time.sleep(SLEEP_FIVE)
        run_and_get_output("hdc shell uinput -T -c 850 1550")
        time.sleep(SLEEP_TWO)
        touch_button("微信")
        task_thread.join()
  
        run_and_get_output("hdc shell chmod 777 /data/local/tmp/hookDecoder")
        run_and_get_output("hdc shell ./data/local/tmp/hookDecoder -f /data/local/tmp/test12.htrace > /data/local/tmp/test12_result.txt")
        run_and_get_output(f"hdc file recv /data/local/tmp/test12.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(f"hdc file recv /data/local/tmp/test12_result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        count = 0
        with open(r'.\..\outputfiles\test12_result.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "tv_nsec" in line:
                    count += 1
        assert count == 0

    @pytest.mark.L0
    def test_usermode_kernel_symbols(self):
        run_and_get_output(r"hdc file recv /data/local/tmp/test1.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(r".\..\inputfiles\trace_streamer_db.exe .\..\outputfiles\test1.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
        conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM data_dict')
        result = cursor.fetchall()
        check = True
        for row in result:
            if 'kallsyms' in row[1]:
                check = False
        assert check
