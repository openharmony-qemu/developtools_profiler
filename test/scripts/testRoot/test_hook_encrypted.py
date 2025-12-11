#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2025 Huawei Device Co., Ltd.
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

def check_symbol(js_symbol, result):
    if (js_symbol):
        for row in result:
            if 'Index.ts' in row[1]:
                return True
    else:
        for row in result:
            if 'system' in row[1]:
                return True
    return False

def check_encrypted(is_js, fp_unwind, is_encrypted=False):
    delete_old_files()
    with open(r".\..\inputfiles\nativehook\config_template.txt", 'r') as file:
        content = file.read()
    subprocess.check_output("hdc shell power-shell setmode 602")
    modified_content = content.replace('startup_mode: true', 'startup_mode: false')
    process_name = "com.ohos.launcher"
    if (is_encrypted):
        process_name = "com.ohos.dongchedi"
    if (not fp_unwind):
        modified_content = modified_content.replace('fp_unwind: true',
                                                    'fp_unwind: false')
    modified_content = modified_content.replace('process_name: "com.example.insight_test_stage"',
                                                'process_name: "' + process_name + '"')
    write_config_file(modified_content)

    task_thread = None
    task_thread = threading.Thread(target=task_template, args=(True,))
    if (is_encrypted):
        subprocess.check_output("hdc shell aa start -a EntryAbility -b com.example.encrypted")
    time.sleep(3)
    task_thread.start()
    if (is_encrypted):
        touch_button("首页")
    else:
        subprocess.check_output("hdc shell uitest uiInput drag 100 800 100 100 1000")
        subprocess.check_output("hdc shell uitest uiInput drag 800 100 100 100 1000")
    time.sleep(3)

    task_thread.join()

    subprocess.check_output(
        r"hdc file recv /data/local/tmp/test.htrace .\..\outputfiles\ ", text=True, encoding="utf-8")
    subprocess.check_output(
        r".\..\inputfiles\trace_streamer_nativehook.exe "
        r".\..\outputfiles\test.htrace -e .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
    conn = sqlite3.connect(r'./../outputfiles/nativehook.db')
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM data_dict')
    result = cursor.fetchall()
    check_result = False
    if (is_js):
        if (is_encrypted):
            check_result = not check_symbol(True, result)
        else:
            check_result = check_symbol(True, result)
    else:
        check_result = check_symbol(False, result)
    return check_result


class TestHookEncrypted:
    @pytest.mark.L0
    def test_hook_encrypted_js_fp(self):
        assert check_encrypted(True, True, True)

    @pytest.mark.L0
    def test_hook_encrypted_native_fp(self):
        assert check_encrypted(False, True, True)

    @pytest.mark.L0
    def test_hook_encrypted_js_dwarf(self):
        assert check_encrypted(True, False, True)

    @pytest.mark.L0
    def test_hook_encrypted_native_dwarf(self):
        assert check_encrypted(False, False, True)

    @pytest.mark.L0
    def test_hook_non_encrypted_js_fp(self):
        assert check_encrypted(True, True)

    @pytest.mark.L0
    def test_hook_non_encrypted_native_fp(self):
        assert check_encrypted(False, True)

    @pytest.mark.L0
    def test_hook_non_encrypted_js_dwarf(self):
        assert check_encrypted(True, False)

    @pytest.mark.L0
    def test_hook_non_encrypted_native_dwarf(self):
        assert check_encrypted(False, False)

