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
import { TabPanePurgTotal } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPanePurgTotal.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPanePurgTotal Test', () => {
    let tabPanePurgTotal = new TabPanePurgTotal();
    let querySysPurgeableTab = sqlit.querySysPurgeableTab;
    querySysPurgeableTab.mockResolvedValue([
        {
            avgSize: 25165824,
            avgSizes: "24.00MB",
            type: "allocator_host",
            maxSize: 25165822,
            maxSizes: "24.00MB",
            minSize: 25165824,
            minSizes: "24.00MB",
        },
        {
            avgSize: 25165824,
            avgSizes: "24.00MB",
            type: "11allocator_host",
            maxSize: 25165824,
            maxSizes: "24.00MB",
            minSize: 25165824,
            minSizes: "24.00MB",
        },
        {
            avgSize: 25165824,
            avgSizes: "24.00MB",
            type: "alloca11tor_host",
            maxSize: 25165824,
            maxSizes: "24.00MB",
            minSize: 25165824,
            minSizes: "24.00MB",
        },
    ]);
    tabPanePurgTotal.init = jest.fn(() => true);
    tabPanePurgTotal.data = {
        anomalyEnergy: [],
        clockMapData:{size: 0},
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
        dmaVmTrackerData: [],
        fsCount: 0,
        funAsync: [],
        funTids: [],
        gpu: {gl: false, gpuWindow: false, gpuTotal: false},
        gpuMemoryAbilityData: [],
        gpuMemoryTrackerData: [],
        hasFps: false,
        irqMapData:{size: 0},
        isCurrentPane: false,
        jankFramesData: [],
        jsCpuProfilerData: [],
        jsMemory: [],
        leftNs: 5050699973,
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
        promiseList: [],
        purgeablePinAbility: [],
        purgeablePinSelection: [],
        purgeablePinVM: [],
        purgeableTotalAbility: [],
        purgeableTotalSelection: [],
        purgeableTotalVM: [],
        recordStartNs: 333185936669149,
        rightNs: 20081096280,
        sdkCounterIds: [],
        sdkSliceIds: [],
        smapsType: [],
        startup: false,
        staticInit: false,
        statisticsSelectData: undefined,
    };

    it('TabPanePurgTotalTest01', function () {
        expect(
            tabPanePurgTotal.sortByColumn({
                key: 'avgSize',
            })
        ).toBeUndefined();
    });
    it('TabPanePurgTotalTest02', function () {
        expect(
            tabPanePurgTotal.sortByColumn({
                key: 'type',
            })
        ).toBeUndefined();
    });
})