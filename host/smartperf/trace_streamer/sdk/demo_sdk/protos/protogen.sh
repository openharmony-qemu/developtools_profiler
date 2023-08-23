#!/bin/bash
# Copyright (c) 2021 Huawei Device Co., Ltd.
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
SOURCE="${BASH_SOURCE[0]}"
cd $(dirname ${SOURCE})
echo "begin to generate proto based files"
SOURCE=$(dirname ${SOURCE})
proto_dir="."
services_dir="$proto_dir/services"
# kernel_version="5.10.79_aarch64"
kernel_version="."
mock_data_dir="$proto_dir/types/plugins/mock_data"
proto_array=("$mock_data_dir/mock_plugin_result.proto")

export LD_LIBRARY_PATH=../../out/linux
for ((i = 0; i < ${#proto_array[@]}; i ++))
do
   newpath=$(dirname ${proto_array[$i]})
   newpath=${newpath:2}
   cppout=../../third_party/protogen/$newpath
   mkdir -p $cppout
   ../../../out/linux/protoc --proto_path=$mock_data_dir --cpp_out=$cppout ${proto_array[$i]}
done
echo "generate proto based files over"
