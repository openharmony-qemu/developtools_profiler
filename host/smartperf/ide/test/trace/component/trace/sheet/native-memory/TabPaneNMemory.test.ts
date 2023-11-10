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

import crypto from 'crypto';

//@ts-ignore
import {
  TabPaneNMemory,
  initFilterTypes,
} from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMemory.js';
// @ts-ignore
import { TabPaneNMSampleList } from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMSampleList.js';

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
// @ts-ignore
import {
  queryNativeHookEventTid,
  queryNativeHookSnapshotTypes,
} from '../../../../../../dist/trace/database/SqlLite.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneNMemory Test', () => {
  document.body.innerHTML = `<div><tabpane-native-memory id="tnm"> </tabpane-native-memory></div>`;
  let tabPaneNMemory = new TabPaneNMemory();
  let val = {
    statisticsSelectData: {
      memoryTap: 1,
    },
  };
  let hook = { eventId: 1 };

  it('TabPaneNMemoryTest01', function () {
    expect(tabPaneNMemory.initFilterTypes()).toBeUndefined();
  });

  it('TabPaneNMemoryTest02', function () {
    let MockNativeHookSnapshotTypes = sqlit.queryNativeHookSnapshotTypes;
    MockNativeHookSnapshotTypes.mockResolvedValue([
      {
        eventType: 'MmapEvent',
        subType: '',
      },
    ]);
    let tab = new TabPaneNMSampleList();
    tabPaneNMemory.startWorker = jest.fn(() => true);
    expect(tabPaneNMemory.initFilterTypes()).toBeUndefined();
  });

  it('TabPaneNMemoryTest09', function () {
    tabPaneNMemory.tblData = jest.fn(() => undefined);
    tabPaneNMemory.tblData.recycleDataSource = jest.fn(() => true);
    tabPaneNMemory.startWorker = jest.fn(() => true);
    expect(tabPaneNMemory.setRightTableData(hook)).toBeUndefined();
  });

  it('TabPaneNMemoryTest018', function () {
    expect(tabPaneNMemory.fromStastics(val)).toBeUndefined();
  });
  it('TabPaneNMemoryTest19', function () {
    expect(tabPaneNMemory.startWorker({}, {})).toBeTruthy();
  });
  it('TabPaneNMemoryTest20', function () {
    expect(tabPaneNMemory.startWorker({}, {})).toBeTruthy();
  });
  it('TabPaneNMemoryTest21', function () {
    expect(tabPaneNMemory.getDataByNativeMemoryWorker([], false)).toBeUndefined();
  });
  it('TabPaneNMemoryTest22', function () {
    expect(tabPaneNMemory.resetFilter()).toBeUndefined();
  });
});
