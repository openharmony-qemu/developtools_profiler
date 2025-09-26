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

SMALL_TRACE_EXPECTED_SIZE = 1024

def task():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hisysevent.txt -o /data/local/tmp/test_hisysevent.htrace -t 20 -s -k"')


def task2():
    run_and_get_output(f'hdc shell "echo 11 > /sys/devices/platform/modem_power/state"')
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hisysevent2.txt -o /data/local/tmp/test_hisysevent2.htrace -t 15 -s -k"')


def task3():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hisysevent3.txt -o /data/local/tmp/test_hisysevent3.htrace -t 15 -s -k"')


def task4():
    run_and_get_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hisysevent4.txt -o /data/local/tmp/test_hisysevent4.htrace -t 15 -s -k"')


class TestHiprofilerHisyseventPlugin:

    # 未设置subscribe_domain参数，检查trace大小及数据
    @pytest.mark.L0
    def test_hisysevent_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\hisysevent_plugin\config_hisysevent.txt /data/local/tmp',
                                text=True, encoding="utf-8")

        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hisysevent.htrace ..\outputfiles',
                         text=True, encoding="utf-8")
        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hisysevent.htrace -e ..\outputfiles\test_hisysevent.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hisysevent.db')
        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hisysevent.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM hisys_all_event')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check

        # # 判断数据库中 domain_id pid event_id 数据
        for row in result:
            # domain_id
            assert (row[1] > 0)
            # pid
            assert (row[6] > 0)
            # event_id
            assert (len(row[11]) > 0)

    # domain为KERNEL_VENDOR，检查trace大小及数据
    @pytest.mark.L0
    def test_hisysevent_plugin2(self):

        # # 抓特定进程1
        subprocess.run(r'hdc file send ..\inputfiles\hisysevent_plugin\config_hisysevent2.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task2, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hisysevent2.htrace ..\outputfiles',
                         text=True, encoding="utf-8")
        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hisysevent2.htrace -e ..\outputfiles\test_hisysevent2.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hisysevent2.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hisysevent2.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM hisys_all_event')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check

    # domain为PROFILER，检查trace大小及数据
    @pytest.mark.L0
    def test_hisysevent_plugin3(self):

        # # 参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hisysevent_plugin\config_hisysevent3.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task3, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hisysevent3.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hisysevent3.htrace -e ..\outputfiles\test_hisysevent3.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hisysevent3.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hisysevent3.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM hisys_all_event')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check
    
    # domain为异常值，检查trace大小及数据
    @pytest.mark.L0
    def test_hisysevent_plugin4(self):

        # # 参数设置为false
        subprocess.run(r'hdc file send ..\inputfiles\hisysevent_plugin\config_hisysevent4.txt /data/local/tmp',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task4, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(r'hdc file recv /data/local/tmp/test_hisysevent4.htrace ..\outputfiles',
                         text=True, encoding="utf-8")

        subprocess.run(
            r'..\inputfiles\trace_streamer_db.exe ..\outputfiles\test_hisysevent4.htrace -e ..\outputfiles\test_hisysevent4.db')

        # 连接数据库文件
        conn = sqlite3.connect(r'..\outputfiles\test_hisysevent4.db')

        # # 创建游标对象
        cursor = conn.cursor()

        # # 执行SQL查询
        check = False
        db_relative_path = "..\\outputfiles\\test_hisysevent4.htrace"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > SMALL_TRACE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM hisys_all_event')
        result = cursor.fetchall()
        if len(result) == 0:
            check = True
        assert check