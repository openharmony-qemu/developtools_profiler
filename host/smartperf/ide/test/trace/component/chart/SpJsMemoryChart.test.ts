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

const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

// @ts-ignore
import { SpArkTsChart } from '../../../../dist/trace/component/chart/SpArkTsChart.js';
// @ts-ignore
import { SpIrqChart } from '../../../../dist/trace/component/chart/SpIrqChart.js';

jest.mock('../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

describe('SpIrqChart Test', () => {
  let spArkTsChart = new SpArkTsChart();
  let irqList = sqlite.queryIrqList;
  let irqListData = [
    {
      name: 'test',
      cpu: 0,
    },
  ];
  irqList.mockResolvedValue(irqListData);
  it('SpArkTsChart01', function () {
    expect(spArkTsChart).not.toBe({ initChart: [], loadJsDatabase: {}, trace: undefined });
  });
});
