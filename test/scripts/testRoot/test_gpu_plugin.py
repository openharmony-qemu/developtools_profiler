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

import os
import pytest
import sqlite3
import subprocess
import sys
import threading
sys.path.append("..")
from tools.utils import run_and_get_output

SMALL_TRACE_EXPECTED_SIZE = 1 * 1024
SMALL_TRACE_EXPECTED_SIZE_2 = 1024 * 1024 * 1024

def task():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_gpu.txt -o /data/local/tmp/test_gpu.htrace -t 15 -s -k"')


def task_process_report_is_false():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_gpu_report_process_is_false.txt -o /data/local/tmp/test_gpu2.htrace -t 15 -s -k"')


def task_no_pid():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_gpu_no_pid.txt -o /data/local/tmp/test_gpu3.htrace -t 15 -s -k"')


class TestHiprofilerGpuPlugin:

    @pytest.mark.L0
    def test_gpu_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\gpu_plugin\config_gpu.txt /data/local/tmp', text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_gpu.htrace ..\outputfiles', text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_gpu.htrace -e ..\outputfiles\test_gpu.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_gpu.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        db_relative_path = "..\\outputfiles\\test_gpu.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        trace_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert trace_size > SMALL_TRACE_EXPECTED_SIZE
        assert trace_size < SMALL_TRACE_EXPECTED_SIZE_2
    
    @pytest.mark.L0
    def test_gpu_plugin_process_report_is_false(self):
        subprocess.run(r'hdc file send ..\inputfiles\gpu_plugin\config_gpu_report_process_is_false.txt /data/local/tmp', text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_process_report_is_false, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_gpu2.htrace ..\outputfiles', text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_gpu2.htrace -e ..\outputfiles\test_gpu2.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_gpu2.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        db_relative_path = "..\\outputfiles\\test_gpu2.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        trace_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert trace_size > SMALL_TRACE_EXPECTED_SIZE
        assert trace_size < SMALL_TRACE_EXPECTED_SIZE_2

    @pytest.mark.L0
    def test_gpu_plugin_no_pid(self):
        subprocess.run(r'hdc file send ..\inputfiles\gpu_plugin\config_gpu_no_pid.txt /data/local/tmp', text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_no_pid, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_gpu3.htrace ..\outputfiles', text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_gpu3.htrace -e ..\outputfiles\test_gpu3.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_gpu3.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        db_relative_path = "..\\outputfiles\\test_gpu3.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        trace_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert trace_size > SMALL_TRACE_EXPECTED_SIZE
        assert trace_size < SMALL_TRACE_EXPECTED_SIZE_2