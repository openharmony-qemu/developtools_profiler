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
import { TabPaneDmaSelectAbility } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneDmaSelectAbility.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
    return {};
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {};
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {};
});
jest.mock('../../../../../../dist/js-heap/model/DatabaseStruct.js', () => {});
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPaneDmaSelectAbility Test', () => {
    let tabPaneDmaSelectAbility = new TabPaneDmaSelectAbility();
    let getTabDmaSelectionData = sqlite.getTabDmaAbilityClickData;
    let dmaSelectionData = [
        {
            startNs: 0,
            fd: 11,
            size: 3200,
            ino: 10,
            expPid: 0,
            bufName: 'aa',
            expName: 'bb',
            expTaskComm: 'expTask',
            processId:2,
            processName:'dd',
            flag:0,
        }
    ];
    let val = [
        {
            startNs: 0,
            rightNs: 1000,
        }
    ];
    tabPaneDmaSelectAbility.init = jest.fn(() => true);
    getTabDmaSelectionData.mockResolvedValue(dmaSelectionData);
    it('TabPaneDmaSelectAbility01', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('',0)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility02', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('process',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility03', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('startNs',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility04', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('expTaskComm',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility05', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('fd',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility05', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('size',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility06', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('ino',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility07', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('expPid',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility08', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('flag',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility09', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('bufName',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility10', function () {
        expect(tabPaneDmaSelectAbility.sortDmaByColumn('expName',1)).toBeUndefined();
    });
    it('TabPaneDmaSelectAbility11', function () {
        expect(tabPaneDmaSelectAbility.queryDmaClickDataByDB(val)).toBeUndefined();
    });
})