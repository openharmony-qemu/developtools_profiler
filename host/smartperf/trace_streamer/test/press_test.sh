#!/bin/bash
# Copyright (C) 2021 Huawei Device Co., Ltd.
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
set -e
repeated=1000
if [ "$#" -ne "0" ];then
    repeated = $1
fi
echo "begin" > repeated.text
for ((i=1; i<=$repeated; i ++))
do
# user_trace_20230222203404.trace
# user_trace_20230222203015
echo $i
./out/macx/trace_streamer ~/SourceData/repeated/user_trace_20230222203404.trace -e 1.db >> repeated.txt
done
echo "end" >> repeated.text