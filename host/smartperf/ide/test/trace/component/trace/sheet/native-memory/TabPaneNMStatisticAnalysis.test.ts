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

import crypto from 'crypto';
// @ts-ignore
import { TabPaneNMStatisticAnalysis } from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMStatisticAnalysis.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';

window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {

    }
});

Object.defineProperty(global.self, 'crypto', {
    value: {
        getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
    },
});

jest.mock('../../../../../../dist/base-ui/chart/pie/LitChartPie.js', () => {
    return {};
});

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

describe('TabPaneNMStatisticAnalysis Test', () => {
    let htmlDivElement = document.createElement('div');
    let tabStatisticAnalysis = new TabPaneNMStatisticAnalysis();
    tabStatisticAnalysis.tableType.reMeauseHeight = jest.fn(() => true);
    tabStatisticAnalysis.soUsageTbl.reMeauseHeight = jest.fn(() => true);
    tabStatisticAnalysis.functionUsageTbl.reMeauseHeight = jest.fn(() => true);
    htmlDivElement.append(tabStatisticAnalysis);
    let dataArray = {
        applyCount: 25336,
        applyCountPercent: '99',
        applySize: 13372754,
        applySizeFormat: '12.75MB',
        applySizePercent: '25023.87',
        existCount: 1011,
        existCountPercent: '98.54',
        existSize: 809825,
        existSizeFormat: '790.84KB',
        existSizePercent: '2.36',
        releaseCount: 52168,
        releaseCountPercent: '99.53',
        releaseSize: 12562929,
        releaseSizeFormat: '11.98MB',
        releaseSizePercent: '1.79',
        tableName: 'Test',
        typeId: 0,
        typeName: 'Test',
    };
    let select = {
        recordStartNs: 8406282873525,
        leftNs: 16648778040,
        rightNs: 48320174407,
        hasFps: false,
        nativeMemory: ['All Heap & Anonymous VM', 'All Heap'],
    };
    let processData = [
        {
            applyId: 22945,
            callChainId: 91,
            count: 1,
            endTs: 4831690110,
            id: 22945,
            libId: 420,
            libName: 'test.z.so',
            pid: 1,
            size: 304,
            startTs: 4806916233,
            symbolId: 429,
            symbolName: 'test',
            tid: 5211,
            type: 0,
        },
    ];
    tabStatisticAnalysis.processData = processData;

    it('statisticAnalysis01', function () {
        tabStatisticAnalysis.initElements();
        tabStatisticAnalysis.data = select;
        expect(tabStatisticAnalysis.currentSelection).toEqual(select);
    });

    it('statisticAnalysis02', function () {
        tabStatisticAnalysis.tabName.textContent = 'Statistic By Library Size';
        tabStatisticAnalysis.eventTypeData = [dataArray];
        let mouseMoveEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ movementX: 1, movementY: 2 });
        tabStatisticAnalysis.back.dispatchEvent(mouseMoveEvent);
        tabStatisticAnalysis.back.dispatchEvent(mouseMoveEvent);
    });

    it('statisticAnalysis03', function () {
        tabStatisticAnalysis.getLibSize(dataArray, select);
        expect(tabStatisticAnalysis.currentLevel).toEqual(1);
    });

    it('statisticAnalysis04', function () {
        expect(tabStatisticAnalysis.getNMFunctionSize(dataArray, select)).toBeUndefined();
    });

    it('statisticAnalysis05', function () {
        tabStatisticAnalysis.calTypeSize(select, processData);
        expect(tabStatisticAnalysis.processData.length).toBe(1);
    });

    it('statisticAnalysis06', function () {
        let processData = [
            {
                applyId: 22945,
                callChainId: 91,
                count: 1,
                endTs: 4831690110,
                id: 22945,
                libId: 420,
                libName: 'test.z.so',
                pid: 1,
                size: 304,
                startTs: 4806916233,
                symbolId: 429,
                symbolName: 'test',
                tid: 5211,
                type: 1,
            },
        ];
        tabStatisticAnalysis.calTypeSize(select, processData);
        expect(tabStatisticAnalysis.currentLevelReleaseCount).toBe(0);
    });

    it('statisticAnalysis07', function () {
        let processData = [
            {
                applyId: 22945,
                callChainId: 91,
                count: 1,
                endTs: 4831690110,
                id: 22945,
                libId: 420,
                libName: 'test.z.so',
                pid: 1,
                size: 304,
                startTs: 4806916233,
                symbolId: 429,
                symbolName: 'test',
                tid: 5211,
                type: 2,
            },
        ];
        tabStatisticAnalysis.calTypeSize(select, processData);
        expect(tabStatisticAnalysis.currentLevelApplySize).toBe(0);
    });

    it('statisticAnalysis08', function () {
        let processData = [
            {
                applyId: 22945,
                callChainId: 91,
                count: 1,
                endTs: 4831690110,
                id: 22945,
                libId: 420,
                libName: 'test.z.so',
                pid: 1,
                size: 304,
                startTs: 4806916233,
                symbolId: 429,
                symbolName: 'test',
                tid: 5211,
                type: 3,
            },
        ];
        tabStatisticAnalysis.calTypeSize(select, processData);
        expect(tabStatisticAnalysis.processData.length).toBe(1);
    });

    it('statisticAnalysis09', function () {
        tabStatisticAnalysis.currentLevel = 0;
        tabStatisticAnalysis.sortByColumn('', 0);
        expect(tabStatisticAnalysis.tableType.recycleDataSource.length).toBe(1);
    });

    it('statisticAnalysis10', function () {
        tabStatisticAnalysis.currentLevel = 1;
        tabStatisticAnalysis.sortByColumn('', 0);
        expect(tabStatisticAnalysis.tableType.recycleDataSource.length).toBe(1);
    });

    it('statisticAnalysis11', function () {
        tabStatisticAnalysis.currentLevel = 2;
        tabStatisticAnalysis.sortByColumn('', 0);
        expect(tabStatisticAnalysis.tableType.recycleDataSource.length).toBe(1);
    });

    it('statisticAnalysis12', function () {
        tabStatisticAnalysis.currentLevel = 0;
        tabStatisticAnalysis.currentLevelData = [{
            tableName: 0
        }, {
            tableName: 1
        }]
        tabStatisticAnalysis.sortByColumn('tableName', 1)
        tabStatisticAnalysis.currentLevelData = [{
            existSize: 0
        }, {
            existSize: 1
        }]
        tabStatisticAnalysis.sortByColumn('existSizeFormat', 1)
        tabStatisticAnalysis.currentLevelData = [{
            existSize: 0
        }, {
            existSize: 1
        }]
        tabStatisticAnalysis.sortByColumn('existSizePercent', 1)
        tabStatisticAnalysis.currentLevelData = [{
            existCount: 0
        }, {
            existCount: 1
        }]
        tabStatisticAnalysis.sortByColumn('existCount', 1)
        tabStatisticAnalysis.currentLevelData = [{
            existCount: 0
        }, {
            existCount: 1
        }]
        tabStatisticAnalysis.sortByColumn('existCountPercent', 1)
        expect(tabStatisticAnalysis.tableType.recycleDataSource.length).toBe(3);
    });

    it('statisticAnalysis13', function () {
        tabStatisticAnalysis.currentLevel = 1;
        tabStatisticAnalysis.currentLevelData = [{
            releaseSize : 0
        }, {
            releaseSize : 1
        }]
        tabStatisticAnalysis.sortByColumn('releaseSizeFormat', 1)
        tabStatisticAnalysis.currentLevelData = [{
            releaseSize : 0
        }, {
            releaseSize : 1
        }]
        tabStatisticAnalysis.sortByColumn('releaseSizePercent', 1)
        tabStatisticAnalysis.currentLevelData = [{
            releaseCount : 0
        }, {
            releaseCount : 1
        }]
        tabStatisticAnalysis.sortByColumn('releaseCount', 1)
        tabStatisticAnalysis.currentLevelData = [{
            releaseCount : 0
        }, {
            releaseCount : 1
        }]
        tabStatisticAnalysis.sortByColumn('releaseCountPercent', 1)
        expect(tabStatisticAnalysis.tableType.recycleDataSource.length).toBe(3);
    });

    it('statisticAnalysis14', function () {
        tabStatisticAnalysis.currentLevel = 2;
        tabStatisticAnalysis.currentLevelData = [{
            applySize : 0
        }, {
            applySize : 1
        }]
        tabStatisticAnalysis.sortByColumn('applySizeFormat', 1)
        tabStatisticAnalysis.currentLevelData = [{
            applySize : 0
        }, {
            applySize : 1
        }]
        tabStatisticAnalysis.sortByColumn('applySizePercent', 1)
        tabStatisticAnalysis.currentLevelData = [{
            applyCount : 0
        }, {
            applyCount : 1
        }]
        tabStatisticAnalysis.sortByColumn('applyCount', 1)
        tabStatisticAnalysis.currentLevelData = [{
            applyCount : 0
        }, {
            applyCount : 1
        }]
        tabStatisticAnalysis.sortByColumn('applyCountPercent', 1)
        expect(tabStatisticAnalysis.tableType.recycleDataSource.length).toBe(3);
    });

    it('statisticAnalysis15', function () {
        tabStatisticAnalysis.isStatistic = true;
        let val = {
            leftNs: 0,
            rightNs: 1000,
            nativeMemoryStatistic: ['All Heap & Anonymous VM', 'All Heap', 'Heap'],
        }
        expect(tabStatisticAnalysis.getNMEventTypeSize(val)).toBeUndefined();
    });

    it('statisticAnalysis16', function () {
        tabStatisticAnalysis.isStatistic = false;
        let val = {
            leftNs: 0,
            rightNs: 1000,
            nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'Heap'],
        }
        expect(tabStatisticAnalysis.getNMEventTypeSize(val)).toBeUndefined();
    });

    it('statisticAnalysis17', function () {
        tabStatisticAnalysis.tabName.textContent
        let mouseMoveEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ movementX: 1, movementY: 2 });
        tabStatisticAnalysis.back.dispatchEvent(mouseMoveEvent);

        tabStatisticAnalysis.isStatistic = false;
        let val = {
            leftNs: 0,
            rightNs: 1000,
            nativeMemory: ['All Heap & Anonymous VM', 'All Heap', 'Heap'],
        }
        expect(tabStatisticAnalysis.getNMEventTypeSize(val)).toBeUndefined();
    });

});
