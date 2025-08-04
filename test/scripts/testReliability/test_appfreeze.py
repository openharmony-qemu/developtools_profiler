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

OUTPUT_PATH = "testRoot/output"
LIB_PATH = "/system/lib"
THRESH = 25000000000
CLICK_TIMES = 150
SWIPE_TIMES = 20
SLEEP_TWENTY = 20
SLEEP_FIVE = 5
SLEEP_FOUR = 4
SLEEP_TWO = 2
SETTING_INDEX = 13
GC_INTERVAL = 10
SLEEP_LONG = 195
WAIT_TIMES = 7
HOOK_SETTINGS_TIMES = 5
TEST_TIMES = 20


def task_cmd(index):
    indexstr = str(index)
    subprocess.check_output(f"hdc shell hiprofiler_cmd -c /data/local/tmp/config" + indexstr + ".txt -o /data/local/tmp/test" + indexstr + ".htrace -t 20 -s -k")


def check_faultlog():
    check = True
    with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if "com.ohos.launcher" in line and ("syswarning" not in line):
                check = False
            if "render_service" in line:
                check = False
            if "com.example.insight_test_stage" in line:
                check = False
    return check


class TestHiprofilerReliability:
    @pytest.mark.L0
    def test_appfreeze_sceneboard_sa(self):
        subprocess.check_output("hdc shell rm /data/local/tmp/test.htrace", text=True, encoding="utf-8")
        subprocess.check_output("hdc shell rm /data/log/reliability/resource_leak/memory_leak/*", text=True, encoding="utf-8")
        try:
            subprocess.check_output(r"del .\..\outputfiles\nativehook.db ", text=True, encoding="utf-8")
            subprocess.check_output(r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
        except Exception as e:
            print(f"An error occurred: {e}")
            pass
        subprocess.check_output("hdc target mount", text=True, encoding="utf-8")
        subprocess.check_output(f"hdc file send .\..\inputfiles\process_resource_limit_reliability.json /data/local/tmp/", text=True, encoding="utf-8")
        subprocess.check_output(f"hdc shell mv /data/local/tmp/process_resource_limit_reliability.json /data/local/tmp/process_resource_limit.json", text=True, encoding="utf-8")
        subprocess.check_output(f"hdc shell cp /data/local/tmp/process_resource_limit.json /system/variant/phone/base/etc/efficiency_manager/", text=True, encoding="utf-8")
        subprocess.check_output("hdc shell reboot", text=True, encoding="utf-8")
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
        subprocess.check_output("hdc shell uitest uinput drag 100 500 100 100 1000")
        time.sleep(SLEEP_FIVE)
        subprocess.check_output("hdc shell uitest uinput drag 100 500 100 100 1000")
        time.sleep(SLEEP_FIVE)
        subprocess.check_output("hdc shell uitest uinput drag 100 500 100 100 1000")

        subprocess.check_output("hdc shell power-shell setmode 602")
        
        subprocess.check_output("hdc shell killall com.example.insight_test_stage")
        subprocess.check_output("hdc shell param set hiview.memleak.test disable")
        subprocess.check_output("hdc shell killall hiview")
        sceneboard = get_pid("com.ohos.launcher")

        i = 0
        while i < CLICK_TIMES:
            subprocess.check_output("hdc shell uinput -T -m 200 1500 2000 1500")
            subprocess.check_output("hdc shell uinput -T -m 2000 1500 200 1500")
            time.sleep(SLEEP_FIVE)
            if ((i % GC_INTERVAL) == 0):
                subprocess.check_output("hdc shell hidumper --mem-jsheap " + str(sceneboard))
            i += 1
        

        subprocess.check_output("hdc shell ls -lh /data/log/faultlog/faultlogger/ > /data/local/tmp/faultlog.txt")
        subprocess.check_output(f"hdc file recv /data/local/tmp/faultlog.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check = True
        with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "com.ohos.launcher" in line and ("syswarning" not in line):
                    check = False
                if "render_service" in line:
                    check = False
        assert check

    def test_appfreeze_profiler_test(self):
        subprocess.check_output("hdc shell rm /data/log/reliability/resource_leak/memory_leak/*", text=True, encoding="utf-8")
        subprocess.check_output("hdc shell param set hiview.memleak.test disable")
        i = 0
        check = True
        while i < TEST_TIMES:
            i += 1
            subprocess.check_output("hdc shell killall hiview")
            subprocess.check_output("hdc shell killall com.example.insight_test_stage")
            subprocess.check_output("hdc shell aa start -a EntryAbility -b com.example.insight_test_stage")
            time.sleep(SLEEP_FOUR)
            touch_button("模板测试")
            time.sleep(1)
            subprocess.check_output("hdc shell uitest uinput drag 100 800 100 100 1000")
            time.sleep(1)
            touch_button("Allocations_Js_Depth")
            i = 0
            while i < CLICK_TIMES:
                touch_button("malloc-release(depth 6)")
                touch_button("small-malloc(depth 7)")
                i += 1
                time.sleep(SLEEP_FIVE)
            
            subprocess.check_output("hdc shell ls -lh /data/log/faultlog/faultlogger/ > /data/local/tmp/faultlog.txt")
            subprocess.check_output(f"hdc file recv /data/local/tmp/faultlog.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
            if not check_faultlog():
                check = False
        assert check

    def test_appfreeze_cmd_settings(self):
        subprocess.check_output(f"hdc file send .\..\inputfiles\nativehook\config13.txt /data/local/tmp/", text=True, encoding="utf-8")
        j = 0
        i = 0
        time.sleep(SLEEP_LONG)
        while j < HOOK_SETTINGS_TIMES:
            j += 1
            task_thread = threading.Thread(target=task_cmd, args=(SETTING_INDEX, ))
            task_thread.start()
            i = 0
            time.sleep(SLEEP_TWO)
            subprocess.check_output("hdc shell killall com.ohos.launcher")
            subprocess.check_output("hdc shell aa start -a EntryAbility -b com.ohos.launcher")
            time.sleep(SLEEP_FIVE)
            while (i < SWIPE_TIMES):
                subprocess.check_output("hdc shell uinput -T -m 200 1500 200 200")
                i += 1
            task_thread.join()
            j += 1

        subprocess.check_output("hdc shell ls -lh /data/log/faultlog/faultlogger/ > /data/local/tmp/faultlog.txt")
        subprocess.check_output(f"hdc file recv /data/local/tmp/faultlog.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
        check = True
        with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "com.ohos.launcher" in line:
                    check = False
                if "render_service" in line:
                    check = False
        assert check == True