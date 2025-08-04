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
import threading
import sqlite3
import os


def get_file_size(file_path):
    size = os.path.getsize(file_path)
    return size


def task():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hidumper.txt -o /data/local/tmp/test_hidumper.htrace -t 30 -s -k"')


def task_nosec():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hidumper_nosec.txt -o /data/local/tmp/test_hidumper_nosec.htrace -t 30 -s -k"')


def task_section_60():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hidumper_60.txt -o /data/local/tmp/test_hidumper_60.htrace -t 30 -s -k"')


def task_fps():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_hidumper_fps.txt -o /data/local/tmp/test_hidumper_fps.htrace -t 30 -s -k"')


def check_process():
    count = 0
    while (count < 5):
        output_text = subprocess.run(f'hdc shell "ps -ef | grep SP_daemon"', stdout=subprocess.PIPE, text=True, check=True)
        process_info = output_text.stdout
        lines = process_info.strip().split('\n')
        check_index = False
        for line in lines:
            if line.find("SP_daemon -profilerfps") != -1:
                check_index = True
        assert (check_index)
        time.sleep(10)
        count = count + 1


class TestHiprofilerHidumpPlugin:
    # 将一秒分成10段 section 为10
    @pytest.mark.L0
    def test_hidump_plugin(self):
        subprocess.check_output(f"hdc file send ..\\inputfiles\\hidumper_plugin\\config_hidumper.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        # 唤醒屏幕
        subprocess.check_call("hdc shell power-shell wakeup", shell=False)
        # 设置屏幕常亮
        subprocess.check_call("hdc shell power-shell setmode 602", shell=False)
        time.sleep(3)
        # 解锁屏幕
        subprocess.check_call("hdc shell uinput -T -g 100 100 500 500", shell=False)
        time.sleep(3)
        subprocess.check_output(f"hdc shell uitest uiInput keyEvent Home", shell=False, 
                                text=True, encoding="utf-8")
        time.sleep(5)
        subprocess.check_output(f"hdc shell uinput -T -c 650 2447", shell=False, text=True, encoding="utf-8")
        time.sleep(1)
        subprocess.check_output(f"hdc shell uinput -T -c 104 1532", shell=False, text=True, encoding="utf-8")
        
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_hidumper.htrace ../outputfiles/', shell=False,
                       text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_hidumper.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_hidumper.htrace -e ../outputfiles/test_hidumper.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_hidumper.db')
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM hidump order by ts limit 0,10')
        result = cursor.fetchall()
        row_count = len(result)
        #检查获得FPS数据是否正确
        assert(row_count == 10)
        for row in result:
            assert(row[2] >= 0)
        # 检查分段有没有成功

        last_row = result[0][1]
        for row in result[1:]:
            assert((row[1] - last_row) == 100 * 1000 * 1000 or (row[1] - last_row) == 1000 * 1000 * 1000)
            last_row = row[1]
        cursor.close()
        conn.close()

    #不分段场景
    @pytest.mark.L0
    def test_hidump_plugin_nosec(self):
        subprocess.check_output(f"hdc file send ..\\inputfiles\\hidumper_plugin\\config_hidumper_nosec.txt /data/local/tmp/", shell=False,
                                 text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_nosec, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_hidumper_nosec.htrace ../outputfiles/', shell=False,
                         text=True, encoding="utf-8")
        # 检查文件大小 能正常抓到trace
        file_size = get_file_size(f"../outputfiles/test_hidumper_nosec.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_hidumper_nosec.htrace -e ../outputfiles/test_hidumper_nosec.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_hidumper_nosec.db')
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM hidump order by ts limit 0,10')
        result = cursor.fetchall()
        row_count = len(result)
        assert(row_count > 0)
        for row in result:
            assert(row[2] >= 0)
        cursor.close()
        conn.close()

    #验证hidumper进程和Sp_damon 进程能否正常拉起和结束
    @pytest.mark.L0
    def test_hidumper_process(self):
        subprocess.check_output(f"hdc file send ..\\inputfiles\\hidumper_plugin\\config_hidumper.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        time.sleep(2)
        task_check = threading.Thread(target=check_process, args=())
        task_check.start()
        task_thread.join()
        task_check.join()
        #检查结束后，子进程是否结束
        output_text = subprocess.run(f'hdc shell "ps -ef | grep SP_daemon"', stdout=subprocess.PIPE, text=True, check=True)
        process_info = output_text.stdout
        lines = process_info.strip().split('\n')
        check_index = False
        for line in lines:
            if line.find("SP_daemon -profilerfps") != -1:
                check_index = True
        assert(check_index == False)
        #检查trace 文件大小
        subprocess.run(f'hdc file recv /data/local/tmp/test_hidumper.htrace ../outputfiles/', shell=False,
                        text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_hidumper.htrace")
        assert (file_size > 1024)

    # 将一秒分成60段 section 为60
    @pytest.mark.L0
    def test_hidump_plugin_section_60(self):
        subprocess.check_output(f"hdc file send ..\\inputfiles\\hidumper_plugin\\config_hidumper_60.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_section_60, args=())
        task_thread.start()
        # 唤醒屏幕
        subprocess.check_call("hdc shell power-shell wakeup", shell=False)
        # 设置屏幕常亮
        subprocess.check_call("hdc shell power-shell setmode 602", shell=False)
        time.sleep(3)
        # 解锁屏幕
        subprocess.check_call("hdc shell uinput -T -g 100 100 500 500", shell=False)
        time.sleep(3)
        subprocess.check_output(f"hdc shell uitest uiInput keyEvent Home", shell=False, 
                                text=True, encoding="utf-8")
        time.sleep(5)
        subprocess.check_output(f"hdc shell uinput -T -c 650 2447", shell=False, text=True, encoding="utf-8")
        time.sleep(1)
        subprocess.check_output(f"hdc shell uinput -T -c 104 1532", shell=False, text=True, encoding="utf-8")
        
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_hidumper_60.htrace ../outputfiles/', shell=False,
                       text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_hidumper_60.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_hidumper_60.htrace -e ../outputfiles/test_hidumper_60.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_hidumper_60.db')
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM hidump order by ts limit 0,10')
        result = cursor.fetchall()
        row_count = len(result)
        #检查获得FPS数据是否正确
        assert(row_count == 0)

    #report_fps 为false
    @pytest.mark.L0
    def test_hidump_plugin_fps(self):
        subprocess.check_output(f"hdc file send ..\\inputfiles\\hidumper_plugin\\config_hidumper_fps.txt /data/local/tmp/", shell=False,
                                 text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_fps, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_hidumper_fps.htrace ../outputfiles/', shell=False,
                         text=True, encoding="utf-8")
        # 检查文件大小 能正常抓到trace
        file_size = get_file_size(f"../outputfiles/test_hidumper_fps.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_hidumper_fps.htrace -e ../outputfiles/test_hidumper_fps.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_hidumper_fps.db')
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM hidump order by ts limit 0,10')
        result = cursor.fetchall()
        row_count = len(result)
        assert(row_count > 0)
        for row in result:
            assert(row[2] == 0)
        cursor.close()
        conn.close()