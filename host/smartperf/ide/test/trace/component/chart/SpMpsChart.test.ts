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
import { SmapsChart } from '../../../../dist/trace/component/chart/SmapsChart.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';

const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
describe('SpMpsChart Test', () => {
  let MockquerySmapsExits = sqlit.querySmapsExits;
  MockquerySmapsExits.mockResolvedValue([
    {
      event_name: 'trace_smaps',
      stat_type: 'received',
      count: 1,
    },
  ]);

  let MockquerySmapsDataMax = sqlit.querySmapsDataMax;
  MockquerySmapsDataMax.mockResolvedValue([
    {
      max_value: 11111,
    },
  ]);
  let trace = new SpChartManager();
  let spMapsChart = new SmapsChart(trace);
  it('SpMpsChart01', function () {
    spMapsChart.init();
    expect(SmapsChart).toBeInstanceOf(Function);
  });
});
