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

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpFrameTimeChart Test', () => {
  let spSystemTrace = new SpSystemTrace();
  let spFrameTimeChart = new SpFrameTimeChart(spSystemTrace);

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

  it('TabPaneFramesTest01', function () {
    expect(spFrameTimeChart.init()).toBeTruthy();
  });
});
