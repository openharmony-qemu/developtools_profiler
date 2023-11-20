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
import { TabPaneHiLogSummary } from '../../../../../../dist/trace/component/trace/sheet/hilog/TabPaneHiLogSummary.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneHilogSummary Test', () => {
  let summaryTab = new TabPaneHiLogSummary();

  let summaryData = {
    leftNs: 0,
    rightNs: 33892044011,
    hiLogs: [{
      id: 2,
      pid: 1119,
      tid: 1172,
      processName: 'process1119',
      startTs: 33872275426,
      level: 'I',
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: upd',
      time: 0,
      depth: 0,
      dur: 0,
    },{
      id: 3,
      pid: 1119,
      tid: 1172,
      processName: 'process1119',
      startTs: 33874375717,
      level: 'W',
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: upd',
      time: 0,
      depth: 0,
      dur: 0,
    },{
      id: 4,
      pid: 1119,
      tid: 1172,
      processName: 'process1119',
      startTs: 33878711051,
      level: 'D',
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: upd',
      time: 0,
      depth: 0,
      dur: 0,
    },{
      id: 5,
      pid: 1119,
      tid: 1172,
      processName: 'process1119',
      startTs: 33885632885,
      level: 'E',
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: upd',
      time: 0,
      depth: 0,
      dur: 0,
    },{
      id: 6,
      pid: 1119,
      tid: 1172,
      processName: 'process1119',
      startTs: 33889724969,
      level: 'F',
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: upd',
      time: 0,
      depth: 0,
      dur: 0,
    },{
      id: 7,
      pid: 1119,
      tid: 1172,
      processName: 'process1119',
      startTs: 33892044011,
      level: 'A',
      tag: 'C02d0c/Hiprofiler',
      context: 'ParseTimeExtend: upd',
      time: 0,
      depth: 0,
      dur: 0,
    }]
  };

  it('TabPaneHilogSummaryTest01', function () {
    summaryTab.data = summaryData;
    expect(summaryTab.data).toBeUndefined();
  });
});
