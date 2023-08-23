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
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
// @ts-ignore
import { SpFreqChart } from '../../../../dist/trace/component/chart/SpFreqChart.js';

const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('spFpsChart Test', () => {
  let spFpsChart = new SpFreqChart(new SpChartManager());

  let mockGetCpuLimitFreq = sqlit.getCpuLimitFreq;
  mockGetCpuLimitFreq.mockResolvedValue([
    {
      startNs: 1000,
      max: 100,
      min: 20,
      cpu: 0,
    },
    {
      startNs: 2000,
      max: 300,
      min: 100,
      cpu: 1,
    },
  ]);

  let mockCpuLimitFreqId = sqlit.getCpuLimitFreqId;
  mockCpuLimitFreqId.mockResolvedValue([
    {
      cpu: 0,
      maxFilterId: 2,
      minFilterId: 1,
    },
    {
      cpu: 1,
      maxFilterId: 2,
      minFilterId: 1,
    },
  ]);

  let mockCpuFreqData = sqlit.queryCpuFreqData;
  mockCpuFreqData.mockResolvedValue([
    {
      cpu: 0,
      value: 100,
      startNS: 2000,
    },
    {
      cpu: 1,
      value: 100,
      startNS: 3000,
    },
  ]);

  let mockCpuState = sqlit.queryCpuState;
  mockCpuState.mockResolvedValue([
    {
      startTs: 1000,
      value: 100,
    },
    {
      startTs: 2000,
      value: 10,
    },
  ]);

  let queryCpuFreqMock = sqlit.queryCpuFreq;
  queryCpuFreqMock.mockResolvedValue([
    {
      cpu: 0,
      filterId: 1,
    },
    {
      cpu: 1,
      filterId: 2,
    },
  ]);

  let queryCpuStateFilter = sqlit.queryCpuStateFilter;
  queryCpuStateFilter.mockResolvedValue([
    {
      cpu: 0,
      filterId: 1,
    },
    {
      cpu: 1,
      filterId: 2,
    },
  ]);

  let queryCpuMaxFreqMock = sqlit.queryCpuMaxFreq;
  queryCpuMaxFreqMock.mockResolvedValue([{ maxFreq: 100 }]);

  let MockgetCpuLimitFreqId = sqlit.getCpuLimitFreqId;
  MockgetCpuLimitFreqId.mockResolvedValue([{ cpu: 1, maxFilterId: 9, minFilterId: 1 }]);

  let MockgetCpuLimitFreqMax = sqlit.getCpuLimitFreqMax;
  MockgetCpuLimitFreqMax.mockResolvedValue([{ maxValue: 100, filterId: 9 }]);

  it('spFpsChart01', function () {
    expect(spFpsChart.init()).toBeDefined();
  });
});
