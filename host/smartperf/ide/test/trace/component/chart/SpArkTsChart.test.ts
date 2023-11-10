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
import { SpArkTsChart } from '../../../../dist/trace/component/chart/SpArkTsChart.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';

const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

describe('SpClockChart Test', () => {
  let arkTsChart = new SpArkTsChart(new SpChartManager());

  let jsCpuProfilerConfig = sqlite.queryJsCpuProfilerConfig;
  let cpuProfilerConfigData = [
    {
      enableCpuProfiler: 1,
      pid: 1553,
      type: -1,
    },
  ];
  jsCpuProfilerConfig.mockResolvedValue(cpuProfilerConfigData);

  let jsCpuProfiler = sqlite.queryJsCpuProfilerData;
  let cpuProfilerData = [
    {
      1: 1,
    },
  ];
  jsCpuProfiler.mockResolvedValue(cpuProfilerData);

  let jsMemory = sqlite.queryJsMemoryData;
  let jsMemoryData = [{}];
  jsMemory.mockResolvedValue(jsMemoryData);

  it('SpClockChart01', function () {
    expect(arkTsChart.initFolder()).not.toBeUndefined();
  });
  it('SpClockChart02', function () {
    expect(arkTsChart.initTimelineChart()).not.toBeUndefined();
  });
  it('SpClockChart03', function () {
    expect(arkTsChart.initSnapshotChart()).not.toBeUndefined();
  });
});
