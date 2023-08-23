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
import { SpHiSysEventChart } from '../../../../dist/trace/component/chart/SpHiSysEventChart.js';
import '../../../../dist/trace/component/chart/SpHiSysEventChart.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
import '../../../../dist/trace/component/chart/SpChartManager.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../dist/trace/component/SpSystemTrace.js';
import '../../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { LitPopover } from '../../../../dist/base-ui/popover/LitPopoverV.js';
import {
  querySystemLocationData,
  querySystemLockData,
  querySystemSchedulerData,
  queryConfigSysEventAppName,
} from '../../../../src/trace/database/SqlLite.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlite = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

describe('SpHiSysEventChart Test', () => {
  let ss = new SpChartManager();
  let spHiSysEventChart = new SpHiSysEventChart(ss);

  let htmlDivElement = document.createElement<LitPopover>('div');
  htmlDivElement.setAttribute('id', 'appNameList');

  let anomalyData = sqlite.queryAnomalyData;
  anomalyData.mockResolvedValue([]);

  let maxStateValue = sqlite.queryMaxStateValue;
  let max = [
    {
      maxValue: 200,
      type: 'state',
    },
    {
      maxValue: 300,
      type: 'sensor',
    },
  ];
  maxStateValue.mockResolvedValue(max);
  let MockExits = sqlite.queryHisystemEventExits;
  MockExits.mockResolvedValue(['trace_hisys_event']);
  let powerData = sqlite.queryPowerData;
  let power = [
    {
      startNS: 5999127353,
      eventName: 'POWER_IDE_AUDIO',
      appKey: 'APPNAME',
      eventValue: 'com.example.himusicdemo,com.example.himusicdemo_js,com.example.himusicdemo_app',
    },
    {
      startNS: 5999127353,
      eventName: 'POWER_IDE_AUDIO',
      appKey: 'BACKGROUND_ENERGY',
      eventValue: '854,258,141',
    },
  ];
  powerData.mockResolvedValue(power);

  let stateData = sqlite.queryStateData;
  stateData.mockResolvedValue([]);

  let sysEventAppName = sqlite.querySyseventAppName;
  let appName = [
    {
      string_value: 'app_name',
    },
  ];
  sysEventAppName.mockResolvedValue(appName);

  let querySystemLocationData = sqlite.querySystemLocationData;
  let querySystemLockData = sqlite.querySystemLockData;
  let querySystemSchedulerData = sqlite.querySystemSchedulerData;
  let queryConfigSysEventAppName = sqlite.queryConfigSysEventAppName;
  let location = [
    {
      ts: 1005938319,
      eventName: 'GNSS_STATE',
      appKey: 'TYPE',
      Value: '1',
    },
    {
      ts: 3005933657,
      eventName: 'GNSS_STATE',
      appKey: 'TAG',
      Value: '2',
    },
  ];

  let lock = [
    {
      ts: 1005938319,
      eventName: 'POWER_RUNNINGLOCK',
      appKey: 'TYPE',
      Value: '1',
    },
    {
      ts: 3005933657,
      eventName: 'POWER_RUNNINGLOCK',
      appKey: 'TAG',
      Value: '2',
    },
  ];

  let work = [
    {
      ts: 1005938319,
      eventName: 'WORK_ADD',
      appKey: 'TYPE',
      Value: '1',
    },
    {
      ts: 3005933657,
      eventName: 'WORK_STOP',
      appKey: 'TAG',
      Value: '2',
    },
  ];

  let process = [
    {
      process_name: 'process1',
    },
  ];
  querySystemLocationData.mockResolvedValue(location);
  querySystemLockData.mockResolvedValue(lock);
  querySystemSchedulerData.mockResolvedValue(work);
  queryConfigSysEventAppName.mockResolvedValue(process);

  it('spHiSysEventChartTest01', function () {
    spHiSysEventChart.init();
    expect(SpHiSysEventChart.app_name).toBeUndefined();
  });

  it('spHiSysEventChartTest02', function () {
    let result = [
      {
        ts: 1005938319,
        eventName: 'WORK_START',
        appKey: 'TYPE',
        Value: '1',
      },
      {
        ts: 3005933657,
        eventName: 'POWER_RUNNINGLOCK',
        appKey: 'TAG,',
        Value: 'DUBAI_TAG_RUNNINGLOCK_REMOVE',
      },
      {
        ts: 4005938319,
        eventName: 'GNSS_STATE',
        appKey: 'STATE',
        Value: 'stop',
      },
      {
        ts: 5005933657,
        eventName: 'POWER_RUNNINGLOCK',
        appKey: 'TAG',
        Value: 'DUBAI_TAG_RUNNINGLOCK_ADD',
      },
      {
        ts: 6005938319,
        eventName: 'GNSS_STATE',
        appKey: 'STATE',
        Value: 'start',
      },
      {
        ts: 9005938319,
        eventName: 'WORK_STOP',
        appKey: 'TYPE',
        Value: '1',
      },
      {
        ts: 10005938319,
        eventName: 'WORK_REMOVE',
        appKey: 'TYPE',
        Value: '1',
      },
    ];
    expect(spHiSysEventChart.getSystemData([result, result, result])).toEqual({
      '0': [
        { count: 1, startNs: 5005933657, token: undefined, type: 1 },
        { count: 0, startNs: 6005938319, token: undefined, type: 1 },
      ],
      '1': [
        { count: 1, startNs: 1005938319, state: 'start', type: 2 },
        { count: 2, startNs: 3005933657, state: 'start', type: 2 },
        { count: 1, startNs: 4005938319, state: 'stop', type: 2 },
        { count: 2, startNs: 5005933657, state: 'start', type: 2 },
        { count: 3, startNs: 6005938319, state: 'start', type: 2 },
        { count: 4, startNs: 9005938319, state: 'start', type: 2 },
        { count: 5, startNs: 10005938319, state: 'start', type: 2 },
      ],
      '2': [
        { count: 1, startNs: 1005938319, type: 0 },
        { count: 0, startNs: undefined, type: 0 },
      ],
    });
  });

  it('spHiSysEventChartTest03', function () {
    expect(spHiSysEventChart.getSystemData([]).length).toBeUndefined();
  });

  it('spHiSysEventChartTest04', function () {
    let result = [
      {
        startNS: 5999127353,
        eventName: 'POWER_IDE_AUDIO',
        appKey: 'APPNAME',
        eventValue: 'com.example.himusicdemo,com.example.himusicdemo_js,com.example.himusicdemo_app',
      },
      {
        startNS: 5999127353,
        eventName: 'POWER_IDE_AUDIO',
        appKey: 'BACKGROUND_ENERGY',
        eventValue: '854,258,141',
      },
      {
        startNS: 5999127353,
        eventName: 'POWER_IDE_BLUETOOTH',
        appKey: 'APPNAME',
        eventValue: 'com.ohos.settings,bt_switch,bt_switch_js,bt_switch_app',
      },
      {
        startNS: 5999127353,
        eventName: 'POWER_IDE_BLUETOOTH',
        appKey: 'BACKGROUND_ENERGY',
        eventValue: '76,12,43,431',
      },
      {
        startNS: 5999127388,
        eventName: 'POWER_IDE_CAMERA',
        appKey: 'APPNAME',
        eventValue: 'com.ohos.camera,com.ohos.camera_app,com.ohos.camera_js,com.ohos.camera_ts',
      },
      {
        startNS: 5999127388,
        eventName: 'POWER_IDE_CAMERA',
        appKey: 'BACKGROUND_ENERGY',
        eventValue: '375,475,255,963',
      },
    ];
    expect(spHiSysEventChart.getPowerData(result)).toStrictEqual(Promise.resolve());
  });

  it('spHiSysEventChartTest05', function () {
    expect(spHiSysEventChart.getPowerData([])).toStrictEqual(Promise.resolve());
  });

  it('spHiSysEventChartTest6', function () {
    expect(spHiSysEventChart.initHtml).toMatchInlineSnapshot(`undefined`);
  });

  it('spHiSysEventChartTest7', function () {
    expect(htmlDivElement.onclick).toBe(null);
  });
});
