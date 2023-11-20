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
import { TabPaneSummary } from '../../../../../../dist/trace/component/trace/sheet/snapshot/TabPaneSummary.js';
//@ts-ignore
import { HeapDataInterface } from '../../../../../../dist/js-heap/HeapDataInterface.js';
//@ts-ignore
import { SpJsMemoryChart } from '../../../../../../dist/trace/component/chart/SpJsMemoryChart.js';

jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
    return {};
});
// @ts-ignore
window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPaneSummary Test', () => {
    let tabPaneSummary = new TabPaneSummary();
    it('TabPaneSummaryTest01', () => {
        document.body.innerHTML = `<tabpane-summary id="sss"> </tabpane-summary>`;
        let tabPaneSummary = document.querySelector('#sss') as TabPaneSummary;
        tabPaneSummary.tbsTable = jest.fn(() => {
            return {
                scrollTop: 0,
            };
        });
        let childenData = [
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
        let ddd = [
            {
                addedCount: 648,
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
                addedCount: 648,
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
            {
                addedCount: 648,
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
            },
        ];
        let htmlDivElement = document.createElement('div');
        tabPaneSummary.leftTheadTable = jest.fn(() => htmlDivElement);
        tabPaneSummary.rightTheadTable = jest.fn(() => htmlDivElement);
        tabPaneSummary.tbsTable = jest.fn(() => {
            return {
                scrollTop: 0,
            };
        });

        tabPaneSummary.rightTheadTable.removeAttribute = jest.fn(() => true);
        tabPaneSummary.rightTheadTable.hasAttribute = jest.fn(() => {});

        tabPaneSummary.leftTheadTable.hasAttribute = jest.fn(() => {});
        tabPaneSummary.leftTheadTable.removeAttribute = jest.fn(() => true);

        SpJsMemoryChart.file = {
            file_name: 'Timeline',
            id: '',
        };
        HeapDataInterface.getInstance().getAllocationStackData = jest.fn(() => {
            return [
                {
                    id: 0,
                    index: 0,
                    name: '',
                    scriptName: '',
                    scriptId: 0,
                    line: 0,
                    column: 0,
                },
                {
                    id: 0,
                    index: 0,
                    name: '',
                    scriptName: 'string',
                    scriptId: 0,
                    line: 0,
                    column: 0,
                },
            ];
        });

        let htmlDivElement1 = document.createElement('div');
        htmlDivElement1.className = 'table';
        tabPaneSummary.tbs.meauseTreeRowElement = jest.fn(() => {
            return [];
        });
        tabPaneSummary.tblSummary.meauseTreeRowElement = jest.fn(() => {
            return [];
        });
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
        HeapDataInterface.getInstance().getRetains = jest.fn(() => retainsData);
        let iconRowClick = new CustomEvent('row-click', <CustomEventInit>{
            detail: {
                data: rowObjectData.data,
            },
            composed: true,
        });

        let iconClick = new CustomEvent('icon-click', <CustomEventInit>{
            detail: {
                data: rowObjectData.data,
            },
            composed: true,
        });
        iconClick.detail.data.isString = jest.fn(() => true);
        tabPaneSummary.tblSummary.dispatchEvent(iconRowClick);
        tabPaneSummary.tblSummary.dispatchEvent(iconClick);
        tabPaneSummary.tbs.dispatchEvent(iconClick);
        tabPaneSummary.sortByLeftTable('distance', 0);
        tabPaneSummary.sortByLeftTable('shallowSize', 1);
        tabPaneSummary.sortByLeftTable('retainedSize', 1);
        tabPaneSummary.sortByLeftTable('objectName', 1);
        HeapDataInterface.getInstance().getClassesListForSummary = jest.fn(() => {
            return retainsData;
        });

        expect(tabPaneSummary.initSummaryData(1, 0, 0)).toBeUndefined();
    });
    it('TabPaneSummaryTest12', () => {
        document.body.innerHTML = `<tabpane-summary id="sss"> </tabpane-summary>`;
        let tabPaneSummary = document.querySelector('#sss') as TabPaneSummary;
        expect(tabPaneSummary.sortByLeftTable('shallowSize', 1)).toBeUndefined();
    });
    it('TabPaneSummaryTest03', () => {
        document.body.innerHTML = `<tabpane-summary id="sss"> </tabpane-summary>`;
        let tabPaneSummary = document.querySelector('#sss') as TabPaneSummary;
        expect(tabPaneSummary.sortByLeftTable('retainedSize', 1)).toBeUndefined();
    });
    it('TabPaneSummaryTest04', () => {
        document.body.innerHTML = `<tabpane-summary id="sss"> </tabpane-summary>`;
        let tabPaneSummary = document.querySelector('#sss') as TabPaneSummary;
        expect(tabPaneSummary.sortByLeftTable('objectName', 1)).toBeUndefined();
    });
    it('TabPaneSummaryTest09', () => {
        expect(tabPaneSummary.clickToggleTable()).toBeUndefined();
    });
    it('TabPaneSummaryTest10', () => {
        expect(tabPaneSummary.classFilter()).toBeUndefined();
    });
});
