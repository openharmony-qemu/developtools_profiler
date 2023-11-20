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

jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
    return {};
});

// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
describe('TabPaneDmaAbility Test', () => {
    let tabPaneDmaAbility = new TabPaneDmaAbility();
    let getTabDmaAbilityData = sqlit.getTabDmaAbilityData;
    getTabDmaAbilityData.mockResolvedValue([
        {
            avgSize: 1111211,
            avgSizes: "32.00MB",
            expTaskComm: "allocator_host",
            maxSize: 25165822,
            maxSizes: "24.00MB",
            minSize: 32222222,
            minSizes: "42.00MB",
            process: "com.ohos.camera(22)",
            processId: 22,
            processName: "com.ohos.camera",
            startNs: 4568285416,
            sumSize: 3222225664,
            sumSizes: "98.00MB",
        },
        {
            avgSize: 2453422,
            avgSizes: "7.00MB",
            expTaskComm: "11allocator_host",
            maxSize: 4355333,
            maxSizes: "55.00MB",
            minSize: 5333211,
            minSizes: "24.00MB",
            process: "com.ohos.camera(1)",
            processId: 1,
            processName: "com.ohos.camera",
            startNs: 4568285416,
            sumSize: 13335433,
            sumSizes: "57.00MB",
        },
        {
            avgSize: 43332178,
            avgSizes: "85.00MB",
            expTaskComm: "alloca11tor_host",
            maxSize: 7753333,
            maxSizes: "55.00MB",
            minSize: 2314425,
            minSizes: "24.00MB",
            process: "com.ohos.c11amera(33)",
            processId: 33,
            processName: "com.ohos.camera",
            startNs: 4568285416,
            sumSize: 23113453,
            sumSizes: "7.00MB",
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
        fsCount:0,
        funAsync:[],
        funTids:[],
        gpu:{gl: false, gpuWindow: false, gpuTotal: false},
        gpuMemoryAbilityData:[],
        gpuMemoryTrackerData:[],
        diskAbilityIds: [],
        diskIOLatency: false,
        diskIOReadIds: [],
        diskIOWriteIds: [],
        diskIOipids: [],
        dmaAbilityData: [],
        dmaVmTrackerData:[],
        hasFps:false,
        irqMapData:{size: 0},
        isCurrentPane:false,
        jankFramesData:[],
        jsCpuProfilerData:[],
        jsMemory:[],
        leftNs:42999114455,
        memoryAbilityIds:[],
        networkAbilityIds:[],
        recordStartNs:3133366669149,
        rightNs:33532121,
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