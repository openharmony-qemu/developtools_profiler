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
import sys
import subprocess
import threading
import time
sys.path.append("..")
from tools.utils import touch_button
from tools.utils import run_and_get_output


SLEEP_TWO = 2
SLEEP_THREE = 3
SLEEP_FIVE = 5
MULTIPLE_RESULT = 3
ONCE_RESULT = 2
CONFIG_INDEX = 10
THRESH = 25000000000


def task(index):
    indexstr = str(index)
    run_and_get_output("hdc shell hiprofiler_cmd -c /data/local/tmp/config10.txt -o /data/local/tmp/test" + indexstr + ".htrace -t 30 -s -k")


class TestHiprofilerNetworkProfiler:
    @pytest.mark.L0
    def test_network_profiler_multiple_times(self):
        run_and_get_output(
            r"hdc file send .\..\inputfiles\network_profiler\config10.txt /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output(
            "hdc shell killall com.example.myapplication523")
        task_thread = threading.Thread(target=task, args=(CONFIG_INDEX, ))
        time.sleep(SLEEP_TWO)
        task_thread.start()
        time.sleep(SLEEP_THREE)
        run_and_get_output(
            "hdc shell aa start -a EntryAbility -b com.example.myapplication523")
        time.sleep(SLEEP_FIVE)
        touch_button("http_request")
        time.sleep(SLEEP_TWO)
        touch_button("http_request")
        time.sleep(SLEEP_TWO)
        touch_button("http_request")
        task_thread.join()
  
        run_and_get_output(
            r"hdc shell chmod 777 /data/local/tmp/hookDecoder")
        run_and_get_output(
            r"hdc shell ./data/local/tmp/hookDecoder -f /data/local/tmp/test10.htrace > /data/local/tmp/test10_result.txt")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test10.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test10_result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        count = 0
        with open(r'.\..\outputfiles\test10_result.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "tv_nsec" in line:
                    count += 1
        assert count == MULTIPLE_RESULT
        # 第二次请求和第三次请求一起被写入trace文件

    @pytest.mark.L0
    def test_network_profiler_one_time(self):
        run_and_get_output(
            r"hdc shell rm /data/local/tmp/test10.htrace")
        run_and_get_output(
            r"hdc shell rm /data/local/tmp/test10_result.txt")
        run_and_get_output(
            r"hdc shell killall com.example.myapplication523")
        run_and_get_output(
            r"hdc file send .\..\inputfiles\network_profiler\config10.txt /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output(
            "hdc shell killall com.example.myapplication523")
        task_thread = threading.Thread(target=task, args=(CONFIG_INDEX, ))
        time.sleep(SLEEP_TWO)
        task_thread.start()
        time.sleep(SLEEP_THREE)
        run_and_get_output(
            "hdc shell aa start -a EntryAbility -b com.example.myapplication523")
        time.sleep(SLEEP_FIVE)
        touch_button("http_request")
        time.sleep(SLEEP_TWO)
        task_thread.join()
  
        run_and_get_output(
            r"hdc shell chmod 777 /data/local/tmp/hookDecoder")
        run_and_get_output(
            r"hdc shell ./data/local/tmp/hookDecoder -f /data/local/tmp/test10.htrace > /data/local/tmp/test10_result.txt")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test10.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test10_result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        count = 0
        with open(r'.\..\outputfiles\test10_result.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "tv_nsec" in line:
                    count += 1
        assert count == ONCE_RESULT
        # 包括文件头