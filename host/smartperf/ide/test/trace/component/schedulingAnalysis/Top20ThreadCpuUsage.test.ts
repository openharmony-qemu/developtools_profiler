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
import { Top20ThreadCpuUsage } from '../../../../dist/trace/component/schedulingAnalysis/Top20ThreadCpuUsage.js';
// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
}));

describe('Top20ThreadCpuUsage Test', () => {
    it('Top20ThreadCpuUsageTest01', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
        expect(top20ThreadCpuUsage).not.toBeUndefined();
    });
    it('Top20ThreadCpuUsageTest02', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
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
        expect(top20ThreadCpuUsage.sortByColumn({key: 'bigTimeStr'},{},data)).toBeUndefined();
    });
    it('Top20ThreadCpuUsageTest03', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
        top20ThreadCpuUsage.queryLogicWorker = jest.fn();
        expect(top20ThreadCpuUsage.queryData()).toBeUndefined();
    });
    it('Top20ThreadCpuUsageTest04', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
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
        expect(top20ThreadCpuUsage.getArrayDataBySize('total',data)).toStrictEqual(
            [{"no": "", "pName": 1, "pid": 1, "size": "big core", "tName": "", "tid": 2, "timeStr": undefined, "total": undefined}, {"no": "", "pName": 1, "pid": 1, "size": "middle core", "tName": "", "tid": 2, "timeStr": undefined, "total": undefined}, {"no": "", "pName": 1, "pid": 1, "size": "small core", "tName": "", "tid": 2, "timeStr": undefined, "total": undefined}]
        );
    });
    it('Top20ThreadCpuUsageTest05', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
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
        expect(top20ThreadCpuUsage.sortByColumn({key: 'midTimeStr'},{},data)).toBeUndefined();
    });
    it('Top20ThreadCpuUsageTest06', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
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
        expect(top20ThreadCpuUsage.sortByColumn({key: 'smallTimeStr'},{},data)).toBeUndefined();
    });
    it('Top20ThreadCpuUsageTest07', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
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
        expect(top20ThreadCpuUsage.sortByColumn({key: 'bigPercent'},{},data)).toBeUndefined();
    });
    it('Top20ThreadCpuUsageTest08', () => {
        let top20ThreadCpuUsage = new Top20ThreadCpuUsage();
        top20ThreadCpuUsage.queryLogicWorker = jest.fn();
        expect(top20ThreadCpuUsage.queryLogicWorker('','',{})).toBeUndefined();
    });
})