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
import { TabPaneStaticInit } from '../../../../../../dist/trace/component/trace/sheet/process/TabPaneStaticInit.js';

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
describe('TabPaneStaticInit Test', () => {
  let tabPaneStaticInit = new TabPaneStaticInit();
  let getTabStaticInit = sqlit.getTabStaticInit;

  getTabStaticInit.mockResolvedValue([
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5769042707,
      dur: 6701042,
      soName: 'dlopen:  /system/lib64/module/net/libsocket.z.so',
    },
    {
      pid: 3913,
      process: 'com.ohos.smartperf',
      startTs: 5696226041,
      dur: 4319791,
      soName: 'dlopen:  system/lib64/extensionability/libinputmethod_extension_module.z.so',
    },
  ]);
  it('TabPaneStaticInit01', function () {
    expect(
      (tabPaneStaticInit.data = {
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
      tabPaneStaticInit.sortByColumn({
        key: 'name',
        sort: () => {},
      })
    ).toBeUndefined();
  });
});
