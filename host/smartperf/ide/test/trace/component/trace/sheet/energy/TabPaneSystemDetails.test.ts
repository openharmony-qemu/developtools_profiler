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

// @ts-ignore
window.ResizeObserver = window.ResizeObserver || jest.fn().mockImplementation(() => ({
    observe: jest.fn(),
    unobserve: jest.fn(),
    disconnect: jest.fn(),
  }));
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
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
      cpus: [1],
      threadIds: [23, 6, 7],
      trackIds: [57, 67, 1],
      funTids: [56, 9],
      heapIds: [9,0],
      nativeMemory: [],
      cpuAbilityIds: [78],
      memoryAbilityIds: [],
      diskAbilityIds: [43,98],
      networkAbilityIds: [],
      leftNs: 546,
      rightNs: 300000000,
      hasFps: false,
      statisticsSelectData: undefined,
      perfSampleIds: [],
      perfCpus: [1,2],
      perfProcess: [],
      perfThread: [],
      perfAll: false,
      systemEnergy: [5,78,1],
      powerEnergy: [54, 56, 0],
      anomalyEnergy: [10, 5, 0, 0],
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
      pid: 2,
      uid: 33,
      state: 0,
      workId: 567,
      name: 'name',
      interval: 112,
      level: 31,
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
      pid: 23,
      uid: 11,
      state: 0,
      workId: 123,
      name: 'name',
      interval: 1011,
      level: 201,
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
      ts: 2444221,
      eventName: 'POWER_RUNNINGLOCK',
      type: 'type',
      pid: 76,
      uid: 23,
      state: 1,
      workId: 'workId',
      name: 'name',
      interval: 1223,
      level: 3421,
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
      ts: 4442111,
      eventName: 'POWER',
      type: 'type',
      pid: 23,
      uid: 337,
      state: 1,
      workId: 'workId',
      name: 'name',
      interval: 10241,
      level: 147754,
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
});
