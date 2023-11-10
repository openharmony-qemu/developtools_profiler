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
import '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMCallTree.js';
// @ts-ignore
import { TabPaneNMCallTree } from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMCallTree.js';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
import { FrameChart } from '../../../../../../dist/trace/component/chart/FrameChart.js';
// @ts-ignore
import { DisassemblingWindow } from '../../../../../../dist/trace/component/DisassemblingWindow.js';
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    observe: jest.fn(),
    unobserve: jest.fn(),
    disconnect: jest.fn(),
  }));

describe('TabPaneNMCallTree Test', () => {
  document.body.innerHTML = '<div><tabpane-nm-calltree id="tree"></tabpane-nm-calltree></div>';
  let tabPaneNMCallTree = document.querySelector<TabPaneNMCallTree>('#tree');
  let dom = new FrameChart();
  dom.setAttribute('id', 'framechart');
  tabPaneNMCallTree.frameChart = dom;
  tabPaneNMCallTree.modal = new DisassemblingWindow();
  tabPaneNMCallTree.filter = new TabPaneFilter();

  it('TabPaneNMCallTreeTest01', function () {
    let hookLeft = {
      ip: '',
      symbolsId: 0,
      pathId: 0,
      processName: '',
      type: 0,
      children: [],
    };
    tabPaneNMCallTree.dataSource = [];
    let groupByWithTid = tabPaneNMCallTree.setRightTableData(hookLeft);
    expect(groupByWithTid).toBeUndefined();
  });

  it('TabPaneNMCallTreeTest02', function () {
    let data = [
      { size: 10, count: 20, children: [] },
      { size: 11, count: 21, children: [] },
      { size: 21, count: 31, children: [] },
    ];
    expect(tabPaneNMCallTree.sortTree(data).length).toBe(3);
  });

  it('TabPaneNMCallTreeTest03', function () {});

  it('TabPaneNMCallTreeTest05', function () {
    let hook = {
      id: '1',
      dur: 1,
      children: [],
    };
    let id = '1';
    expect(tabPaneNMCallTree.getParentTree([hook], { id }, [])).not.toBeUndefined();
  });
  it('TabPaneNMCallInfoTest06', function () {
    let hook = {
      eventId: '1',
      dur: 1,
      children: [],
    };
    expect(tabPaneNMCallTree.getChildTree([hook], '1', [])).not.toBeUndefined();
  });
  it('TabPaneNMCallInfoTest07', function () {
    document.body.innerHTML = "<div id='filter' tree></div>";
    let table = document.querySelector('#filter');
    table!.setAttribute('tree', '1');
    tabPaneNMCallTree.filter = table;
    tabPaneNMCallTree.filter.showThird = jest.fn(() => {
      false;
    });
    expect(tabPaneNMCallTree.showBottomMenu()).toBeUndefined();
  });
  it('TabPaneNMCallInfoTest08', function () {
    let isShow = 1;
    document.body.innerHTML = "<div id='filter' tree></div>";
    let table = document.querySelector('#filter');
    table!.setAttribute('tree', '1');
    tabPaneNMCallTree.filter = table;
    tabPaneNMCallTree.filter.showThird = jest.fn(() => {
      false;
    });
    expect(tabPaneNMCallTree.showBottomMenu(isShow)).toBeUndefined();
  });

  it('TabPaneNMCallInfoTest09', function () {
    tabPaneNMCallTree.filter.initializeFilterTree = jest.fn();
    tabPaneNMCallTree.initFilterTypes = jest.fn();
    tabPaneNMCallTree.native_type = jest.fn(() => ['All Heap & Anonymous VM', 'All Heap', 'All Anonymous VM']);
    tabPaneNMCallTree.getDataByWorkerQuery = jest.fn();
    tabPaneNMCallTree.data = {
      leftNs: 0,
      rightNs: 500,
      nativeMemory: 'All Heap & Anonymous VM',
    };
    expect(tabPaneNMCallTree.data).toBeUndefined();
  });
  it('TabPaneNMCallTreeTest10', function () {
    let data = [
      { size: 10, count: 20, children: [] },
      { size: 11, count: 21, children: [] },
      { size: 21, count: 31, children: [] },
    ];
    expect(tabPaneNMCallTree.setLTableData(data)).toBeUndefined();
  });
  it('TabPaneNMCallTreeTest12', function () {
    expect(tabPaneNMCallTree.initFilterTypes()).toBeUndefined();
  });
  it('TabPaneNMCallTreeTest14', function () {
    expect(tabPaneNMCallTree.getDataByWorkerQuery({}, {})).toBeUndefined();
  });

  it('TabPaneNMCallTreeTest15', function () {
    let data = [
      {
        callTreeConstraints: {
          inputs: [1],
        },
        dataMining: 20,
        callTree: [],
        icon: 'tree',
      },
    ];
    expect(tabPaneNMCallTree.switchFlameChart(data)).toBeUndefined();
  });
  it('TabPaneNMCallTreeTest16', function () {
    let filterData = {
      callTree: [{}, {}],
      dataMining: {
        concat: jest.fn(() => true),
      },
      callTreeConstraints: {
        checked: false,
      },
    };
    tabPaneNMCallTree.currentSelection = jest.fn(() => true);
    tabPaneNMCallTree.currentSelection.nativeMemory = jest.fn(() => true);
    expect(tabPaneNMCallTree.refreshAllNode(filterData)).toBeUndefined();
  });
});
