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
from tools.utils import MID_TRACE_EXPECTED_SIZE_2, SMALL_TRACE_EXPECTED_SIZE_2


def task():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hiperf.txt -o /data/local/tmp/test_hiperf.htrace -t 10 -s -k"')


def task2():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hiperf2.txt -o /data/local/tmp/test_hiperf2.htrace -t 10 -s -k"')


def task3():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hiperf3.txt -o /data/local/tmp/test_hiperf3.htrace -t 10 -s -k"')


def task4():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hiperf4.txt -o /data/local/tmp/test_hiperf4.htrace -t 10 -s -k"')


def task5():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hiperf5.txt -o /data/local/tmp/test_hiperf5.htrace -t 10 -s -k"')


class TestHiprofilerhiperfPlugin:
    # 配置is_root为false，检查trace数据正确性
    @pytest.mark.L0
    def test_hiperf_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\hiperf_plugin\config_hiperf.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hiperf.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hiperf.htrace -e ..\outputfiles\test_hiperf.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hiperf.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hiperf.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > MID_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM perf_callchain order by ip limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True

        # # 判断数据库中 callchain_id name数据
        for row in result:
            assert (row[1] > 0)
            assert (row[7] > 0)

        cursor.execute('SELECT * FROM perf_files')
        result2 = cursor.fetchall()
        if len(result2):
            check = True
        assert check

    # 输入不存在的outfile_name文件路径检查trace数据正确性
    @pytest.mark.L0
    def test_hiperf_plugin2(self):
        subprocess.run(r'hdc file send ..\inputfiles\hiperf_plugin\config_hiperf2.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task2, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hiperf2.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hiperf2.htrace -e ..\outputfiles\test_hiperf2.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hiperf2.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hiperf2.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM perf_callchain')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

        cursor.execute('SELECT * FROM perf_files')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    # 配置is_root为true，检查trace数据正确性
    @pytest.mark.L0
    def test_hiperf_plugin3(self):

        # # 参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hiperf_plugin\config_hiperf3.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task3, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hiperf3.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hiperf3.htrace -e ..\outputfiles\test_hiperf3.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hiperf3.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hiperf3.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM perf_callchain')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

        cursor.execute('SELECT * FROM perf_files')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    # 未输入record_args参数，检查trace数据正确性
    @pytest.mark.L0
    def test_hiperf_plugin4(self):

        # # 参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hiperf_plugin\config_hiperf4.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task4, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hiperf4.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hiperf4.htrace -e ..\outputfiles\test_hiperf4.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hiperf4.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hiperf4.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM perf_callchain')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

        cursor.execute('SELECT * FROM perf_files')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    # 配置log_level为1，检查trace数据正确性
    @pytest.mark.L0
    def test_hiperf_plugin5(self):

        # # 参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hiperf_plugin\config_hiperf5.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task5, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hiperf5.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hiperf5.htrace -e ..\outputfiles\test_hiperf5.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hiperf5.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hiperf5.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > MID_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM perf_callchain order by ip limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check

        # # 判断数据库中 callchain_id name数据
        for row in result:
            assert (row[1] > 0)
            assert (row[7] > 0)