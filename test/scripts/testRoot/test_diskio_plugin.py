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
from tools.utils import MID_FILE_EXPECTED_SIZE


def task():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_diskio.txt -o /data/local/tmp/test_diskio.htrace -t 10 -s -k"')


class TestHiprofilerDiskioPlugin:

    @pytest.mark.L0
    def test_diskio_plugin(self):
        subprocess.run(r'hdc file send ..\inputfiles\config_diskio.txt /data/local/tmp',
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
        db_relative_path = "..\\outputfiles\\test_diskio.db"
        absolute_path = os.path.abspath(db_relative_path)
        db_size = os.path.getsize(absolute_path)
        # # 判断数据库大小
        assert db_size > MID_FILE_EXPECTED_SIZE
        table_list = [a for a in cursor.execute('SELECT name FROM sqlite_master WHERE type = "table"')]
        cursor.execute('SELECT * FROM diskio')
        result = cursor.fetchall()
        if len(result):
            check = True
        assert check