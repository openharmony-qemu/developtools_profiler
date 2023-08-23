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

const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {}
});

window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
      disconnect: jest.fn(),
      observe: jest.fn(),
      unobserve: jest.fn(),
    }));

describe('TabPaneNMCallTree Test', () => {
  document.body.innerHTML = '<div><tabpane-nm-calltree id="ddd"></tabpane-nm-calltree></div>';
  let tabPaneNMCallTree = document.querySelector<TabPaneNMCallTree>('#ddd');
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
    tabPaneNMCallTree.setRightTableData = jest.fn(() => true);
    let groupByWithTid = tabPaneNMCallTree.setRightTableData(hookLeft);
    expect(groupByWithTid).toBeTruthy();
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

  it('TabPaneNMCallTreeTest04', function () {
    expect(tabPaneNMCallTree.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            padding: 10px 10px 0 10px;
            display: flex;
            flex-direction: column;
        }
        .show{
            display: flex;
            flex: 1;
        }
        #nm-call-tree-filter {
            border: solid rgb(216,216,216) 1px;
            float: left;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
        selector{
            display: none;
        }
        .nm-call-tree-progress{
            bottom: 33px;
            position: absolute;
            height: 1px;
            left: 0;
            right: 0;
        }
        .nm-call-tree-loading{
            bottom: 0;
            position: absolute;
            left: 0;
            right: 0;
            width:100%;
            background:transparent;
            z-index: 999999;
        }
    </style>
    <div class="nm-call-tree-content" style="display: flex;flex-direction: row">
    
    <selector id='show_table' class="show">
        <lit-slicer style="width:100%">
        <div id="left_table" style="width: 65%">
            <tab-native-data-modal id="modal"></tab-native-data-modal>
            <lit-table id="tb-filesystem-calltree" style="height: auto" tree>
                <lit-table-column class="nm-call-tree-column" width="60%" title="Symbol Name" data-index="symbolName" key="symbolName"  align="flex-start">
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="Size" data-index="heapSizeStr" key="heapSizeStr"  align="flex-start" order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="%" data-index="heapPercent" key="heapPercent" align="flex-start"  order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="Count" data-index="countValue" key="countValue" align="flex-start" order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="%" data-index="countPercent" key="countPercent" align="flex-start" order>
                </lit-table-column>
                <lit-table-column class="nm-call-tree-column" width="1fr" title="  " data-index="type" key="type"  align="flex-start" >
                    <template>
                        <img src="img/library.png" size="20" v-if=" type == 1 ">
                        <img src="img/function.png" size="20" v-if=" type == 0 ">
                        <div v-if=" type == - 1 "></div>
                    </template>
                </lit-table-column>
            </lit-table>
            
        </div>
        <lit-slicer-track class="nm-call-tree-slicer-track" ></lit-slicer-track>
        <lit-table id="tb-filesystem-list" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
            <span slot="head">Heaviest Stack Trace</span>
            <lit-table-column class="nm-call-tree-column" width="30px" title="" data-index="type" key="type"  align="flex-start" >
                <template>
                    <img src="img/library.png" size="20" v-if=" type == 1 ">
                    <img src="img/function.png" size="20" v-if=" type == 0 ">
                </template>
            </lit-table-column>
            <lit-table-column class="nm-call-tree-column" width="1fr" title="" data-index="symbolName" key="symbolName"  align="flex-start"></lit-table-column>
        </lit-table>
        </div>
        </lit-slicer>
     </selector>
     <tab-pane-filter id="nm-call-tree-filter" first second icon ></tab-pane-filter>
     <lit-progress-bar class="progress nm-call-tree-progress"></lit-progress-bar>
    <selector class="nm-call-tree-selector" id='show_chart'>
        <tab-framechart id='framechart' style='width: 100%;height: auto'> </tab-framechart>
    </selector>  
    <div class="loading nm-call-tree-loading"></div>
    </div>"
`);
  });
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
    expect(tabPaneNMCallTree.showButtomMenu()).toBeUndefined();
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
    expect(tabPaneNMCallTree.showButtomMenu(isShow)).toBeUndefined();
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
  it('TabPaneNMCallTreeTest11', function () {
    let data = [
      { callTreeConstraints:{
          inputs:[1]
        }
        , dataMining: 20, callTree: [] ,icon : 'block'},
      { callTreeConstraints:{
          inputs:[1]
        }, dataMining: 21, callTree: [] ,icon : 'block'},
      { callTreeConstraints:{
          inputs:[1]
        }, dataMining: 31, callTree: [] ,icon : 'block'},
    ];
    expect(tabPaneNMCallTree.switchFlameChart(data)).toBeUndefined();
  });
  it('TabPaneNMCallTreeTest12', function () {
    expect(tabPaneNMCallTree.initFilterTypes()).toBeUndefined();
  });
  it('TabPaneNMCallTreeTest13', function () {
    let data = [
      { id: 0, count: 20, children: [] },
      { id: 1, count: 21, children: [] },
      { id: 2, count: 31, children: [] },
    ];
    expect(tabPaneNMCallTree.setRightTableData(data)).toBeTruthy();
  });
  it('TabPaneNMCallTreeTest14', function () {
    expect(tabPaneNMCallTree.getDataByWorkerQuery({},{})).toBeUndefined();
  });

  it('TabPaneNMCallTreeTest15', function () {
    let data = [
      { callTreeConstraints:{
          inputs:[1]
        }
        , dataMining: 20, callTree: [] ,icon : 'tree'},
      { callTreeConstraints:{
          inputs:[1]
        }, dataMining: 21, callTree: [] ,icon : 'tree'},
      { callTreeConstraints:{
          inputs:[1]
        }, dataMining: 31, callTree: [] ,icon : 'tree'},
    ];
    expect(tabPaneNMCallTree.switchFlameChart(data)).toBeUndefined();
  });
});
