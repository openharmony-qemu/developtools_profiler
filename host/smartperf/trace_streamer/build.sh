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
PARAMS=$*
echo $PARAMS
echo "begin to check input"
SOURCE="${BASH_SOURCE[0]}"
cd $(dirname ${SOURCE})
./pare_third_party.sh
target_os="linux"
target_dir="linux"
gn_path="linux"
is_debug="false"
is_clean="false"
target="trace_streamer"
gn="gn"
ninja="ninja"
case "$OSTYPE" in
  solaris*) echo "SOLARIS" ;;
  darwin*)  gn_path="macx" target_os="macx" ;;
  linux*)   gn_path="linux" target_os="linux"  ;;
  bsd*)     echo "is bsd os" ;;
  msys*)    gn_path="windows" target_os="windows" gn="gn.exe" ninja="ninja.exe"  ;;
  *)        echo "unknown: $OSTYPE" ;;
esac
usage="Usage: $basename $0 wasm/test/fuzz/protoc debug/release/clean"

./dl_tools.sh $gn_path

if { [ "$1" == "dubaisdk" ] || [ "$1" == "sdkdemo" ] || [ "$1" == "wasm" ] || [ "$1" == "test" ] || [ "$1" == "fuzz" ]; } && [ "$#" -ne 0 ];then
    TARGET_DIR=$1
    if [[ $PARAMS == *"debug"* ]]; then
        TARGET_DIR=$1"_debug"
    fi
    if [ ! -f "out/$TARGET_DIR/clang_x64/developtools/smartperf_host/protoc" ] && [ "$1" != "protoc" ];then
        ./build.sh protoc
        mkdir -p out/$TARGET_DIR/clang_x64/developtools/smartperf_host
        cp out/$target_os/clang_x64/developtools/smartperf_host/protoc out/$TARGET_DIR/clang_x64/developtools/smartperf_host/protoc
    fi
    if [ ! -f "out/$TARGET_DIR/clang_x64/developtools/smartperf_host/protoreader_plugin" ] && [ "$1" != "spb" ] && [ -f "out/$TARGET_DIR/clang_x64/developtools/smartperf_host/protoc" ];then
        ./build.sh spb
        mkdir -p out/$TARGET_DIR/clang_x64/developtools/smartperf_host
        cp out/$target_os/clang_x64/developtools/smartperf_host/protoreader_plugin out/$TARGET_DIR/clang_x64/developtools/smartperf_host/protoreader_plugin
    fi
fi
if [ $target_os == "windows" ];then
    cp .gn_win .gn
else
    cp .gn_unix .gn
fi
if [ "$1" == "windows" ];then
    echo "gn only support linux and wasm build currently"
    if [ ! -d "out/windows" ];then
        mkdir out/windows
    fi
    touch out/windows/trace_streamer.exe
    exit
fi

if [ "$#" -ne "0" ];then
    if [ "$1" == "wasm" ];then
        ./dl_emsdk.sh
        target="wasm"
    fi
    if [ "$1" == "test" ];then
        target="test"
    fi
    if [ "$1" == "fuzz" ];then
        target="fuzz"
    fi
    if [ "$1" == "protoc" ];then
        target="protoc"
    fi
    if [ "$1" == "sdkdemo" ];then
        target="sdkdemo"
    fi
    if [ "$1" == "dubaisdk" ];then
        target="dubaisdk"
    fi
    if [ "$1" == "sdkdemotest" ];then
        target="sdkdemotest"
    fi
    if [ "$1" == "spb" ];then
        target="spb"
    fi
fi
target_operator="$2"
if [ "$target" == "wasm" ] && [ "$target_os" == "windows" ];then
    echo "!!!build wasm on winows will occur unknown error, strongly suggest you build wasm on linux(Ubuntu)"
    exit
fi
./build_operator.sh $is_debug $target $target_os $is_clean $gn_path $gn $ninja $target_operator
