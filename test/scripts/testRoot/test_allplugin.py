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
import time
import threading
from tools.utils import run_and_get_output

LIB_PATH = "/system/lib"
THRESH = 25000000000
SLEEP_TIME = 2
CONFIG_INDEX = 8


def task(index):
    indexstr = str(index)
    run_and_get_output("hdc shell hiprofiler_cmd -c /data/local/tmp/config" + indexstr + ".txt -o /data/local/tmp/test" + indexstr + ".htrace -t 20 -s -k")


class TestHiprofilerFtrace:
    @pytest.mark.L0
    def test_allplugin(self):
        run_and_get_output(r"hdc file send .\..\inputfiles\ftrace\config8.txt /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output(r"hdc file send .\..\inputfiles\hookDecoder /data/local/tmp/", text=True, encoding="utf-8")
        run_and_get_output(r"hdc file send .\..\inputfiles\malloctest /data/local/tmp/", text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=(CONFIG_INDEX, ))
        time.sleep(SLEEP_TIME)
        task_thread.start()
        run_and_get_output(f"hdc shell uitest uiInput drag 100 100 800 100 1000")
        run_and_get_output(f"hdc shell uitest uiInput drag 800 100 100 100 1000")
        task_thread.join()
  
        run_and_get_output(
            r"hdc shell chmod 777 /data/local/tmp/hookDecoder")
        run_and_get_output(
            r"hdc shell ./data/local/tmp/hookDecoder -f /data/local/tmp/test8.htrace > /data/local/tmp/test8_result.txt")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test8.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test8_result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check_sceneboard = False
        check_cpu = False
        with open(r'.\..\outputfiles\test8_result.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "com.ohos.launcher" in line:
                    check_sceneboard = True
                if "cpu5" in line:
                    check_cpu = True
        assert (check_sceneboard and check_cpu)