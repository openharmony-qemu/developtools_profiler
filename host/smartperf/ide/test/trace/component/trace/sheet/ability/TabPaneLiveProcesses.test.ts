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
//@ts-ignore
import { TabPaneLiveProcesses } from '../../../../../../dist/trace/component/trace/sheet/ability/TabPaneLiveProcesses.js';
const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});
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

describe('TabPaneLiveProcesses Test', function () {
  let tabPaneLiveProcesses = new TabPaneLiveProcesses();
  let val = [
    {
      startNs: 0,
      rightNs: 1000,
      leftNs:0,
    }
  ];
  let getTabLiveProcessData = sqlite.getTabLiveProcessData;
  let liveProcessData = [
    {
      processId: 0,
      cpu: 1000,
      threads:2,
      memory:2,
      processName:'aa',
      responsibleProcess:1,
      userName:2,
      cpuTime:4,
      diskReads:20,
      diskWrite:10,
    }
  ];
  getTabLiveProcessData.mockResolvedValue(liveProcessData);
  it('TabPaneLiveProcessesTest01 ', function () {
    tabPaneLiveProcesses.queryLiveResult.length = 1;
    expect(tabPaneLiveProcesses.filterData()).toBeUndefined();
  });

  it('TabPaneLiveProcessesTest02 ', function () {
    const live = {
      processId: 1,
      processName: '',
      responsibleProcess: '',
      userName: '',
      cpu: '1',
      threads: -1,
      memory: '',
      diskReads: -1,
      diskWrite: -1,
    };
    expect(tabPaneLiveProcesses.toLiveProcessArray(live)).not.toBeUndefined();
  });

  it('TabPaneLiveProcessesTest03 ', function () {
    expect(
      tabPaneLiveProcesses.sortByColumn({
        key: 'startTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneLiveProcessesTest07 ', function () {
    expect(
      tabPaneLiveProcesses.sortByColumn({
        key: 'cpuTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneLiveProcessesTest04 ', function () {
    expect(
      tabPaneLiveProcesses.sortByColumn({
        key: !'startTime' || !'cpuTime',
      })
    ).toBeUndefined();
  });

  it('TabPaneLiveProcessesTest09 ', function () {
    expect(
      tabPaneLiveProcesses.sortByColumn({
        key: 'memory',
      })
    ).toBeUndefined();
  });

  it('TabPaneLiveProcessesTest05', function () {
    expect(tabPaneLiveProcesses.timeFormat(70000)).toBe('1 min 10 s 0 ms ');
  });

  it('TabPaneLiveProcessesTest06', function () {
    expect(tabPaneLiveProcesses.timeFormat(2000)).toBe('2 s 0 ms ');
  });

  it('TabPaneLiveProcessesTest07', function () {
    expect(tabPaneLiveProcesses.timeFormat(3600002)).toBe('1 h 2 ms ');
  });

  it('TabPaneLiveProcessesTest08', function () {
    expect(tabPaneLiveProcesses.timeFormat(10)).toBe('10 ms ');
  });
  it('TabPaneLiveProcessesTest09', function () {
    expect(tabPaneLiveProcesses.queryDataByDB(val)).toBeUndefined();
  });
});
