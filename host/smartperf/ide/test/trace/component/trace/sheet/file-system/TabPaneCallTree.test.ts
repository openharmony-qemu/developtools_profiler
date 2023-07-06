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
import { TabPaneCallTree } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneCallTree.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneCallTree.js';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
import '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
import { FrameChart } from '../../../../../../dist/trace/component/chart/FrameChart.js';
import '../../../../../../dist/trace/component/chart/FrameChart.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {};
});

import crypto from 'crypto';
import {showButtonMenu} from "../../../../../../src/trace/component/trace/sheet/SheetUtils.js";

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

describe('TabPaneCallTree Test', () => {
    let data = {
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
    it('TabPaneCallTreeTest01', function () {
        document.body.innerHTML = `<tabpane-calltree id="calltree"></tabpane-calltree>`;
        let calltree = document.querySelector<TabPaneCallTree>('#calltree');
        let filter = new TabPaneFilter();
        calltree.callTreeFilter = filter;
        let frameChart = new FrameChart();
        calltree.frameChart = frameChart;
        calltree.callTreeFilter.getDataLibrary = jest.fn(() => true);
        calltree.data = data;
        expect(calltree.currentSelection).not.toBeUndefined();
    });

    it('TabPaneCallTreeTest02', function () {
        document.body.innerHTML = `<tabpane-calltree id="calltree"></tabpane-calltree>`;
        let calltree = document.querySelector<TabPaneCallTree>('#calltree');
        let filter = new TabPaneFilter();
        calltree.callTreeFilter = filter;
        let frameChart = new FrameChart();
        calltree.frameChart = frameChart;
        calltree.callTreeFilter.getDataLibrary = jest.fn(() => true);
        calltree.data = data;
        let call = {
            id: '1',
            dur: 1,
            children: [],
        };
        expect(calltree.setRightTableData(call)).toBeUndefined();
    });

    it('TabPaneCallTreeTest03', function () {
        document.body.innerHTML = `<tabpane-calltree id="calltree"></tabpane-calltree>`;
        let calltree = document.querySelector<TabPaneCallTree>('#calltree');
        let filter = new TabPaneFilter();
        calltree.callTreeFilter = filter;
        calltree.showButtonMenu = jest.fn(() => true);
        calltree.showButtonMenu(calltree.callTreeFilter, true);
        expect(calltree.callTreeFilter.getAttribute('tree')).toBe(null);
        calltree.showButtonMenu(calltree.callTreeFilter, false);
    });

    it('TabPaneCallTreeTest04', function () {
        document.body.innerHTML = `<tabpane-calltree id="calltree"></tabpane-calltree>`;
        let calltree = document.querySelector<TabPaneCallTree>('#calltree');
        let resultData = [
            {
                addr: '',
                canCharge: false,
                count: 67,
                depth: 0,
                drawCount: 0,
                drawDur: 0,
                drawSize: 0,
                dur: 43334510310,
                frame: { x: 0, y: 30, width: 594, height: 20 },
                id: '38',
                ip: '',
                isDraw: false,
                isSearch: false,
                isSelected: false,
                isStore: 0,
                lib: '',
                libName: '',
                parentId: '',
                path: '',
                pathId: 0,
                percent: 0.3642222150324375,
                pid: 0,
                processName: '',
                searchShow: true,
                self: '0s',
                selfDur: 0,
                size: 0,
                symbol: 'symbol',
                symbolName: 'symbolName',
                symbolsId: 0,
                textMetricsWidth: 62.7783203125,
                type: 0,
                weight: '43.33s ',
                weightPercent: '36.4%',
                children: [],
            },
        ];
        calltree.setLTableData(resultData);
        expect(calltree.callTreeDataSource.length).toEqual(1);
    });
});
