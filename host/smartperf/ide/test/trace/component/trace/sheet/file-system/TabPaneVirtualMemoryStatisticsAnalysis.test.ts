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
import { TabPaneVirtualMemoryStatisticsAnalysis } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneVirtualMemoryStatisticsAnalysis.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneVirtualMemoryStatisticsAnalysis.js';
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

describe('TabPaneVirtualMemoryStatisticsAnalysis Test', () => {
    document.body.innerHTML = `<tabpane-virtual-memory-statistics-analysis id="statistics-analysis"></tabpane-virtual-memory-statistics-analysis>`;
    let tabPane = document.querySelector<TabPaneVirtualMemoryStatisticsAnalysis>('#statistics-analysis');
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
    it('tabPaneVirtualMemoryStatisticsAnalysis01', function () {
        let litTable = new LitTable();
        tabPane.appendChild(litTable);
        let filter = new TabPaneFilter();
        tabPane.filter = filter;
        tabPane.loadingList = [];
        tabPane.data = param;
        expect(tabPane.vmStatisticsAnalysisSelection).toBeUndefined();
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis02', function () {
        expect(tabPane.clearData()).toBeUndefined();
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis03', function () {
        tabPane.vmStatisticsAnalysisProcessData = jest.fn(() => true);
        let data = [
            {
                "type": 7,
                "callChainId": 12,
                "dur": 30625,
                "pid": 1374,
                "tid": 1374,
                "threadName": "com.ohos.mms",
                "processName": "com.ohos.mms(1374)",
                "libId": 344,
                "symbolId": 727,
                "libName": "libmmi-util.z.so",
                "symbolName": "OHOS::MMI::GetThisThreadIdOfString()"
            },
            {
                "type": 7,
                "callChainId": 24,
                "dur": 42000,
                "pid": 1818,
                "tid": 1374,
                "threadName": null,
                "processName": "RSRenderThread(1818)",
                "libId": null,
                "symbolId": null,
                "libName": "",
                "symbolName": "0x7f7380e670 ()"
            }
        ];
        tabPane.getVirtualMemoryProcess(data, processData);
        expect(tabPane.vmStatisticsAnalysisProcessData).not.toBeUndefined();
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis04', function () {
        tabPane.vmStatisticsAnalysisProcessData = processData;
        tabPane.getVirtualMemoryType(item, param);
        expect(tabPane.vmStatisticsAnalysisProgressEL.loading).toBeFalsy();
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis05', function () {
        tabPane.vmStatisticsAnalysisProcessData = processData;
        tabPane.getVirtualMemoryThread(item, param);
        expect(tabPane.currentLevel).toEqual(2);
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis06', function () {
        tabPane.vmStatisticsAnalysisProcessData = processData;
        tabPane.getVirtualMemorySo(item, param);
        expect(tabPane.currentLevel).toEqual(3);
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis07', function () {
        tabPane.vmStatisticsAnalysisProcessData = processData;
        tabPane.getVirtualMemoryFunction(item, param);
        expect(tabPane.currentLevel).toEqual(4);
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis08', function () {
        expect(tabPane.typeIdToString(1)).toEqual('File Backed In');
    });

    it('tabPaneVirtualMemoryStatisticsAnalysis09', function () {
        expect(tabPane.typeIdToString(7)).toEqual('Copy On Writer');
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis10', function () {
        expect(tabPane.getPieChartData(res).length).toEqual(1);
    });
    it('tabPaneVirtualMemoryStatisticsAnalysis11', function () {
        tabPane.currentLevel = 0;
        let paras = [
            {
                type: 2,
                callChainId: 1,
                dur: 4757959,
                pid: 237,
                tid: 237,
                threadName: 'jbd2/mmcblk0p11',
                processName: 'jbd2/mmcblk0p11(237)',
                libId: 263,
                symbolId: 12560,
                libName: 'kallsyms',
                symbolName: 'submit_bh',
            },
            {
                type: 2,
                callChainId: 1,
                dur: 4673084,
                pid: 237,
                tid: 237,
                threadName: 'jbd2/mmcblk0p11',
                processName: 'jbd2/mmcblk0p11(237)',
                libId: 263,
                symbolId: 12560,
                libName: 'kallsyms',
                symbolName: 'submit_bh',
            },
        ];
        tabPane.currentLevelData = paras;
        expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
        tabPane.currentLevel = 1;
        expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
        tabPane.currentLevel = 2;
        expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
        tabPane.currentLevel = 3;
        expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
        tabPane.currentLevel = 4;
        expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
        tabPane.currentLevel = 0;
        expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
        tabPane.currentLevel = 1;
        expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
        tabPane.currentLevel = 2;
        expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
        tabPane.currentLevel = 3;
        expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
        tabPane.currentLevel = 4;
        expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
        tabPane.currentLevel = 0;
        expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
        tabPane.currentLevel = 1;
        expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
        tabPane.currentLevel = 2;
        expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
        tabPane.currentLevel = 3;
        expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
        tabPane.currentLevel = 4;
        expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
    });
})