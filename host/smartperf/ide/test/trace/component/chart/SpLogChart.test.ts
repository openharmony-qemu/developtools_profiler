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
import { SpLogChart } from '../../../../dist/trace/component/chart/SpLogChart.js';

const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpLogChart Test', () => {
  let logChart = new SpLogChart(new SpChartManager());
  let queryLog = sqlite.queryLogData;
  let queryLogData = [
    {
      id: 2,
      startTs: 0,
      level: 'I',
      depth: 1,
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: update ts with 0 to 33727453411',
      time: 1502029274737385200,
      pid: 1119,
      tid: 1172,
      processName: 'hiprofiler_plug',
      dur: 1,
      frame: {
        x: 0,
        y: 7,
        width: 1,
        height: 7,
      },
    },
    {
      id: 37,
      startTs: 76402674,
      level: 'W',
      depth: 2,
      tag: 'C01300/AbilityManagerService',
      context: '[ability_manager_service.cpp(UpdateCallerInfo:6178)]UpdateCallerInfo caller abilityRecord is null.',
      time: 1502029274813788000,
      pid: 558,
      tid: 558,
      processName: 'foundation',
      dur: 1,
      frame: {
        x: 3,
        y: 14,
        width: 1,
        height: 7,
      },
    },
    {
      id: 99,
      startTs: 579581683,
      level: 'E',
      depth: 3,
      tag: 'C01300/AbilityManagerService',
      context: '[ability_interceptor.cpp(CheckCrowdtest:104)]GetApplicaionInfo from bms failed.',
      time: 1502029275316967000,
      pid: 558,
      tid: 558,
      processName: 'foundation',
      dur: 1,
      frame: {
        x: 29,
        y: 21,
        width: 1,
        height: 7,
      },
    },
  ];
  queryLog.mockResolvedValue(queryLogData);
  it('SpLogChart01', function () {
    expect(logChart.init()).toBeDefined();
  });
});
