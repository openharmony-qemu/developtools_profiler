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
import { TabPanePowerDetails } from '../../../../../../dist/trace/component/trace/sheet/energy/TabPanePowerDetails.js';
import '../../../../../../dist/trace/component/trace/sheet/energy/TabPanePowerDetails.js';

// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

describe('TabPanePowerDetails Test', () => {
  document.body.innerHTML = `<lit-table id="tb-power-details-energy"></lit-table>`;
  let litTable = document.querySelector('#tb-power-details-energy') as LitTable;
  it('TabPanePowerDetailsTest01', function () {
    let tabPanePowerDetails = new TabPanePowerDetails();
    tabPanePowerDetails.tblPowerDetails = jest.fn(() => litTable);
    let MockPowerDetailsData = sqlit.getTabPowerDetailsData;
    let detail = [
      {
        ts: 5999127353,
        eventName: 'POWER_IDE_AUDIO',
        appKey: 'APPNAME',
        eventValue: 'com.example.himusicdemo,com.example.himusicdemo_js,com.example.himusicdemo_app',
      },
      {
        ts: 5999127353,
        eventName: 'POWER_IDE_AUDIO',
        appKey: 'BACKGROUND_DURATION',
        eventValue: '524,854,612',
      },
      {
        ts: 5999127353,
        eventName: 'POWER_IDE_BLUETOOTH',
        appKey: 'APPNAME',
        eventValue: 'com.ohos.settings,bt_switch,bt_switch_js,bt_switch_app',
      },
      {
        ts: 5999127353,
        eventName: 'POWER_IDE_BLUETOOTH',
        appKey: 'BACKGROUND_DURATION',
        eventValue: '325,124,51,52',
      },
      {
        ts: 5999127353,
        eventName: 'POWER_IDE_CAMERA',
        appKey: 'APPNAME',
        eventValue: 'com.ohos.camera,com.ohos.camera_app,com.ohos.camera_js,com.ohos.camera_ts',
      },
      {
        ts: 5999127353,
        eventName: 'POWER_IDE_CAMERA',
        appKey: 'BACKGROUND_DURATION',
        eventValue: '356,325,854,365',
      },
    ];
    MockPowerDetailsData.mockResolvedValue(detail);
    let list = {
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
      rightNs: 1000,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
      systemEnergy: [0, 1, 2],
      powerEnergy: [0, 1, 2],
      anomalyEnergy: [0, 1, 2],
    };
    tabPanePowerDetails.tblPowerDetails.recycleDataSource = jest.fn(() => list);
    tabPanePowerDetails.data = list;
    expect(tabPanePowerDetails.data).toBeUndefined();
  });

  it('TabPanePowerDetailsTest02', function () {
    let tabPanePowerDetails = new TabPanePowerDetails();
    expect(tabPanePowerDetails.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .power-details-table{
            height: auto;
        }
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        </style>
        <lit-table id="tb-power-details-energy" class="power-details-table">
            <lit-table-column order width="100px" title="" data-index="event" key="event" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="60px" title="UID" data-index="uid" key="uid" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="80px" title="Charge" data-index="charge" key="charge" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="200px" title="Foreground Duration(ms)" data-index="foreground_duration" key="foreground_duration" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="200px" title="Foreground Energy(mAs)" data-index="foreground_energy" key="foreground_energy" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="200px" title="Background Duration(ms)" data-index="background_duration" key="background_duration" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="200px" title="Background Energy(mAs)" data-index="background_energy" key="background_energy" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="190px" title="Screen On Duration(ms)" data-index="screen_on_duration" key="screen_on_duration" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="180px" title="Screen On Energy(mAs)" data-index="screen_on_energy" key="screen_on_energy" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="190px" title="Screen Off Duration(ms)" data-index="screen_off_duration" key="screen_off_duration" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="190px" title="Screen Off Energy(mAs)" data-index="screen_off_energy" key="screen_off_energy" align="flex-start" >
            </lit-table-column>
             <lit-table-column order width="150px" title="Foreground Count" data-index="foreground_count" key="foreground_count" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="150px" title="Background Count" data-index="background_count" key="background_count" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="150px" title="Screen On Count" data-index="screen_on_count" key="screen_on_count" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="150px" title="Screen Off Count" data-index="screen_off_count" key="screen_off_count" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="170px" title="Background Time(ms)" data-index="background_time" key="background_time" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="160px" title="Screen On Time(ms)" data-index="screen_on_time" key="screen_on_time" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="160px" title="Screen Off Time(ms)" data-index="screen_off_time" key="screen_off_time" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="110px" title="Energy(mAs)" data-index="energy" key="energy" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="80px" title="Load(%)" data-index="load" key="load" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="100px" title="Usage(ms)" data-index="usage" key="usage" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="120px" title="Duration(ms)" data-index="duration" key="duration" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="100px" title="Camera Id" data-index="camera_id" key="camera_id" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="80px" title="Count" data-index="count" key="count" align="flex-start" >
            </lit-table-column>
            <lit-table-column order width="140px" title="Energy Percent(%)" data-index="energyConsumptionRatio" key="energyConsumptionRatio" align="flex-start" >
            </lit-table-column>
        </lit-table>
        "
`);
  });
});
