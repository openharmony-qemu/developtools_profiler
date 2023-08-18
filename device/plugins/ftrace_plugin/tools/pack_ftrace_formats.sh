#!/bin/bash
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
set -e
FTRACE_DIR=/sys/kernel/debug/tracing
TARGET_DIR=snapshot
if [ "$1" != "" ]; then
	TARGET_DIR=$1
fi

find $FTRACE_DIR -name format | sed "s|$FTRACE_DIR/||g" | sort  > format.list

for i in $(cat format.list); do
	subdir=$(dirname $i)
	dstdir=$TARGET_DIR/$subdir
	if [ ! -e "$dstdir" ]; then
		mkdir -v -p $dstdir
	fi
	if [ -e "$TARGET_DIR/$i" ]; then
		echo "Skip $i ..."
	else
		cp -v $FTRACE_DIR/$i $dstdir
	fi
done

tar czvf $TARGET_DIR.tar.gz $TARGET_DIR
