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
from tools.utils import SMALL_TRACE_EXPECTED_SIZE_3, MID_TRACE_EXPECTED_SIZE


def task():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_diskio.txt -o /data/local/tmp/test_diskio.htrace -t 10 -s -k"')


def task_ex():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_diskio_ex.txt -o /data/local/tmp/test_diskio_ex.htrace -t 10 -s -k"')


def task_no_report():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_diskio_no_report.txt -o /data/local/tmp/test_diskio_no_report.htrace -t 10 -s -k"')


class TestHiprofilerDiskioPlugin:

    @pytest.mark.L0
    def test_diskio_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\diskio\config_diskio.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_diskio.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_diskio.htrace -e ..\outputfiles\test_diskio.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_diskio.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_diskio.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        print("liuwei db_size0 = ", db_size)
        # # 判断数据库大小
        assert db_size > MID_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM diskio')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中rd_count wr_count rd_count_speed rd_count_speed 数据
        for row in result:
            assert (row[6] > 0)
            assert (row[7] > 0)
            assert (row[8] > 0)
            assert (row[9] > 0)

    @pytest.mark.L0
    def test_diskio_plugin_report_ex(self):
        subprocess.run(r'hdc file send ..\inputfiles\diskio\config_diskio_ex.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_ex, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_diskio_ex.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_diskio_ex.htrace -e ..\outputfiles\test_diskio_ex.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_diskio_ex.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_diskio_ex.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE_3
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM diskio')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中rd_count wr_count rd_count_speed rd_count_speed 数据
        for row in result:
            assert (row[6] == 0)
            assert (row[7] == 0)
            assert (row[8] == 0)
            assert (row[9] == 0)

    @pytest.mark.L0
    def test_diskio_plugin_no_report(self):
        subprocess.run(r'hdc file send ..\inputfiles\diskio\config_diskio_no_report.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_no_report, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_diskio_no_report.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_diskio_no_report.htrace -e ..\outputfiles\test_diskio_no_report.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_diskio_no_report.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_diskio_no_report.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size < SMALL_TRACE_EXPECTED_SIZE_3
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM diskio')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check
        # # 判断数据库中rd_count wr_count rd_count_speed rd_count_speed 数据
        for row in result:
            assert (row[6] == 0)
            assert (row[7] == 0)
            assert (row[8] == 0)
            assert (row[9] == 0)