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
import sys
sys.path.append("..")
from tools.utils import *
import threading


class TestHiprofilerReliability:
    @pytest.mark.L0
    def test_reliability_nocrash(self):
        check = True
        with open(r'.\..\outputfiles\faultlog.txt', 'r') as file:
            lines = file.readlines()
            for line in lines:
                if "hiprofilerd" in line:
                    check = False
                if "hiprofiler_plugins" in line:
                    check = False
                if "native_daemon" in line:
                    check = False
                if "com.example.insight_test_stage" in line:
                    check = False
                if "com.ohos.launcher" in line:
                    check = False
        assert check == True