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
import { TabPanePerfAnalysis } from '../../../../../../dist/trace/component/trace/sheet/hiperf/TabPanePerfAnalysis.js';
//@ts-ignore
import {queryHiPerfProcessCount} from "../../../../../../dist/trace/database/SqlLite.js";
import crypto from 'crypto';

// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
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

jest.mock('../../../../../../dist/base-ui/chart/pie/LitChartPie.js', () => {
    return {};
});

jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {};
});

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

describe('TabPanePerfAnalysis Test', () => {
    it('TabPanePerfAnalysisTest01 ', function () {
       let tabPanePerfAnalysis = new TabPanePerfAnalysis()
        expect(tabPanePerfAnalysis.clearData()).toBeUndefined();
    });
    it('TabPanePerfAnalysisTest02 ', function () {
        let itemClick = new CustomEvent('click', <CustomEventInit>{
            detail: {
                ...{},
                data: {},
            },
            composed: true,
        });
        document.body.innerHTML = `
        <tabpane-perf-analysis id="stc"></tabpane-perf-analysis>`;
        let tabPanePerfAnalysis = document.getElementById('stc') as TabPanePerfAnalysis;
        tabPanePerfAnalysis.tabName!.textContent = "Statistic By Thread Count";
        tabPanePerfAnalysis.allProcessCount = {
            "countFormat": "14.00s",
            "percent": "100.00",
            "count": 0,
            "allCount": 13773,
            "pid": ""
        };
        tabPanePerfAnalysis.pidData = [{
            "tableName": "hiperf(28917)",
            "pid": 28917,
            "percent": "43.42",
            "countFormat": "10.00s",
            "count": 10387
        },{
            "tableName": "render_service(388)",
            "pid": 388,
            "percent": "17.99",
            "countFormat": "4.00s",
            "count": 4303
        },{
            "tableName": "hilogd(233)",
            "pid": 233,
            "percent": "6.76",
            "countFormat": "2.00s",
            "count": 1616
        },{
            "tableName": "power_host(374)",
            "pid": 374,
            "percent": "0.25",
            "countFormat": "59.00ms",
            "count": 59
        }];
        tabPanePerfAnalysis.tableProcess.reMeauseHeight = jest.fn(() => true);
        tabPanePerfAnalysis.back!.dispatchEvent(itemClick);
        expect(tabPanePerfAnalysis.getBack()).toBeUndefined();
    });
    it('TabPanePerfAnalysisTest03', function () {
        let itemClick = new CustomEvent('click', <CustomEventInit>{
            detail: {
                ...{},
                data: {},
            },
            composed: true,
        });
        document.body.innerHTML = `
        <tabpane-perf-analysis id="slc"></tabpane-perf-analysis>`;
        let tabPanePerfAnalysis = document.getElementById('slc') as TabPanePerfAnalysis;
        tabPanePerfAnalysis.tabName!.textContent = "Statistic By Library Count";
        tabPanePerfAnalysis.allThreadCount = {
            "countFormat": "24.00s",
            "percent": "100.00",
            "count": 0,
            "allCount": 23922,
            "pid": ""
        };
        tabPanePerfAnalysis.threadData = [{
            "tableName": "kworker/0:0H-mmc_complete(28750)",
            "pid": 28750,
            "percent": "0.10",
            "countFormat": "24.00ms",
            "count": 24
        },{
            "tableName": "com.ohos.callui(1362)",
            "pid": 1362,
            "percent": "0.09",
            "countFormat": "22.00ms",
            "count": 22
        }];
        tabPanePerfAnalysis.currentSelection = {
            leftNs: 14957548626,
            recordStartNs: 30369799963682,
            rightNs: 21422844177,
            perfThread:[]
        };
        tabPanePerfAnalysis.perfTableThread.reMeauseHeight = jest.fn(() => true);
        tabPanePerfAnalysis.back!.dispatchEvent(itemClick);
        expect(tabPanePerfAnalysis.getBack()).toBeUndefined();
    });

    it('TabPanePerfAnalysisTest04', function () {
        let itemClick = new CustomEvent('click', <CustomEventInit>{
            detail: {
                ...{},
                data: {},
            },
            composed: true,
        });
        document.body.innerHTML = `
        <tabpane-perf-analysis id="slc"></tabpane-perf-analysis>`;
        let tabPanePerfAnalysis = document.getElementById('slc') as TabPanePerfAnalysis;
        tabPanePerfAnalysis.tabName!.textContent = "Statistic By Function Count";
        tabPanePerfAnalysis.allLibCount = {
            "countFormat": "28.00s",
            "percent": "200.00",
            "count": 1,
            "allCount": 23966,
            "pid": ""
        };
        tabPanePerfAnalysis.soData = [{
            "tableName": "kworker/0:0H-mmc_complete(28750)",
            "pid": 28750,
            "percent": "0.10",
            "countFormat": "24.00ms",
            "count": 24
        },{
            "tableName": "com.ohos.callui(1362)",
            "pid": 1362,
            "percent": "0.09",
            "countFormat": "22.00ms",
            "count": 22
        }];
        tabPanePerfAnalysis.currentSelection = {
            leftNs: 1495754,
            recordStartNs: 30369799,
            rightNs: 214228,
            perfThread:[]
        };
        tabPanePerfAnalysis.tableSo.reMeauseHeight = jest.fn(() => true);
        tabPanePerfAnalysis.back!.dispatchEvent(itemClick);
        expect(tabPanePerfAnalysis.getBack()).toBeUndefined();
    });

    it('TabPanePerfAnalysisTest05', function () {
        let tabPanePerfAnalysis = new TabPanePerfAnalysis()
        expect(tabPanePerfAnalysis.sortByColumn({key: 'startTime'}, {sort: 1})).toBeUndefined();
    });
    it('TabPanePerfAnalysisTest06', function () {
        let tabPanePerfAnalysis = new TabPanePerfAnalysis()
        expect(tabPanePerfAnalysis.totalCountData(1)).toStrictEqual(
            {"allCount": 1, "count": 0, "countFormat": "1.00ms", "percent": "100.00", "pid": ""}
        );
    });
    it('TabPanePerfAnalysisTest07', function () {
        let tabPanePerfAnalysis = new TabPanePerfAnalysis();
        let res = [{
            count:1,
            length:1
        }]
        expect(tabPanePerfAnalysis.getPieChartData(res)).toStrictEqual(
            [{"count": 1, "length": 1}]
        );
    });

    it('TabPanePerfAnalysisTest08', function () {
        let tabPanePerfAnalysis = new TabPanePerfAnalysis()
        let queryHiPerf = sqlit.queryHiPerfProcessCount;
        queryHiPerf.mockResolvedValue([
            {
                "pid": 174,
                "time": 11799859602,
                "threadName": "sugov:0",
                "tid": 174,
                "id": 28347,
                "callchain_id": 10972,
                "processName": "sugov:0(174)"
            },
            {
                "pid": 388,
                "time": 11811453353,
                "threadName": "render_service",
                "tid": 871,
                "id": 28355,
                "callchain_id": 10974,
                "processName": "render_service(388)"
            },
            {
                "pid": 28826,
                "time": 11820687229,
                "threadName": "kworker/2:2-events_freezable",
                "tid": 28826,
                "id": 28361,
                "callchain_id": 10976,
                "processName": "kworker/2:2-events_freezable(28826)"
            },
            {
                "pid": 28917,
                "time": 11831719814,
                "threadName": "hiperf",
                "tid": 28922,
                "id": 28372,
                "callchain_id": 51,
                "processName": "hiperf(28917)"
            }
        ])
        let para = {
            leftNs:11799195238,
            rightNs:16844304830,
            cpus: [1,2,3],
            threads: [4,5,6],
            processes: [7,8,9],
            perfThread:[4,5,6],
            perfProcess:[4,5,6]
        }
        tabPanePerfAnalysis.tableProcess.reMeauseHeight = jest.fn(() => true);
        tabPanePerfAnalysis.getHiperfProcess(para)
        expect(tabPanePerfAnalysis.clearData()).toBeUndefined();
    });

    it('TabPanePerfAnalysisTest09', function () {
        let tabPanePerfAnalysis = new TabPanePerfAnalysis()
        let para = {
            count: 5,
            tid: 1,
            pid: 2,
            libId: 3
        }
        let processArr = [
            {
                "pid": 233,
                "time": 7978660718,
                "threadName": "hilogd",
                "tid": 235,
                "id": 19165,
                "callchain_id": 7492
            },
            {
                "pid": 233,
                "time": 8092040146,
                "threadName": "hilogd",
                "tid": 235,
                "id": 19408,
                "callchain_id": 7578
            },
            {
                "pid": 233,
                "time": 8117205732,
                "threadName": "hilogd",
                "tid": 235,
                "id": 19496,
                "callchain_id": 7618
            }
        ];
        tabPanePerfAnalysis.processData = processArr;
        tabPanePerfAnalysis.tableFunction.reMeauseHeight = jest.fn(() => true);
        tabPanePerfAnalysis.getHiperfFunction(para, null);
        expect(tabPanePerfAnalysis.clearData()).toBeUndefined();
    });

    it('TabPanePerfAnalysisTest10', function () {
        document.body.innerHTML = `
        <tabpane-perf-analysis id="slc"></tabpane-perf-analysis>`;
        let tabPanePerfAnalysis = document.getElementById('slc') as TabPanePerfAnalysis;
        tabPanePerfAnalysis.currentLevel = 0;
        tabPanePerfAnalysis.currentLevelData = [
            {
                "pid": 28917,
                "tid": 28922,
                "percent": "93.37",
                "countFormat": "3.00s",
                "count": 3339,
                "tableName": "ld-musl-aarch64.so.1",
                "libId": 10,
                "isHover": false
            },
            {
                "pid": 28917,
                "tid": 28922,
                "percent": "0.73",
                "countFormat": "26.00ms",
                "count": 26,
                "tableName": "libc++.so",
                "libId": 6,
                "isHover": false
            }
        ];
        tabPanePerfAnalysis.allProcessCount = {
            "countFormat": "13.00s",
            "percent": "100.00",
            "count": 0,
            "allCount": 13186,
            "pid": "",
            "isHover": false
        };
        tabPanePerfAnalysis.allThreadCount = {
            "countFormat": "6.00s",
            "percent": "100.00",
            "count": 0,
            "allCount": 5662,
            "pid": "",
            "isHover": false
        };
        tabPanePerfAnalysis.allLibCount = {
            "countFormat": "5.00s",
            "percent": "100.00",
            "count": 0,
            "allCount": 4840,
            "pid": "",
            "isHover": false
        };
        tabPanePerfAnalysis.allSymbolCount = {
            "countFormat": "4.00s",
            "percent": "100.00",
            "count": 0,
            "allCount": 4489,
            "pid": "",
            "isHover": false
        };
        tabPanePerfAnalysis.sortByColumn("tableName", 0);
        tabPanePerfAnalysis.sortByColumn("tableName", 1);
        tabPanePerfAnalysis.sortByColumn("countFormat", 2);
        tabPanePerfAnalysis.sortByColumn("percent", 3);
        tabPanePerfAnalysis.currentLevel = 1;
        tabPanePerfAnalysis.sortByColumn("tableName", 0);
        tabPanePerfAnalysis.sortByColumn("tableName", 1);
        tabPanePerfAnalysis.sortByColumn("countFormat", 2);
        tabPanePerfAnalysis.sortByColumn("percent", 3);
        tabPanePerfAnalysis.currentLevel = 2;
        tabPanePerfAnalysis.sortByColumn("tableName", 0);
        tabPanePerfAnalysis.sortByColumn("tableName", 1);
        tabPanePerfAnalysis.sortByColumn("countFormat", 2);
        tabPanePerfAnalysis.sortByColumn("percent", 3);
        tabPanePerfAnalysis.currentLevel = 3;
        tabPanePerfAnalysis.sortByColumn("tableName", 0);
        tabPanePerfAnalysis.sortByColumn("tableName", 1);
        tabPanePerfAnalysis.sortByColumn("countFormat", 2);
        tabPanePerfAnalysis.sortByColumn("percent", 3);
        expect(tabPanePerfAnalysis).toBeTruthy();
    });

    it('TabPanePerfAnalysisTest11', function () {
        document.body.innerHTML = `
        <tabpane-perf-analysis id="slc"></tabpane-perf-analysis>`;
        let tabPanePerfAnalysis = document.getElementById('slc') as TabPanePerfAnalysis;
        let item = {
            "tableName": "hiperf(28917)",
            "pid": 28917,
            "percent": "43.53",
            "countFormat": "8.00s",
            "count": 8460,
            "isHover": true
        };
        let val = {
            perfThread: [],
            perfProcess: [],
        }
        tabPanePerfAnalysis.processData = [
            {
                "sampleId": 0,
                "tid": 12,
                "count": 1,
                "threadState": "Running",
                "pid": 12,
                "eventCount": 14,
                "threadName": "rcu_sched",
                "processName": "rcu_sched",
                "libId": 106,
                "libName": "[kernel.kallsyms]",
                "symbolId": 148,
                "symbolName": "perf_trace_sched_wakeup_template"
            },
            {
                "sampleId": 0,
                "tid": 193,
                "count": 1,
                "threadState": "Running",
                "pid": 193,
                "eventCount": 14,
                "threadName": "irq/31-rga",
                "processName": "irq/31-rga",
                "libId": 106,
                "libName": "[kernel.kallsyms]",
                "symbolId": 148,
                "symbolName": "perf_trace_sched_wakeup_template"
            }];
        tabPanePerfAnalysis.perfTableThread.reMeauseHeight = jest.fn(() => true);
        tabPanePerfAnalysis.getHiperfThread(item, val);
        expect(tabPanePerfAnalysis).toBeTruthy();
    });
})