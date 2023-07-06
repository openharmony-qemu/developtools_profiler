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
import { TabPaneCpuByThread } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneCpuByThread.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

describe('TabPaneCpuByThread Test', () => {
  let tabPaneCpuByThread = new TabPaneCpuByThread();

  it('TabPaneCpuByThreadTest01', function () {
    expect(
      tabPaneCpuByThread.sortByColumn({
        key: 'number',
        sort: () => {},
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByThreadTest02', function () {
    expect(
      tabPaneCpuByThread.sortByColumn({
        key: 'pid' || 'wallDuration' || 'avgDuration' || 'occurrences',
      })
    ).toBeUndefined();
  });

  it('TabPaneCpuByThreadTest03', function () {
    let mockgetTabCpuByThread = sqlit.getTabCpuByThread;
    mockgetTabCpuByThread.mockResolvedValue([
      { process: 'test', wallDuration: 10, occurrences: 10, thread: '' },
      { process: 'test2', wallDuration: 11, occurrences: 11, thread: '' },
    ]);
    let a = { rightNs: 1, cpus: [11, 12, 13] };
    expect((tabPaneCpuByThread.data = a)).toBeTruthy();
  });

  it('TabPaneCpuByThreadTest04', function () {
    let mockgetTabCpuByThread = sqlit.getTabCpuByThread;
    mockgetTabCpuByThread.mockResolvedValue([]);
    let a = { rightNs: 1, cpus: [11, 12, 13] };
    expect((tabPaneCpuByThread.data = a)).toBeTruthy();
  });

  it('TabPaneCpuByThreadTest05', function () {
    expect(tabPaneCpuByThread.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .cpu-by-thread-label{
            width: 100%;
            height: 20px;
        }
        :host{
            width: auto;
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <label id="time-range" class="cpu-by-thread-label" style="text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-cpu-thread" style="height:calc( 30vh - 25px )" >
            
        </lit-table>
        "
`);
  });
});
