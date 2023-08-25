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
import { Top20FrequencyThread } from '../../../../dist/trace/component/schedulingAnalysis/Top20FrequencyThread.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('Top20FrequencyThread Test', () => {
    it('Top20FrequencyThreadTest01', () => {
        let top20FrequencyThread = new Top20FrequencyThread();
        expect(top20FrequencyThread).not.toBeUndefined();
    });
    it('Top20FrequencyThreadTest02', () => {
        let top20FrequencyThread = new Top20FrequencyThread();
        expect(
            top20FrequencyThread.sortByColumn({
                key: 'number',
            })
        ).toBeUndefined();
    });
    it('Top20FrequencyThreadTest03', () => {
        let top20FrequencyThread = new Top20FrequencyThread();
        top20FrequencyThread.queryLogicWorker = jest.fn();
        expect(top20FrequencyThread.queryData()).toBeUndefined();
    });
    it('Top20FrequencyThreadTest04', () => {
        let top20FrequencyThread = new Top20FrequencyThread();
        top20FrequencyThread.queryLogicWorker = jest.fn();
        let res = [
            {
                length:21,
                time:'',
                totalDur:1
            }
        ]
        expect(top20FrequencyThread.getPieChartData(res)).toStrictEqual([{"length": 21, "time": "", "totalDur": 1}]);
    });
    it('Top20FrequencyThreadTest05', () => {
        let top20FrequencyThread = new Top20FrequencyThread();
        top20FrequencyThread.queryLogicWorker = jest.fn();
        expect(top20FrequencyThread.queryLogicWorker('','',{})).toBeUndefined();
    });
    it('Top20FrequencyThreadTest06', () => {
        let top20FrequencyThread = new Top20FrequencyThread();
        top20FrequencyThread.init = jest.fn();
        expect(top20FrequencyThread.init()).toBeUndefined();
    });
})