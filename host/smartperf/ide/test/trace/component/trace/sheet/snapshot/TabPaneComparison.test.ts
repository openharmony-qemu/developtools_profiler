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
//@ts-ignore
import { TabPaneComparison } from '../../../../../../dist/trace/component/trace/sheet/snapshot/TabPaneComparison.js';
import '../../../../../../dist/trace/component/trace/sheet/snapshot/TabPaneComparison.js';

//@ts-ignore
import { HeapDataInterface } from '../../../../../../dist/js-heap/HeapDataInterface.js';

jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
    return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {
        snapshotDataSource: () => {},
        removeAttribute: () => {},
    };
});

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
      disconnect: jest.fn(),
      observe: jest.fn(),
      unobserve: jest.fn(),
  }));

describe('TabPaneComparison Test', () => {
    let data = {
        end_time: 5373364415,
        file_name: 'Snapshot1',
        frame: { height: 40, width: 24, x: 272, y: 0 },
        id: 1,
        pid: 4243,
        start_time: 4937360912,
        textMetricsWidth: 60.697265625,
    };
    let dataList = [
        {
            id: 0,
            file_name: 'Snapshot0',
            start_time: 0,
            end_time: 435811002,
            pid: 4243,
            textMetricsWidth: 50.5810546875,
        },
    ];

    let childenData = [
        {
            addedCount: 6491111,
            addedIndx: [319, 326],
            addedSize: 38936,
            childCount: 1296,
            children: [],
            classChildren: [],
            deletedIdx: [325, 338],
            deltaCount: 0,
            deltaSize: -16,
            distance: -1,
            edgeCount: 0,
            edgeName: 'edgeName',
            fileId: 0,
            hasNext: true,
            id: -1,
            index: 0,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'nodeName',
            objectName: 'objectName',
            removedCount: 648,
            removedSize: 38952,
            retainedSize: -1,
            retains: [],
            shallowSize: -1,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 1,
            traceNodeId: -1,
            type: 4,
        },
        {
            addedCount: 649,
            addedIndx: [319, 326],
            addedSize: 38936,
            childCount: 1296,
            children: [],
            classChildren: [],
            deletedIdx: [325, 338],
            deltaCount: 0,
            deltaSize: -16,
            distance: -1,
            edgeCount: 0,
            edgeName: '',
            fileId: 1,
            hasNext: true,
            id: -1,
            index: 0,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'nodeName',
            objectName: 'objectName',
            removedCount: 648,
            removedSize: 38952,
            retainedSize: -1,
            retains: [],
            shallowSize: -1,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 1,
            traceNodeId: -1,
            type: 4,
        },
    ];
    let ddd = [
        {
            addedCount: 6482222,
            addedIndx: [319, 326],
            addedSize: 38936,
            childCount: 1296,
            children: [],
            classChildren: [],
            deletedIdx: [325, 338],
            deltaCount: 0,
            deltaSize: -16,
            distance: -1,
            edgeCount: 0,
            edgeName: 'edgeName',
            fileId: 0,
            hasNext: true,
            id: -1,
            index: 0,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'nodeName',
            objectName: 'objectName',
            removedCount: 648,
            removedSize: 38952,
            retainedSize: -1,
            retains: [],
            shallowSize: -1,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 1,
            traceNodeId: -1,
            type: 4,
            isString: jest.fn(() => true),
        },
        {
            addedCount: 6483333,
            addedIndx: [319, 326],
            addedSize: 38936,
            childCount: 1296,
            children: [],
            classChildren: [],
            deletedIdx: [325, 338],
            deltaCount: 0,
            deltaSize: -16,
            distance: -1,
            edgeCount: 0,
            edgeName: '',
            fileId: 1,
            hasNext: true,
            id: -1,
            index: 0,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'nodeName',
            objectName: 'objectName',
            removedCount: 648,
            removedSize: 38952,
            retainedSize: -1,
            retains: [],
            shallowSize: -1,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 1,
            traceNodeId: -1,
            type: 4,
            isString: jest.fn(() => true),
        },
        {
            addedCount: 6485555,
            addedIndx: [319, 326],
            addedSize: 38936,
            childCount: 1296,
            children: [],
            classChildren: [],
            deletedIdx: [325, 338],
            deltaCount: 0,
            deltaSize: -16,
            distance: -1,
            edgeCount: 0,
            edgeName: '',
            fileId: 2,
            hasNext: true,
            id: -1,
            index: 0,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'nodeName',
            objectName: 'objectName',
            removedCount: 648,
            removedSize: 38952,
            retainedSize: -1,
            retains: [],
            shallowSize: -1,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 1,
            traceNodeId: -1,
            type: 2,
            isString: jest.fn(() => true),
        },
    ];
    let rowObjectData = {
        top: 0,
        height: 0,
        rowIndex: 0,
        data: {
            status: true,
            targetFileId: 12,
            children: childenData,
            getChildren: () => {},
        },
        expanded: true,
        rowHidden: false,
        children: childenData,
        depth: -1,
    };

    let iconClick = new CustomEvent('icon-click', <CustomEventInit>{
        detail: {
            ...rowObjectData.data,
            data: rowObjectData.data,
        },
        composed: true,
    });
    let iconRowClick = new CustomEvent('row-click', <CustomEventInit>{
        detail: {
            ...rowObjectData.data,
            data: ddd[0],
        },
        composed: true,
    });
    let iconColumnClickData = new CustomEvent('column-click', <CustomEventInit>{
        detail: {
            key: 'addedCount',
            sort: 1,
        },
        composed: true,
    });
    let iconColumnClick = new CustomEvent('column-click', <CustomEventInit>{
        detail: {
            key: 'addedCount',
            sort: 1,
        },
        composed: true,
    });
    let iconkeyUpClick = new CustomEvent('keyup', <CustomEventInit>{
        detail: {
            key: 'addedCount',
            sort: 1,
        },
        composed: true,
    });
    document.body.innerHTML = `
        <tabpane-comparison id="sss"></tabpane-comparison>`;
    let tabPaneComparisons = document.getElementById('sss') as TabPaneComparison;

    HeapDataInterface.getInstance().getClassListForComparison = jest.fn(() => []);
    HeapDataInterface.getInstance().getNextForComparison = jest.fn(() => ddd);
    let htmlInputElement = document.createElement('input');
    htmlInputElement.value = 'input';
    tabPaneComparisons.search = jest.fn(() => htmlInputElement);
    let heapDataInterface = new HeapDataInterface();
    heapDataInterface.fileStructs = [data];
    let htmlDivElement = document.createElement('div');
    tabPaneComparisons.leftTheadTable = jest.fn(() => htmlDivElement);
    tabPaneComparisons.comparisonTable = jest.fn(() => htmlDivElement);
    tabPaneComparisons.rightTheadTable = jest.fn(() => htmlDivElement);
    tabPaneComparisons.rightTheadTable.removeAttribute = jest.fn(() => true);
    tabPaneComparisons.rightTheadTable.hasAttribute = jest.fn(() => {});
    tabPaneComparisons.leftTheadTable = jest.fn(() => htmlDivElement);
    tabPaneComparisons.leftTheadTable.hasAttribute = jest.fn(() => {});
    tabPaneComparisons.leftTheadTable.removeAttribute = jest.fn(() => true);

    it('TabPaneComparisonTest01', () => {
        tabPaneComparisons.comparisonsData = jest.fn(() => ddd);
        tabPaneComparisons.comparisonTableEl.reMeauseHeight = jest.fn(() => true);
        tabPaneComparisons.initComparison(data, dataList);

        let retainsData = [
            {
                shallowSize: 10,
                retainedSize: 10,
                shallowPercent: 10,
                retainedPercent: 10,
                distance: 1000000001,
                nodeName: 'nodeName',
                objectName: 'objectName',
                edgeName: 'edgeName',
                children: childenData,
            },
            {
                shallowSize: 1,
                retainedSize: 1,
                shallowPercent: 1,
                retainedPercent: 1,
                distance: 100000000,
                nodeName: 'nodeName',
                objectName: 'objectName',
                edgeName: 'edgeName',
                children: childenData,
            },
        ];
        HeapDataInterface.getInstance().getRetains = jest.fn(() => retainsData);

        tabPaneComparisons.comparisonTableEl!.dispatchEvent(iconClick);
        tabPaneComparisons.comparisonTableEl!.dispatchEvent(iconRowClick);
        tabPaneComparisons.comparisonTableEl!.dispatchEvent(iconColumnClickData);
        tabPaneComparisons.retainerTableEl.reMeauseHeight = jest.fn(() => true);
        tabPaneComparisons.retainerTableEl!.dispatchEvent(iconColumnClick);

        tabPaneComparisons.sortComprisonByColumn('addedCount', 0);
        tabPaneComparisons.sortComprisonByColumn('removedCount', 1);
        tabPaneComparisons.sortComprisonByColumn('addedCount', 1);
        tabPaneComparisons.sortComprisonByColumn('deltaCount', 1);
        tabPaneComparisons.sortComprisonByColumn('objectName', 1);
        tabPaneComparisons.sortComprisonByColumn('addedSize', 1);
        tabPaneComparisons.sortComprisonByColumn('removedSize', 1);
        tabPaneComparisons.sortComprisonByColumn('deltaSize', 1);

        tabPaneComparisons.sortRetainerByColumn('distance', 0);
        tabPaneComparisons.sortRetainerByColumn('shallowSize', 1);
        tabPaneComparisons.sortRetainerByColumn('retainedSize', 1);
        tabPaneComparisons.sortRetainerByColumn('objectName', 1);
        expect(tabPaneComparisons.comparisonTableEl!.snapshotDataSource).toEqual([]);
    });

    it('TabPaneComparisonTest2', () => {
        tabPaneComparisons.retainsData = [
            {
                distance: 1,
            },
        ];
        tabPaneComparisons.retainerTableEl!.dispatchEvent(iconClick);
        expect(tabPaneComparisons.retainsData).not.toBe([]);
    });

    it('TabPaneComparisonTest12', () => {
        let tabPaneComparison = new TabPaneComparison();
        let data = [
            {
                end_time: 5373364415,
                file_name: 'Snapshot1',
                frame: { height: 40, width: 24, x: 272, y: 0 },
                id: 1,
                pid: 4243,
                start_time: 4937360912,
                textMetricsWidth: 60.697265625,
            },
        ];
        expect(tabPaneComparison.initSelect(0, data)).toBeUndefined();
    });
});
