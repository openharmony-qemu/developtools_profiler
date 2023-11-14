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
import { TabPaneEnergyAnomaly } from '../../../../../../dist/trace/component/trace/sheet/energy/TabPaneEnergyAnomaly.js';
import '../../../../../../dist/trace/component/trace/sheet/energy/TabPaneEnergyAnomaly.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

describe('TabPanePowerBattery Test', () => {
  it('TabPaneEnergyAnomalyTest01', function () {
    let tabPaneEnergyAnomaly = new TabPaneEnergyAnomaly();
    let MockAnomalyDetailedData = sqlit.queryAnomalyDetailedData;
    let battery = [
      {
        ts: 11611696002,
        eventName: 'ANOMALY_SCREEN_OFF_ENERGY',
        appKey: 'BATTERY_DRAIN',
        Value: '10',
      },
      {
        ts: 11611696002,
        eventName: 'ANOMALY_SCREEN_OFF_ENERGY',
        appKey: 'BATTERY_GAS_GUAGE',
        Value: '980',
      },
      {
        ts: 15612568649,
        eventName: 'ANOMALY_RUNNINGLOCK',
        appKey: 'APPNAME',
        Value: 'com.example.powerhap',
      },
      {
        ts: 15612568649,
        eventName: 'ANOMALY_RUNNINGLOCK',
        appKey: 'COUNT',
        Value: '1',
      },
      {
        ts: 17611804002,
        eventName: 'ANORMALY_APP_ENERGY',
        appKey: 'APPNAME',
        Value:
          '*dpm_others*,*dpm_rom*,/system/bin/hilogd,' +
          '/system/bin/render_service,' +
          '/system/bin/wifi_hal_service,' +
          'bluetooth_servi,com.example.baseanimation,' +
          'com.example.ohos_location_js,' +
          'com.ohos.launcher,com.ohos.settings,' +
          'hidumper_servic,hwc_host,' +
          'kernel_kworker,softbus_server',
      },
      {
        ts: 17611804002,
        eventName: 'ANORMALY_APP_ENERGY',
        appKey: 'BGENERGY',
        Value: '11726,79745,6209,249329,1680,8694,3061,457,402,17064,4087,16403,32965,2895',
      },
    ];
    MockAnomalyDetailedData.mockResolvedValue(battery);
    let tabPaneAnomalyDetailedData = {
      cpus: [],
      threadIds: [85,6,9],
      trackIds: [46,0],
      funTids: [8,0,76],
      heapIds: [67,89],
      nativeMemory: [],
      cpuAbilityIds: [21, 54],
      memoryAbilityIds: [54 ,78],
      diskAbilityIds: [5,0],
      networkAbilityIds: [9],
      leftNs: 3200,
      rightNs: 425900,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [45,85],
      perfCpus: [],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
      systemEnergy: [99,5],
      powerEnergy: [0, 87, 65],
      anomalyEnergy: [670, 18, 782],
    };

    tabPaneEnergyAnomaly.data = tabPaneAnomalyDetailedData;
  });
});
