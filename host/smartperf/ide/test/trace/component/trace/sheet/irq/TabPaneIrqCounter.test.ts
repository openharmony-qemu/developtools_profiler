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
import { TabPaneIrqCounter } from '../../../../../../dist/trace/component/trace/sheet/irq/TabPaneIrqCounter.js';
// @ts-ignore
import { IrqStruct } from '../../../../../../dist/trace/database/ui-worker/ProcedureWorkerIrq.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneIrqCounter Test', () => {
  let tabPaneIrqCounter = new TabPaneIrqCounter();
  let map = new Map();
  map.set('irq', [new IrqStruct()]);
  let frameData = {
    leftNs: 253,
    rightNs: 1252,
    argSetId: 5,
    startNS: 11111,
    dur: 22222,
    name: 'irq',
    irqMapData: map,
    framesData: [
      {
        id: 25,
        ts: 254151,
        dur: 1202,
        name: '1583',
        argSetId: 5,
        type: '0',
        pid: 20,
        gpu_dur: 2568,
        app_dur: 110,
      },
    ],
  };

  it('TabPaneIrqCounterTest01', function () {
    tabPaneIrqCounter.data = frameData;
    expect(tabPaneIrqCounter.data).toBeUndefined();
  });

  it('TabPaneIrqCounterTest02', function () {
    expect(
      tabPaneIrqCounter.sortByColumn({
        key: 'jankType',
      })
    ).toBeUndefined();
  });
});
