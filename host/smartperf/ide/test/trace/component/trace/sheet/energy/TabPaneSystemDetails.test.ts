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
import { TabPaneSystemDetails } from '../../../../../../dist/trace/component/trace/sheet/energy/TabPaneSystemDetails.js';
import '../../../../../../dist/trace/component/trace/sheet/energy/TabPaneSystemDetails.js';

import { querySysLocationDetailsData, querySysLockDetailsData } from '../../../../../../src/trace/database/SqlLite.js';
// @ts-ignore
import { SpHiSysEventChart } from '../../../../../../dist/trace/component/chart/SpHiSysEventChart.js';
import '../../../../../../dist/trace/component/chart/SpHiSysEventChart.js';

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
  it('TabPaneSystemDetailsTest01', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    tabPaneSystemDetails.tblSystemDetails = jest.fn(() => true);
    tabPaneSystemDetails.detailsTbl = jest.fn(() => true);
    tabPaneSystemDetails.tblSystemDetails!.recycleDataSource = jest.fn(() => []);
    tabPaneSystemDetails.detailsTbl!.recycleDataSource = jest.fn(() => []);
    let MockquerySystemWorkData = sqlit.querySystemWorkData;
    SpHiSysEventChart.app_name = '111';

    let querySystemWorkData = [
      {
        ts: 0,
        eventName: 'WORK_ADD',
        appKey: 'workid',
        appValue: '1',
      },
      {
        ts: 1005938319,
        eventName: 'WORK_ADD',
        appKey: 'name',
        appValue: 'nnnn',
      },
      {
        ts: 3005938319,
        eventName: 'WORK_START',
        appKey: 'workid',
        appValue: '1',
      },
      {
        ts: 3005938319,
        eventName: 'WORK_START',
        appKey: 'name',
        appValue: 'nnnn',
      },
      {
        ts: 5005938319,
        eventName: 'WORK_STOP',
        appKey: 'workid',
        appValue: '1',
      },
      {
        ts: 5005938319,
        eventName: 'WORK_STOP',
        appKey: 'name',
        appValue: 'nnnn',
      },
    ];
    MockquerySystemWorkData.mockResolvedValue(querySystemWorkData);

    let MockLockData = sqlit.querySysLockDetailsData;
    let lockDetails = [
      {
        ts: 1005938319,
        eventName: 'POWER_RUNNINGLOCK',
        appKey: 'tag',
        appValue: 'DUBAI_TAG_RUNNINGLOCK_ADD',
      },
      {
        ts: 1005938319,
        eventName: 'POWER_RUNNINGLOCK',
        appKey: 'message',
        appValue: 'token=123',
      },
      {
        ts: 3005933657,
        eventName: 'POWER_RUNNINGLOCK',
        appKey: 'tag',
        appValue: 'DUBAI_TAG_RUNNINGLOCK_REMOVE',
      },
      {
        ts: 3005933657,
        eventName: 'POWER_RUNNINGLOCK',
        appKey: 'message',
        appValue: 'token=123',
      },
    ];
    MockLockData.mockResolvedValue(lockDetails);

    let MockLocationData = sqlit.querySysLocationDetailsData;
    let locationDetails = [
      {
        ts: 1005938319,
        eventName: 'GNSS_STATE',
        appKey: 'state',
        appValue: 'start',
      },
      {
        ts: 1005938319,
        eventName: 'GNSS_STATE',
        appKey: 'pid',
        appValue: '11',
      },
      {
        ts: 3005933657,
        eventName: 'GNSS_STATE',
        appKey: 'state',
        appValue: 'stop',
      },
      {
        ts: 3005933657,
        eventName: 'GNSS_STATE',
        appKey: 'pid',
        appValue: '11',
      },
    ];
    MockLocationData.mockResolvedValue(locationDetails);

    let tabPaneSystemDetailsData = {
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
      rightNs: 300000000000,
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

    tabPaneSystemDetails.data = tabPaneSystemDetailsData;
    expect(tabPaneSystemDetails.data).toBeUndefined();
  });

  it('TabPaneSystemDetailsTest02', function () {
    let tabPaneSystem = new TabPaneSystemDetails();
    tabPaneSystem.tblSystemDetails = jest.fn(() => true);
    tabPaneSystem.detailsTbl = jest.fn(() => true);
    tabPaneSystem.tblSystemDetails!.recycleDataSource = jest.fn(() => []);
    tabPaneSystem.detailsTbl!.recycleDataSource = jest.fn(() => []);
    let MockSystemWorkData = sqlit.querySystemWorkData;
    MockSystemWorkData.mockResolvedValue([]);
    let MockSystemLockData = sqlit.querySysLockDetailsData;
    MockSystemLockData.mockResolvedValue([]);
    let MockSystemLocationData = sqlit.querySysLocationDetailsData;
    MockSystemLocationData.mockResolvedValue([]);
    let tabPaneSystemDetailsData = {
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

    tabPaneSystem.data = tabPaneSystemDetailsData;
    expect(tabPaneSystem.data).toBeUndefined();
  });

  it('TabPaneSystemDetailsTest03', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    tabPaneSystemDetails.tblSystemDetails = jest.fn(() => true);
    tabPaneSystemDetails.detailsTbl = jest.fn(() => true);
    tabPaneSystemDetails.tblSystemDetails!.recycleDataSource = jest.fn(() => []);
    tabPaneSystemDetails.detailsTbl!.recycleDataSource = jest.fn(() => []);
    let data = {
      ts: 0,
      eventName: 'Event Name',
      type: 'type',
      pid: 0,
      uid: 0,
      state: 0,
      workId: 'workId',
      name: 'name',
      interval: 0,
      level: 0,
      tag: 'tag:',
      message: 'message',
      log_level: 'log_level',
    };

    expect(tabPaneSystemDetails.convertData(data)).toBeUndefined();
  });

  it('TabPaneSystemDetailsTest04', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    tabPaneSystemDetails.tblSystemDetails = jest.fn(() => true);
    tabPaneSystemDetails.detailsTbl = jest.fn(() => true);
    tabPaneSystemDetails.tblSystemDetails!.recycleDataSource = jest.fn(() => []);
    tabPaneSystemDetails.detailsTbl!.recycleDataSource = jest.fn(() => []);
    let data = {
      ts: 0,
      eventName: 'GNSS_STATE',
      type: 'type',
      pid: 0,
      uid: 0,
      state: 0,
      workId: 'workId',
      name: 'name',
      interval: 0,
      level: 0,
      tag: 'tag:',
      message: 'message',
      log_level: 'log_level',
    };

    expect(tabPaneSystemDetails.convertData(data)).toBeUndefined();
  });

  it('TabPaneSystemDetailsTest05', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    tabPaneSystemDetails.tblSystemDetails = jest.fn(() => true);
    tabPaneSystemDetails.detailsTbl = jest.fn(() => true);
    tabPaneSystemDetails.tblSystemDetails!.recycleDataSource = jest.fn(() => []);
    tabPaneSystemDetails.detailsTbl!.recycleDataSource = jest.fn(() => []);
    let data = {
      ts: 0,
      eventName: 'POWER_RUNNINGLOCK',
      type: 'type',
      pid: 0,
      uid: 0,
      state: 0,
      workId: 'workId',
      name: 'name',
      interval: 0,
      level: 0,
      tag: 'tag:',
      message: 'message',
      log_level: 'log_level',
    };
    expect(tabPaneSystemDetails.convertData(data)).toBeUndefined();
  });

  it('TabPaneSystemDetailsTest06', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    tabPaneSystemDetails.tblSystemDetails = jest.fn(() => true);
    tabPaneSystemDetails.detailsTbl = jest.fn(() => true);
    tabPaneSystemDetails.tblSystemDetails!.recycleDataSource = jest.fn(() => []);
    tabPaneSystemDetails.detailsTbl!.recycleDataSource = jest.fn(() => []);
    let data = {
      ts: 0,
      eventName: 'POWER',
      type: 'type',
      pid: 0,
      uid: 0,
      state: 0,
      workId: 'workId',
      name: 'name',
      interval: 0,
      level: 0,
      tag: 'tag:',
      message: 'message',
      log_level: 'log_level',
    };

    expect(tabPaneSystemDetails.convertData(data)).toBeUndefined();
  });

  it('TabPaneSystemDetailsTest08', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    let cc = [
      {
        ts: -14000,
        workId: 44,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_ADD',
      },
      {
        ts: 10000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_START',
      },
      {
        ts: 12000,
        workId: 22,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_ADD',
      },
      {
        ts: 14000,
        workId: 44,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_START',
      },
      {
        ts: 20000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_STOP',
      },
      {
        ts: 22000,
        workId: 22,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_START',
      },
      {
        ts: 30000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_START',
      },
      {
        ts: 32000,
        workId: 22,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_STOP',
      },
      {
        ts: 40000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_STOP',
      },
      {
        ts: 42000,
        workId: 22,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_START',
      },
      {
        ts: 50000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_START',
      },
      {
        ts: 52000,
        workId: 22,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_STOP',
      },
      {
        ts: 60000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_STOP',
      },
      {
        ts: 62000,
        workId: 22,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_REMOVE',
      },
      {
        ts: 64000,
        workId: 44,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_STOP',
      },
      {
        ts: 70000,
        workId: 11,
        name: SpHiSysEventChart.app_name,
        eventName: 'WORK_REMOVE',
      },
    ];
    tabPaneSystemDetails.getConvertData = jest.fn(() => cc);
    let systemWorkData = tabPaneSystemDetails.getSystemWorkData();

    expect(systemWorkData).toStrictEqual([]);
  });

  it('TabPaneSystemDetailsTest07', function () {
    let tabPaneSystemDetails = new TabPaneSystemDetails();
    expect(tabPaneSystemDetails.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px 0 10px;
        }
        .sys-detail-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        </style>
        <div class="sys-detail-content" style="display: flex;flex-direction: column">
            <div style="display: flex;flex-direction: row">
                <lit-slicer style="width:100%">
                    <div class="box-details" style="width: 100%">
                        <lit-table id="tb-system-data" style="height: auto">
                            <lit-table-column class="sys-detail-column" width="300px" title="" data-index="eventName" key="eventName"  align="flex-start" order>
                            </lit-table-column>
                            <lit-table-column class="sys-detail-column" width="300px" title="" data-index="ts" key="ts"  align="flex-start" order>
                            </lit-table-column>
                        </lit-table>
                    </div>
                    <lit-slicer-track ></lit-slicer-track>
                    <lit-table id="tb-system-details-data" no-head hideDownload style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)">
                        <lit-table-column class="sys-detail-column" width="100px" title="" data-index="key" key="key"  align="flex-start" >
                        </lit-table-column>
                        <lit-table-column class="sys-detail-column" width="1fr" title="" data-index="value" key="value"  align="flex-start">
                        </lit-table-column>
                    </lit-table>
                </lit-slicer>
            </div>
            <lit-progress-bar class="progress sys-detail-progress"></lit-progress-bar>
        </div>
        "
`);
  });
});
