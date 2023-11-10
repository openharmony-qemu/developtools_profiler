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
import { SpFrameTimeChart } from '../../../../dist/trace/component/chart/SpFrameTimeChart.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
// @ts-ignore
import { FlagsConfig } from '../../../../dist/trace/component/SpFlags.js';

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpFrameTimeChart Test', () => {
  let manager = new SpChartManager();
  let spFrameTimeChart = new SpFrameTimeChart(manager);

  let queryFrameTime = sqlite.queryFrameTimeData;
  let queryFrameTimeData = [
    {
      pid: 256,
    },
  ];
  queryFrameTime.mockResolvedValue(queryFrameTimeData);

  let queryExpectedFrame = sqlite.queryExpectedFrameDate;
  let queryExpectedFrameDate = [
    {
      dur: 2585,
      depth: 1,
    },
    {
      dur: 6688,
      depth: 1,
    },
  ];
  queryExpectedFrame.mockResolvedValue(queryExpectedFrameDate);

  let queryActualFrame = sqlite.queryActualFrameDate;
  let queryActualFrameDate = [
    {
      dur: 6878,
      depth: 1,
    },
    {
      dur: 6238,
      depth: 1,
    },
  ];
  queryActualFrame.mockResolvedValue(queryActualFrameDate);

  let frameApp = sqlite.queryFrameApp;
  let frameAppData = [
    {
      appName: 'test0',
    },
    {
      appName: 'test1',
    },
    {
      appName: 'test2',
    },
  ];
  frameApp.mockResolvedValue(frameAppData);

  let frameAnimation = sqlite.queryFrameAnimationData;
  let frameAnimationData = [
    { animationId: 1, dynamicEndTs: 4774481414, dynamicStartTs: 4091445476, ts: 4091445476 },
    {
      animationId: 2,
      dynamicEndTs: 8325095997,
      dynamicStartTs: 7652588184,
      ts: 7652588184,
    },
  ];
  frameAnimation.mockResolvedValue(frameAnimationData);

  let frameDynamic = sqlite.queryFrameDynamicData;
  let frameDynamicData = [
    { alpha: '1.00', appName: 'test0', height: 2772, id: 74, ts: 28565790, width: 1344, x: 0, y: 0 },
    {
      alpha: '1.00',
      appName: 'test0',
      height: 2772,
      id: 75,
      ts: 42341310,
      width: 1344,
      x: 0,
      y: 0,
    },
  ];
  frameDynamic.mockResolvedValue(frameDynamicData);

  let frameSpacing = sqlite.queryFrameSpacing;
  let frameSpacingData = [
    {
      currentFrameHeight: 2768,
      currentFrameWidth: 1344,
      currentTs: 17535295995,
      frameSpacingResult: 0.1,
      id: 1216,
      nameId: 'test0',
      preFrameHeight: 2767,
      preFrameWidth: 1343,
      preTs: 17523356412,
      x: 0,
      y: 1,
    },
    {
      currentFrameHeight: 2768,
      currentFrameWidth: 1344,
      currentTs: 17546478287,
      frameSpacingResult: 0,
      id: 1217,
      nameId: 'test0',
      preFrameHeight: 2768,
      preFrameWidth: 1344,
      preTs: 17535295995,
      x: 0,
      y: 1,
    },
  ];
  frameSpacing.mockResolvedValue(frameSpacingData);

  let physical = sqlite.queryPhysicalData;
  let physicalData = [{ physicalFrameRate: 90, physicalHeight: 2772, physicalWidth: 1344 }];
  physical.mockResolvedValue(physicalData);

  it('TabPaneFramesTest01', function () {
    expect(spFrameTimeChart.init()).toBeTruthy();
  });

  it('TabPaneFramesTest02', function () {
    FlagsConfig.updateFlagsConfig('AnimationAnalysis', 'Enabled');
    spFrameTimeChart.initAnimatedScenesChart(
      TraceRow.skeleton(),
      {
        pid: 1,
        processName: 'render_service',
      },
      TraceRow.skeleton()
    );
    expect(spFrameTimeChart.flagConfig?.AnimationAnalysis).toEqual('Enabled');
  });
  it('TabPaneFramesTest03', function () {
    expect(spFrameTimeChart.frameNoExpandTimeOut()).toBeTruthy();
  });
  it('TabPaneFramesTest04', function () {
    expect(spFrameTimeChart.frameExpandTimeOut()).toBeTruthy();
  });
  it('TabPaneFramesTest05', function () {
    let frameData = [{
      currentTs:23,
      currentFrameWidth:9,
      currentFrameHeight:5,
      x:2,
      y:78,
    },
      {
        currentTs:12,
        currentFrameWidth:9,
        currentFrameHeight:5,
        x:21,
        y:78,
      },
    ];
    let deviceStruct = [{
      physicalWidth:40,
      physicalHeight:41,
    }];
    spFrameTimeChart.flagConfig = jest.fn(()=>true);
    spFrameTimeChart.flagConfig.physicalWidth = jest.fn(()=>true);
    expect(spFrameTimeChart.dataProcessing(frameData,deviceStruct)).toBeUndefined();
  });
});
