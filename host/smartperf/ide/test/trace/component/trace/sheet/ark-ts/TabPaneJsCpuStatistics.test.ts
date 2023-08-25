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
import { TabPaneJsCpuStatistics } from '../../../../../../dist/trace/component/trace/sheet/ark-ts/TabPaneJsCpuStatistics.js';
import '../../../../../../dist/trace/component/trace/sheet/ark-ts/TabPaneJsCpuStatistics.js';
import { JsCpuProfilerStatisticsStruct } from '../../../../../../dist/trace/bean/JsStruct.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {}
});
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
    return {
        recycleDataSource: () => {},
        removeAttribute: () => {},
        reMeauseHeight: () => {},
        addEventListener: () => {},
    };
});

// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

describe('TabPaneJsCpuCallTree Test', () => {
    document.body.innerHTML = `<tabpane-js-cpu-statistics id="statistics"></tabpane-js-cpu-statistics>`;
    let tabPaneJsCpuStatistics = document.querySelector<TabPaneJsCpuStatistics>('#statistics');
    let res = [
        {
            type: 'a',
            time: 0,
            timeStr:'st',
            percentage:'s',
        },
        {
            type: 'a',
            time: 345,
            timeStr:'st',
            percentage:'s',
        }
    ]
    it('TabPaneJsCpuStatisticsTest01', () => {
        tabPaneJsCpuStatistics.init = jest.fn(()=>true)
        tabPaneJsCpuStatistics.data = {
            rightNs: 5,
            leftNs: 1,
        }
        expect(tabPaneJsCpuStatistics.data).toBeUndefined();
    });
    it('TabPaneJsCpuStatisticsTest02', () => {
        tabPaneJsCpuStatistics.statisticsTable = jest.fn(()=>true)
        tabPaneJsCpuStatistics.statisticsTable.addEventListener = jest.fn(()=>true)
        tabPaneJsCpuStatistics.statisticsTable.reMeauseHeight = jest.fn(()=>true)
        expect(tabPaneJsCpuStatistics.queryPieChartDataByType(res)).toBeUndefined();
    });
    it('TabPaneJsCpuStatisticsTest03', () => {
        expect(tabPaneJsCpuStatistics.totalData(res)).toEqual(
            {"percentage": "100.0", "time": 345, "timeStr": "345.0 ns", "type": ""}
        );
    });
    it('TabPaneJsCpuStatisticsTest04', () => {
        tabPaneJsCpuStatistics.toStatisticsStruct = jest.fn(()=>true)
        expect(tabPaneJsCpuStatistics.setStatisticsData([''],res)).toStrictEqual([true]);
    });
    it('TabPaneJsCpuStatisticsTest05', () => {
        expect(
            tabPaneJsCpuStatistics.sortByColumn({
                key: 'time',
                sort:0,
                type: 'number',
            })
        ).toBeUndefined();
    });
});
