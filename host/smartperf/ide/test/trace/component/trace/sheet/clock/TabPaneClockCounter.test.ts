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
import { TabPaneClockCounter } from '../../../../../../dist/trace/component/trace/sheet/clock/TabPaneClockCounter.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneClockCounter Test', () => {
  let clockCounter = new TabPaneClockCounter();
  let map = new Map();
  map.set('clock', [
    {
      filterId: 255,
      value: 1252,
      startNS: 4515,
      dur: 5255,
      delta: 415,
    },
  ]);
  let clockCounterData = {
    leftNs: 253,
    rightNs: 1252,
    clockMapData: map,
  };

  it('TabPaneClockCounterTest01', function () {
    clockCounter.data = clockCounterData;
    expect(clockCounter.data).toBeUndefined();
  });

  it('TabPaneClockCounterTest02', function () {
    expect(
      clockCounter.sortByColumn({
        key: 'number',
      })
    ).toBeUndefined();
  });
});
