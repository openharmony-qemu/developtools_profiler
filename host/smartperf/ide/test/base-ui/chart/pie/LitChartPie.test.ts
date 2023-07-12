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

  // it('litChartPieTest02', function () {
  //   document.body.innerHTML = `
  //       <div>
  //           <lit-chart-pie id='chart-pie'></lit-chart-pie>
  //       </div> `;
  //   let clo = document.getElementById('chart-pie') as LitChartPie;
  //   let mouseMoveEvent: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ movementX: 1, movementY: 2 });
  //   // this.cfg!.hoverHandler
  //   clo.canvas.dispatchEvent(mouseMoveEvent);
  // });

  it('litChartPieTest03', function () {
    Utils.uuid = jest.fn(() => {
      return Math.floor(Math.random() * 10 + 1);
    });
    LitChartPieData.isPointIsCircle = jest.fn().mockResolvedValue(true);
    document.body.innerHTML = `
        <div>
            <lit-chart-pie id='chart-pie'></lit-chart-pie>
        </div> `;
    let clo = document.getElementById('chart-pie') as LitChartPie;
    clo.config = {
      appendPadding: 0,
      data: [
        {
          cpu: 1,
          value: 1325000,
          sum: 204991136,
          sumTimeStr: '204.99ms ',
          min: '22.92μs ',
          max: '28.37ms ',
          avg: '1.09ms ',
          count: 188,
          ratio: '35.46',
        },
        {
          cpu: 1,
          value: 1700000,
          sum: 113649487,
          sumTimeStr: '113.65ms ',
          min: '9.90μs ',
          max: '14.07ms ',
          avg: '697.24μs ',
          count: 163,
          ratio: '19.66',
        },
        {
          cpu: 1,
          value: 1421000,
          sum: 100750003,
          sumTimeStr: '100.75ms ',
          min: '32.81μs ',
          max: '25.12ms ',
          avg: '3.73ms ',
          count: 27,
          ratio: '17.43',
        },
        {
          cpu: 1,
          value: 884000,
          sum: 66958334,
          sumTimeStr: '66.96ms ',
          min: '16.82ms ',
          max: '27.30ms ',
          avg: '22.32ms ',
          count: 3,
          ratio: '11.58',
        },
        {
          cpu: 1,
          value: 960000,
          sum: 62210416,
          sumTimeStr: '62.21ms ',
          min: '93.23μs ',
          max: '20.34ms ',
          avg: '6.91ms ',
          count: 9,
          ratio: '10.76',
        },
        {
          cpu: 1,
          value: 1517000,
          sum: 21867712,
          sumTimeStr: '21.87ms ',
          min: '9.90μs ',
          max: '8.28ms ',
          avg: '1.21ms ',
          count: 18,
          ratio: '3.78',
        },
        {
          cpu: 1,
          value: 1604000,
          sum: 6372917,
          sumTimeStr: '6.37ms ',
          min: '33.85μs ',
          max: '2.80ms ',
          avg: '531.08μs ',
          count: 12,
          ratio: '1.10',
        },
        {
          cpu: 1,
          value: 1037000,
          sum: 1141667,
          sumTimeStr: '1.14ms ',
          min: '25.00μs ',
          max: '1.12ms ',
          avg: '570.83μs ',
          count: 2,
          ratio: '0.20',
        },
        {
          cpu: 1,
          value: 1229000,
          sum: 91667,
          sumTimeStr: '91.67μs ',
          min: '91.67μs ',
          max: '91.67μs ',
          avg: '91.67μs ',
          count: 1,
          ratio: '0.02',
        },
        {
          cpu: 1,
          value: 1133000,
          sum: 76042,
          sumTimeStr: '76.04μs ',
          min: '76.04μs ',
          max: '76.04μs ',
          avg: '76.04μs ',
          count: 1,
          ratio: '0.01',
        },
      ],
      angleField: 'sum',
      colorField: 'value',
      radius: -10,
      label: {
        type: 'outer',
      },

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
      angleClick: () => {},
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mouseout', <MouseEventInit>{ movementX: 1, movementY: 2 });
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
      appendPadding: 0,
      data: [
        {
          cpu: 1,
          value: 1325000,
          sum: 204991136,
          sumTimeStr: '204.99ms ',
          min: '22.92μs ',
          max: '28.37ms ',
          avg: '1.09ms ',
          count: 188,
          ratio: '35.46',
        },
        {
          cpu: 1,
          value: 1700000,
          sum: 113649487,
          sumTimeStr: '113.65ms ',
          min: '9.90μs ',
          max: '14.07ms ',
          avg: '697.24μs ',
          count: 163,
          ratio: '19.66',
        },
        {
          cpu: 1,
          value: 1421000,
          sum: 100750003,
          sumTimeStr: '100.75ms ',
          min: '32.81μs ',
          max: '25.12ms ',
          avg: '3.73ms ',
          count: 27,
          ratio: '17.43',
        },
        {
          cpu: 1,
          value: 884000,
          sum: 66958334,
          sumTimeStr: '66.96ms ',
          min: '16.82ms ',
          max: '27.30ms ',
          avg: '22.32ms ',
          count: 3,
          ratio: '11.58',
        },
        {
          cpu: 1,
          value: 960000,
          sum: 62210416,
          sumTimeStr: '62.21ms ',
          min: '93.23μs ',
          max: '20.34ms ',
          avg: '6.91ms ',
          count: 9,
          ratio: '10.76',
        },
        {
          cpu: 1,
          value: 1517000,
          sum: 21867712,
          sumTimeStr: '21.87ms ',
          min: '9.90μs ',
          max: '8.28ms ',
          avg: '1.21ms ',
          count: 18,
          ratio: '3.78',
        },
        {
          cpu: 1,
          value: 1604000,
          sum: 6372917,
          sumTimeStr: '6.37ms ',
          min: '33.85μs ',
          max: '2.80ms ',
          avg: '531.08μs ',
          count: 12,
          ratio: '1.10',
        },
        {
          cpu: 1,
          value: 1037000,
          sum: 1141667,
          sumTimeStr: '1.14ms ',
          min: '25.00μs ',
          max: '1.12ms ',
          avg: '570.83μs ',
          count: 2,
          ratio: '0.20',
        },
        {
          cpu: 1,
          value: 1229000,
          sum: 91667,
          sumTimeStr: '91.67μs ',
          min: '91.67μs ',
          max: '91.67μs ',
          avg: '91.67μs ',
          count: 1,
          ratio: '0.02',
        },
        {
          cpu: 1,
          value: 1133000,
          sum: 76042,
          sumTimeStr: '76.04μs ',
          min: '76.04μs ',
          max: '76.04μs ',
          avg: '76.04μs ',
          count: 1,
          ratio: '0.01',
        },
      ],
      angleField: 'sum',
      colorField: 'value',
      radius: 1,
      label: {
        type: 'outer',
      },
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
      angleClick: () => {},
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mouseout', <MouseEventInit>{ movementX: 1, movementY: 2 });
    clo.canvas.dispatchEvent(mouseOutEvent);
    expect(clo.config).not.toBeUndefined();
    // clo.dataSource = [
    //     {pid:1, pName:"1", tid:1, tName:"11", total:12, size:"big core",timeStr:'11'},
    //     {pid:2, pName:"2", tid:2, tName: "222", total:13, size:"big core",timeStr:'22'}
    // ]
    // expect(clo.data[0].obj.pid).toBe(2)
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
          value: 1325000,
          sum: 204991136,
          sumTimeStr: '204.99ms ',
          min: '22.92μs ',
          max: '28.37ms ',
          avg: '1.09ms ',
          count: 188,
          ratio: '35.46',
        },
        {
          cpu: 1,
          value: 1700000,
          sum: 113649487,
          sumTimeStr: '113.65ms ',
          min: '9.90μs ',
          max: '14.07ms ',
          avg: '697.24μs ',
          count: 163,
          ratio: '19.66',
        },
        {
          cpu: 1,
          value: 1421000,
          sum: 100750003,
          sumTimeStr: '100.75ms ',
          min: '32.81μs ',
          max: '25.12ms ',
          avg: '3.73ms ',
          count: 27,
          ratio: '17.43',
        },
        {
          cpu: 1,
          value: 884000,
          sum: 66958334,
          sumTimeStr: '66.96ms ',
          min: '16.82ms ',
          max: '27.30ms ',
          avg: '22.32ms ',
          count: 3,
          ratio: '11.58',
        },
        {
          cpu: 1,
          value: 960000,
          sum: 62210416,
          sumTimeStr: '62.21ms ',
          min: '93.23μs ',
          max: '20.34ms ',
          avg: '6.91ms ',
          count: 9,
          ratio: '10.76',
        },
        {
          cpu: 1,
          value: 1517000,
          sum: 21867712,
          sumTimeStr: '21.87ms ',
          min: '9.90μs ',
          max: '8.28ms ',
          avg: '1.21ms ',
          count: 18,
          ratio: '3.78',
        },
        {
          cpu: 1,
          value: 1604000,
          sum: 6372917,
          sumTimeStr: '6.37ms ',
          min: '33.85μs ',
          max: '2.80ms ',
          avg: '531.08μs ',
          count: 12,
          ratio: '1.10',
        },
        {
          cpu: 1,
          value: 1037000,
          sum: 1141667,
          sumTimeStr: '1.14ms ',
          min: '25.00μs ',
          max: '1.12ms ',
          avg: '570.83μs ',
          count: 2,
          ratio: '0.20',
        },
        {
          cpu: 1,
          value: 1229000,
          sum: 91667,
          sumTimeStr: '91.67μs ',
          min: '91.67μs ',
          max: '91.67μs ',
          avg: '91.67μs ',
          count: 1,
          ratio: '0.02',
        },
        {
          cpu: 1,
          value: 1133000,
          sum: 76042,
          sumTimeStr: '76.04μs ',
          min: '76.04μs ',
          max: '76.04μs ',
          avg: '76.04μs ',
          count: 1,
          ratio: '0.01',
        },
      ],
      angleField: 'sum',
      colorField: 'value',
      radius: 1,
      label: {
        type: 'outer',
      },
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
      angleClick: () => {},
      interactions: [
        {
          type: 'element-active',
        },
      ],
    };
    let mouseOutEvent: MouseEvent = new MouseEvent('mousemove', <MouseEventInit>{ movementX: 1, movementY: 2 });
    clo.canvas.dispatchEvent(mouseOutEvent);
    expect(clo.config).not.toBeUndefined();
    clo.dataSource = [
      {
        cpu: 1,
        value: 1325000,
        sum: 204991136,
        sumTimeStr: '204.99ms ',
        min: '22.92μs ',
        max: '28.37ms ',
        avg: '1.09ms ',
        count: 188,
        ratio: '35.46',
      },
      {
        cpu: 1,
        value: 1700000,
        sum: 113649487,
        sumTimeStr: '113.65ms ',
        min: '9.90μs ',
        max: '14.07ms ',
        avg: '697.24μs ',
        count: 163,
        ratio: '19.66',
      },
    ];
    clo.centerX = 10;
    clo.centerY = 10;

    let mouseMoveEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ movementX: 1, movementY: 2 });
    clo.canvas.dispatchEvent(mouseMoveEvent);
  });
});
