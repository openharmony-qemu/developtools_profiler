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

jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
    return {};
});

// @ts-ignore
import { JsCpuProfilerUIStruct, JsCpuProfilerChartFrame, JsCpuProfilerTabStruct, JsCpuProfilerStatisticsStruct } from '../../../dist/trace/bean/JsStruct.js';

describe('JsStruct Test', () => {
    let jsCpuProfilerUIStruct = new JsCpuProfilerUIStruct();
    let jsCpuProfilerChartFrame = new JsCpuProfilerChartFrame();
    let jsCpuProfilerTabStruct = new JsCpuProfilerTabStruct();
    let jsCpuProfilerStatisticsStruct = new JsCpuProfilerStatisticsStruct();

    it('JsCpuProfilerUIStructTest01', function () {
        jsCpuProfilerUIStruct = {
            name: '',
            depth: 0,
            selfTime: 0,
            totalTime: 0,
            url:'',
            line: 0,
            column: 0,
            scriptName: '',
            id: 0,
            parentId: 0,
        }
        expect(jsCpuProfilerUIStruct).not.toBeUndefined();
        expect(jsCpuProfilerUIStruct).toMatchInlineSnapshot(`
{
  "column": 0,
  "depth": 0,
  "id": 0,
  "line": 0,
  "name": "",
  "parentId": 0,
  "scriptName": "",
  "selfTime": 0,
  "totalTime": 0,
  "url": "",
}
`);
    });
    it('JsCpuProfilerChartFrameTest02', function () {
        jsCpuProfilerChartFrame = {
            name: '',
            depth: 0,
            selfTime: 0,
            totalTime: 0,
            url:'',
            line: 0,
            column: 0,
            scriptName: '',
            id: 0,
            parentId: 0,
            startTime: 0,
            endTime: 0,
            children: [],
            samplesIds: [],
            isSelect: false,
        }
        expect(jsCpuProfilerChartFrame).not.toBeUndefined();
        expect(jsCpuProfilerChartFrame).toMatchInlineSnapshot(`
{
  "children": [],
  "column": 0,
  "depth": 0,
  "endTime": 0,
  "id": 0,
  "isSelect": false,
  "line": 0,
  "name": "",
  "parentId": 0,
  "samplesIds": [],
  "scriptName": "",
  "selfTime": 0,
  "startTime": 0,
  "totalTime": 0,
  "url": "",
}
`);
    });
    it('JsCpuProfilerTabStructTest02', function () {
        jsCpuProfilerTabStruct = {
            name: '',
            depth: 0,
            selfTime: 0,
            totalTime: 0,
            url:'',
            line: 0,
            column: 0,
            scriptName: '',
            id: 0,
            parentId: 0,
            children:[],
            chartFrameChildren: [],
            isSelected: false,
            totalTimePercent:  '',
            selfTimePercent: '',
            symbolName: '',
            selfTimeStr: '',
            totalTimeStr:'',
            isSearch: false
        }
        expect(jsCpuProfilerTabStruct).not.toBeUndefined();
        expect(jsCpuProfilerTabStruct).toMatchInlineSnapshot(`
{
  "chartFrameChildren": [],
  "children": [],
  "column": 0,
  "depth": 0,
  "id": 0,
  "isSearch": false,
  "isSelected": false,
  "line": 0,
  "name": "",
  "parentId": 0,
  "scriptName": "",
  "selfTime": 0,
  "selfTimePercent": "",
  "selfTimeStr": "",
  "symbolName": "",
  "totalTime": 0,
  "totalTimePercent": "",
  "totalTimeStr": "",
  "url": "",
}
`);
    });
    it('JsCpuProfilerStatisticsStructTest02', function () {
        jsCpuProfilerStatisticsStruct = {
            type: '',
            time: 0,
            timeStr: '',
            percentage: ''
        }
        expect(jsCpuProfilerStatisticsStruct).not.toBeUndefined();
        expect(jsCpuProfilerStatisticsStruct).toMatchInlineSnapshot(`
{
  "percentage": "",
  "time": 0,
  "timeStr": "",
  "type": "",
}
`);
    });
});
