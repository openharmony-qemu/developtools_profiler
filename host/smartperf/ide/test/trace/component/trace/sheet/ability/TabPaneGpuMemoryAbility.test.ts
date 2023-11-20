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
import { TabPaneGpuMemoryAbility } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneGpuMemoryAbility.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {}
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
    return {};
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPaneGpuMemoryAbility Test', () => {
    let tabPaneGpuMemoryAbility = new TabPaneGpuMemoryAbility();
    let getTabGpuMemoryAbilityData = sqlit.getTabGpuMemoryAbilityData;
    getTabGpuMemoryAbilityData.mockResolvedValue([
        {
            avgSize: 711756458.666667,
            avgSizes: "678.78MB",
            gpuName: "mali0",
            maxSize: 816918528,
            maxSizes: "779.07MB",
            minSize: 643006464,
            minSizes: "613.22MB",
            process: "render_service(1071)",
            processId: 1071,
            processName: "render_service",
            startNs: 9551711978,
            sumSize: 2135269376,
            sumSizes: "1.99GB",
        },
        {
            avgSize: 905216,
            avgSizes: "884.00KB",
            gpuName: "mali0",
            isHover: false,
            maxSize: 905216,
            maxSizes: "884.00KB",
            minSize: 905216,
            minSizes: "884.00KB",
            process: "/system/bin/aosp_graphic_temp_service(609)",
            processId: 609,
            processName: "/system/bin/aosp_graphic_temp_service",
            startNs: 4568285416,
            sumSize: 2715648,
            sumSizes: "2.59MB",
        },
    ]);
    tabPaneGpuMemoryAbility.init = jest.fn(() => true);
    getTabGpuMemoryAbilityData.data = {
        cpuFreqLimitDatas: [],
        cpuStateFilterIds: [],
        anomalyEnergy: [],
        clockMapData: {},
        cpuAbilityIds: [],
        cpuFreqFilterIds: [],
        cpus: [],
        diskAbilityIds: [],
        diskIOLatency: false,
        diskIOReadIds: [],
        dmaVmTrackerData:[],
        fsCount:0,
        funAsync:[],
        funTids:[],
        diskIOWriteIds: [],
        diskIOipids: [],
        dmaAbilityData: [],
        gpu:{gl: false, gpuWindow: false, gpuTotal: false},
        gpuMemoryAbilityData:[1,23,''],
        gpuMemoryTrackerData:[],
        hasFps:false,
        irqMapData:{size: 0},
        isCurrentPane:false,
        jankFramesData:[],
        jsCpuProfilerData:[],
        jsMemory:[],
        leftNs:3244455,
        memoryAbilityIds:[],
        networkAbilityIds:[],
        recordStartNs:44443669149,
        rightNs:6543728831,
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
    it('TabPaneGpuMemoryAbilityTest01', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('process',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest02', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('startNs',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest03', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('sumSizes',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest04', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('avgSize',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest05', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('minSize',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest06', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('maxSize',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest06', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('maxSize',1)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest07', function () {
        expect(tabPaneGpuMemoryAbility.queryDataByDB(val)).toBeUndefined();
    });
    it('TabPaneGpuMemoryAbilityTest08', function () {
        expect(tabPaneGpuMemoryAbility.sortGpuMemoryByColumn('',0)).toBeUndefined();
    });
})