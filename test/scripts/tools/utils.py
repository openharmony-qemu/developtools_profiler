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

OUTPUT_PATH = "testModule/output"
PID_INDEX = 7


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
    output = subprocess.check_output(f"hdc shell uitest uiInput click {dx} {dy}")


def get_layout_tree():
    output = subprocess.check_output("hdc shell uitest dumpLayout", text=True)
    path = output.strip().split(":")[-1]
    subprocess.check_output(f"hdc file recv {path} .\..\inputfiles\layout.json")
    subprocess.check_output("hdc shell rm " + path)
    with open(".\..\inputfiles\layout.json", encoding="utf-8") as f:
        tree = json.load(f)
    return tree


def touch_button(text):
    layout_tree = get_layout_tree()
    location = get_location_by_text(layout_tree, text)
    touch(location[0], location[1])


def get_pid(name):
    subprocess.check_output("hdc shell ps -ef | grep " + name + " > /data/local/tmp/pids.txt")
    subprocess.check_output(f"hdc file recv /data/local/tmp/pids.txt .\..\outputfiles\ ", text=True, encoding="utf-8")
    with open(r'.\..\outputfiles\pids.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            if line.split()[PID_INDEX] == name:
                return line.split()[1]
    return 0
