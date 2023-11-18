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

if [ ! -d "perf_include/hiviewdfx/hilog" ];then
   rm -rf hiviewdfx_hilog perf_include/hiviewdfx/hilog
   mkdir -p perf_include/hiviewdfx/hilog
   git clone --depth=1 https://gitee.com/openharmony/hiviewdfx_hilog.git
   mv hiviewdfx_hilog/interfaces/native/innerkits/include/ perf_include/hiviewdfx/hilog
   rm -rf hiviewdfx_hilog
fi

if [ ! -d "perf_include/hiviewdfx/faultloggerd" ];then
   rm -rf hiviewdfx_faultloggerd perf_include/hiviewdfx/faultloggerd
   mkdir -p perf_include/hiviewdfx/faultloggerd/interfaces/innerkits
   git clone --depth=1 git@gitee.com:openharmony/hiviewdfx_faultloggerd.git
   mv hiviewdfx_faultloggerd/common/ perf_include/hiviewdfx/faultloggerd
   mv hiviewdfx_faultloggerd/interfaces/common/ perf_include/hiviewdfx/faultloggerd/interfaces
   mv hiviewdfx_faultloggerd/interfaces/nonlinux/ perf_include/hiviewdfx/faultloggerd/interfaces
   mv hiviewdfx_faultloggerd/interfaces/innerkits/unwinder/ perf_include/hiviewdfx/faultloggerd/interfaces/innerkits
   find  perf_include/hiviewdfx/faultloggerd -type f -name "*.gn" -delete
    $cp ../prebuilts/patch_hiperf/hiviewdfx_BUILD.gn ../third_party/perf_include/hiviewdfx/BUILD.gn
   rm -rf hiviewdfx_faultloggerd
   rm -rf perf_include/hiviewdfx/common/build
   rm -rf perf_include/hiviewdfx/common/cutil
   rm perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/dfx_regs_x86_64.cpp
    $sed -i '/HiLogPrint/s/^/\/\/ /' perf_include/hiviewdfx/faultloggerd/common/dfxlog/dfx_log.cpp
    $sed -i '/TRAP_BRANCH/s/^/\/\/ /' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/dfx_signal.cpp
    $sed -i '/TRAP_HWBKPT/s/^/\/\/ /' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/dfx_signal.cpp
    $sed -i '/is_ohos/s/is_ohos/true/g' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/dfx_mmap.cpp
    $sed -i '/is_ohos/s/is_ohos/true/g' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/include/dfx_regs.h
    $sed -i '/#include <vector>/a #include "debug_logger.h"' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/include/unwinder.h
    $sed -i '/VerifyFilePath/s/const std::vector<const std::string>/std::vector\<std::string\>/g' perf_include/hiviewdfx/faultloggerd/common/dfxutil/dfx_util.h
    $sed -i '/VerifyFilePath/s/const std::vector<const std::string>/std::vector\<std::string\>/g' perf_include/hiviewdfx/faultloggerd/common/dfxutil/dfx_util.cpp
    $sed -i '/getpid() == gettid()/s/getpid() == gettid()/false/g' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/unwinder.cpp
    $sed -i '/!realpath(path, realPath)/s/!realpath(path, realPath)/false/g' perf_include/hiviewdfx/faultloggerd/common/dfxutil/dfx_util.cpp
    $sed -i '/#include "dfx_util.h"/a #include "utilities.h"' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/dfx_mmap.cpp
    $sed -i '/#include <string>/a #include "utilities.h"' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/include/dfx_mmap.h
    $sed -i '/#ifndef is_ohos_lite/s/#ifndef is_ohos_lite/#if false/g' perf_include/hiviewdfx/faultloggerd/interfaces/innerkits/unwinder/dfx_elf.cpp
