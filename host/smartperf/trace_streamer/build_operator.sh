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
ext=""
clang_dir="clang_x64"
target_dir="linux"
subsys_name="developtools"
part_name="smartperf_host"
is_debug="$1"
target="$2"
target_os="$3"
is_clean="$4"
gn_path="$5"
gn="$6"
ninja="$7"
target_operator="$8"
if [ "$#" -ge "7" ];then
    if [ "$target" != "trace" ] && [ "$target" != "linux" ] && [ "$target" != "windows" ] &&
        [ "$target" != "macx" ] && [ "$target" != "trace_streamer" ] && [ "$target" != "wasm" ] &&
        [ "$target" != "test" ] && [ "$target" != "spb" ] && [ "$target" != "fuzz" ] &&
        [ "$target" != "protoc" ] && [ "$target" != "sdkdemo" ] && [ "$target" != "dubaisdk" ] && [ "$target" != "sdkdemotest" ];then
        echo "failed"
        exit
    fi
    if [ "$target_operator" != "" ] && [ "$target_operator" != "debug" ] && [ "$target_operator" != "release" ] && [ "$target_operator" != "clean" ];then
        if [ "$target_operator" == "protoc" ];then
        target=$target_operator
        else
        echo "failed"
        exit
        fi
    fi
    if [ "$target_operator" == "debug" ];then
        is_debug="true"
    elif [ "$target_operator" == "clean" ];then
        is_clean="true"
    else
        is_debug="false"
    fi
    echo "platform is $target_os"
    echo "isdebug: $is_debug"
    echo "isclean: $is_clean"
else
    echo "$usage"
    echo "It is not recommended to execute this file and use it by build.sh."
    echo "use default input paramter"
    echo "platform is $target_os"
    echo "target is $target"
    echo "is_debug:$is_debug"
    exit
fi
if [ "$is_debug" != "false" ];then
    ext="_debug"
fi

if [ "$target" == "test" ] || [ "$target" == "fuzz" ] || [ "$target"="wasm" ] || [ "$target"="sdkdemo" ] || [ "$target"="sdkdemotest" ];then
    target_dir=$target
else
    target_dir=$target_os
fi
if [ "$target" == "trace_streamer" ] || [ "$target" == "trace" ] || [ "$target" == "spb" ] || [ "$target" == "protoc" ];then
    target_dir=$target_os
fi
echo "target_dir:" $target_dir
echo "target:" $target

if [ "$is_clean" == "true"  ];then
    prebuilts/$gn_path/$gn gen out/"$target_dir""$ext"/$clang_dir/$subsys_name/$part_name --clean
    prebuilts/$gn_path/$ninja -C out/"$target_dir""$ext"/$clang_dir/$subsys_name/$part_name -t clean
else
    prebuilts/$gn_path/$gn gen out/"$target_dir""$ext"/$clang_dir/$subsys_name/$part_name --args='is_debug='"$is_debug"' target="'"$target"'" target_os="'"$target_os"'" is_independent_compile=true'
    echo "begin to build ..."
    prebuilts/$gn_path/$ninja -C out/"$target_dir""$ext"/$clang_dir/$subsys_name/$part_name
    if [ "$target" != "protoc" ] && [ "$target" != "spb" ];then
        cp -r out/"$target_dir""$ext"/$clang_dir/$subsys_name/$part_name/* out/"$target_dir""$ext"/
    fi
fi
