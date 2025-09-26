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

SMALL_TRACE_EXPECTED_SIZE_3 = 1 * 1024
SMALL_TRACE_EXPECTED_SIZE_2 = 1024 * 1024 * 1024


def task():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_cpu.txt -o /data/local/tmp/test_cpu.htrace -t 15 -s -k"')


def task_invalid_pid():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_cpu_invalid_pid.txt -o /data/local/tmp/test_cpu2.htrace -t 15 -s -k"')


def task_no_pid():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_cpu_no_pid.txt -o /data/local/tmp/test_cpu3.htrace -t 15 -s -k"')


def task_process_is_false():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_cpu_process_is_false.txt -o /data/local/tmp/test_cpu4.htrace -t 15 -s -k"')


class TestHiprofilerCpuPlugin:

    @pytest.mark.L0
    def test_cpu_plugin(self):
        subprocess.run(
            r'hdc file send ..\inputfiles\cpu_plugin\config_cpu.txt /data/local/tmp',
            text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(
            r'hdc file recv /data/local/tmp/test_cpu.htrace ..\outputfiles',
            text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_cpu.htrace -e ..\outputfiles\test_cpu.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_cpu.db')

        # # 创建游标对象
        cursor = conn.cursor()
        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_cpu.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        assert db_size > SMALL_TRACE_EXPECTED_SIZE_3

        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM cpu_usage')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中user_load system_load 数据
        for row in result:
            assert (row[3] > 0)
            assert (row[4] > 0)

    @pytest.mark.L0
    def test_cpu_plugin_invalid_pid(self):
        subprocess.run(r'hdc file send ..\inputfiles\cpu_plugin\config_cpu_invalid_pid.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_invalid_pid, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_cpu2.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_cpu2.htrace -e ..\outputfiles\test_cpu2.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_cpu2.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_cpu2.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2

        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM cpu_usage')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    @pytest.mark.L0
    def test_cpu_plugin_no_pid(self):
        subprocess.run(r'hdc file send ..\inputfiles\cpu_plugin\config_cpu_no_pid.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_no_pid, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_cpu3.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_cpu3.htrace -e ..\outputfiles\test_cpu3.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_cpu3.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_cpu3.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2

        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM cpu_usage')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    @pytest.mark.L0
    def test_cpu_plugin_process_info_is_false(self):
        subprocess.run(r'hdc file send ..\inputfiles\cpu_plugin\config_cpu_process_is_false.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_process_is_false, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_cpu4.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_cpu4.htrace -e ..\outputfiles\test_cpu4.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_cpu4.db')

        # # 创建游标对象
        cursor = conn.cursor()
        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_cpu4.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        assert db_size > SMALL_TRACE_EXPECTED_SIZE_3

        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM cpu_usage')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中user_load system_load 数据
        for row in result:
            assert (row[3] == 0)
            assert (row[4] == 0)