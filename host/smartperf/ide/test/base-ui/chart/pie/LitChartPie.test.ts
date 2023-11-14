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
import { LitChartPie } from '../../../../dist/base-ui/chart/pie/LitChartPie.js';
// @ts-ignore
import { Utils } from '../../../../dist/trace/component/trace/base/Utils.js';
const LitChartPieData = require('../../../../dist/base-ui/chart/pie/LitChartPieData.js');
jest.mock('../../../../dist/base-ui/chart/pie/LitChartPieData.js');

const scrollHeight = 8000;
const clientHeight = 1000;
const clientWidth = 1000;

const fakeWindow = {
  scrollTop: 0,
};
beforeAll(() => {
  jest.spyOn(document.documentElement, 'scrollHeight', 'get').mockImplementation(() => scrollHeight);
  jest.spyOn(document.documentElement, 'clientHeight', 'get').mockImplementation(() => clientHeight);
  jest.spyOn(document.documentElement, 'clientWidth', 'get').mockImplementation(() => clientWidth);
  jest.spyOn(document.documentElement, 'scrollTop', 'get').mockImplementation(() => fakeWindow.scrollTop);
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('litChartPie Test', () => {
  it('litChartPieTest01', function () {
    let litChartPie = new LitChartPie();
    expect(litChartPie).not.toBeUndefined();
  });

  it('litChartPieTest03', function () {
    Utils.uuid = jest.fn(() => {
      return Math.floor(Math.random() * 10 + 1);
    });
    LitChartPieData.isPointIsCircle = jest.fn().mockResolvedValue(true);
    document.body.innerHTML = ` <div><lit-chart-pie id='chart-pie'></lit-chart-pie></div> `;
    let clo = document.getElementById('chart-pie') as LitChartPie;
    clo.config = {
      appendPadding: 20,
      data: [
        {
          cpu: 1,
          value: 345021,
          sum: 2111136,
          sumTimeStr: '233.99ms ',
          min: '32.12μs ',
          max: '3.47ms ',
          avg: '1.19ms ',
          count: 238,
          ratio: '33.46',
        },
        {
          cpu: 1,
          value: 1100000,
          sum: 111649487,
          sumTimeStr: '113.65ms ',
          min: '9.90μs ',
          max: '14.07ms ',
          avg: '697.24μs ',
          count: 113,
          ratio: '19.66',
        },
        {
          cpu: 1,
          value: 1411000,
          sum: 1005403,
          sumTimeStr: '100.75ms ',
          min: '32.81μs ',
          max: '25.12ms ',
          avg: '3.73ms ',
          count: 17,
          ratio: '17.43',
        },
        {
          cpu: 1,
          value: 884100,
          sum: 66958331,
          sumTimeStr: '66.96ms ',
          min: '16.82ms ',
          max: '27.30ms ',
          avg: '22.32ms ',
          count: 31,
          ratio: '11.58',
        },
        {
          cpu: 1,
          value: 960001,
          sum: 6223411,
          sumTimeStr: '62.21ms ',
          min: '93.23μs ',
          max: '20.34ms ',
          avg: '6.91ms ',
          count: 91,
          ratio: '10.76',
        },
        {
          cpu: 1,
          value: 1517001,
          sum: 2131,
          sumTimeStr: '21.87ms ',
          min: '9.90μs ',
          max: '8.28ms ',
          avg: '1.21ms ',
          count: 181,
          ratio: '3.78',
        },
        {
          cpu: 1,
          value: 1632,
          sum: 637321,
          sumTimeStr: '6.37ms ',
          min: '33.85μs ',
          max: '2.80ms ',
          avg: '531.08μs ',
          count: 121,
          ratio: '21.10',
        },
        {
          cpu: 1,
          value: 103201,
          sum: 13261,
          sumTimeStr: '31.14ms ',
          min: '25.00μs ',
          max: '1.12ms ',
          avg: '570.83μs ',
          count: 21,
          ratio: '0.20',
        },
        {
          cpu: 1,
          value: 12321,
          sum: 91661,
          sumTimeStr: '91.67μs ',
          min: '3.67μs ',
          max: '65.67μs ',
          avg: '32.67μs ',
          count: 11,
          ratio: '0.02',
        },
        {
          cpu: 1,
          value: 113201,
          sum: 76041,
          sumTimeStr: '76.04μs ',
          min: '23.04μs ',
          max: '56.04μs ',
          avg: '32.04μs ',
          count: 11,
          ratio: '0.01',
        },
      ],
      angleField: 'sum',
      colorField: 'value',
      radius: -10,
      label: {
        type: 'outer',
      },

      tip: (test: any) => {
        return `<div>
                                <div>frequency:${test.obj.value}</div> 
                                <div>min:${test.obj.min}</div>
                                <div>max:${test.obj.max}</div>
                                <div>average:${test.obj.avg}</div>
                                <div>duration:${test.obj.sumTimeStr}</div>
                                <div>ratio:${test.obj.ratio}%</div>
                            </div>
                                `;
      },
      angleClick: () => {},
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mouseout', <MouseEventInit>{ movementX: 1, movementY: 3 });
    clo.canvas.dispatchEvent(mouseOutEvent);
    expect(clo.config).not.toBeUndefined();
  });

  it('litChartPieTest04', function () {
    Utils.uuid = jest.fn(() => {
      return Math.floor(Math.random() * 10 + 1);
    });
    LitChartPieData.isPointIsCircle = jest.fn().mockResolvedValue(false);
    document.body.innerHTML = `
        <div>
            <lit-chart-pie id='chart-pie'></lit-chart-pie>
        </div> `;
    let clo = document.getElementById('chart-pie') as LitChartPie;
    clo.config = {
      appendPadding: 3,
      data: [
        {
          cpu: 4,
          value: 1325300,
          sum: 204992136,
          sumTimeStr: '204.99ms ',
          min: '12.92μs ',
          max: '38.37ms ',
          avg: '2.09ms ',
          count: 188,
          ratio: '35.46',
        },
        {
          cpu: 1,
          value: 1200000,
          sum: 112649487,
          sumTimeStr: '113.65ms ',
          min: '9.90μs ',
          max: '14.07ms ',
          avg: '697.24μs ',
          count: 123,
          ratio: '19.66',
        },
        {
          cpu: 1,
          value: 1421002,
          sum: 100750002,
          sumTimeStr: '100.75ms ',
          min: '32.81μs ',
          max: '25.12ms ',
          avg: '3.73ms ',
          count: 22,
          ratio: '17.43',
        },
        {
          cpu: 1,
          value: 884002,
          sum: 66958332,
          sumTimeStr: '66.96ms ',
          min: '16.82ms ',
          max: '27.30ms ',
          avg: '22.32ms ',
          count: 32,
          ratio: '11.58',
        },
        {
          cpu: 1,
          value: 960020,
          sum: 62210426,
          sumTimeStr: '62.21ms ',
          min: '93.23μs ',
          max: '20.34ms ',
          avg: '6.91ms ',
          count: 29,
          ratio: '10.76',
        },
        {
          cpu: 1,
          value: 1517020,
          sum: 21867712,
          sumTimeStr: '21.87ms ',
          min: '9.90μs ',
          max: '8.28ms ',
          avg: '1.21ms ',
          count: 28,
          ratio: '3.78',
        },
        {
          cpu: 1,
          value: 1602000,
          sum: 6372217,
          sumTimeStr: '6.37ms ',
          min: '33.85μs ',
          max: '2.80ms ',
          avg: '531.08μs ',
          count: 212,
          ratio: '1.10',
        },
        {
          cpu: 1,
          value: 1037002,
          sum: 1141627,
          sumTimeStr: '1.14ms ',
          min: '25.00μs ',
          max: '1.12ms ',
          avg: '570.83μs ',
          count: 22,
          ratio: '0.20',
        },
        {
          cpu: 1,
          value: 1229200,
          sum: 91662,
          sumTimeStr: '91.67μs ',
          min: '91.67μs ',
          max: '91.67μs ',
          avg: '91.67μs ',
          count: 21,
          ratio: '0.02',
        },
        {
          cpu: 1,
          value: 12100,
          sum: 7122,
          sumTimeStr: '76.04μs ',
          min: '71.04μs ',
          max: '79.04μs ',
          avg: '98.04μs ',
          count: 12,
          ratio: '0.01',
        },
      ],
      label: {
        type: 'outer',
      },
      angleField: 'sum',
      colorField: 'value',
      radius: 22,
      tip: (testObj: any) => {
        return `<div>
                                <div>frequency:${testObj.obj.value}</div> 
                                <div>min:${testObj.obj.min}</div>
                                <div>max:${testObj.obj.max}</div>
                                <div>average:${testObj.obj.avg}</div>
                                <div>duration:${testObj.obj.sumTimeStr}</div>
                                <div>ratio:${testObj.obj.ratio}%</div>
                            </div>
                                `;
      },
      interactions: [
        {
          type: 'active',
        },
      ],
      angleClick: () => {},
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mouseout', <MouseEventInit>{ movementX: 1, movementY: 4 });
    clo.canvas.dispatchEvent(mouseOutEvent);
    expect(clo.config).not.toBeUndefined();
  });

  it('litChartPieTest05', function () {
    Utils.uuid = jest.fn(() => {
      return Math.floor(Math.random() * 10 + 1);
    });
    LitChartPieData.isPointIsCircle = jest.fn().mockResolvedValue(true);
    document.body.innerHTML = `
        <div  width="100px" height="100px">
            <lit-chart-pie style='width:100px height:100px' width="100px" height="100px" id='chart-pie'></lit-chart-pie>
        </div> `;
    let clo = document.getElementById('chart-pie') as LitChartPie;
    jest.spyOn(clo, 'clientHeight', 'get').mockImplementation(() => clientHeight);
    jest.spyOn(clo, 'clientWidth', 'get').mockImplementation(() => clientWidth);
    clo.config = {
      appendPadding: 0,
      showChartLine: true,
      data: [
        {
          cpu: 1,
          value: 1335000,
          sum: 234991136,
          sumTimeStr: '204.99ms ',
          min: '22.92μs ',
          max: '28.37ms ',
          avg: '1.09ms ',
          count: 388,
          ratio: '35.46',
        },
        {
          cpu: 1,
          value: 1730000,
          sum: 113349487,
          sumTimeStr: '113.65ms ',
          min: '9.90μs ',
          max: '14.07ms ',
          avg: '697.24μs ',
          count: 133,
          ratio: '19.66',
        },
        {
          cpu: 1,
          value: 1421003,
          sum: 100750003,
          sumTimeStr: '100.75ms ',
          min: '32.81μs ',
          max: '25.12ms ',
          avg: '3.73ms ',
          count: 23,
          ratio: '17.43',
        },
        {
          cpu: 1,
          value: 884300,
          sum: 66958334,
          sumTimeStr: '66.96ms ',
          min: '16.82ms ',
          max: '27.30ms ',
          avg: '22.32ms ',
          count: 33,
          ratio: '11.58',
        },
        {
          cpu: 1,
          value: 960003,
          sum: 62213416,
          sumTimeStr: '62.21ms ',
          min: '93.23μs ',
          max: '20.34ms ',
          avg: '6.91ms ',
          count: 93,
          ratio: '10.76',
        },
        {
          cpu: 1,
          value: 1517300,
          sum: 214012,
          sumTimeStr: '21.87ms ',
          min: '9.90μs ',
          max: '8.28ms ',
          avg: '1.21ms ',
          count: 38,
          ratio: '3.78',
        },
        {
          cpu: 1,
          value: 1604003,
          sum: 6372917,
          sumTimeStr: '6.37ms ',
          min: '33.85μs ',
          max: '2.80ms ',
          avg: '531.08μs ',
          count: 13,
          ratio: '1.10',
        },
        {
          cpu: 1,
          value: 1037003,
          sum: 1141637,
          sumTimeStr: '1.14ms ',
          min: '25.00μs ',
          max: '1.12ms ',
          avg: '570.83μs ',
          count: 23,
          ratio: '0.20',
        },
        {
          cpu: 1,
          value: 1229300,
          sum: 91637,
          sumTimeStr: '91.67μs ',
          min: '91.67μs ',
          max: '91.67μs ',
          avg: '91.67μs ',
          count: 31,
          ratio: '0.02',
        },
        {
          cpu: 1,
          value: 1133300,
          sum: 76342,
          sumTimeStr: '76.04μs ',
          min: '78.04μs ',
          max: '76.04μs ',
          avg: '732.04μs ',
          count: 13,
          ratio: '0.01',
        },
      ],
      tip: (obj: any) => {
        return `<div>
                                <div>frequency:${obj.obj.value}</div> 
                                <div>min:${obj.obj.min}</div>
                                <div>max:${obj.obj.max}</div>
                                <div>average:${obj.obj.avg}</div>
                                <div>duration:${obj.obj.sumTimeStr}</div>
                                <div>ratio:${obj.obj.ratio}%</div>
                            </div>
                                `;
      },
      angleField: 'summary',
      colorField: 'value',
      radius: 32,
      label: {
        type: 'outer',
      },
      angleClick: () => {},
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ movementX: 1, movementY: 5 });
    clo.canvas.dispatchEvent(mouseOutEvent);
    expect(clo.config).not.toBeUndefined();
    clo.dataSource = [
      {
        cpu: 1,
        value: 1345000,
        sum: 244991136,
        sumTimeStr: '204.99ms ',
        min: '22.92μs ',
        max: '28.37ms ',
        avg: '1.09ms ',
        count: 488,
        ratio: '35.46',
      },
      {
        cpu: 1,
        value: 1740000,
        sum: 114649487,
        sumTimeStr: '113.65ms ',
        min: '9.90μs ',
        max: '14.07ms ',
        avg: '697.24μs ',
        count: 463,
        ratio: '19.66',
      },
    ];
    clo.centerX = 10;
    clo.centerY = 10;

    let mouseMoveEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ movementX: 1, movementY: 2 });
    clo.canvas.dispatchEvent(mouseMoveEvent);
  });
  it('litChartPieTest06', function () {
    let litChartPie = new LitChartPie();
    litChartPie.pieTipEL = jest.fn(() => true);
    litChartPie.pieTipEL.style = jest.fn(() => true);
    expect(litChartPie.showTip(1, 4, 'ab')).toBeUndefined();
  });
});
