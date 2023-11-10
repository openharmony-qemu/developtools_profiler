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
import { SpSystemTrace } from '../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { SpNativeMemoryChart } from '../../../../dist/trace/component/chart/SpNativeMemoryChart.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);
const intersectionObserverMock = () => ({
  observe: () => null,
});
// @ts-ignore
window.ResizeObserver = window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    unobserve: jest.fn(),
    observe: jest.fn(),
  }));
describe('SpNativeMemoryChart Test', () => {
  let chartManager = new SpChartManager();
  let spNativeMemoryChart = new SpNativeMemoryChart(chartManager);

  let queryNativeHookStatisticsCount = sqlit.queryNativeHookStatisticsCount;
  queryNativeHookStatisticsCount.mockResolvedValue([
    {
      num: 2,
    },
  ]);

  let queryNativeMemoryRealTime = sqlit.queryNativeMemoryRealTime;
  queryNativeMemoryRealTime.mockResolvedValue([
    {
      ts: 1502013097360370200,
      clock_name: 'realtime',
    },
  ]);

  let queryBootTime = sqlit.queryBootTime;
  queryBootTime.mockResolvedValue([
    {
      ts: -557295431,
      clock_name: 'boottime',
    },
  ]);

  let nativeHookProcess = sqlit.queryNativeHookProcess;
  nativeHookProcess.mockResolvedValue([
    {
      ipid: 0,
      pid: 0,
      name: 'name',
    },
  ]);

  let heapGroupByEvent = sqlit.queryHeapGroupByEvent;
  heapGroupByEvent.mockResolvedValue([
    {
      eventType: 'AllocEvent',
      sumHeapSize: 10,
    },
  ]);

  it('SpNativeMemoryChart01', function () {
    expect(spNativeMemoryChart.initChart()).toBeDefined();
  });
  it('SpNativeMemoryChart02', function () {
    expect(spNativeMemoryChart.getNativeMemoryStatisticByChartType()).toBeDefined();
  });
  it('SpNativeMemoryChart03', function () {
    expect(spNativeMemoryChart.getNativeMemoryDataByChartType()).toBeDefined();
  });
});
