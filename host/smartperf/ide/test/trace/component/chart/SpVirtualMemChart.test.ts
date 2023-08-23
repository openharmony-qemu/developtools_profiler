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
import { SpVirtualMemChart } from '../../../../dist/trace/component/chart/SpVirtualMemChart.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { TraceRow } from '../../../../dist/trace/component/trace/base/TraceRow.js';

const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

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

describe('SpVirtualMemChart Test', () => {
  let spVirtualMemChart = new SpVirtualMemChart(new SpSystemTrace());
  let MockVirtualMemory = sqlit.queryVirtualMemory;
  MockVirtualMemory.mockResolvedValue([
    {
      id: 0,
      name: 'name',
    },
  ]);

  let MockVirtualMemoryData = sqlit.queryVirtualMemoryData;
  MockVirtualMemoryData.mockResolvedValue([
    {
      startTime: 0,
      value: 20,
      filterID: 0,
    },
  ]);

  it('SpVirtualMemChart01', function () {
    spVirtualMemChart.init();
    expect(spVirtualMemChart).toBeDefined();
  });

  it('SpVirtualMemChart02', function () {
    let folder = new TraceRow({
      canvasNumber: 1,
      alpha: false,
      contextId: '2d',
      isOffScreen: SpSystemTrace.isCanvasOffScreen,
    });
    spVirtualMemChart.initVirtualMemoryRow(folder, 2, 'name', 2);
    expect(spVirtualMemChart).toBeDefined();
  });
});
