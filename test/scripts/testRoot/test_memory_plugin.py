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
from string import Template
import os


def get_file_size(file_path):
    size = os.path.getsize(file_path)
    return size


def task():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_memory_plugin.txt -o /data/local/tmp/test_memory_kernel.htrace -t 60 -s -k"')



def task_vminfo():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_memory_vmeminfo.txt -o /data/local/tmp/test_memory_vmeminfo.htrace -t 60 -s -k"')


def task_vmtracker():
    subprocess.check_output(f'hdc shell "hiprofiler_cmd -c /data/local/tmp/config_memory_vmtracker.txt -o /data/local/tmp/test_memory_vmtracker.htrace -t 60 -s -k"')


def write_str_file(file_path, large_string):
    lines = large_string.split('\n')
    with open(file_path, 'w') as file:
        for line in lines:
            file.write(line + '\n')


class TestHiprofilerMemoryPlugin:
    # 检查内核的 memory info 
    @pytest.mark.L0
    def test_memory_plugin_kernel(self):
        subprocess.check_output(f"hdc file send ..\inputfiles\memory_plugin\config_memory_plugin.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task, args=())
        task_thread.start()
        
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_memory_kernel.htrace ../outputfiles/', shell=False,
                         text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_memory_kernel.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_memory_kernel.htrace -e ../outputfiles/test_memory_kernel.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_memory_kernel.db')
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute("select * from sys_mem_measure,sys_event_filter where filter_id = id and name ='sys.mem.mapped'")
        result = cursor.fetchall()
        row_count = len(result)
        #检查获得sys_mem 数据是否正确
        assert(row_count > 0)
        #检查是否存在map 的事件
        for row in result:
            assert(row[3] >= 0)
        #检查是否存在sys_mem_total 事件
        cursor = conn.cursor()
        cursor.execute("select * from sys_mem_measure,sys_event_filter where filter_id = id and name ='sys.mem.total'")
        result = cursor.fetchall()
        row_count = len(result)
        assert(row_count > 0)
        for row in result:
            assert(row[3] >= 0)
        cursor.close()
        conn.close()

    # 检查内核的 virture memory info stats
    @pytest.mark.L0
    def test_memory_plugin_vmeminfo(self):
        subprocess.check_output(f"hdc file send ..\inputfiles\memory_plugin\config_memory_vmeminfo.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_vminfo, args=())
        task_thread.start()
        
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_memory_vmeminfo.htrace ../outputfiles/', shell=False,
                         text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_memory_vmeminfo.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_memory_vmeminfo.htrace -e ../outputfiles/test_memory_vmeminfo.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_memory_vmeminfo.db')
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute("select * from sys_mem_measure,sys_event_filter where filter_id = id and name ='sys.virtual.mem.nr.free.pages'")
        result = cursor.fetchall()
        row_count = len(result)
        #检查获得free pages 数据是否正确
        assert(row_count > 0)
        #检查是否存在 free pages 的事件
        for row in result:
            assert(row[3] >= 0)
        #检查是否存在active file  事件
        cursor = conn.cursor()
        cursor.execute("select * from sys_mem_measure,sys_event_filter where filter_id = id and name ='sys.virtual.mem.nr.active_file'")
        result = cursor.fetchall()
        row_count = len(result)
        assert(row_count > 0)
        for row in result:
            assert(row[3] >= 0)
        cursor.close()
        conn.close()

    # 检查某一个进程的vm ,观测点，DMA 数据，smaps 数据。
    @pytest.mark.L0
    def test_memory_plugin_vmtracker(self):
        #获得32位还是64位   
        sys_bit = subprocess.run(f"hdc shell getconf LONG_BIT", stdout=subprocess.PIPE, text=True, check=True)
        sysinfo = sys_bit.stdout
        if sysinfo.strip() == "32":
            subprocess.check_output(f"hdc shell aa start -a com.ohos.settings.MainAbility -b com.ohos.settings", shell=False, text=True, encoding="utf-8")
            time.sleep(2)
            pid_text = subprocess.run(f"hdc shell pidof 'com.ohos.settings'", stdout=subprocess.PIPE, text=True, check=True)
        else:
            #获得该应用的进程PID
            pid_text = subprocess.run(f"hdc shell pidof 'com.ohos.launcher'", stdout=subprocess.PIPE, text=True, check=True)
        pidinfo = pid_text.stdout
        #读文本文件,修改对应的pid
        file_content = Template('request_id: 1                             \n'
                                ' session_config {                         \n'
                                '  buffers {                               \n'
                                '   pages: 16384                           \n'
                                '  }                                       \n'
                                ' }                                        \n'
                                ' plugin_configs {                         \n'
                                '  plugin_name: "memory-plugin"            \n'
                                '  sample_interval: 5000                   \n'
                                '  config_data {                           \n'
                                '   report_process_tree: false             \n'
                                '   report_sysmem_mem_info: false          \n'
                                '   report_sysmem_vmem_info: false         \n'
                                '   report_process_mem_info: true          \n'
                                '   report_app_mem_info: false             \n'
                                '   report_app_mem_by_memory_service: false\n'
                                '   pid: ${s1}                             \n'
                                '   report_purgeable_ashmem_info: true     \n'
                                '   report_dma_mem_info: true              \n'
                                '   report_gpu_mem_info: true              \n'
                                '   report_smaps_mem_info: true            \n'
                                '   report_gpu_dump_info: true             \n'
                                '  }                                       \n'
                                ' }                                        \n')
        vmfile = file_content.safe_substitute(s1=pidinfo.strip())
        #写入文件
        write_str_file("../inputfiles/memory_plugin/config_memory_vmtracker.txt", vmfile)

        subprocess.check_output(f"hdc file send ..\inputfiles\memory_plugin\config_memory_vmtracker.txt /data/local/tmp/", shell=False,
                                text=True, encoding="utf-8")
        task_thread = threading.Thread(target=task_vmtracker, args=())
        task_thread.start()
        task_thread.join()
        subprocess.run(f'hdc file recv /data/local/tmp/test_memory_vmtracker.htrace ../outputfiles/', shell=False,
                         text=True, encoding="utf-8")
        # 检查文件大小
        file_size = get_file_size(f"../outputfiles/test_memory_vmtracker.htrace")
        assert (file_size > 1024)
        subprocess.check_output(
            r"../inputfiles/trace_streamer_db.exe ../outputfiles/test_memory_vmtracker.htrace -e ../outputfiles/test_memory_vmtracker.db")
        # 连接数据库文件
        conn = sqlite3.connect(r'../outputfiles/test_memory_vmtracker.db')
        #抓取结束后,检查是否存在hidumper 进程
        output_text = subprocess.run(f'hdc shell "ps -ef | grep hidumper"', stdout=subprocess.PIPE, text=True, check=True)
        process_info = output_text.stdout
        lines = process_info.strip().split('\n')
        check_index = False
        for line in lines:
            if line.find("hidumper -s") != -1:
                check_index = True
        # 结束后不存在hidumper 子进程
        assert (check_index == False)
        # # 创建游标对象
        cursor = conn.cursor()
        cursor.execute("select * from memory_dma limit 0,10")
        result = cursor.fetchall()
        row_count = len(result)
        #检查存在dma 数据
        assert(row_count > 0)
        for row in result:
            assert(row[3] > 0 and row[4] > 0 and row[5] > 0 and row[6] > 0 and row[7] > 0 and row[8] > 0 and row[9] > 0)
        #检查是否存在smaps 
        cursor.execute("select * from smaps limit 0,10")
        result = cursor.fetchall()
        row_count = len(result)
        assert(row_count > 0)
        for row in result:
            assert(row[2] != '' and row[3] != '' and row[7] >= 0 and row[8] >= 0)
        #检查是否存在GPU数据
        cursor.execute("select * from memory_process_gpu limit 0,10")
        result = cursor.fetchall()
        row_count = len(result)
        assert(row_count > 0)
        for row in result:
            assert(row[3] > 0 and row[7] > 0)
        cursor.close()
        conn.close()