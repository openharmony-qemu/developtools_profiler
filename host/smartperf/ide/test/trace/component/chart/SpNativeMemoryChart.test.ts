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
describe('SpNativeMemoryChart Test', () => {
  let spNativeMemoryChart = new SpNativeMemoryChart(new SpSystemTrace());

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
});
