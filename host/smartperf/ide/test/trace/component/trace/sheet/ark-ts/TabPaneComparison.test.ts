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
import { TabPaneComparison } from '../../../../../../dist/trace/component/trace/sheet/ark-ts/TabPaneComparison.js';
import '../../../../../../dist/trace/component/trace/sheet/ark-ts/TabPaneComparison.js';

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
            addedCount: 6061,
            addedIndx: [319, 16],
            addedSize: 2036,
            childCount: 1326,
            children: [],
            classChildren: [],
            deletedIdx: [11, 338],
            deltaCount: 2,
            deltaSize: -16,
            distance: -1,
            edgeCount: 11,
            edgeName: '12',
            fileId: 10,
            hasNext: true,
            id: 124,
            index: 45,
            isAdd: false,
            isHover: true,
            isSelected: false,
            nextId: [],
            nodeName: 'InternalAccessor',
            objectName: 'InternalAccessor@123',
            removedCount: 612,
            removedSize: 3632,
            retainedSize: 154,
            retains: [],
            shallowSize: 632,
            showBox: false,
            showCut: true,
            status: true,
            targetFileId: 1,
            traceNodeId: 32,
            type: 43,
        },
        {
            addedCount: 659,
            addedIndx: [5, 3256],
            addedSize: 1356,
            childCount: 396,
            children: [],
            classChildren: [],
            deletedIdx: [11, 308],
            deltaCount: 88,
            deltaSize: -16,
            distance: 56,
            edgeCount: 80,
            edgeName: 'a',
            fileId: 1,
            hasNext: true,
            id: 12,
            index: 310,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [1],
            nodeName: 'JSNativePointer',
            objectName: 'JSNativePointerX56',
            removedCount: 6118,
            removedSize: 3832,
            retainedSize: 59,
            retains: [],
            shallowSize: 121,
            showBox: true,
            showCut: false,
            status: true,
            targetFileId: 1,
            traceNodeId: 87,
            type: 74,
        },
    ];
    let ddd = [
        {
            addedCount: 6432,
            addedIndx: [19, 16],
            addedSize: 316,
            childCount: 1196,
            children: [],
            classChildren: [],
            deletedIdx: [1, 318],
            deltaCount: 0,
            deltaSize: -16,
            distance: 43,
            edgeCount: 30,
            edgeName: 'Accessor12',
            fileId: 0,
            hasNext: true,
            id: 103,
            index: 130,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'AccessorData',
            objectName: 'AccessorDataX32',
            removedCount: 6328,
            removedSize: 33252,
            retainedSize: 21,
            retains: [],
            shallowSize: 23,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 231,
            traceNodeId: 3,
            type: 234,
            isString: jest.fn(() => true),
        },
        {
            addedCount: 6563,
            addedIndx: [14, 306],
            addedSize: 336,
            childCount: 136,
            children: [],
            classChildren: [],
            deletedIdx: [23, 18],
            deltaCount: 0,
            deltaSize: -16,
            distance: -1,
            edgeCount: 31,
            edgeName: 'GlobalObject56',
            fileId: 41,
            hasNext: true,
            id: 54,
            index: 40,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'GlobalObject',
            objectName: 'GlobalObjectX21',
            removedCount: 648,
            removedSize: 32,
            retainedSize: 8,
            retains: [],
            shallowSize: -1,
            showBox: false,
            showCut: true,
            status: true,
            targetFileId: 31,
            traceNodeId: 45,
            type: 424,
            isString: jest.fn(() => true),
        },
        {
            addedCount: 5,
            addedIndx: [67, 326],
            addedSize: 316,
            childCount: 66,
            children: [],
            classChildren: [],
            deletedIdx: [325, 91],
            deltaCount: 0,
            deltaSize: -16,
            distance: 33,
            edgeCount: 0,
            edgeName: 'ProtoChangeDetails',
            fileId: 34,
            hasNext: true,
            id: -1,
            index: 20,
            isAdd: false,
            isHover: false,
            isSelected: false,
            nextId: [],
            nodeName: 'ProtoChangeDetails',
            objectName: 'ProtoChangeDetailsX12',
            removedCount: 648,
            removedSize: 312,
            retainedSize: 34,
            retains: [],
            shallowSize: 56,
            showBox: false,
            showCut: false,
            status: true,
            targetFileId: 61,
            traceNodeId: 651,
            type: 40,
            isString: jest.fn(() => true),
        },
    ];
    let rowObjectData = {
        top: 2,
        height: 0,
        rowIndex: 2,
        data: {
            status: true,
            targetFileId: 121,
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
                shallowSize: 4,
                retainedSize: 1,
                shallowPercent: 4,
                retainedPercent: 1,
                distance: 100000000,
                nodeName: 'TSObjectType',
                objectName: 'TSObjectTypeX23',
                edgeName: 'TSObjectType',
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

    it('TabPaneComparisonTest02', () => {
        tabPaneComparisons.retainsData = [
            {
                distance: 1,
            },
        ];
        tabPaneComparisons.retainerTableEl!.dispatchEvent(iconClick);
        expect(tabPaneComparisons.retainsData).not.toBe([]);
    });
});
