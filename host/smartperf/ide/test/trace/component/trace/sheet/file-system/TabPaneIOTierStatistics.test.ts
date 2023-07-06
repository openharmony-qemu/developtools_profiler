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
import { TabPaneIOTierStatistics } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIOTierStatistics.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIOTierStatistics.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
import {getTabPaneIOTierStatisticsData} from "../../../../../../dist/trace/database/SqlLite.js";
// @ts-ignore
window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

Object.defineProperty(global.self, 'crypto', {
    value: {
        getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
    },
});

describe('TabPaneIOTierStatistics Test', () => {
    document.body.innerHTML = `<tabpane-io-tier-statistics id="io-tier-statistics"></tabpane-io-tier-statistics>`;
    let tabPane = document.querySelector<TabPaneIOTierStatistics>('#io-tier-statistics');

    let param = {
        anomalyEnergy: [],
        clockMapData: { size: 0 },
        cpuAbilityIds: [],
        cpuFreqFilterIds: [],
        cpuFreqLimitDatas: [],
        cpuStateFilterIds: [],
        cpus: [],
        diskAbilityIds: [],
        diskIOLatency: false,
        diskIOReadIds: [2, 7, 1, 3, 4, 5, 6],
        diskIOWriteIds: [2, 7, 1, 3, 4, 5, 6],
        diskIOipids: [2, 7, 1, 3, 4, 5, 6],
        fileSysVirtualMemory: false,
        fileSystemType: [],
        fsCount: 0,
        funAsync: [],
        funTids: [],
        hasFps: false,
        irqMapData: { size: 0 },
        jsMemory: [],
        leftNs: 964699689,
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
        processTrackIds: [],
        promiseList: [],
        recordStartNs: 780423789228,
        rightNs: 24267556624,
        sdkCounterIds: [],
        sdkSliceIds: [],
        smapsType: [],
        systemEnergy: [],
        threadIds: [],
        virtualTrackIds: [],
        vmCount: 0,
    };
    it('ioTierStatistics01', function () {
        let queryResult = sqlit.getTabPaneIOTierStatisticsData;
        queryResult.mockResolvedValue([
            {
                "pid": 186,
                "pname": "kworker/u8:4",
                "tier": 0,
                "ipid": 2,
                "path": "-",
                "count": 3,
                "allDuration": 19543418,
                "minDuration": 6408209,
                "maxDuration": 6668084,
                "avgDuration": 6514472.66666667
            },
            {
                "pid": 186,
                "pname": "kworker/u8:4",
                "tier": 0,
                "ipid": 2,
                "path": "/data/thermal/config/configLevel",
                "count": 1,
                "allDuration": 5916167,
                "minDuration": 5916167,
                "maxDuration": 5916167,
                "avgDuration": 5916167
            },
            {
                "pid": 186,
                "pname": "kworker/u8:4",
                "tier": 0,
                "ipid": 2,
                "path": "/data/local/tmp/hiebpf.data",
                "count": 2,
                "allDuration": 9192751,
                "minDuration": 2386417,
                "maxDuration": 6806334,
                "avgDuration": 4596375.5
            },
            {
                "pid": 237,
                "pname": "jbd2/mmcblk0p11",
                "tier": 0,
                "ipid": 7,
                "path": "-",
                "count": 7,
                "allDuration": 32377630,
                "minDuration": 2749251,
                "maxDuration": 5033292,
                "avgDuration": 4625375.71428571
            }
        ]);
        tabPane.data = param;
        expect(tabPane.ioTierStatisticsSelectionParam).not.toBeUndefined();
    });
});
