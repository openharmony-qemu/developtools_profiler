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

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

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
        ts: 23337353,
        eventName: 'POWER_IDE_AUDIO',
        appKey: 'APPNAME',
        eventValue: 'com.example.himusicdemo,com.example.himusicdemo_js,com.example.himusicdemo_app',
      },
      {
        ts: 32119127353,
        eventName: 'POWER_IDE_AUDIO',
        appKey: 'BACKGROUND_DURATION',
        eventValue: '524,854,612',
      },
      {
        ts: 3111117353,
        eventName: 'POWER_IDE_BLUETOOTH',
        appKey: 'APPNAME',
        eventValue: 'com.ohos.settings,bt_switch,bt_switch_js,bt_switch_app',
      },
      {
        ts: 311111137353,
        eventName: 'POWER_IDE_BLUETOOTH',
        appKey: 'BACKGROUND_DURATION',
        eventValue: '325,124,51,52',
      },
      {
        ts: 1387357353,
        eventName: 'POWER_IDE_CAMERA',
        appKey: 'APPNAME',
        eventValue: 'com.ohos.camera,com.ohos.camera_app,com.ohos.camera_js,com.ohos.camera_ts',
      },
      {
        ts: 15422127353,
        eventName: 'POWER_IDE_CAMERA',
        appKey: 'BACKGROUND_DURATION',
        eventValue: '356,325,854,365',
      },
    ];
    MockPowerDetailsData.mockResolvedValue(detail);
    let list = {
      cpus: [3],
      threadIds: [],
      trackIds: [10, 45, 7],
      funTids: [6, 89, 0],
      heapIds: [9, 44],
      nativeMemory: [],
      cpuAbilityIds: [7, 0, 66],
      memoryAbilityIds: [7,77,0],
      diskAbilityIds: [5,76],
      networkAbilityIds: [85,3,56],
      leftNs: 966,
      rightNs: 1000,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [10, 35,7],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
      systemEnergy: [20, 412, 2],
      powerEnergy: [564, 2],
      anomalyEnergy: [145, 56],
    };
    tabPanePowerDetails.tblPowerDetails.recycleDataSource = jest.fn(() => list);
    tabPanePowerDetails.data = list;
    expect(tabPanePowerDetails.data).toBeUndefined();
  });
});
