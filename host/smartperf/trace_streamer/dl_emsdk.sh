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
if [ -d "prebuilts/emsdk" ] && [ ! -d "prebuilts/emsdk/emsdk/emscripten" ];then
    rm -rf prebuilts/emsdk
fi
if [ ! -d "prebuilts/emsdk" ];then
    echo "you need emsdk to compile wasm"
    if [ ! -d "tools" ];then
        mkdir tools
    fi
    if [ ! -d "tools/emsdk" ];then
        cd tools
        git clone https://github.com/emscripten-core/emsdk.git --depth=1
        cd emsdk
        git pull
        ./emsdk install 3.1.12
        ./emsdk activate 3.1.12
        cd ../../
    fi
    if [ ! -d "prebuilts/emsdk" ];then
        mkdir prebuilts/emsdk
    fi
    if [ ! -d "prebuilts/emsdk/emsdk" ];then
        mkdir prebuilts/emsdk/emsdk
    fi
    if [ ! -d "prebuilts/emsdk/node" ];then
        mkdir prebuilts/emsdk/node
    fi
    mv tools/emsdk/upstream/* prebuilts/emsdk/emsdk
    mv tools/emsdk/node/* prebuilts/emsdk/node
fi
