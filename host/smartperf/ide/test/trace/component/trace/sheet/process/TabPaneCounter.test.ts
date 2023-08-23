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
import { TabPaneCounter } from '../../../../../../dist/trace/component/trace/sheet/process/TabPaneCounter.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
describe('TabPaneCounter Test', () => {
  let tabPaneCounter = new TabPaneCounter();
  it('TabPaneCounterTest01', function () {
    expect(
      tabPaneCounter.groupByTrackIdToMap([
        {
          id: 0,
          trackId: 0,
          name: '',
          value: 0,
          startTime: 0,
        },
      ])
    );
  });

  it('TabPaneCounterTest02', function () {
    expect(
      tabPaneCounter.createSelectCounterData(
        [
          {
            id: 0,
            trackId: 0,
            name: '',
            value: 0,
            startTime: 0,
          },
        ],
        0,
        1
      )
    );
  });

  it('TabPaneCounterTest03', function () {
    expect(
      tabPaneCounter.sortByColumn({
        key: 'name',
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneCounterTest06', function () {
    expect(
      tabPaneCounter.sortByColumn({
        key: 'number',
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneCounterTest04', function () {
    let mockgetTabCounters = sqlit.getTabCounters;
    mockgetTabCounters.mockResolvedValue(
      { trackId: 11, name: 'test', value: 111, startTime: 142445 },
      { trackId: 11, name: 'test', value: 222, startTime: 142446 }
    );
    let a = { rightNs: 1, trackIds: [11, 12, 13] };
    expect((tabPaneCounter.data = a)).toBeTruthy();
  });

  it('TabPaneCounterTest05', function () {
    let mockgetTabCounters = sqlit.getTabCounters;
    mockgetTabCounters.mockResolvedValue([]);
    let a = { rightNs: 1, trackIds: [11, 12, 13] };
    expect((tabPaneCounter.data = a)).toBeTruthy();
  });

  it('TabPaneCounterTest06', function () {
    expect(tabPaneCounter.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .counter-label{
            width: 100%;
        }
        :host{
            flex-direction: column;
            display: flex;
            padding: 10px 10px;
        }
        </style>
        <label id="time-range" class="counter-label" style="height: 20px;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-counter" style="height: auto">
            <lit-table-column width="25%" title="Name" data-index="name" key="name"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column width="1fr" title="Delta value" data-index="delta" key="delta"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Rate /s" data-index="rate" key="rate"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Weighted avg value" data-index="avgWeight" key="avgWeight"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Count" data-index="count" key="count"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="First value" data-index="first" key="first"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Last value" data-index="last" key="last"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Min value" data-index="min" key="min"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column width="1fr" title="Max value" data-index="max" key="max"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        "
`);
  });
});
