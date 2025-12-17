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
import zipfile
import subprocess
import re
import time
import json
import stat

OUTPUT_PATH = "testModule/output"
PID_INDEX = 7
SMALL_FILE_EXPECTED_SIZE = 1 * 1024
SMALL_FILE_EXPECTED_SIZE_2 = 5 * 1024
MID_FILE_EXPECTED_SIZE = 100 * 1024


def run_and_get_output(input, text=True, encoding="utf-8", shell = False):
    output = subprocess.check_output(input.split(), text=text, encoding=encoding, shell=shell)
    print(f"input cmd: {input}")
    print(f"output: {output}")
    return output

def task_template(extend=False):
    if extend:
        subprocess.check_output(
            r"hdc shell hiprofiler_cmd -c /data/local/tmp/config.txt -o /data/local/tmp/test.htrace -t 50 -s -k",
            text=True, encoding="utf-8")
    else:
        subprocess.check_output(
            r"hdc shell hiprofiler_cmd -c /data/local/tmp/config.txt -o /data/local/tmp/test.htrace -t 30 -s -k",
            text=True, encoding="utf-8")

def write_config_file(content):
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    mode = stat.S_IWUSR | stat.S_IRUSR
    with os.fdopen(os.open(r".\..\inputfiles\nativehook\config.txt", flags, mode), 'w') as file:
        file.write(content)
    subprocess.check_output(r"hdc file send .\..\inputfiles\nativehook\config.txt /data/local/tmp/",
                            text=True, encoding="utf-8")

def write_str_file(file_path, large_string):
    lines = large_string.split('\n')
    with open(file_path, 'w') as file:
        for line in lines:
            file.write(line + '\n')

def delete_old_files():
    try:
        subprocess.check_output(r"del .\..\inputfiles\nativehook\config.txt", text=True, encoding="utf-8")
        subprocess.check_output(r"del .\..\outputfiles\test.htrace", text=True, encoding="utf-8")
        subprocess.check_output(r"del .\..\inputfiles\layout.json", text=True, encoding="utf-8")
        subprocess.check_output(r"del .\..\outputfiles\nativehook.db", text=True, encoding="utf-8")
        subprocess.check_output("hdc shell rm /data/local/tmp/test.htrace", text=True, encoding="utf-8")
    except Exception as e:
        print(f"An error occurred: {e}")
        pass


def get_path_by_attribute(tree, key, value):
    attributes = tree['attributes']
    if attributes is None:
        print("tree contains no attributes")
        return None
    path = []
    if attributes.get(key) == value:
        return path
    for index, child in enumerate(tree['children']):
        child_path = path + [index]
        result = get_path_by_attribute(child, key, value)
        if result is not None:
            return child_path + result
    return None


def get_element_by_path(tree, path):
    if len(path) == 1:
        return tree['children'][path[0]]
    return get_element_by_path(tree['children'][path[0]], path[1:])


def get_location_by_text(tree, text):
    path = get_path_by_attribute(tree, "text", text)
    if path is None or len(path) == 0:
        print("text not found in layout file")
    element = get_element_by_path(tree, path)
    locations = element['attributes']['bounds'].replace('[', '').replace(']', ' ').replace(',', ' ').strip().split()
    return int((int(locations[0]) + int(locations[2])) / 2), int((int(locations[1]) + int(locations[3])) / 2)


def touch(dx, dy):
    run_and_get_output(f"hdc shell uitest uiInput click {dx} {dy}")


def get_layout_tree():
    output = subprocess.check_output("hdc shell uitest dumpLayout", text=True)
    path = output.strip().split(":")[-1]
    run_and_get_output(f"hdc file recv {path} .\..\inputfiles\layout.json")
    run_and_get_output("hdc shell rm " + path)
    with open(".\..\inputfiles\layout.json", encoding="utf-8") as f:
        tree = json.load(f)
    return tree


def touch_button(text):
    layout_tree = get_layout_tree()
    location = get_location_by_text(layout_tree, text)
    touch(location[0], location[1])


def get_pid(name):
    run_and_get_output("hdc shell ps -ef | grep " + name + " > /data/local/tmp/pids.txt")
    run_and_get_output(f"hdc file recv /data/local/tmp/pids.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    with open(r'.\..\outputfiles\pids.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if line.split()[PID_INDEX] == name:
                return line.split()[1]
    return 0


def get_pid_by_process_name(process_name):
    pid = None
    cmd = f"hdc shell \"pidof {process_name}\""
    try:
        pid = subprocess.check_output(cmd, encoding="utf-8", text=True)
        pid = int(pid.strip())
    except subprocess.CalledProcessError as e:
        print(f"Command failed: {cmd}\nError: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    return pid