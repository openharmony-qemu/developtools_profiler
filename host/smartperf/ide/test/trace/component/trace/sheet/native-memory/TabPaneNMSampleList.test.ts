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
import { TabPaneNMSampleList } from '../../../../../../dist/trace/component/trace/sheet/native-memory/TabPaneNMSampleList.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table';
// @ts-ignore
import { NativeHookSampleQueryInfo, NativeHookSamplerInfo } from '../../../../../../dist/trace/bean/NativeHook.js';
// @ts-ignore
import { NativeMemory } from '../../../../../../dist/trace/bean/NativeHook.js';
// @ts-ignore
import { queryAllHookData } from '../../../../../../dist/trace/database/SqlLite.js';
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
describe('TabPaneNMSampleList Test', () => {
  document.body.innerHTML = '<tabpane-native-sample id="ddt"></tabpane-native-sample>';
  let tabPaneNMSampleList = document.querySelector<TabPaneNMSampleList>('#ddt');

  TabPaneNMSampleList.source = [
    {
      current: '',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: '',
      growth: '',
      total: 0,
      totalGrowth: '',
      existing: 0,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: -1,
    },
  ];
  TabPaneNMSampleList.filterSelect = '0';

  tabPaneNMSampleList.currentSelection = jest.fn(() => true);
  let dat = {
    cpus: [],
    threadIds: [],
    trackIds: [],
    funTids: [],
    heapIds: [],
    nativeMemory: [],
    leftNs: 0,
    rightNs: 0,
    hasFps: false,
    statisticsSelectData: undefined,
  };

  let NativeHookSnapshotTypes = [
    {
      eventId: -1,
      current: 0,
      eventType: '',
      subType: '',
      growth: 0,
      existing: 0,
      addr: '',
      startTs: 0,
      endTs: 0,
      total: 0,
      children: [],
    },
  ];

  let MockNativeHookSnapshotTypes = sqlit.queryNativeHookSnapshotTypes;

  MockNativeHookSnapshotTypes.mockResolvedValue([new NativeHookSampleQueryInfo()]);


  let samplerInfo = [
    {
      current: '',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: '',
      growth: '',
      total: 0,
      totalGrowth: '',
      existing: 0,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: -1,
      threadId: 0,
      threadName: '',
    },
    {
      current: '',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: '',
      growth: '',
      total: 0,
      totalGrowth: '',
      existing: 0,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: -1,
      threadId: 0,
      threadName: '',
    }
  ];

  tabPaneNMSampleList.data = dat;
  it('TabPaneNMSampleListTest01', function () {
    expect(TabPaneNMSampleList.serSelection(dat)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest02', function () {
    let sampleData = new NativeMemory();

    let MockqueryAllHookData = sqlit.queryAllHookData;
    MockqueryAllHookData.mockResolvedValue([new NativeHookSampleQueryInfo()]);

    expect(TabPaneNMSampleList.addSampleData(sampleData)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest04', function () {
    let snapshot = {
      current: '',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: '',
      growth: '',
      total: 0,
      totalGrowth: '',
      existing: 0,
      children: [],
      tempList: [],
      timestamp: '',
      eventId: -1,
    };

    let snapshotLeft = {
      current: '',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: '',
      growth: '',
      total: 0,
      totalGrowth: '',
      existing: 0,
      children: [snapshot],
      tempList: [],
      timestamp: '',
      eventId: -1,
    };

    let snapshotRight = {
      current: '',
      currentSize: 0,
      startTs: 0,
      heapSize: 0,
      snapshot: '',
      growth: '',
      total: 0,
      totalGrowth: '',
      existing: 0,
      children: [snapshot],
      tempList: [],
      timestamp: '',
      eventId: -1,
    };
    expect(TabPaneNMSampleList.prepChild(snapshotLeft, snapshotRight)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest10', function () {
    expect(TabPaneNMSampleList.data).toBeUndefined();
  });

  it('TabPaneNMSampleListTest11', function () {
    expect(TabPaneNMSampleList.initTypes()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest05', function () {
    expect(tabPaneNMSampleList.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        .nm-sample-tbl {
            height: auto;
        }
        :host{
            padding: 10px 10px 0 10px;
            display: flex;
            flex-direction: column;
        }
        </style>
        <lit-slicer style="width:100%">
        <div class="nm-sample-content" style="width: 65%">
            <lit-table id="tb-native-sample" class="nm-sample-tbl" tree>
                <lit-table-column class="nm-sample-column" width="25%" title="Snapshot" data-index="snapshot" key="snapshot"  align="flex-start" >
                </lit-table-column>
                <lit-table-column class="nm-sample-column" width="1fr" title="Timestamp" data-index="timestamp" key="timestamp"  align="flex-start"  >
                </lit-table-column>
                <lit-table-column class="nm-sample-column" width="1fr" title="Net Growth" data-index="growth" key="growth"  align="flex-start"  >
                </lit-table-column>
                <lit-table-column class="nm-sample-column" width="1fr" title="Total Growth" data-index="totalGrowth" key="totalGrowth"  align="flex-start"  >
                </lit-table-column>
                <lit-table-column class="nm-sample-column" width="1fr" title="# Existing" data-index="existing" key="existing"  align="flex-start"  >
                </lit-table-column>
            </lit-table>
            <tab-pane-filter id="filter" first></tab-pane-filter>
        </div>
        <lit-slicer-track ></lit-slicer-track>
        <lit-table id="tb-native-data" no-head style="height: auto;border-left: 1px solid var(--dark-border1,#e2e2e2)" hideDownload>
            <lit-table-column class="nm-sample-column" width="80px" title="" data-index="type" key="type"  align="flex-start" >
                <template>
                    <div v-if=" type == -1 ">Thread:</div>
                    <img src="img/library.png" size="20" v-if=" type == 1 ">
                    <img src="img/function.png" size="20" v-if=" type == 0 ">
                </template>
            </lit-table-column>
            <lit-table-column class="nm-sample-column" width="1fr" title="" data-index="title" key="title"  align="flex-start">
            </lit-table-column>
        </lit-table>
        </lit-slicer>
        "
`);
  });

  it('TabPaneNMSampleListTest09', function () {
    let rootSample = new NativeHookSamplerInfo();

    let merageSample = {
      growth: 1,
      endTs: 2,
      startTs: 2,
      addr: '1',
      eventId: 0,
    };
    expect(TabPaneNMSampleList.merageSampleData(1, 1, rootSample, merageSample)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest12', function () {
    let rootSample = new NativeHookSamplerInfo();

    let MockqueryAllHookData = sqlit.queryAllHookData;

    MockqueryAllHookData.mockResolvedValue([
      {
        eventId: 1,
        eventType: 'aa',
        subType: 1,
        addr: 'aaaa',
        growth: 2,
        startTs: 11111,
        endTs: 211111,
      },
    ]);
    expect(TabPaneNMSampleList.queryAllHookInfo(dat, rootSample)).toBeUndefined();
  });

  it('TabPaneNMSampleListTest13', function () {
    TabPaneNMSampleList.samplerInfoSource = [{
      current:  '',
      currentSize:  0,
      startTs:  0,
      heapSize:  0,
      snapshot:  '',
      growth:  '',
      total:  0,
      totalGrowth:  '',
      existing:  0,
      children: samplerInfo,
      tempList: samplerInfo,
      timestamp:  '',
      eventId:  -1,
      threadId:  0,
      threadName:  '',
    }]
    TabPaneNMSampleList.filterSelect = '0';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest14', function () {
    TabPaneNMSampleList.samplerInfoSource = [{
      current:  '',
      currentSize:  0,
      startTs:  0,
      heapSize:  0,
      snapshot:  '',
      growth:  '',
      total:  0,
      totalGrowth:  '',
      existing:  0,
      children: samplerInfo,
      tempList: samplerInfo,
      timestamp:  '',
      eventId:  -1,
      threadId:  0,
      threadName:  '',
    }]
    TabPaneNMSampleList.filterSelect = '1';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest15', function () {
    TabPaneNMSampleList.samplerInfoSource = [{
      current:  '',
      currentSize:  0,
      startTs:  0,
      heapSize:  0,
      snapshot:  '',
      growth:  '',
      total:  0,
      totalGrowth:  '',
      existing:  0,
      children: [],
      tempList: [],
      timestamp:  '',
      eventId:  -1,
      threadId:  0,
      threadName:  '',
    }]
    TabPaneNMSampleList.filterSelect = '1';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest16', function () {
    TabPaneNMSampleList.samplerInfoSource = [{
      current:  '',
      currentSize:  0,
      startTs:  0,
      heapSize:  0,
      snapshot:  '',
      growth:  '',
      total:  0,
      totalGrowth:  '',
      existing:  0,
      children: samplerInfo,
      tempList: samplerInfo,
      timestamp:  '',
      eventId:  -1,
      threadId:  0,
      threadName:  '',
    }]
    TabPaneNMSampleList.filterSelect = '2';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });

  it('TabPaneNMSampleListTest17', function () {
    TabPaneNMSampleList.samplerInfoSource = [{
      current:  '',
      currentSize:  0,
      startTs:  0,
      heapSize:  0,
      snapshot:  '',
      growth:  '',
      total:  0,
      totalGrowth:  '',
      existing:  0,
      children: [],
      tempList: [],
      timestamp:  '',
      eventId:  -1,
      threadId:  0,
      threadName:  '',
    }]
    TabPaneNMSampleList.filterSelect = '2';
    expect(tabPaneNMSampleList.filterAllList()).toBeUndefined();
  });


});
