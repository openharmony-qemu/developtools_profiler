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
import { TabPaneCpuByProcess } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneCpuByProcess.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
describe('TabPaneCpuByProcess Test', () => {
  let tabPaneCpuByProcess = new TabPaneCpuByProcess();

  it('TabPaneCpuByProcessTest01', function () {
    expect(
      tabPaneCpuByProcess.sortByColumn({
        key: 'number',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByProcessTest05', function () {
    expect(
      tabPaneCpuByProcess.sortByColumn({
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByProcessTest04', function () {
    expect(
      tabPaneCpuByProcess.sortByColumn({
        key: 'pid' || 'wallDuration' || 'avgDuration' || 'occurrences',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByProcessTest04', function () {
    expect(tabPaneCpuByProcess.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .cpu-process-label{
            font-size: 10pt;
            margin-bottom: 5px;
            text-align: end;
        }
        :host{
            border: 0;
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="cpu-process-time-range" class="cpu-process-label" style="width: 100%;height: 20px;">Selected range:0.0 ms</label>
        <lit-table id="tb-cpu-process" style="height: auto">
            <lit-table-column class="cpu-process-column" order width="30%" title="Process" data-index="process" key="process" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="PID" data-index="pid" key="pid" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="Wall duration(ms)" data-index="wallDuration" key="wallDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="Avg Wall duration(ms)" data-index="avgDuration" key="avgDuration" align="flex-start" order>
            </lit-table-column>
            <lit-table-column class="cpu-process-column" order width="1fr" title="Occurrences" data-index="occurrences" key="occurrences" align="flex-start" order>
            </lit-table-column>
        </lit-table>
        "
`);
  });
});
