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

LIB_PATH = "/system/lib"
THRESH = 25000000000
SLEEP_TWO = 2
SLEEP_THIRTY = 30
CONFIG_INDEX = 6
PORT_INDEX = 5
MALLOCTIME_INDEX = 3
LISTENURI_INDEX = 3


def task(index):
    indexstr = str(index)
    subprocess.check_output("hdc shell hiprofiler_cmd -c /data/local/tmp/config" + indexstr + ".txt -o /data/local/tmp/test" + indexstr + ".htrace -t 20 -s -k")


def malloctest():
    subprocess.check_output("hdc shell chmod 777 /data/local/tmp/malloctest")
    subprocess.check_output("hdc shell ./data/local/tmp/malloctest 10 1024 1000000 > /data/local/tmp/malloctest.txt")


class TestHiprofilerMalloctime:
    @pytest.mark.L0
    def test_malloctime(self):
        subprocess.check_output(f"hdc file send .\inputfiles\nativehook\config6.txt /data/local/tmp/", text=True, encoding="utf-8")
        malloc_thread = threading.Thread(target=malloctest)
        task_thread = threading.Thread(target=task, args=(CONFIG_INDEX, ))
        malloc_thread.start()
        time.sleep(SLEEP_TWO)
        task_thread.start()
        time.sleep(SLEEP_THIRTY)
        malloc_thread.join()
        task_thread.join()
  
        subprocess.check_output(f"hdc file recv /data/local/tmp/malloctest.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        malloctime = 0

        with open(r'.\..\outputfiles\malloctest.txt', 'r') as file:
            lines = file.readlines()
            malloctime = int(lines[MALLOCTIME_INDEX].split()[MALLOCTIME_INDEX])
        assert (malloctime < THRESH)

    @pytest.mark.L0
    def test_listenuri(self):
        port = 0
        task_thread = threading.Thread(target=task, args=(CONFIG_INDEX, ))
        task_thread.start()
        subprocess.check_output("hdc shell hiprofiler_cmd -q > /data/local/tmp/cmdtmp.txt")
        subprocess.check_output(f"hdc file recv /data/local/tmp/cmdtmp.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check = False
        with open(r'.\..\outputfiles\cmdtmp.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "port" in line:
                    port = int(line[PORT_INDEX:])
                
            
        subprocess.check_output("hdc shell netstat -anp | grep " + str(port) + " > /data/local/tmp/uri.txt")
        subprocess.check_output(f"hdc file recv /data/local/tmp/uri.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check = False
        with open(r'.\..\outputfiles\uri.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "LISTEN" in line:
                    target = line.split()[LISTENURI_INDEX]
                    if "127.0.0.1" in target:
                        check = True
        task_thread.join()
        assert check
                