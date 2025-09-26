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
WAIT_TIMES = 7
SLEEP_TWENTY = 20
SLEEP_FIVE = 5
GET_PID_TIME = 30
TOUCH_TIMES = 67


def get_daemon_pid():
    run_and_get_output(
        r"hdc shell ps -ef | grep daemon > /data/local/tmp/daemon.txt")
    run_and_get_output(
        r"hdc file recv /data/local/tmp/daemon.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    check = True
    with open(r'.\..\outputfiles\daemon.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if "native_daemon sa" in line:
                return line.split()[1]
    return -1


class TestHiprofilerReliability:
    @pytest.mark.L0
    def test_badfd(self):
        run_and_get_output("hdc shell rm /data/local/tmp/test.htrace")
        try:
            run_and_get_output(
                r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
            run_and_get_output(
                r"del .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
        except Exception as e:
            print(f"An error occurred: {e}")
            pass
        run_and_get_output("hdc target mount", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc file send .\..\inputfiles\process_resource_limit.json /system/variant/phone/base/etc/efficiency_manager", text=True, encoding="utf-8")
        run_and_get_output("hdc shell reboot", text=True, encoding="utf-8")
        time.sleep(SLEEP_TWENTY)
        j = 0
        while j < WAIT_TIMES:
            output = subprocess.check_output("hdc list targets", text=True, encoding="utf-8")
            if output == '[Empty]\n\n':
                time.sleep(SLEEP_FIVE)
                j += 1
            else:
                break
        
        #解除锁屏
        run_and_get_output("hdc shell uitest uiInput drag 100 500 100 100 1000")
        time.sleep(SLEEP_FIVE)
        run_and_get_output("hdc shell uitest uiInput drag 100 500 100 100 1000")
        time.sleep(SLEEP_FIVE)
        run_and_get_output("hdc shell uitest uiInput drag 100 500 100 100 1000")

        run_and_get_output("hdc shell power-shell setmode 602")
        
        run_and_get_output("hdc shell killall com.example.insight_test_stage")
        run_and_get_output("hdc shell param set hiview.memleak.test enable")
        run_and_get_output("hdc shell killall hiview")

        run_and_get_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
        process = subprocess.Popen(['hdc', 'shell', 'dmesg -w | grep avc > /data/local/tmp/avc.txt'])
        process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep BADFD > /data/local/tmp/daemonhilog.txt'])
        time.sleep(1)
        touch_button("模板测试")
        time.sleep(1)
        run_and_get_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
        time.sleep(1)
        touch_button("Allocations_Js_Depth")
        i = 0
        daemon_pid = 0
        while i < TOUCH_TIMES:
            touch_button("malloc-release(depth 6)")
            if (i == GET_PID_TIME):
                daemon_pid = get_daemon_pid()
            touch_button("small-malloc(depth 7)")
            i += 1
        process.terminate()
        process_hilog.terminate()
        run_and_get_output(
            r"hdc file recv /data/local/tmp/avc.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        run_and_get_output(
            r"hdc file recv /data/local/tmp/daemonhilog.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check = True
        with open(r'.\..\outputfiles\avc.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "BADFD" in line:
                    if "pid=" + str(daemon_pid) + " tid=" in line:
                        check = False

        with open(r'.\..\outputfiles\daemonhilog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "BADFD" in line:
                    if "pid=" + str(daemon_pid) + " tid=" in line:
                        check = False
        assert check == True