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
// import { it } from "mocha"
import { TabPaneBoxChild } from '../../../../../../dist/trace/component/trace/sheet/cpu/TabPaneBoxChild.js';
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneBoxChild Test', () => {
  document.body.innerHTML = `<div id="div"></div>`;
  let element = document.querySelector('#div') as HTMLDivElement;
  let tabPaneBoxChild = new TabPaneBoxChild();
  element.appendChild(tabPaneBoxChild);
  tabPaneBoxChild.loadDataInCache = true;
  let getTabBox = sqlit.getTabBoxChildData;
  let data = [
    {
      process: '',
      processId: 12,
      thread: '',
      state: 2,
      threadId: 3,
      duration: 1,
      startNs: 17,
      cpu: 2,
      priority: 1,
    },
  ];
  getTabBox.mockResolvedValue(data);
  tabPaneBoxChild.data = {
    cpus: [],
    threadIds: [],
    trackIds: [],
    funTids: [],
    heapIds: [],
    leftNs: 0,
    rightNs: 233,
    hasFps: false,
    state:'',
    processId:0,
    threadId: 0
  };


  it('TabPaneBoxChildTest01', function () {
    expect(
      tabPaneBoxChild.sortByColumn({
        key: 'number',
      })
    ).toBeUndefined();
  });

  it('TabPaneCounterTest02', function () {
    expect(
      tabPaneBoxChild.sortByColumn({
        sort: () => {},
      })
    ).toBeUndefined();
  });
  it('TabPaneCounterTest03', function () {
    let val = [
      {
        leftNs: 11,
        rightNs: 34,
        state: true,
        processId: 3,
        threadId: 1,
      },
    ];
    expect(tabPaneBoxChild.getDataByDB(val)).toBeUndefined();
  });
});
