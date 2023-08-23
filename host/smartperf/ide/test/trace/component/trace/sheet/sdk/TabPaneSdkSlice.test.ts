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
import { TabPaneSdkSlice } from '../../../../../../dist/trace/component/trace/sheet/sdk/TabPaneSdkSlice.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';

// @ts-ignore
import {TabUtil} from "../../../../../../dist/trace/component/trace/sheet/sdk/TabUtil.js";

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

describe('TabPaneSdkSlice Test', () => {
  let tabPaneSdkSlice = new TabPaneSdkSlice();
  document.body.innerHTML = `<lit-table id="tb-counter"></lit-table>`;
  let litTable = document.querySelector('#tb-counter') as LitTable;
  it('TabPaneSdkSliceTest00', () => {
    let mockStartTime = sqlite.queryStartTime;
    let startTime: Array<any> = [
      {
        start_ts: 1000,
      },
    ];
    mockStartTime.mockResolvedValue(startTime);
    let totalTime = sqlite.queryTotalTime;
    let totalData: Array<any> = [
      {
        recordStartNS: 1000,
        recordEndNS: 3000,
        total: 2000,
      },
    ];
    totalTime.mockResolvedValue(totalData);
    let mockSdkSliceData = sqlite.getTabSdkSliceData;
    let sliceData = [
      {
        start_ts: 1000,
        end_ts: 1500,
        type: 0,
        value: 100,
      },
      {
        start_ts: 2000,
        end_ts: 2500,
        type: 0,
        value: 100,
      },
      {
        start_ts: 3000,
        end_ts: 3500,
        type: 0,
        value: 100,
      },
      {
        start_ts: 4000,
        end_ts: 4500,
        type: 0,
        value: 100,
      },
      {
        start_ts: 5000,
        end_ts: 5500,
        type: 0,
        value: 100,
      },
    ];
    mockSdkSliceData.mockResolvedValue(sliceData);
    let slice = new TabPaneSdkSlice();
    slice.tblSdkSlice = jest.fn(() => litTable);
    slice.tblSdkSlice.appendChild = jest.fn(() => true);
    let map = new Map();
    let jsonCofigStr =
      '{"settingConfig":{"configuration":{"counters":{"enum":["ARM_Mali-TTRx_JS1_ACTIVE","ARM_Mali-TTRx_JS0_ACTIVE","ARM_Mali-TTRx_GPU_ACTIVE","ARM_Mali-TTRx_FRAG_ACTIVE"],\n' +
      '    "type":"string"},"stop_gator":{"default":"true","description":"stop_gator","type":"boolean"},"version":{"default":"1","description":"gatordversion","type":"number"}},"name":"mailG77"},\n' +
      '    "tableConfig":{"showType":[{"columns":[{"column":"ts","displayName":"TimeStamp","showType":[1,3],"type":"INTEGER"},{"column":"counter_id","displayName":"MonitorValue","showType":[1,3],"type":"INTEGER"},\n' +
      '    {"column":"value","displayName":"Value","showType":[1,3],"type":"INTEGER"}],"inner":{"columns":[{"column":"counter_name","displayName":"","showType":[0],"type":"STRING"},\n' +
      '    {"column":"counter_id","displayName":"","showType":[0],"type":"INTEGER"}],"tableName":"mock_plugin_counterobj_table"},"tableName":"mock_plugin_counter_table"},\n' +
      '    {"columns":[{"column":"start_ts","displayName":"startts","showType":[2,3],"type":"INTEGER"},{"column":"end_ts","displayName":"endts","showType":[2,3],"type":"INTEGER"},\n' +
      '    {"column":"slice_id","displayName":"slice_id","showType":[2,3],"type":"INTEGER"},{"column":"value","displayName":"Value","showType":[2,3],"type":"INTEGER"}],\n' +
      '    "inner":{"columns":[{"column":"slice_name","displayName":"","showType":[0],"type":"STRING"},{"column":"slice_id","displayName":"","showType":[0],"type":"INTEGER"}],\n' +
      '    "tableName":"mock_plugin_sliceobj_table"},"tableName":"mock_plugin_slice_table"}]}}';
    let datamap = {
      jsonConfig: jsonCofigStr,
      disPlayName: 'common_mock',
      pluginName: 'mock-plugin',
    };
    map.set('1', datamap);
    SpSystemTrace.SDK_CONFIG_MAP = map;
    let data = {
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
      statisticsSelectData: [],
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
      sdkSliceIds: ['a-b', 'b-c', 'd-e'],
    };
    slice.tblSdkSlice.recycleDataSource = jest.fn(() => data);
    slice.data = data;
    expect(slice.data).toBeUndefined();
  });

  it('TabPaneSdkSliceTest01', () => {
    expect(tabPaneSdkSlice.parseJson([])).toBe('');
  });

  it('TabPaneSdkSliceTest02', () => {
    let type = {
      columns: [{ showType: 'slice' }],
    };
    expect(TabUtil.getTableType(type)).toBe('');
  });

  it('TabPaneSdkSliceTest03', () => {
    expect(tabPaneSdkSlice.initDataElement()).toBeUndefined();
  });

  it('TabPaneSdkSliceTest04', function () {
    tabPaneSdkSlice.tblSdkSlice = jest.fn(() => true);
    tabPaneSdkSlice.tblSdkSlice!.recycleDataSource = jest.fn(() => true);
    expect(
      tabPaneSdkSlice.sortByColumn({
        key: '',
        sort: '',
      })
    ).toBeUndefined();
  });

  it('TabPaneSdkSliceTest05', () => {
    expect(tabPaneSdkSlice.initHtml()).toMatchInlineSnapshot(`
"
<style>
.sdk-slice-table{
    height: 20px;
    margin-bottom: 5px;
}
:host{
    padding: 10px 10px;
    flex-direction: column;
    display: flex;
}
</style>
<div class="sdk-slice-content" class="sdk-slice-table" style="display: flex;align-items: center;flex-direction: row;">
            <stack-bar id="sdk-slice-stack-bar" style="flex: 1"></stack-bar>
            <label id="sdk-slice-time-range"  style="width: auto;text-align: end;font-size: 10pt;">Selected range:0.0 ms</label>
        </div>
<lit-table id="tb-sdk-slice" class="sdk-slice-tbl" style="height: auto">
</lit-table>
        "
`);
  });
  it('TabPaneSdkSliceTest06', () => {
    expect(tabPaneSdkSlice.isDateIntersection(5,5,1,6)).toBeTruthy();
  });
  it('TabPaneSdkSliceTest07', () => {
    expect(tabPaneSdkSlice.isDateIntersection(5,5,1,6)).toBeTruthy();
  });
  it('TabPaneSdkSliceTest08', () => {
    expect(tabPaneSdkSlice.isDateIntersection(1,5,5,3)).toBeTruthy();
  });
});
