/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import hidebug from '@ohos.hidebug'
import fs from '@ohos.file.fs'
import process from '@ohos.process'
import featureAbility from '@ohos.ability.featureAbility'

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("HidebugJsTest", function () {
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all testcases
         */
         console.info('HidebugJsTest beforeAll called')
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all testcases
         */
         console.info('HidebugJsTest afterAll called')
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each testcases
         */
         console.info('HidebugJsTest beforeEach called')
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each testcases
         */
         console.info('HidebugJsTest afterEach called')
    })

    async function msleep(time) {
        let promise = new Promise((resolve, reject) => {
            setTimeout(() => resolve("done!"), time)
        });
        let result = await promise;
    }

    /**
     * test
     *
     * @tc.name: HidebugJsTest_001
     * @tc.desc: 检测cpuProfiler采集的cpuprofiler数据是否含有js napi callframe信息
     * @tc.type: FUNC
     * @tc.require: issueI5NXHX
     */
    it('HidebugJsTest_001', 0, function () {
        console.info("---------------------------HidebugJsTest_001----------------------------------");
        try {
            let timestamp = Date.now();
            let filename = "cpuprofiler_" + timestamp.toString();
            hidebug.startProfiling(filename);
            for (let i = 0; i < 3; i++) {
                hidebug.getSharedDirty();
            }
            hidebug.stopProfiling();
            let path = "/proc/self/root/data/storage/el2/base/files/" + filename + ".json";
            let data = fs.readTextSync(path);
            if (data.includes("napi")) {
                expect(true).assertTrue();
            } else {
                expect(false).assertTrue();
            }
        } catch (err) {
            console.error('HidebugJsTest_001 has failed for ' + err);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_002
     * @tc.desc: startJsCpuProfiling/stopJsCpuProfiling的正常测试, startProfiling/stopProfiling的更新版本
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_002', 0, function () {
        console.info("---------------------------HidebugJsTest_002----------------------------------");
        try {
            let timestamp = Date.now();
            let filename = "cpuprofiler_" + timestamp.toString();
            hidebug.startJsCpuProfiling(filename);
            for (let i = 0; i < 3; i++) {
                hidebug.getSharedDirty();
            }
            hidebug.stopJsCpuProfiling();
            let path = "/proc/self/root/data/storage/el2/base/files/" + filename + ".json";
            let data = fs.readTextSync(path);
            if (data.includes("napi")) {
                expect(true).assertTrue();
            } else {
                expect(false).assertTrue();
            }
        } catch (err) {
            console.error('HidebugJsTest_002 has failed for ' + err);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_003
     * @tc.desc: startJsCpuProfiling/stopJsCpuProfiling的异常测试, startProfiling/stopProfiling的更新版本
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_003', 0, function () {
        console.info("---------------------------HidebugJsTest_003----------------------------------");
        try {
            hidebug.startJsCpuProfiling();
            for (let i = 0; i < 3; i++) {
                hidebug.getSharedDirty();
            }
            hidebug.stopJsCpuProfiling();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(error.code === "401").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_004
     * @tc.desc: dumpJsHeapData的正常测试, dumpHeapData的更新版本
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_004', 0, function () {
        console.info("---------------------------HidebugJsTest_004----------------------------------");
        try {
            hidebug.dumpJsHeapData("heapData");
            expect(true).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
        }
    })

    /**
     * @tc.name: HidebugJsTest_005
     * @tc.desc: dumpJsHeapData的异常测试, dumpHeapData的更新版本
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_005', 0, function () {
        console.info("---------------------------HidebugJsTest_005----------------------------------");
        try {
            hidebug.dumpJsHeapData();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(error.code === "401").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_006
     * @tc.desc: getServiceDump的正常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_006', 0, function () {
        console.info("---------------------------HidebugJsTest_006----------------------------------");
        let context = featureAbility.getContext();
        context.getFilesDir().then((data) => {
            const path = data + "/serviceInfo1.txt";
            console.info("output path: " + path);
            let file = fs.openSync(path, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
            const serviceId = 10;
            const args = new Array("allInfo");
            try {
              hidebug.getServiceDump(serviceId, file.fd, args);
              expect(true).assertTrue();
            } catch (error) {
              console.info(error.code);
              console.info(error.message);
            }
            fs.closeSync(file);
        })
    })

    /**
     * @tc.name: HidebugJsTest_007
     * @tc.desc: getServiceDump的异常测试，参数错误
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_007', 0, function () {
        console.info("---------------------------HidebugJsTest_007----------------------------------");
        let context = featureAbility.getContext();
        context.getFilesDir().then((data) => {
            const path = data + "/serviceInfo2.txt";
            console.info("output path: " + path);
            let file = fs.openSync(path, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
            const serviceId = 10;
            const args = new Array("allInfo");
            try {
                hidebug.getServiceDump(serviceId);
            } catch (error) {
              console.info(error.code);
              console.info(error.message);
              expect(error.code === "401").assertTrue();
            }
            fs.closeSync(file);
        })
    })

    /**
     * @tc.name: HidebugJsTest_008
     * @tc.desc: getServiceDump的异常测试，查询system ability失败
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
     it('HidebugJsTest_008', 0, function () {
        console.info("---------------------------HidebugJsTest_008----------------------------------");
        let context = featureAbility.getContext();
        context.getFilesDir().then((data) => {
            const path = data + "/serviceInfo3.txt";
            console.info("output path: " + path);
            let file = fs.openSync(path, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
            const serviceId = -10;
            const args = new Array("allInfo");
            try {
                hidebug.getServiceDump(serviceId, file.fd, args);
            } catch (error) {
              console.info(error.code);
              console.info(error.message);
              expect(error.code === "11400101").assertTrue();
            }
            fs.closeSync(file);
        })
    })

        /**
     * @tc.name: HidebugJsTest_009
     * @tc.desc: getAppNativeMemInfo的正常测试, getVss()/getPss()/getSharedDirty()/getPrivateDirty()的更新版本
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
        it('HidebugJsTest_009', 0, function () {
            console.info("---------------------------HidebugJsTest_009----------------------------------");
            try {
                let nativeMemInfo = hidebug.getAppNativeMemInfo();
                expect(nativeMemInfo.pss >= 0).assertTrue();
                expect(nativeMemInfo.vss >= 0).assertTrue();
                expect(nativeMemInfo.rss >= 0).assertTrue();
                expect(nativeMemInfo.sharedDirty >= 0).assertTrue();
                expect(nativeMemInfo.privateDirty >= 0).assertTrue();
                expect(nativeMemInfo.sharedClean >= 0).assertTrue();
                expect(nativeMemInfo.privateClean >= 0).assertTrue();
            } catch (error) {
                console.info(error.code);
                console.info(error.message);
                expect(false).assertTrue();
            }
        })

        /**
         * @tc.name: HidebugJsTest_010
         * @tc.desc: getSystemMemInfo()的正常测试
         * @tc.type: FUNC
         * @tc.require: issueI5VY8L
         */
        it('HidebugJsTest_010', 0, function () {
            console.info("---------------------------HidebugJsTest_010----------------------------------");
            try {
                let systemMemInfo = hidebug.getSystemMemInfo();
                expect(systemMemInfo.totalMem >= 0).assertTrue();
                expect(systemMemInfo.freeMem >= 0).assertTrue();
                expect(systemMemInfo.availableMem >= 0).assertTrue();
            } catch (error) {
                console.info(error.code);
                console.info(error.message);
                expect(false).assertTrue();
            }
        })

    /**
     * @tc.name: HidebugJsTest_011
     * @tc.desc: getSystemCpuUsage的正常测试，查询system cpu usage
     * @tc.type: FUNC
     * @tc.require: issueI90Z36
     */
    it('HidebugJsTest_011', 0, function () {
        console.info("---------------------------HidebugJsTest_011----------------------------------");
        try {
            let sysCpuUsage = hidebug.getSystemCpuUsage();
            expect(sysCpuUsage >= 0 && sysCpuUsage <= 1).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_012
     * @tc.desc: getAppMemoryLimit正常测试
     * @tc.type: FUNC
     * @tc.require: issueI8ZX7S
     */
    it('HidebugJsTest_012', 0, function () {
        console.info("---------------------------HidebugJsTest_012----------------------------------");
        try {
            let temp = hidebug.getAppMemoryLimit();
            expect(temp.rssLimit >= BigInt(0)).assertTrue();
            expect(temp.vssLimit >= BigInt(0)).assertTrue();
            expect(temp.vmHeapLimit >= BigInt(0)).assertTrue();
            expect(temp.vmTotalHeapSize >= BigInt(0)).assertTrue();
        } catch (error) {
            expect().assertFail();
        }
    })

    /**
     * @tc.name: HidebugJsTest_013
     * @tc.desc: getAppVMMemoryInfo正常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_013', 0, function () {
        console.info("---------------------------HidebugJsTest_013----------------------------------");
        try {
            let result = hidebug.getAppVMMemoryInfo();
            expect(result.allArraySize >= 0 && result.totalHeap >= 0 && result.heapUsed >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_014
     * @tc.desc: getAppThreadCpuUsage正常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_014', 0, function () {
        console.info("---------------------------HidebugJsTest_014----------------------------------");
        try {
            let appThreadCpuUsage = hidebug.getAppThreadCpuUsage();
            expect(appThreadCpuUsage.length >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_015
     * @tc.desc: StartAppTraceCapture正常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_015', 0, function () {
        console.info("---------------------------HidebugJsTest_015----------------------------------");
        try {
            let tags = [hidebug.tags.ABILITY_MANAGER];
            let flag = hidebug.TraceFlag.MAIN_THREAD;
            let limitSize = 1024 * 1024;
            let fileName = hidebug.startAppTraceCapture(tags, flag, limitSize);
            for (let i = 0; i < 3; i++) {
                hidebug.getSharedDirty();
            }
            hidebug.stopAppTraceCapture();
            expect(fileName.length > 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_016
     * @tc.desc: getVMRuntimeStats测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_016', 0, function () {
        console.info("---------------------------HidebugJsTest_016----------------------------------");
        try {
            let runtimeStats = hidebug.getVMRuntimeStats();
            expect(runtimeStats["ark.gc.gc-count"] >= 0).assertTrue();
            expect(runtimeStats["ark.gc.gc-time"] >= 0).assertTrue();
            expect(runtimeStats["ark.gc.gc-bytes-allocated"] >= 0).assertTrue();
            expect(runtimeStats["ark.gc.gc-bytes-freed"] >= 0).assertTrue();
            expect(runtimeStats["ark.gc.fullgc-longtime-count"] >= 0).assertTrue();
            expect(runtimeStats["others"] === undefined).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_017
     * @tc.desc: getVMRuntimeStat正常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_017', 0, function () {
        console.info("---------------------------HidebugJsTest_017----------------------------------");
        try {
            let gcCount = hidebug.getVMRuntimeStat("ark.gc.gc-count");
            let gcTime = hidebug.getVMRuntimeStat("ark.gc.gc-time");
            let gcBytesAllocated = hidebug.getVMRuntimeStat("ark.gc.gc-bytes-allocated");
            let gcBytesFreed = hidebug.getVMRuntimeStat("ark.gc.gc-bytes-freed");
            let fullGcLongTimeCount = hidebug.getVMRuntimeStat("ark.gc.fullgc-longtime-count");
            expect(gcCount >= 0).assertTrue();
            expect(gcTime >= 0).assertTrue();
            expect(gcBytesAllocated >= 0).assertTrue();
            expect(gcBytesFreed >= 0).assertTrue();
            expect(fullGcLongTimeCount >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_018
     * @tc.desc: getVMRuntimeStat参数异常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_018', 0, function () {
        console.info("---------------------------HidebugJsTest_018----------------------------------");
        try {
            hidebug.getVMRuntimeStat("others");
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code === "401").assertTrue();
            expect(error.message === "Invalid parameter, unknown property.").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_019
     * @tc.desc: setAppResourceLimit正常测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_019', 0, function () {
        console.info("---------------------------HidebugJsTest_019----------------------------------");
        try {
            let type = "js_heap";
            let value = 85;
            let enabledDebugLog = false;
            hidebug.setAppResourceLimit(type, value, enabledDebugLog);
        } catch (error) {
            console.info(error.code);
            expect(error.code === "401").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_020
     * @tc.desc: StartAppTraceCapture错误传参测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_020', 0, function () {
        console.info("---------------------------HidebugJsTest_020----------------------------------");
        try {
            let tags = [hidebug.tags.ABILITY_MANAGER];
            let flag = 123;
            let limitSize = 1024 * 1024;
            let fileName = hidebug.startAppTraceCapture(tags, flag, limitSize);
            for (let i = 0; i < 3; i++) {
                hidebug.getSharedDirty();
            }
            hidebug.stopAppTraceCapture();
            expect().assertFail();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(error.code === "401").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_021
     * @tc.desc: StartAppTraceCapture重复启动测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_021', 0, function () {
        console.info("---------------------------HidebugJsTest_021----------------------------------");
        let fileName = "";
        try {
            let tags = [hidebug.tags.ABILITY_MANAGER];
            let flag = hidebug.TraceFlag.MAIN_THREAD;
            let limitSize = 1024 * 1024;
            fileName = hidebug.startAppTraceCapture(tags, flag, limitSize);
            for (let i = 0; i < 3; i++) {
                hidebug.getSharedDirty();
            }
            fileName = hidebug.startAppTraceCapture(tags, flag, limitSize);
            hidebug.stopAppTraceCapture();
            expect().assertFail();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            if (fileName.length > 0) {
                hidebug.stopAppTraceCapture();
            }
            expect(error.code === "11400102").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_022
     * @tc.desc: StartAppTraceCapture未启动直接关闭测试
     * @tc.type: FUNC
     * @tc.require: issueI5VY8L
     */
    it('HidebugJsTest_022', 0, function () {
        console.info("---------------------------HidebugJsTest_022----------------------------------");
        try {
            hidebug.stopAppTraceCapture();
            expect().assertFail();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(error.code === "11400105").assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_023
     * @tc.desc: getSharedDirty测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_023', 0, function () {
        console.info("---------------------------HidebugJsTest_023----------------------------------");
        try {
            let sharedDirty = hidebug.getSharedDirty();
            expect(sharedDirty >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_024
     * @tc.desc: getPrivateDirty测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_024', 0, function () {
        console.info("---------------------------HidebugJsTest_024----------------------------------");
        try {
            let privateDirty = hidebug.getPrivateDirty();
            expect(privateDirty >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_025
     * @tc.desc: getPss测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_025', 0, function () {
        console.info("---------------------------HidebugJsTest_025----------------------------------");
        try {
            let pss = hidebug.getPss();
            expect(pss >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_026
     * @tc.desc: getVss测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_026', 0, function () {
        console.info("---------------------------HidebugJsTest_026----------------------------------");
        try {
            let vss = hidebug.getVss();
            expect(vss >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_027
     * @tc.desc: getCpuUsage测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_027', 0, function () {
        console.info("---------------------------HidebugJsTest_027----------------------------------");
        try {
            let cpuUsage = hidebug.getCpuUsage();
            expect(cpuUsage >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_028
     * @tc.desc: getNativeHeapSize测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_028', 0, function () {
        console.info("---------------------------HidebugJsTest_028----------------------------------");
        try {
            let nativeHeapSize = hidebug.getNativeHeapSize();
            expect(nativeHeapSize >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_029
     * @tc.desc: getNativeHeapAllocatedSize测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_029', 0, function () {
        console.info("---------------------------HidebugJsTest_029----------------------------------");
        try {
            let nativeHeapAllocatedSize = hidebug.getNativeHeapAllocatedSize();
            expect(nativeHeapAllocatedSize >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_030
     * @tc.desc: getNativeHeapFreeSize测试
     * @tc.type: FUNC
     */
    it('HidebugJsTest_030', 0, function () {
        console.info("---------------------------HidebugJsTest_030----------------------------------");
        try {
            let nativeHeapFreeSize = hidebug.getNativeHeapFreeSize();
            expect(nativeHeapFreeSize >= 0).assertTrue();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_031
     * @tc.desc: startProfiling启动两次
     * @tc.type: FUNC
     */
    it('HidebugJsTest_031', 0, async function () {
        console.info("---------------------------HidebugJsTest_031----------------------------------");
        try {
            let timestamp1 = Date.now();
            let filename1 = "cpuprofiler_" + timestamp1.toString();
            hidebug.startProfiling(filename1);
            await msleep(1000);
            hidebug.stopProfiling();
            let path1 = "/proc/self/root/data/storage/el2/base/files/" + filename1 + ".json";
            expect(fs.accessSync(path1)).assertTrue();

            let timestamp2 = Date.now();
            let filename2 = "cpuprofiler_" + timestamp2.toString();
            hidebug.startProfiling(filename2);
            await msleep(1000);
            hidebug.stopProfiling();
            let path2 = "/proc/self/root/data/storage/el2/base/files/" + filename2 + ".json";
            expect(fs.accessSync(path2)).assertTrue();
        } catch (err) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_032
     * @tc.desc: startJsCpuProfiling启动两次
     * @tc.type: FUNC
     */
    it('HidebugJsTest_032', 0, async function () {
        console.info("---------------------------HidebugJsTest_032----------------------------------");
        try {
            let timestamp1 = Date.now();
            let filename1 = "cpuprofiler_" + timestamp1.toString();
            hidebug.startJsCpuProfiling(filename1);
            await msleep(1000);
            hidebug.stopJsCpuProfiling();
            let path1 = "/proc/self/root/data/storage/el2/base/files/" + filename1 + ".json";
            expect(fs.accessSync(path1)).assertTrue();

            let timestamp2 = Date.now();
            let filename2 = "cpuprofiler_" + timestamp2.toString();
            hidebug.startJsCpuProfiling(filename2);
            await msleep(1000);
            hidebug.stopJsCpuProfiling();
            let path2 = "/proc/self/root/data/storage/el2/base/files/" + filename2 + ".json";
            expect(fs.accessSync(path2)).assertTrue();
        } catch (err) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_033
     * @tc.desc: isDebugState,未连接调试状态下
     * @tc.type: FUNC
     * @tc.require: issueIAC8K0
     */
    it('HidebugJsTest_033', 0, function () {
        console.info("---------------------------HidebugJsTest_033----------------------------------");
        try {
            let result = hidebug.isDebugState();
            expect(result).assertFalse();
        } catch (error) {
            console.info(error.code);
            console.info(error.message);
        }
    })

    /**
     * @tc.name: HidebugJsTest_034
     * @tc.desc: getGraphicsMemory
     * @tc.type: FUNC
     */
    it('HidebugJsTest_034', 0, async function () {
        console.info("---------------------------HidebugJsTest_034----------------------------------");
        try {
            let graphicMemory = await hidebug.getGraphicsMemory();
            expect(graphicMemory >= 0).assertTrue();
        } catch (err) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })

    /**
     * @tc.name: HidebugJsTest_035
     * @tc.desc: getGraphicsMemorySync
     * @tc.type: FUNC
     */
    it('HidebugJsTest_035', 0, function () {
        console.info("---------------------------HidebugJsTest_035----------------------------------");
        try {
            let graphicMemory = hidebug.getGraphicsMemorySync();
            expect(graphicMemory >= 0).assertTrue();
        } catch (err) {
            console.info(error.code);
            console.info(error.message);
            expect(false).assertTrue();
        }
    })
})
