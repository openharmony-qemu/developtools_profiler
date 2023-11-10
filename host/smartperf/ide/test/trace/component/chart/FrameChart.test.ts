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
import { FrameChart, Module } from '../../../../dist/trace/component/chart/FrameChart.js';
// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { ChartMode, ChartStruct } from '../../../../dist/trace/bean/FrameChartStruct.js';

jest.mock('../../../../dist/trace/component/SpSystemTrace.js', () => {
  return {};
});
jest.mock('../../../../dist/js-heap/model/DatabaseStruct.js', () => {});

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

jest.mock('../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('FrameChart Test', () => {
  let node = [{ children: '' }, { children: { length: 0 } }];
  let node1 = [{ children: '' }, { children: { length: 10 } }];
  let selectData = [(length = 1)];
  document.body.innerHTML = '<sp-application><tab-framechart id="ccc"></tab-framechart></sp-application>';
  let frameChart = new FrameChart();
  frameChart.data = [{
    isDraw : false,
    depth:  0,
    symbol:  '',
    lib: '',
    size:  0,
    count: 0,
    dur:  0,
    searchSize:  0,
    searchCount: 0,
    searchDur: 0,
    drawSize:  0,
    drawCount: 0,
    drawDur:  0,
    parent:  undefined,
    children:  [],
    percent:  0,
    addr: '',
    isSearch: false,
    isChartSelect: false,
    isChartSelectParent: false
  }]
  it('FrameChartTest01', function () {
    frameChart.tabPaneScrollTop = false;
    expect(frameChart.tabPaneScrollTop).toBeFalsy();
  });

  it('FrameChartTest02', function () {
    frameChart.createRootNode();
    let index = frameChart.scale(2);
    expect(index).toBe(undefined);
  });

  it('FrameChartTest03', function () {
    frameChart.translationDraw = jest.fn(() => true);
    expect(frameChart.translation()).toBeUndefined();
  });

  it('FrameChartTest04', function () {
    frameChart.translationDraw = jest.fn(() => true);
    expect(frameChart.translation(-1)).toBeUndefined();
  });

  it('FrameChartTest05', function () {
    frameChart.selectTotalCount = false;
    expect(frameChart.selectTotalCount).toBeFalsy();
  });

  it('FrameChartTest06', function () {
    frameChart._mode = 1;
    frameChart.drawScale = jest.fn(() => true);
    expect(frameChart.calculateChartData()).not.toBeUndefined();
  });

  it('FrameChartTest07', function () {
    expect(frameChart.updateCanvas()).toBeUndefined();
  });

  it('FrameChartTest08', function () {
    frameChart.translationDraw = jest.fn(() => true);
    frameChart.lastCanvasXInScale = 0;
    expect(frameChart.translationByScale()).toBe(undefined);
  });

  it('FrameChartTest09', function () {
    frameChart.translationDraw = jest.fn(() => true);
    frameChart.canvasX = 4;
    frameChart.lastCanvasXInScale = 1;
    expect(frameChart.translationByScale()).toBe(undefined);
  });

  it('FrameChartTest10', function () {
    frameChart.translationDraw = jest.fn(() => true);
    expect(frameChart.translationByScale(1)).toBe(undefined);
  });

  it('FrameChartTest11', function () {
    frameChart.calculateChartData = jest.fn(() => true);
    frameChart.xPoint = 1;
    frameChart.createRootNode();
    expect(frameChart.translationDraw()).toBeTruthy();
  });

  it('FrameChartTest12', function () {
    expect(frameChart.onMouseClick({ button: 0 })).toBeUndefined();
  });

  it('FrameChartTest13', function () {
    expect(frameChart.drawFrameChart(node)).toBeUndefined();
  });


  it('FrameChartTest14', function () {
    expect(frameChart.onMouseClick({ button: 2 })).toBeUndefined();
  });

  it('FrameChartTest15 ', function () {
    expect(frameChart.mode).toBeUndefined();
  });

  it('FrameChartTest16', function () {
    frameChart.mode = false;
    expect(frameChart.mode).toBeFalsy();
  });

  it('FrameChartTest17', function () {
    frameChart.caldrawArgs = jest.fn(() => true);
    expect(frameChart.caldrawArgs()).toBeTruthy();
  });

  it('FrameChartTest18', function () {
    expect(frameChart.data).toBeFalsy();
  });

  it('FrameChartTest19', function () {
    expect(frameChart.addChartClickListener(() => {})).toBeUndefined();
  });

  it('FrameChartTest20', function () {
    expect(frameChart.removeChartClickListener(() => {})).toBeUndefined();
  });

  it('FrameChartTest21', function () {
    frameChart._mode = 1;
    frameChart.drawScale = jest.fn(() => true);
    expect(frameChart.drawScale()).toBeTruthy();
  });

  it('FrameChartTest22', function () {
    frameChart._mode = 2;
    frameChart.drawScale = jest.fn(() => true);
    expect(frameChart.drawScale()).toBeTruthy();
  });

  it('FrameChartTest23', function () {
    frameChart._mode = 3;
    frameChart.drawScale = jest.fn(() => true);
    expect(frameChart.drawScale()).toBeTruthy();
  });

  it('FrameChartTest24', function () {
    expect(frameChart.resetTrans()).toBeUndefined();
  });

  it('FrameChartTest25', function () {
    expect(frameChart.onMouseClick({ button: 2 })).toBeUndefined();
  });

  it('FrameChartTest26', function () {
    frameChart._mode = ChartMode.Byte;
    frameChart.drawScale = jest.fn(() => true);
    frameChart.currentData = [
      {
        drawSize: 10,
        size: 20,
        frame: {
          x: 10,
          y: 40,
          width: 9,
          height: 3,
        },
      },
    ];
    expect(frameChart.calculateChartData()).not.toBeUndefined();
  });
  it('FrameChartTest27', function () {
    frameChart._mode = ChartMode.Count;
    frameChart.drawScale = jest.fn(() => true);
    frameChart.currentData = [
      {
        drawSize: 23,
        size: 12,
        frame: {
          x: 29,
          y: 40,
          width: 56,
          height: 3,
        },
      },
    ];
    expect(frameChart.calculateChartData()).not.toBeUndefined();
  });
  it('FrameChartTest28', function () {
    frameChart._mode = ChartMode.Duration;
    frameChart.drawScale = jest.fn(() => true);
    frameChart.currentData = [
      {
        drawSize: 78,
        size: 12,
        frame: {
          x: 29,
          y: 50,
          width: 56,
          height: 12,
        },
      },
    ];
    expect(frameChart.calculateChartData()).not.toBeUndefined();
  });
  it('FrameChartTest29 ', function () {
    let node = [
      {
        parent: [
          {
            drawCount: 23,
            drawDur: 12,
            drawSize: 45,
          },
        ],
      },
    ];
    let module = [{
      drawCount: 0,
      drawDur: 78,
      drawSize: 9,
    }]
    expect(frameChart.setParentDisplayInfo(node, module)).toBeUndefined();
  });
});
