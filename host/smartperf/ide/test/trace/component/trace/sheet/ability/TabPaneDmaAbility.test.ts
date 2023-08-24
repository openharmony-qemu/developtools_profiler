/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
// @ts-ignore
import { TabPaneDmaAbility } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneDmaAbility.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPaneDmaAbility Test', () => {
    let tabPaneDmaAbility = new TabPaneDmaAbility();
    let getTabDmaAbilityData = sqlit.getTabDmaAbilityData;
    getTabDmaAbilityData.mockResolvedValue([
        {
            avgSize: 25165824,
            avgSizes: "24.00MB",
            expTaskComm: "allocator_host",
            maxSize: 25165822,
            maxSizes: "24.00MB",
            minSize: 25165824,
            minSizes: "24.00MB",
            process: "com.ohos.camera(22)",
            processId: 22,
            processName: "com.ohos.camera",
            startNs: 4568285416,
            sumSize: 25165824,
            sumSizes: "24.00MB",
        },
        {
            avgSize: 25165824,
            avgSizes: "24.00MB",
            expTaskComm: "11allocator_host",
            maxSize: 25165824,
            maxSizes: "24.00MB",
            minSize: 25165824,
            minSizes: "24.00MB",
            process: "com.ohos.camera(1)",
            processId: 1,
            processName: "com.ohos.camera",
            startNs: 4568285416,
            sumSize: 25165824,
            sumSizes: "24.00MB",
        },
        {
            avgSize: 25165824,
            avgSizes: "24.00MB",
            expTaskComm: "alloca11tor_host",
            maxSize: 25165824,
            maxSizes: "24.00MB",
            minSize: 25165824,
            minSizes: "24.00MB",
            process: "com.ohos.c11amera(33)",
            processId: 33,
            processName: "com.ohos.camera",
            startNs: 4568285416,
            sumSize: 25165824,
            sumSizes: "24.00MB",
        },
    ]);
    tabPaneDmaAbility.init = jest.fn(() => true);
    tabPaneDmaAbility.data = {
        anomalyEnergy: [],
        clockMapData: {},
        cpuAbilityIds: [],
        cpuFreqFilterIds: [],
        cpuFreqLimitDatas: [],
        cpuStateFilterIds: [],
        cpus: [],
        diskAbilityIds: [],
        diskIOLatency: false,
        diskIOReadIds: [],
        diskIOWriteIds: [],
        diskIOipids: [],
        dmaAbilityData: [],
        dmaVmTrackerData:[],
        fsCount:0,
        funAsync:[],
        funTids:[],
        gpu:{gl: false, gpuWindow: false, gpuTotal: false},
        gpuMemoryAbilityData:[],
        gpuMemoryTrackerData:[],
        hasFps:false,
        irqMapData:{size: 0},
        isCurrentPane:false,
        jankFramesData:[],
        jsCpuProfilerData:[],
        jsMemory:[],
        leftNs:486814455,
        memoryAbilityIds:[],
        networkAbilityIds:[],
        recordStartNs:333185936669149,
        rightNs:16125728831,
        sdkCounterIds:[],
        sdkSliceIds:[],
        smapsType:[],
        startup:false,
        staticInit:false,
        statisticsSelectData:undefined,

    };
    let val = [
        {
            leftNs: 0,
            rightNs: 1000,
        }
    ];
    tabPaneDmaAbility.init = jest.fn(() => true);
    it('TabPaneDmaAbilityTest01', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('process',1)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest02', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('expTaskComm',1)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest03', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('sumSizes',1)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest04', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('avgSize',1)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest05', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('minSize',1)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest06', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('maxSize',1)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest07', function () {
        expect(tabPaneDmaAbility.queryDataByDB(val)).toBeUndefined();
    });
    it('TabPaneDmaAbilityTest08', function () {
        expect(tabPaneDmaAbility.sortDmaByColumn('',0)).toBeUndefined();
    });
})