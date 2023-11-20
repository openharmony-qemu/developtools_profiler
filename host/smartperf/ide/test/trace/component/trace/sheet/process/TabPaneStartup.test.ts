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
import { TabPaneStartup } from '../../../../../../dist/trace/component/trace/sheet/process/TabPaneStartup.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorkerAppStartup.js', () => {
  return {
    AppStartupStruct: {
      getStartupName: jest.fn().mockReturnValue('Unknown Start Step'),
    },
  };
});
describe('TabPaneStartup Test', () => {
  let tabPaneStartup = new TabPaneStartup();
  let getTabStartups = sqlit.getTabStartups;

  getTabStartups.mockResolvedValue([
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5651745832,
      dur: 38654167,
      startName: 0,
    },
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5690399999,
      dur: 43619792,
      startName: 1,
    },
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5734019791,
      dur: 23194270,
      startName: 2,
    },
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5757214061,
      dur: 115679167,
      startName: 3,
    },
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5872893228,
      dur: 62756250,
      startName: 4,
    },
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5968040103,
      dur: 29438021,
      startName: 5,
    },
  ]);
  it('TabPaneStartupTest01', function () {
    expect(
      (tabPaneStartup.data = {
        recordStartNs: 94574874464,
        leftNs: 5521679251,
        rightNs: 6407693386,
        hasFps: false,
        perfAll: false,
        fileSysVirtualMemory: false,
        diskIOLatency: false,
        fsCount: 0,
        vmCount: 0,
        isCurrentPane: false,
        startup: true,
        staticInit: true,
        processIds: [3913],
      })
    );
  });

  it('TabPaneCounterTest02', function () {
    expect(
      tabPaneStartup.sortByColumn({
        key: 'name',
        sort: () => {},
      })
    ).toBeUndefined();
  });
});
