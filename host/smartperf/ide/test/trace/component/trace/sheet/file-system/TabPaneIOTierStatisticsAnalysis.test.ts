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
import { TabPaneIOTierStatisticsAnalysis } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIOTierStatisticsAnalysis.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIOTierStatisticsAnalysis.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));
Object.defineProperty(global.self, 'crypto', {
    value: {
        getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
    },
});

describe('TabPaneIOTierStatisticsAnalysis Test', () => {
    document.body.innerHTML = `<tabpane-tb-vm-statistics id="statistics-analysis"></tabpane-tb-vm-statistics>`;
    let tabPane = document.querySelector<TabPaneIOTierStatisticsAnalysis>('#statistics-analysis');
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
    let processData = [
        {
            callChainId: 13,
            dur: 240916,
            libId: 539,
            libName: 'libName.z.so',
            pid: 911,
            processName: 'processName(911)',
            symbolId: 799,
            symbolName: 'symbolName',
            threadName: 'threadName',
            tid: 404,
            type: 0,
        },
    ];
    let item = {
        durFormat: '194.23ms ',
        duration: 194230478,
        isHover: true,
        percent: '99.00',
        pid: 3744,
        tableName: 'test(3744)',
    };
    let res = [
        {
            durFormat: '194.23ms ',
            duration: 194230478,
            isHover: true,
            percent: '99.00',
            pid: 3744,
            tableName: 'test(3744)',
        },
    ];
    let itemClick = new CustomEvent('click', <CustomEventInit>{
        detail: {
            ...{},
            data: {},
        },
        composed: true,
    });
    it('tabPaneIOTierStatisticsAnalysis01', function () {
        let litTable = new LitTable();
        tabPane.appendChild(litTable);
        let filter = new TabPaneFilter();
        tabPane.filter = filter;
        tabPane.loadingList = [];
        tabPane.data = param;
        expect(tabPane.ioTierStatisticsAnalysisSelection).toBeUndefined();
    });
    it('tabPaneIOTierStatisticsAnalysis02', function () {
        expect(tabPane.clearData()).toBeUndefined();
    });
    it('tabPaneIOTierStatisticsAnalysis03', function () {
        tabPane.processData = jest.fn(() => true);
        tabPane.getIOTierProcess(param, processData);
        expect(tabPane.processData).not.toBeUndefined();
    });
    it('tabPaneIOTierStatisticsAnalysis04', function () {
        tabPane.processData = processData;
        tabPane.getIOTierType(item, param);
        expect(tabPane.progressEL.loading).toBeFalsy();
    });
    it('tabPaneIOTierStatisticsAnalysis05', function () {
        tabPane.processData = processData;
        tabPane.getIOTierThread(item, param);
        expect(tabPane.currentLevel).toEqual(2);
    });
    it('tabPaneIOTierStatisticsAnalysis06', function () {
        tabPane.processData = processData;
        tabPane.getIOTierSo(item, param);
        expect(tabPane.currentLevel).toEqual(3);
    });
    it('tabPaneIOTierStatisticsAnalysis07', function () {
        tabPane.processData = processData;
        tabPane.getIOTierFunction(item, param);
        expect(tabPane.currentLevel).toEqual(4);
    });
    it('tabPaneIOTierStatisticsAnalysis08', function () {
        expect(tabPane.typeIdToString(1)).toEqual('DATA_READ');
    });

    it('tabPaneIOTierStatisticsAnalysis09', function () {
        expect(tabPane.typeIdToString(2)).toEqual('DATA_WRITE');
    });

    it('tabPaneIOTierStatisticsAnalysis10', function () {
        expect(tabPane.typeIdToString(3)).toEqual('METADATA_READ');
    });
    it('tabPaneIOTierStatisticsAnalysis11', function () {
        expect(tabPane.typeIdToString(4)).toEqual('METADATA_WRITE');
    });
    it('tabPaneIOTierStatisticsAnalysis12', function () {
        expect(tabPane.getPieChartData(res).length).toEqual(1);
    });
})