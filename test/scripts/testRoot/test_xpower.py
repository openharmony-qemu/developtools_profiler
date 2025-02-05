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


def get_file_size(file_path):
    size = os.path.getsize(file_path)
    return size


def task():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/inputfiles/xpower_plugin/config_xpower.txt -o /data/local/tmp/test_xpower.htrace -t 30 -s -k"')


def task_total():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/inputfiles/xpower_plugin/config_xpower_total.txt -o /data/local/tmp/test_xpower_total.htrace -t 35 -s -k"')


class TestHiprofilerXPowerPlugin:
    @pytest.mark.L0
    def test_xpowerplugin_app(self):
        subprocess.check_output(f"hdc file send ./inputfiles/xpower_plugin/config_xpower.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        subprocess.check_output(f"hdc shell aa start -a com.huawei.hmos.settings.MainAbility -b com.huawei.hmos.settings", shell=False, 
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        task_thread.join()
        subprocess.check_output(f"hdc file recv /data/local/tmp/test_xpower.htrace ./outputfiles/ ", shell=False,
                                text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"./outputfiles/test_xpower.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"./inputfiles/trace_streamer_db.exe ./outputfiles/test_xpower.htrace -e ./outputfiles/test_xpower.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'./outputfiles/test_xpower.db')
        # 创建游标对象
        cursor = conn.cursor()
        # 执行SQL查询
        cursor.execute("select end_ts - start_ts as time from trace_range")
        result = cursor.fetchall()
        #断言trace 时长27秒
        for row in result:
            assert (row[0] == 27 * 1000 * 1000 * 1000)
        cursor.execute("select count(0) from xpower_measure")
        result = cursor.fetchall()
        for row in result:
            assert (row[0] > 0)
        
        cursor.execute("select * from xpower_measure where filter_id = 0 order by ts limit 0,10")
        result = cursor.fetchall()
        for row in result:
            assert (row[3] > 0)
        #电池电量
        cursor.execute("select * from xpower_measure where filter_id = 2 order by ts limit 0,10")
        result = cursor.fetchall()
        for row in result:
            assert (row[3] > 0)
        cursor.close()
        conn.close()

    @pytest.mark.L0
    def test_xpowerplugin_total(self):
        subprocess.check_output(f"hdc file send ./inputfiles/xpower_plugin/config_xpower_total.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_total, args=())
        task_thread.start()
        subprocess.check_output(f"hdc shell aa start -a com.huawei.hmos.settings.MainAbility -b com.huawei.hmos.settings", shell=False, 
                                text=True, encoding="utf-8")
        subprocess.check_output(f"hdc shell aa start -a com.huawei.hmos.photos.MainAbility -b com.huawei.hmos.photos", shell=False, 
                                text=True, encoding="utf-8")
        task_thread.join()
        subprocess.check_output(f"hdc file recv /data/local/tmp/test_xpower_total.htrace ./outputfiles/ ", shell=False,
                                text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"./outputfiles/test_xpower_total.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"./inputfiles/trace_streamer_db.exe ./outputfiles/test_xpower_total.htrace -e ./outputfiles/test_xpower_total.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'./outputfiles/test_xpower_total.db')
        # 创建游标对象
        cursor = conn.cursor()
        # 执行SQL查询
        cursor.execute("select end_ts - start_ts as time from trace_range")
        result = cursor.fetchall()
        #断言trace 时长33秒
        for row in result:
            assert (row[0] == 33 * 1000 * 1000 * 1000)
        cursor.execute("select count(0) from xpower_measure")
        result = cursor.fetchall()
        for row in result:
            assert (row[0] > 0)
        #电量百分比
        cursor.execute("select * from xpower_measure, measure_filter where filter_id = id and name = 'Battery.Level' limit 0,10")
        result = cursor.fetchall()
        for row in result:
            assert (row[3] > 0)
        #外壳温度
        cursor.execute("select * from xpower_measure, measure_filter where filter_id = id and name = 'ThermalReport.ShellTemp' limit 0,10")
        result = cursor.fetchall()
        for row in result:
            assert (row[3] > 0)
        #温度等级
        cursor.execute("select * from xpower_measure, measure_filter where filter_id = id and name = 'ThermalReport.ThermalLevel' limit 0,10")
        result = cursor.fetchall()
        for row in result:
            assert (row[3] >= 0)
        cursor.close()
        conn.close()