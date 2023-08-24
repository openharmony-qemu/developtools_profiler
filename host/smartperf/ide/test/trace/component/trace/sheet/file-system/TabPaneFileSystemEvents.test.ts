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

import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFileSystemEvents.js';

// @ts-ignore
import { TabPaneFileSystemEvents } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFileSystemEvents.js';
// @ts-ignore
import {LitTable} from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import {TabPaneFilter} from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
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
describe('TabPaneFileSystemEvents Test', () => {
    document.body.innerHTML = `<tabpane-filesystem-event id="files"></tabpane-filesystem-event>`;
    let tabPaneFileSystemEvents = document.querySelector('#files') as TabPaneFileSystemEvents;
    let filterSource = [
        {
            backtrace: ['0x7faa10f228', '(10 other frames)'],
            callchainId: 13,
            depth: 10,
            dur: 240916,
            durStr: '240.92μs ',
            fd: 14,
            fileId: 546,
            isHover: false,
            path: '/data/local/tmp/test',
            process: 'power_host[911]',
            startTs: 285141821,
            startTsStr: '285ms 141μs 821ns ',
            symbol: '0x7faa10f228',
            type: 0,
            typeStr: 'OPEN',
        },
        {
            backtrace: ['0x7faa10f228', '(10 other frames)'],
            callchainId: 15,
            depth: 10,
            dur: 7583,
            durStr: '7.58μs ',
            fd: 14,
            fileId: null,
            isHover: false,
            path: '-',
            process: 'test[911]',
            startTs: 285449632,
            startTsStr: '285ms 449μs 821ns ',
            symbol: '0x7faa10f228',
            type: 1,
            typeStr: 'CLOSE',
        },
    ];

    let param = {
        anomalyEnergy: [],
        clockMapData: {size: 0},
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
        irqMapData: {size: 0},
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
        fileSystemFsData:{title:'All'}
    };

    it('TabPaneFileStatisticsTest01', function () {
        tabPaneFileSystemEvents.filterSource = filterSource;
        expect(tabPaneFileSystemEvents.sortFsSysEventTable('', 0)).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest02', function () {
        tabPaneFileSystemEvents.filterSource = filterSource;
        expect(tabPaneFileSystemEvents.sortFsSysEventTable('startTsStr', 1)).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest03', function () {
        tabPaneFileSystemEvents.filterSource = filterSource;
        expect(tabPaneFileSystemEvents.sortFsSysEventTable('durStr', 1)).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest04', function () {
        tabPaneFileSystemEvents.filterSource = filterSource;
        expect(tabPaneFileSystemEvents.sortFsSysEventTable('process', 2)).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest05', function () {
        tabPaneFileSystemEvents.filterSource = filterSource;
        expect(tabPaneFileSystemEvents.sortFsSysEventTable('thread', 2)).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest06', function () {
        tabPaneFileSystemEvents.filterSource = filterSource;
        expect(tabPaneFileSystemEvents.sortFsSysEventTable('typeStr', 2)).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest07', function () {
        let litTable = new LitTable();
        tabPaneFileSystemEvents.appendChild(litTable);
        let filter = new TabPaneFilter();
        tabPaneFileSystemEvents.filter = filter;
        tabPaneFileSystemEvents.loadingList = [];
        tabPaneFileSystemEvents.data = param;
        expect(tabPaneFileSystemEvents.currentSelection).not.toBeUndefined();
    });

    it('TabPaneFileStatisticsTest08', function () {
        let litTable = new LitTable();
        tabPaneFileSystemEvents.appendChild(litTable);
        let filter = new TabPaneFilter();
        tabPaneFileSystemEvents.filter = filter;
        tabPaneFileSystemEvents.loadingList = [];
        tabPaneFileSystemEvents.data = param;
        tabPaneFileSystemEvents.setProcessFilter();
        expect(tabPaneFileSystemEvents.pathList).toEqual(['All Path']);
    });

    it('TabPaneFileStatisticsTest09', function () {
        let litTable = new LitTable();
        tabPaneFileSystemEvents.appendChild(litTable);
        let filter = new TabPaneFilter();
        tabPaneFileSystemEvents.filter = filter;
        tabPaneFileSystemEvents.loadingList = [];
        tabPaneFileSystemEvents.data = param;
        expect(tabPaneFileSystemEvents.filterData()).toBeUndefined();
    });

    it('TabPaneFileStatisticsTest10', function () {
        let litTable = new LitTable();
        tabPaneFileSystemEvents.appendChild(litTable);
        let filter = new TabPaneFilter();
        tabPaneFileSystemEvents.filter = filter;
        tabPaneFileSystemEvents.loadingList = [];
        tabPaneFileSystemEvents.data = param;
        tabPaneFileSystemEvents.fromStastics(param);
        expect(tabPaneFileSystemEvents.filterEventType).toEqual('0');
    });
});
