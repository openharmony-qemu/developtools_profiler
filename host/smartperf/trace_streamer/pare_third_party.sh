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
patch='patch'
sed='sed'
cp='cp'
rm='rm'
case "$OSTYPE" in
  darwin*)  sed="gsed" ;;
esac
if [ ! -d "third_party" ];then
    mkdir third_party
fi
cd third_party

if [ ! -f "sqlite/BUILD.gn" ];then
    rm -rf sqlite
    git clone git@gitee.com:openharmony/third_party_sqlite.git
    if [ -d "third_party_sqlite" ];then
        mv third_party_sqlite sqlite
        $cp ../prebuilts/patch_sqlite/sqlite3build.gn ../third_party/sqlite/BUILD.gn
    fi
fi
if [ ! -f "protobuf/BUILD.gn" ];then
    rm -rf protobuf
    git clone git@gitee.com:openharmony/third_party_protobuf.git
    if [ -d "third_party_protobuf" ];then
        mv third_party_protobuf protobuf
        $cp ../prebuilts/patch_protobuf/protobufbuild.gn ../third_party/protobuf/BUILD.gn
    fi
fi

if [ ! -f "googletest/BUILD.gn" ];then
    rm -rf googletest
    git clone git@gitee.com:openharmony/third_party_googletest.git
    if [ -d "third_party_googletest" ];then
        mv third_party_googletest googletest
        $cp ../prebuilts/patch_googletest/googletestbuild.gn ../third_party/googletest/BUILD.gn
        $patch -p0 ../third_party/googletest/googletest/include/gtest/internal/gtest-internal.h ../prebuilts/patch_googletest/gtest_internal.h.patch
        $patch -p0 ../third_party/googletest/googletest/include/gtest/internal/gtest-port.h ../prebuilts/patch_googletest/gtest_port.h.patch
        $patch -p0 ../third_party/googletest/googletest/include/gtest/gtest-message.h ../prebuilts/patch_googletest/gtest-message.h.patch
        $sed -i "/using ::std::string/s/^\(.*\)$/\/\/\1/g" ../third_party/googletest/googletest/include/gtest/hwext/gtest-tag.h
    fi
fi

if [ ! -f "json/BUILD.gn" ];then
    rm -rf json
    git clone git@gitee.com:openharmony/third_party_json.git
    if [ -d "third_party_json" ];then
        mv third_party_json json
    fi
fi

if [ ! -f "libunwind/BUILD.gn" ];then
    rm -rf libunwind
    git clone git@gitee.com:openharmony/third_party_libunwind.git
    if [ -d "third_party_libunwind" ];then
        mv third_party_libunwind libunwind
        $cp ../prebuilts/patch_libunwind/libunwindbuild.gn libunwind/BUILD.gn
    fi
fi

if [ ! -f "perf_include/libbpf/linux/perf_event.h" ];then
   mkdir -p perf_include/libbpf/linux
   rm -rf perf_event.h
   curl https://gitee.com/openharmony/third_party_libbpf/raw/20221117/github.com/libbpf/libbpf/refs/tags/v0.7.0/include/uapi/linux/perf_event.h > perf_event.h
   mv perf_event.h perf_include/libbpf/linux/perf_event.h
   $patch -p0 perf_include/libbpf/linux/perf_event.h ../prebuilts/patch_perf_event/perf_event.h.patch
fi

if [ ! -f "perf_include/musl/elf.h" ];then
   mkdir -p perf_include/musl
   rm -rf elf.h
   curl https://gitee.com/openharmony/third_party_musl/raw/master/include/elf.h > elf.h
   mv elf.h perf_include/musl/elf.h
fi

if [ ! -f "hiperf/BUILD.gn" ];then
    rm -rf hiperf developtools_hiperf
    git clone -b OpenHarmony-3.2-Release --depth=1 git@gitee.com:openharmony/developtools_hiperf.git
    if [ -d "developtools_hiperf" ];then
        mv developtools_hiperf hiperf
        $cp ../prebuilts/patch_hiperf/BUILD.gn ../third_party/hiperf/BUILD.gn
        # report.h
        # remove #include "report_json_file.h"
        $sed -i "/#include \"report_json_file.h\"/s/^\(.*\)$/\/\/\1/g" hiperf/include/report.h
        $sed -i "/#include <gtest\/gtest_prod.h>/s/^\(.*\)$/\/\/\1/g" hiperf/include/debug_logger.h
        $sed -i "/#include <gtest\/gtest_prod.h>/s/^\(.*\)$/\/\/\1/g" hiperf/include/utilities.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/virtual_thread.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/callstack.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/symbols_file.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/virtual_runtime.h
        # elf_parser.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/report.h
        #include <../musl/include/elf.h>
        # 替换为
        #include <elf.h>
        $sed -i "s/..\/musl\/include\/elf.h/elf.h/g" hiperf/include/elf_parser.h
        # virtual_thread.h
        # HIPERF_DEBUG 替换为 ALWAYSTRUE
        $sed -i "s/HIPERF_DEBUG/ALWAYSTRUE/g" hiperf/include/virtual_thread.h
        $cp ../prebuilts/patch_hiperf/file_ex.h hiperf/include/nonlinux/linux
        $cp ../prebuilts/patch_hiperf/unique_fd.h hiperf/include/nonlinux/linux
        $sed -i "/using __s8 = char;/a #define unw_word_t uint64_t" hiperf/include/nonlinux/linux/types.h
        $sed -i '/^void Report::PrepareConsole(/,/^}/ s/^.*$/\/\/&/; /^void Report::PrepareConsole(/,/return;/ s/^[[:blank:]]*/    /' hiperf/src/report.cpp
        $sed -i '/namespace HiPerf {/avoid Report::PrepareConsole(){ return;}' hiperf/src/report.cpp
    fi
fi

if [ ! -f "bounds_checking_function/BUILD.gn" ];then
    rm -rf bounds_checking_function
    git clone git@gitee.com:openharmony/third_party_bounds_checking_function.git bounds_checking_function
    $cp ../prebuilts/patch_bounds_checking_function/bounds_checking_functionbuild.gn bounds_checking_function/BUILD.gn
fi
