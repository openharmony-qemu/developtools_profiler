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
import { TabPanePurgPin } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPanePurgPin.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
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

describe('TabPanePurgPin Test', () => {
    let tabPanePurgPin = new TabPanePurgPin();
    let querySysPurgeableTab = sqlit.querySysPurgeableTab;
    querySysPurgeableTab.mockResolvedValue([
        {
            avgSize: 21223420,
            avgSizes: "34.310MB",
            type: "allocator_host",
            maxSize: 3634425822,
            maxSizes: "24.00MB",
            minSize: 7645824,
            minSizes: "98.80MB",
        },
        {
            avgSize: 62177543,
            avgSizes: "87.00MB",
            type: "11allocator_host",
            maxSize: 34322561,
            maxSizes: "24.36MB",
            minSize: 6652824,
            minSizes: "432.00MB",
        },
        {
            avgSize: 43221110,
            avgSizes: "323.00MB",
            type: "alloca11tor_host",
            maxSize: 75400211,
            maxSizes: "44.00MB",
            minSize: 41300064,
            minSizes: "32.00MB",
        },
    ]);
    let data =  [{
            avgSize: 2318995,
            avgSizes: "541.00MB",
            type: "allocator_host",
            maxSize: 4565822,
            maxSizes: "24.00MB",
            minSize: 76533,
            minSizes: "55.00MB",
        },
        {
            avgSize: 234190,
            avgSizes: "321.08MB",
            type: "11allocator_host",
            maxSize: 9781100,
            maxSizes: "6.00MB",
            minSize: 12146,
            minSizes: "4.75MB",
        }]
    tabPanePurgPin.init = jest.fn(() => true);
    tabPanePurgPin.data = {
        anomalyEnergy: [],
        clockMapData:{size: 0},
        funAsync: [],
        funTids: [],
        gpu: {gl: false, gpuWindow: false, gpuTotal: false},
        gpuMemoryAbilityData: [],
        cpuAbilityIds: [],
        cpuFreqFilterIds: [],
        cpuFreqLimitDatas: [],
        cpuStateFilterIds: [],
        cpus: [],
        promiseList: [],
        purgeablePinAbility: data,
        purgeablePinSelection: [],
        purgeablePinVM: data,
        purgeableTotalAbility: [],
        purgeableTotalSelection: [],
        diskAbilityIds: [],
        diskIOLatency: false,
        diskIOReadIds: [],
        diskIOWriteIds: [],
        diskIOipids: [],
        dmaAbilityData: [],
        dmaVmTrackerData: [],
        fsCount: 0,
        gpuMemoryTrackerData: [],
        hasFps: false,
        irqMapData:{size: 0},
        isCurrentPane: false,
        jankFramesData: [],
        jsCpuProfilerData: [],
        jsMemory: [],
        leftNs: 50699973,
        memoryAbilityIds: [],
        nativeMemory: [],
        nativeMemoryStatistic: [],
        networkAbilityIds: [],
        perfAll: false,
        perfCpus: [],
        perfProcess: [],
        perfSampleIds: [],
        perfThread: [],
        powerEnergy: [],
        processIds: [],
        processTrackIds: [],
        purgeableTotalVM: [],
        recordStartNs: 3654444469149,
        rightNs: 246888888566280,
        sdkCounterIds: [],
        sdkSliceIds: [],
        smapsType: [],
        startup: false,
        staticInit: false,
        statisticsSelectData: undefined,
    };

    it('TabPanePurgPinTest01', function () {
        expect(
            tabPanePurgPin.sortByColumn({
                key: 'avgSize',
            })
        ).toBeUndefined();
    });
})