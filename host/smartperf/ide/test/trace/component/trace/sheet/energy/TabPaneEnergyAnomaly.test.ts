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

    tabPaneEnergyAnomaly.data = tabPaneAnomalyDetailedData;
  });

  it('TabPaneEnergyAnomalyTest02', function () {
    let tabPaneEnergyAnomaly = new TabPaneEnergyAnomaly();
    expect(tabPaneEnergyAnomaly.initHtml()).toMatchInlineSnapshot(`
"
        <style>
            .anomaly-title{
                display: flex;
                width: 95%;
                background: var(--dark-background,#ffffff);
                top: 0;
                position: sticky;
            }
            .anomaly-title h2{
                padding: 0 10px;
                font-size: 16px;
                font-weight: 400;
                width: 50%;
                visibility: visible;
            }
            .scroll-area{
                overflow-y: auto;
                height: auto;
                display: flex;
                
            }
            .left-table{
                padding: 0 10px;
                width: 50%;
            }
            .anomaly-table{
                height: auto;
            }
        </style>
        <div style="width: 100%;height: auto;position: relative">
            <div id="anomaly-details" class="anomaly-title" style="margin-left: 12px;display: block">
                <h2 id="leftTitle"></h2>
            </div>
            <div class="scroll-area">
                <div class="left-table">
                    <lit-table id="anomalyselectionTbl" no-head class="anomaly-table">
                        <lit-table-column key="name" align="flex-start"  width="180px" title="name" data-index="name" >
                        </lit-table-column>
                        <lit-table-column key="value" align="flex-start" title="value" data-index="value">
                        </lit-table-column>
                    </lit-table>
                </div>
            </div>
        </div>
        "
`);
  });
});
