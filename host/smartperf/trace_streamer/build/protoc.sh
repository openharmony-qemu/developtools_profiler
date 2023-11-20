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
THIS_DIR=$(dirname ${BASH_SOURCE[0]})
PROJECT_TOP=$(realpath $THIS_DIR/..)
TAIL_DIR="."
SUBSYS_DIR="."
if [[ "$3" == *"developtools"* ]]; then
  TAIL_DIR="thirdparty/protobuf"
  SUBSYS_DIR="developtools/smartperf_host"
  PROJECT_TOP=$(realpath $THIS_DIR/../../../..)
fi

OHOS_X64_OUT=$PROJECT_TOP/$2/
LIBCXX_X64_OUT=$PROJECT_TOP/$1/ndk/libcxx/linux_x86_64
SUBSYS_X64_OUT=$PROJECT_TOP/$2/$TAIL_DIR

PROTOC_DIR=$PROJECT_TOP/$2/$TAIL_DIR
PROTOC=$PROJECT_TOP/$2/$TAIL_DIR/protoc
OPT_PLUGIN_PROTOREADER_PATH=$PROJECT_TOP/$2/$SUBSYS_DIR/protoreader_plugin
OPT_PLUGIN_PROTOREADER="--plugin=protoc-gen-plugin=$PROJECT_TOP/$2/$SUBSYS_DIR/protoreader_plugin --plugin_out=wrapper_namespace=ProtoReader"
OPT_OUT=--opt_out
OPT_PROTOREADER_OUT=--cpp_out
TMP=$2
PROTO_OUT_DIR="$PROJECT_TOP/${TMP%/*}/$3" # path of the new proto file

echo "PROTOC_DIR = $PROTOC_DIR"
if ls "$PROTOC_DIR"/*.dylib 1>/dev/null 2>&1; then
  cp $PROTOC_DIR/*.dylib $PROJECT_TOP/$2/$SUBSYS_DIR/
else
  echo "*.dylib Not Found!"
fi

PARAMS=$*
echo "PARAMS = $PARAMS"
PARAMS_FILTER="$1 $2 $3"

# creat pb file
PARAMS_SRC=${PARAMS:${#PARAMS_FILTER}}

# avoid conflict, param4=--plugin* means ipc plugin, generate encode file if opt plugin exist
if [[ "$4" != --plugin* ]]; then
  if [ -f "$OPT_PLUGIN_PROTOREADER_PATH" ]; then
    echo "generate protobuf optimize code OPT_PLUGIN_PROTOREADER = $OPT_PLUGIN_PROTOREADER"
    LD_LIBRARY_PATH=$LIBCXX_X64_OUT:$SUBSYS_X64_OUT exec $PROTOC $OPT_PLUGIN_PROTOREADER:$5 $PARAMS_SRC
  fi
fi
echo "EXEC: LD_LIBRARY_PATH=$LIBCXX_X64_OUT:$SUBSYS_X64_OUT $PROTOC $PARAMS_SRC"
LD_LIBRARY_PATH=$LIBCXX_X64_OUT:$SUBSYS_X64_OUT exec $PROTOC $PARAMS_SRC