fi
if [ ! -f "hiperf/BUILD.gn" ];then
    rm -rf hiperf developtools_hiperf
    git clone --depth=1 git@gitee.com:openharmony/developtools_hiperf.git
    if [ -d "developtools_hiperf" ];then
        mv developtools_hiperf hiperf
        $cp ../prebuilts/patch_hiperf/BUILD.gn ../third_party/hiperf/BUILD.gn
        $cp ../prebuilts/patch_hiperf/file_ex.h hiperf/include/nonlinux/linux
        $cp ../prebuilts/patch_hiperf/unique_fd.h hiperf/include/nonlinux/linux
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/virtual_thread.h
        $sed -i "s/HIPERF_DEBUG/ALWAYSTRUE/g" hiperf/include/virtual_thread.h
        $sed -i "/#include \"report_json_file.h\"/s/^\(.*\)$/\/\/\1/g" hiperf/include/report.h
        $sed -i "/#include <gtest\/gtest_prod.h>/s/^\(.*\)$/\/\/\1/g" hiperf/include/debug_logger.h
        $sed -i "/#include <gtest\/gtest_prod.h>/s/^\(.*\)$/\/\/\1/g" hiperf/include/utilities.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/virtual_thread.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/callstack.h
        $sed -i '/unwinder.h/s/^/\/\/ /' hiperf/include/callstack.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/symbols_file.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/virtual_runtime.h
        $sed -i "/FRIEND_TEST/s/^\(.*\)$/\/\/\1/g" hiperf/include/report.h

        $sed -i "s/HIPERF_DEBUG/ALWAYSTRUE/g" hiperf/include/virtual_thread.h
        $sed -i "/using __s8 = char;/a #define unw_word_t uint64_t" hiperf/include/nonlinux/linux/types.h
        $sed -i '/^void Report::PrepareConsole(/,/^}/ s/^.*$/\/\/&/; /^void Report::PrepareConsole(/,/return;/ s/^[[:blank:]]*/    /' hiperf/src/report.cpp
        $sed -i '/namespace HiPerf {/avoid Report::PrepareConsole(){ return;}' hiperf/src/report.cpp
        $sed -i '/HITRACE_METER_NAME/s/^/\/\/ /' hiperf/src/callstack.cpp
        $sed -i '/hitrace_meter.h/s/^/\/\/ /' hiperf/src/callstack.cpp
        $sed -i '/dlfcn.h/s/^/\/\/ /' hiperf/src/callstack.cpp
        $sed -i '/dfx_ark.h/s/^/\/\/ /' hiperf/src/callstack.cpp
        $sed -i '/dfx_regs.h/s/^/\/\/ /' hiperf/src/callstack.cpp
        $sed -i '/return DoUnwind2/s/^/\/\/ /' hiperf/src/callstack.cpp
        $sed -i '/#if defined(is_ohos) && is_ohos/s/defined(is_ohos) && is_ohos/true/g' hiperf/src/virtual_runtime.cpp
        $sed -i '/#if defined(is_ohos) && is_ohos/s/defined(is_ohos) && is_ohos/true/g' hiperf/include/virtual_runtime.h
        $sed -i '/symbolsTable, elfFile_, elfPath/s/symbolsTable, elfFile_, elfPath/symbolsTable, elfFile_, filePath_/g' hiperf/src/symbols_file.cpp
    fi
fi

if [ ! -f "bounds_checking_function/BUILD.gn" ];then
    rm -rf bounds_checking_function
    git clone git@gitee.com:openharmony/third_party_bounds_checking_function.git bounds_checking_function
    $cp ../prebuilts/patch_bounds_checking_function/bounds_checking_functionbuild.gn bounds_checking_function/BUILD.gn
fi

if [ ! -f "commonlibrary/c_utils/base/include/nocopyable.h" ];then
    rm -rf commonlibrary
    git clone git@gitee.com:openharmony/commonlibrary_c_utils.git
    if [ -d "commonlibrary_c_utils" ];then
        mkdir -p commonlibrary/c_utils/base/include
        $cp commonlibrary_c_utils/base/include/nocopyable.h commonlibrary/c_utils/base/include
        rm -rf commonlibrary_c_utils
    fi
fi

if [ ! -f "profiler/device/plugins/ftrace_plugin/include/ftrace_common_type.h" ];then
    rm -rf profiler
    git clone git@gitee.com:openharmony/developtools_profiler.git
    if [ -d "developtools_profiler" ];then
        mkdir -p profiler/device/plugins/ftrace_plugin/include
        $cp developtools_profiler/device/plugins/ftrace_plugin/include/ftrace_common_type.h profiler/device/plugins/ftrace_plugin/include
        $cp developtools_profiler/device/plugins/ftrace_plugin/include/ftrace_namespace.h profiler/device/plugins/ftrace_plugin/include
        rm -rf developtools_profiler
    fi
fi
