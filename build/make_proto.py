#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
import sys
import shutil


def replace_lite_option(target_path, protofilepath, type):
    target_file_path = os.path.basename(protofilepath)
    prototype = "_" + type + ".proto"
    target_file_path = target_path + '/' + target_file_path.replace(".proto", prototype)
    shutil.copyfile(protofilepath, target_file_path)
    # replease lite flag, import file name, add package name
    with open(target_file_path, 'r+') as content:
        newcontent = content.read()
        if type == "standard":
            newcontent = newcontent.replace('option optimize_for = LITE_RUNTIME;\n', '')
            newcontent = newcontent.replace('syntax = "proto3";', 'syntax = "proto3";\npackage ForStandard;')
            newcontent = newcontent.replace('.proto";', '_standard.proto";')
        elif type == "lite":
            newcontent = newcontent.replace('grpc.service";\n',
                                            'grpc.service";\n\noption optimize_for = LITE_RUNTIME;\n\npackage LITE;\n')
        content.seek(0, 0)
        content.write(newcontent)
        content.truncate()
        content.close()


def main():
    type = sys.argv[1]
    target_dir = sys.argv[2]
    i = 3
    while i < len(sys.argv):
        proto_file_path = sys.argv[i]
        replace_lite_option(target_dir, proto_file_path, "standard")
        if type == "lite":
            replace_lite_option(target_dir, proto_file_path, "lite")
        i += 1

if __name__ == '__main__':
    main()
