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
import sys
sys.path.append("..")
from tools.utils import *
import threading

OUTPUT_PATH = "testModule/output"
LIB_PATH = "/system/lib"
THRESH = 25000000000
SLEEP_TIME = 2
CONFIG_INDEX = 11


def task(index):
    indexstr = str(index)
    subprocess.check_output(f"hdc shell hiprofiler_cmd -c /data/local/tmp/config" + indexstr + ".txt -o /data/local/tmp/test" + indexstr + ".htrace -t 20 -s -k")


class TestHiprofilerMalloctime:
    @pytest.mark.L0
    def test_sched_blocked_reason(self):
        subprocess.check_output(r"hdc file send .\..\inputfiles\ftrace\config11.txt /data/local/tmp/", text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=(CONFIG_INDEX, ))
        time.sleep(SLEEP_TIME)
        task_thread.start()
        subprocess.check_output(f"hdc shell uitest uiInput drag 100 100 800 100 1000")
        subprocess.check_output(f"hdc shell uitest uiInput drag 800 100 100 100 1000")
        task_thread.join()
  
        subprocess.check_output(f"hdc shell chmod 777 /data/local/tmp/hookDecoder")
        subprocess.check_output(f"hdc shell ./data/local/tmp/hookDecoder -f /data/local/tmp/test11.htrace > /data/local/tmp/test11_result.txt")
        subprocess.check_output(f"hdc file recv /data/local/tmp/test11.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
        subprocess.check_output(f"hdc file recv /data/local/tmp/test11_result.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        sched_blocked_reason = False
        with open(r'.\..\outputfiles\test11_result.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "+0x" in line and "/0x" in line:
                    sched_blocked_reason = True
        assert sched_blocked_reason