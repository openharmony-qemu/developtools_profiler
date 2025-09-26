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
MID_TRACE_EXPECTED_SIZE = 1 * 1024
MID_TRACE_EXPECTED_SIZE_2 = 1 * 1024


def task():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_process.txt -o /data/local/tmp/test_process.htrace -t 10 -s -k"')


def task2():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_process2.txt -o /data/local/tmp/test_process2.htrace -t 10 -s -k"')


def task3():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_process3.txt -o /data/local/tmp/test_process3.htrace -t 10 -s -k"')


def task4():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_process4.txt -o /data/local/tmp/test_process4.htrace -t 10 -s -k"')


def task5():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_process5.txt -o /data/local/tmp/test_process5.htrace -t 10 -s -k"')


def task6():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_process6.txt -o /data/local/tmp/test_process6.htrace -t 10 -s -k"')


class TestHiprofilerProcessPlugin:

    # 所有配置都为true，检查trace数据正确性
    @pytest.mark.L0
    def test_process_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\process_plugin\config_process.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_process.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_process.htrace -e ..\outputfiles\test_process.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_process.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_process.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > MID_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM live_process order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check

        # # 判断数据库中 process_id cpu_usage pss_info disk_reads 数据
        for row in result:
            # process_id
            assert (row[3] > 0)
            # cpu_usage
            assert (row[8] > 0)
            # pss_info
            assert (row[9] > 0)
            # disk_reads
            assert (row[12] >= 0)

    # 全部参数设置为false检查trace大小及数据
    @pytest.mark.L0
    def test_process_plugin2(self):
        subprocess.run(r'hdc file send ..\inputfiles\process_plugin\config_process2.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task2, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_process2.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_process2.htrace -e ..\outputfiles\test_process2.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_process2.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_process2.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM live_process')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    # 未设置report_process_tree参数检查trace大小及数据
    @pytest.mark.L0
    def test_process_plugin3(self):
        subprocess.run(r'hdc file send ..\inputfiles\process_plugin\config_process3.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task3, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_process3.htrace ..\outputfiles',
                         text=True, encoding="utf-8")
        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_process3.htrace -e ..\outputfiles\test_process3.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_process3.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_process3.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        assert db_size < MID_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM live_process order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    # 未设置report_diskio参数检查trace大小及数据
    @pytest.mark.L0
    def test_process_plugin4(self):
        subprocess.run(r'hdc file send ..\inputfiles\process_plugin\config_process4.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task4, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_process4.htrace ..\outputfiles',
                         text=True, encoding="utf-8")
        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_process4.htrace -e ..\outputfiles\test_process4.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_process4.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_process4.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        assert db_size < MID_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM live_process order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中 process_id cpu_usage pss_info disk_reads 数据
        for row in result:
            # process_id
            assert (row[3] > 0)
            # cpu_usage
            assert (row[8] > 0)
            # pss_info
            assert (row[9] > 0)
            # disk_reads
            assert (row[12] >= 0)

    # 未设置report_pss参数检查trace大小及数据
    @pytest.mark.L0
    def test_process_plugin5(self):
        subprocess.run(r'hdc file send ..\inputfiles\process_plugin\config_process5.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task5, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_process5.htrace ..\outputfiles',
                         text=True, encoding="utf-8")
        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_process5.htrace -e ..\outputfiles\test_process5.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_process5.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_process5.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        assert db_size < MID_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM live_process order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中 process_id cpu_usage pss_info disk_reads数据
        for row in result:
            # process_id
            assert (row[3] > 0)
            # cpu_usage
            assert (row[8] > 0)
            # pss_info
            assert (row[9] == 0)
            # disk_reads
            assert (row[12] >= 0)

    # 未设置report_cpu参数检查trace大小及数据
    @pytest.mark.L0
    def test_process_plugin6(self):
        subprocess.run(r'hdc file send ..\inputfiles\process_plugin\config_process6.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task6, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_process6.htrace ..\outputfiles',
                         text=True, encoding="utf-8")
        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_process6.htrace -e ..\outputfiles\test_process6.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_process6.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_process6.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        assert db_size < MID_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM live_process order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中 process_id cpu_usage pss_info disk_reads 数据
        for row in result:
            # process_id
            assert (row[3] > 0)
            # cpu_usage
            assert (row[8] == 0)
            # pss_info
            assert (row[9] > 0)
            # disk_reads
            assert (row[12] >= 0)