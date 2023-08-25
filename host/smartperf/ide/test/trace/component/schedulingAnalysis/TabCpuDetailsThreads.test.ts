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
import { TabCpuDetailsThreads } from '../../../../dist/trace/component/schedulingAnalysis/TabCpuDetailsThreads.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('TabCpuDetailsThreads Test', () => {
    it('TabCpuDetailsThreadsTest01', () => {
        let tabCpuDetailsThreads = new TabCpuDetailsThreads();
        expect(
            tabCpuDetailsThreads.sortByColumn({
                key: 'number',
            })
        ).toBeUndefined();
    });
    it('TabCpuDetailsThreadsTest02', () => {
        let tabCpuDetailsThreads = new TabCpuDetailsThreads();
        let data = [
            {
                pid: 1,
                pName: 1,
                tid: 2,
                tName: '',
                total: 1,
                size: 'middle core',
                no: '',
                timeStr: '',
            }
        ]
        tabCpuDetailsThreads.cpuDetailsThreadUsageTbl.reMeauseHeight = jest.fn();
        expect(tabCpuDetailsThreads.queryPieChartDataByType(data)).toBeUndefined();
    });
    it('TabCpuDetailsThreadsTest02', () => {
        let tabCpuDetailsThreads = new TabCpuDetailsThreads();
        tabCpuDetailsThreads.init = jest.fn();
        expect(tabCpuDetailsThreads.init(1,{})).toBeUndefined();
    });
})