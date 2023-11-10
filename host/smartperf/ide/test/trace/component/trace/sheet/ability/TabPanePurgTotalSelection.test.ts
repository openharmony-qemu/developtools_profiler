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
import { TabPanePurgTotalSelection } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPanePurgTotalSelection.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {};
});
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPanePurgTotalSelection Test', () => {
    let tabPanePurgTotalSelection = new TabPanePurgTotalSelection();
    let querySysPurgeableSelectionTab = sqlit.querySysPurgeableSelectionTab;
    querySysPurgeableSelectionTab.mockResolvedValue([
        {
            value: 9854,
            name: "96.00MB",
        },
        {
            value: 4789,
            name: "52.00MB",
        },
        {
            value: 4345621,
            name: "34.47MB",
        },
    ]);
    let queryProcessPurgeableSelectionTab = sqlit.queryProcessPurgeableSelectionTab;
    queryProcessPurgeableSelectionTab.mockResolvedValue([
        {
            value: 15445,
            name: "9.00MB",
        },
        {
            value: 524,
            name: "89.00MB",
        },
        {
            value: 5598,
            name: "67.00MB",
        },
    ]);
    tabPanePurgTotalSelection.data = {
        processTrackIds: [],
        promiseList: [],
        purgeablePinAbility: [],
        purgeablePinSelection: [],
        purgeablePinVM: [],
        purgeableTotalAbility: [],
        purgeableTotalSelection: [],
        anomalyEnergy: [],
        clockMapData:{size: 0},
        cpuAbilityIds: [],
        cpuFreqFilterIds: [],
        cpuFreqLimitDatas: [],
        cpuStateFilterIds: [],
        cpus: [],
        diskAbilityIds: [],
        diskIOLatency: false,
        funAsync: [],
        funTids: [],
        gpu: {gl: false, gpuWindow: false, gpuTotal: false},
        gpuMemoryAbilityData: [],
        gpuMemoryTrackerData: [],
        hasFps: false,
        diskIOReadIds: [],
        diskIOWriteIds: [],
        diskIOipids: [],
        dmaAbilityData: [],
        dmaVmTrackerData: [],
        fsCount: 0,
        irqMapData:{size: 0},
        isCurrentPane: false,
        jankFramesData: [],
        jsCpuProfilerData: [],
        jsMemory: [],
        leftNs: 120699973,
        memoryAbilityIds: [],
        nativeMemory: [],
        nativeMemoryStatistic: [],
        networkAbilityIds: [],
        perfAll: false,
        perfThread: [],
        powerEnergy: [],
        processIds: [],
        purgeableTotalVM: [],
        recordStartNs: 3392146669149,
        rightNs: 958742096280,
        sdkCounterIds: [],
        sdkSliceIds: [],
        smapsType: [],
        startup: false,
        staticInit: false,
        statisticsSelectData: undefined,
        perfCpus: [],
        perfProcess: [],
        perfSampleIds: [],
    };

    it('TabPanePurgTotalSelectionTest01', function () {
        expect(tabPanePurgTotalSelection.data).toBeUndefined();
    });
    it('TabPanePurgTotalSelectionTest02', function () {
        expect(tabPanePurgTotalSelection.queryTableData('ability',21)).toBeTruthy();
    });
    it('TabPanePurgTotalSelectionTest03', function () {
        expect(tabPanePurgTotalSelection.queryTableData('VM',21)).toBeTruthy();
    });
})