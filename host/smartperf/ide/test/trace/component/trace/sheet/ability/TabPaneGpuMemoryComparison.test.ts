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
import { TabPaneGpuMemoryComparison } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneGpuMemoryComparison.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/select/LitSelect.js', () => {
    return {};
});
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
    return {};
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
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {
        snapshotDataSource: () => {},
        removeAttribute: () => {},
    };
});

describe('TabPaneGpuMemoryComparison Test', () => {
    let tabPaneGpuMemoryComparison = new TabPaneGpuMemoryComparison();
    let getTabGpuMemoryComparisonData = sqlite.getTabGpuMemoryComparisonData;
    let gpuMemoryComparisonData = [
        {
            startNs: 0,
            value:100,
            gpuNameId:10,
            processId:2,
            processName:'a',
        }
    ];
    let datalist = [
        {
            name: 'Snapshot2',
            startNs: 201326561,
            type: 'ability',
            value: 17,
        },
        {
            name: 'Snapshot1',
            startNs: 32110343,
            type: 'ability',
            value: 67,
        },
    ];
    tabPaneGpuMemoryComparison.init = jest.fn(() => true);
    getTabGpuMemoryComparisonData.mockResolvedValue(gpuMemoryComparisonData);
    it('TabPaneGpuMemoryComparison01', function () {
        expect(tabPaneGpuMemoryComparison.queryDataByDB(10)).toBeTruthy();
    });
    it('TabPaneGpuMemoryComparison02', function () {
        expect(tabPaneGpuMemoryComparison.getComparisonData(10)).toBeTruthy();
    });
    it('TabPaneGpuMemoryComparison03', function () {
        expect(tabPaneGpuMemoryComparison.comparisonDataByDB(10,datalist)).toBeTruthy();
    });
    it('TabPaneGpuMemoryComparison04', function () {
        expect(tabPaneGpuMemoryComparison.selectStamps(datalist)).toBeUndefined();
    });
})