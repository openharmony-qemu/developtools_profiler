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
import { TabPaneSlices } from '../../../../../../dist/trace/component/trace/sheet/process/TabPaneSlices.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneSlices Test', () => {
  let tabPaneSlices = new TabPaneSlices();

  it('TabPaneSlicesTest01', function () {
    expect(
      tabPaneSlices.sortByColumn({
        key: 'name',
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneSlicesTest05', function () {
    expect(
      tabPaneSlices.sortByColumn({
        key: !'name',
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneSlicesTest04', function () {
    expect(tabPaneSlices.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .slice-label{
            height: 20px;
        }
        :host{
            display: flex;
            padding: 10px 10px;
            flex-direction: column;
        }
        </style>
        <label id="time-range" class="slice-label" style="width: 100%;text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-slices" style="height: auto">
            <lit-table-column class="slices-column" title="Name" width="500px" data-index="name" key="name"  align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="slices-column" title="Wall duration(ms)" width="1fr" data-index="wallDuration" key="wallDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="slices-column" title="Avg Wall duration(ms)" width="1fr" data-index="avgDuration" key="avgDuration"  align="flex-start" order >
            </lit-table-column>
            <lit-table-column class="slices-column" title="Occurrences" width="1fr" data-index="occurrences" key="occurrences"  align="flex-start" order >
            </lit-table-column>
        </lit-table>
        "
`);
  });
});
