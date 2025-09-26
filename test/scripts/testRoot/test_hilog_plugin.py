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
MID_TRACE_EXPECTED_SIZE = 1 * 1024


def task():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hilog.txt -o /data/local/tmp/test_hilog.htrace -t 10 -s -k"')


def task2():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hilog2.txt -o /data/local/tmp/test_hilog2.htrace -t 10 -s -k"')


def task3():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hilog3.txt -o /data/local/tmp/test_hilog3.htrace -t 10 -s -k"')


def task4():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hilog4.txt -o /data/local/tmp/test_hilog4.htrace -t 10 -s -k"')


class TestHiprofilerHilogPlugin:

    @pytest.mark.L0
    def test_hilog_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\hilog_plugin\config_hilog.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hilog.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hilog.htrace -e ..\outputfiles\test_hilog.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hilog.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hilog.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE_3
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM log order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中level tag 数据
        for row in result:
            assert (row[4] == 'I')
            assert (len(row[5]) > 0)

    @pytest.mark.L0
    def test_hilog_plugin2(self):

        # # 抓特定进程1 的log数据
        subprocess.run(r'hdc file send ..\inputfiles\hilog_plugin\config_hilog2.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task2, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hilog2.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hilog2.htrace -e ..\outputfiles\test_hilog2.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hilog2.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hilog2.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_2
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM log order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check

    @pytest.mark.L0
    def test_hilog_plugin3(self):
        
        # # 日志等级设置为ERROR、参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hilog_plugin\config_hilog3.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task3, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hilog3.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hilog3.htrace -e ..\outputfiles\test_hilog3.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hilog3.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hilog3.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM log order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中level tag 数据
        for row in result:
            assert (row[4] == 'E')
            assert (len(row[5]) > 0)

    @pytest.mark.L0
    def test_hilog_plugin4(self):
        
        # # pid设置为无效id、参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hilog_plugin\config_hilog4.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task4, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hilog4.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hilog4.htrace -e ..\outputfiles\test_hilog4.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hilog4.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hilog4.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM log order by ts limit 0,10')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check