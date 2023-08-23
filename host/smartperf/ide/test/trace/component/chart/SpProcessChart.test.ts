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
import { SpProcessChart } from '../../../../dist/trace/component/chart/SpProcessChart.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../dist/trace/component/SpSystemTrace.js';
import {
  getMaxDepthByTid,
  queryAllActualData,
  queryAllExpectedData,
  queryAllJankProcess,
} from '../../../../src/trace/database/SqlLite.js';
// @ts-ignore
import { SpChartManager } from '../../../../dist/trace/component/chart/SpChartManager.js';
const sqlit = require('../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../dist/trace/database/SqlLite.js');

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpProcessChart Test', () => {
  let MockqueryProcessAsyncFunc = sqlit.queryProcessAsyncFunc;

  MockqueryProcessAsyncFunc.mockResolvedValue([
    {
      tid: 1,
      pid: 1,
      threadName: '1',
      track_id: 3,
      startTs: 1111,
      dur: 2000000,
      funName: 'deliverInputEvent',
      parent_id: 4,
      id: 5,
      cookie: 'ff',
      depth: 5,
      argsetid: 6,
    },
  ]);
  let processContentCount = sqlit.queryProcessContentCount;
  processContentCount.mockResolvedValue([
    {
      pid: 1,
      switch_count: 2,
      thread_count: 3,
      slice_count: 4,
      mem_count: 5,
    },
  ]);
  let queryProcessThreads = sqlit.queryProcessThreads;
  queryProcessThreads.mockResolvedValue([
    {
      utid: 1,
      hasSched: 0,
      pid: 1,
      tid: 4,
      processName: 'process',
      threadName: 'thread',
    },
  ]);
  let queryProcessThreadsByTable = sqlit.queryProcessThreadsByTable;
  queryProcessThreadsByTable.mockResolvedValue([
    {
      pid: 1,
      tid: 0,
      processName: 'process',
      threadName: 'thread',
    },
  ]);
  let getAsyncEvents = sqlit.getAsyncEvents;
  getAsyncEvents.mockResolvedValue([
    {
      pid: 1,
      startTime: 100000,
    },
  ]);
  let queryProcessMem = sqlit.queryProcessMem;
  queryProcessMem.mockResolvedValue([
    {
      trackId: 1,
      trackName: 'trackName',
      upid: 2,
      pid: 1,
      processName: 'processName',
    },
  ]);
  let queryEventCountMap = sqlit.queryEventCountMap;
  queryEventCountMap.mockResolvedValue([
    {
      eventName: 'eventName',
      count: 1,
    },
  ]);
  let queryProcess = sqlit.queryProcess;
  queryProcess.mockResolvedValue([
    {
      pid: 1,
      processName: 'processName',
    },
  ]);

  let queryProcessByTable = sqlit.queryProcessByTable;
  queryProcessByTable.mockResolvedValue([
    {
      pid: 2,
      processName: 'processName',
    },
  ]);

  let getMaxDepthByTid = sqlit.getMaxDepthByTid;
  getMaxDepthByTid.mockResolvedValue([
    {
      tid: 1,
      maxDepth: 1,
    },
    {
      tid: 2,
      maxDepth: 2,
    },
  ]);
  let queryAllJankProcess = sqlit.queryAllJankProcess;
  queryAllJankProcess.mockResolvedValue([
    {
      pid: 1,
    },
  ]);

  let queryAllExpectedData = sqlit.queryAllExpectedData;
  queryAllExpectedData.mockResolvedValue([
    {
      id: 41,
      ts: 749660047,
      name: 1159,
      type: 1,
      dur: 16657682,
      pid: 1242,
      cmdline: 'render_service',
    },
    {
      id: 45,
      ts: 766321174,
      name: 1160,
      type: 1,
      dur: 16657682,
      pid: 1,
      cmdline: 'render_service',
    },
  ]);

  let queryAllActualData = sqlit.queryAllActualData;
  queryAllActualData.mockResolvedValue([
    {
      id: 40,
      ts: 750328000,
      name: 1159,
      type: 0,
      dur: 22925000,
      src_slice: '36',
      jank_tag: 1,
      dst_slice: null,
      pid: 1242,
      cmdline: 'render_service',
      frame_type: 'render_service',
    },
    {
      id: 44,
      ts: 773315000,
      name: 1160,
      type: 0,
      dur: 17740000,
      src_slice: '38,42',
      jank_tag: 1,
      dst_slice: null,
      pid: 1,
      cmdline: 'render_service',
      frame_type: 'render_service',
    },
  ]);

  let spSystemTrace = new SpSystemTrace();
  let spProcessChart = new SpProcessChart(spSystemTrace);
  it('SpProcessChart01', function () {
    spProcessChart.initAsyncFuncData();
    spProcessChart.init();
    expect(spProcessChart).toBeDefined();
  });

  it('SpProcessChart02', function () {
    expect(spProcessChart.initAsyncFuncData()).not.toBeUndefined();
  });

  it('SpProcessChart03', function () {
    spProcessChart.initAsyncFuncData();
    spProcessChart.initDeliverInputEvent();
    expect(spProcessChart.processAsyncFuncArray.length).toEqual(1);
  });
});
