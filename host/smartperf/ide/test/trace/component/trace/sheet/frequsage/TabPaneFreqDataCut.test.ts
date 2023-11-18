/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
import { TabPaneFreqDataCut } from '../../../../../../dist/trace/component/trace/sheet/frequsage/TabPaneFreqDataCut.js';

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/base-ui/table/lit-table.js', () => {
  return {
    snapshotDataSource: () => {
    },
    removeAttribute: () => {
    },
  };
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('TabPaneFreqDataCut Test', () => {
  let data = {
    leftNs: 0,
    rightNs: 0,
    processIds: [1, 2],
  };
  let getTabRunningPercent = sqlite.getTabRunningPercent;
  getTabRunningPercent.mockResolvedValue([
    {
      pid: 1,
      tid: 1,
      state: 'Running',
      cpu: 0,
      dur: 0,
      ts: 1,
    }
  ]);
  let queryCpuFreqFilterId = sqlite.queryCpuFreqFilterId;
  queryCpuFreqFilterId.mockResolvedValue([{
    id: 1,
    cpu: 0,
  }]);
  let queryCpuFreqUsageData = sqlite.queryCpuFreqUsageData;
  queryCpuFreqUsageData.mockResolvedValue([{
    value: '',
    dur: '',
    startNS: 0,
    filter_id: 1,
  }]);
  it('TabPaneFreqDataCutTest01 ', function () {
    let tabPaneFreqDataCut = new TabPaneFreqDataCut();
    tabPaneFreqDataCut.data = data;
    expect(tabPaneFreqDataCut.data).toBeUndefined();
  });
  it('TabPaneFreqDataCutTest02', () => {
    let tabPaneFreqDataCut = new TabPaneFreqDataCut();
    const arr = [
      {percent: 50, children: []},
      {percent: 75.123456, children: [{percent: 80, children: []}]},
    ];
    tabPaneFreqDataCut.fixedDeal(arr);
    expect(arr[0].percent).toBe('50.00');
    expect(arr[1].percent).toBe('75.12');
    expect(arr[1].children[0].percent).toBe('80.00');
  });
  it('TabPaneFreqDataCutTest03', () => {
    let tabPaneFreqDataCut = new TabPaneFreqDataCut();
    const threadArr = [
      {pid: 1, tid: 1, children: []},
      {pid: 2, tid: 2, children: []},
    ];
    const totalData = [
      {pid: 1, tid: 1},
    ];
    tabPaneFreqDataCut.mergeTotalData(threadArr, totalData);
    expect(threadArr).toEqual([
      {pid: 1, tid: 1, children: [{pid: 1, tid: 1, thread: 'TotalData'}]},
      {pid: 2, tid: 2, children: []},
    ]);
  });
  it('TabPaneFreqDataCutTest04 ', function () {
    let tabPaneFreqDataCut = new TabPaneFreqDataCut();
    const obj = {
      children: [],
      count: 0,
      dur: 0,
      percent: 0
    };
    const arr = [
      {count: 1, dur: 10, percent: 10},
      {count: 2, dur: 20, percent: 20},
      {count: 0, dur: 0, percent: 0},
      {count: 3, dur: 30, percent: 30}
    ];
    tabPaneFreqDataCut.mergeCycleData(obj, arr);
    expect(obj.children.length).toBe(3);
    expect(obj.children).toContainEqual({count: 1, dur: 10, percent: 10});
    expect(obj.children).toContainEqual({count: 2, dur: 20, percent: 20});
    expect(obj.children).toContainEqual({count: 3, dur: 30, percent: 30});
    expect(obj.count).toBe(6);
    expect(obj.dur).toBe(60);
    expect(obj.percent).toBe(60);
  });
  it('TabPaneFreqDataCutTest05 ', function () {
    let tabPaneFreqDataCut = new TabPaneFreqDataCut();
    const resList = [
      {cpu: 'A', freq: 1, id: 1, dur: 10, percent: 50, count: 1},
      {cpu: 'A', freq: 1, id: 1, dur: 20, percent: 30, count: 2},
      {cpu: 'B', freq: 2, id: 2, dur: 15, percent: 40, count: 1},
      {cpu: 'B', freq: 2, id: 2, dur: 25, percent: 20, count: 2},
    ];
    tabPaneFreqDataCut.mergeData(resList);
    expect(resList).toEqual([
      {cpu: 'A', freq: 1, id: 1, dur: 30, percent: 80, count: 3},
      {cpu: 'B', freq: 2, id: 2, dur: 40, percent: 60, count: 3}
    ]);
  });
});