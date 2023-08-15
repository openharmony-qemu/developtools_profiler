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
            const pid = process.pid;
            let path = "/proc/" + pid + "/root/data/storage/el2/base/files/" + filename + ".json";
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
            const pid = process.pid;
            let path = "/proc/" + pid + "/root/data/storage/el2/base/files/" + filename + ".json";
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
})