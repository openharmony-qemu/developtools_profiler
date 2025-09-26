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
import os
import threading
import sqlite3
from tools.utils import run_and_get_output


def get_file_size(file_path):
    size = os.path.getsize(file_path)
    return size


def task():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_ftrace.txt -o /data/local/tmp/test_ftrace.htrace -t 10 -s -k"')


def task_event():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_ftrace_event.txt -o /data/local/tmp/test_ftrace_event.htrace -t 10 -s -k"')


def task_freq():
    run_and_get_output(
        r'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_ftrace_freq.txt -o /data/local/tmp/test_ftrace_freq.htrace -t 30 -s -k"')


class TestHiprofilerFtracePlugin:
    @pytest.mark.L0
    def test_ftraceplugin(self):
        run_and_get_output(r'hdc file send ..\\inputfiles\\ftrace\\config_ftrace.txt /data/local/tmp/',
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()

        run_and_get_output(f"hdc file recv /data/local/tmp/test_ftrace.htrace ../outputfiles/ ",
                                text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_ftrace.htrace")
        assert (file_size > 1024)
        run_and_get_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_ftrace.htrace -e ../outputfiles/test_ftrace.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_ftrace.db')
        # 创建游标对象
        cursor = conn.cursor()
        # 执行SQL查询
        # 检查binder
        cursor.execute("select * from callstack where cat ='binder' limit 0,10")
        result = cursor.fetchall()
        for row in result:
            assert(row[5] == 'binder transaction' or row[5] == 'binder reply' or row[5] == 'binder transaction async' or row[5] == 'binder async rcv')
        cursor.close()
        conn.close()
    
    @pytest.mark.L0
    def test_ftrace_events(self):
        run_and_get_output(f"hdc file send ..\\inputfiles\\ftrace\\config_ftrace_event.txt /data/local/tmp/",
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_event, args=())
        task_thread.start()
        task_thread.join()
        run_and_get_output(f"hdc file recv /data/local/tmp/test_ftrace_event.htrace ../outputfiles/ ",
                                text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_ftrace_event.htrace")
        assert (file_size > 1024)
        assert (file_size < 1024 * 1024 * 1024)
        run_and_get_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_ftrace_event.htrace -e ../outputfiles/test_ftrace_event.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_ftrace_event.db')
        # 创建游标对象
        cursor = conn.cursor()
        # 执行SQL查询
        check_wake = False
        check_newtask = False
        check_exit = False
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        for table in table_list:
            cursor.execute('SELECT * FROM ' + table[0])
            result = cursor.fetchall()
            for row in result:
                if 'sched_wakeup' in row:
                    check_wake = True
                if 'task_newtask' in row:
                    check_newtask = True
                if 'sched_process_exit' in row:
                    check_exit = True
        # 检查 wakeup 和waking 事件
        cursor.execute("select * from instant where name = 'sched_wakeup' limit 10")
        result = cursor.fetchall()
        for row in result:
            assert(row[2] > 0)
            assert(row[3] > 0)
        cursor.execute("select * from instant where name = 'sched_waking' limit 10")
        result = cursor.fetchall()
        for row in result:
            assert(row[2] > 0)
            assert(row[3] > 0)
        cursor.close()
        conn.close()
        assert check_wake
        assert check_newtask
        assert check_exit
    
    @pytest.mark.L0
    def test_ftrace_freq(self):
        run_and_get_output(
            r"hdc file send ..\inputfiles\ftrace\config_ftrace_freq.txt /data/local/tmp/",
            text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_freq, args=())
        task_thread.start()
        task_thread.join()
        run_and_get_output(
            r"hdc file recv /data/local/tmp/test_ftrace_freq.htrace ../outputfiles/ ",
            text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_ftrace_freq.htrace")
        assert (file_size > 1024)
        run_and_get_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_ftrace_freq.htrace -e ../outputfiles/test_ftrace_freq.db")
    
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_ftrace_freq.db')
        # 创建游标对象
        cursor = conn.cursor()
         # 执行SQL查询
        cursor.execute("select end_ts - start_ts as time from trace_range")
        result = cursor.fetchall()
        for row in result:
            assert(row[0] > 10 * 1000 * 1000 * 1000)
        #检查cpu 频率
        cursor.execute("select count(0) from cpu_measure_filter where name = 'cpu_frequency'")
        result = cursor.fetchall()
        for row in result:
            assert(row[0] == 12)
        
        cursor.execute("select * from measure,cpu_measure_filter where filter_id = id and name ='cpu_frequency' limit 10")
        result = cursor.fetchall()
        for row in result:
            assert(row[3] > 0)
        cursor.close()
        conn.close()