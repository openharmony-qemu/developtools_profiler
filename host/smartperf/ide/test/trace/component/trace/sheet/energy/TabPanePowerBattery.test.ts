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
import { TabPanePowerBattery } from '../../../../../../dist/trace/component/trace/sheet/energy/TabPanePowerBattery.js';
import '../../../../../../dist/trace/component/trace/sheet/energy/TabPanePowerBattery.js';
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

describe('TabPanePowerBattery Test', () => {
  it('TabPanePowerBatteryTest01', function () {
    document.body.innerHTML = `<lit-table id="tb-power-battery-energy"></lit-table>`;
    let litTable = document.querySelector('#tb-power-battery-energy') as LitTable;
    let tabPanePowerBattery = new TabPanePowerBattery();
    tabPanePowerBattery.tblPower = jest.fn(() => litTable);
    let MockPowerBatteryData = sqlit.getTabPowerBatteryData;
    let battery = [
      {
        ts: 1000,
        eventName: 'POWER_IDE_BATTERY',
        appKey: 'appname',
        eventValue: 'POWER_IDE,POWER_IDE,POWER_IDE,POWER_IDE',
      },
      {
        ts: 1000,
        eventName: 'POWER_IDE_BATTERY',
        appKey: 'appname',
        eventValue: 'POWER_IDE,POWER_IDE,POWER_IDE,POWER_IDE',
      },
    ];
    MockPowerBatteryData.mockResolvedValue(battery);
    let tabPanePowerBatteryData = {
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
    tabPanePowerBattery.tblPower.recycleDataSource = jest.fn(() => tabPanePowerBatteryData);
    tabPanePowerBattery.data = tabPanePowerBatteryData;
  });

  it('TabPanePowerBatteryTest02', function () {
    let tabPanePowerBattery = new TabPanePowerBattery();
    expect(tabPanePowerBattery.initHtml()).toMatchInlineSnapshot(`
"
        <style>
            .power-battery-bottom-scroll-area{
                display: flex;
                height: auto;
                overflow-y: auto;
                margin-top: 1.2em;
            }
            .power-battery-battery-canvas{
                width: 50%;
                padding: 0 10px;
            }
         
        </style>
        <div style="width: 100%;height: auto;position: relative">
            <div class="power-battery-bottom-scroll-area">
                <div class="power-battery-battery-canvas">
                    <lit-table id="tb-power-battery-energy" no-head style="height: auto">
                        <lit-table-column title="name" data-index="name" key="name" align="flex-start"  width="180px"></lit-table-column>
                        <lit-table-column title="value" data-index="value" key="value" align="flex-start" ></lit-table-column>
                    </lit-table>
                </div>
            </div>
        </div>
        "
`);
  });
});
