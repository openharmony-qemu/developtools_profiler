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
import { TabPaneCpuUsage } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneCpuUsage.js';
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneCpuUsage Test', () => {
  let tabPaneCpuUsage = new TabPaneCpuUsage();

  let mockGetTabCpuUsage = sqlit.getTabCpuUsage;
  let mockGetTabCpuFreq = sqlit.getTabCpuFreq;

  mockGetTabCpuUsage.mockResolvedValue([]);
  mockGetTabCpuFreq.mockResolvedValue([]);

  let selectionData = {
    cpus: [],
    threadIds: [],
    trackIds: [],
    funTids: [],
    heapIds: [],
    nativeMemory: [],
    cpuAbilityIds: [],
    memoryAbilityIds: [],
    diskAbilityIds: [],
    networkAbilityIds: [],
    leftNs: 0,
    rightNs: 0,
    hasFps: false,
    statisticsSelectData: undefined,
    perfSampleIds: [],
    perfCpus: [],
    perfProcess: [],
    perfThread: [],
    perfAll: false,
  };

  it('TabPaneCpuUsageTest01', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        0,
        true
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest08', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        0,
        false
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest09', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        2,
        true
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest10', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        2,
        false
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest02', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        1,
        true
      )
    ).toBeUndefined();
  });

  it('TabPaneCpuUsageTest03', function () {
    expect(
      tabPaneCpuUsage.sortTable(
        [
          [1, 2, 3, 9, 6, 4],
          [5, 2, 1, 4, 9, 6],
        ],
        1,
        false
      )
    ).toBeUndefined();
  });
  it('TabPaneCpuUsageTest04', function () {
    let result = tabPaneCpuUsage.sortFreq([
      {
        cpu: 0,
        value: 0,
        startNs: 0,
        dur: 0,
      },
      {
        cpu: 1,
        value: 2,
        startNs: 2,
        dur: 4,
      },
    ]);
    expect(result[0][0]).toBe(2);
  });
  it('TabPaneCpuUsageTest05', function () {
    expect(
      tabPaneCpuUsage.getFreqTop3(
        {
          cpu: 0,
          usage: 0,
          usageStr: 'usage',
          top1: 1,
          top2: 2,
          top3: 3,
          top1Percent: 11,
          top1PercentStr: 'Str1',
          top2Percent: 22,
          top2PercentStr: 'Str2',
          top3Percent: 33,
          top3PercentStr: 'Str3',
        },
        undefined,
        undefined,
        undefined,
        1
      )
    ).toBeUndefined();
  });
  it('TabPaneCpuUsageTest06', function () {
    let result = tabPaneCpuUsage.groupByCpuToMap([
      {
        cpu: 0,
        value: 0,
        startNs: 0,
        dur: 0,
      },
      {
        cpu: 1,
        value: 2,
        startNs: 2,
        dur: 4,
      },
    ]);
    expect(result.get(0).length).toBe(1);
  });

  it('TabPaneCurrentSelectionTest07', function () {
    expect(tabPaneCpuUsage.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .cpu-usage-label{
            width: 100%;
            height: 20px;
        }
        :host{
            padding: 10px 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <label id="time-range" class="cpu-usage-label" style="text-align: end;font-size: 10pt;margin-bottom: 5px">Selected range:0.0 ms</label>
        <lit-table id="tb-cpu-usage" style="height: auto">
            <lit-table-column class="cpu-usage-column" order width="1fr" title="CPU" data-index="cpu" key="cpu" align="flex-start">
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="Usage" data-index="usageStr" key="usageStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="CPU Freq Top1(M)" data-index="top1" key="top1" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="Top1 percent(%)" data-index="top1PercentStr" key="top1PercentStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="CPU Freq Top2(M)" data-index="top2" key="top2" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="Top2 percent(%)" data-index="top2PercentStr" key="top2PercentStr" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="CPU Freq Top3(M)" data-index="top3" key="top3" align="flex-start" >
            </lit-table-column>
            <lit-table-column class="cpu-usage-column" order width="1fr" title="Top3 percent(%)" data-index="top3PercentStr" key="top3PercentStr" align="flex-start" >
            </lit-table-column>
        </lit-table>
        "
`);
  });

  it('TabPaneCpuUsageTest11', function () {
    document.body.innerHTML = `<div id="CpuUsage"></div>`;
    let tabPaneCpuUsage = document.querySelector('#CpuUsage') as TabPaneCpuUsage;
    expect(tabPaneCpuUsage.sortFreq).toBe(undefined);
  });
});
