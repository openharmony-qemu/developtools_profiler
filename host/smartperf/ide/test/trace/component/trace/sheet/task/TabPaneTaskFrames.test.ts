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
import { TabPaneTaskFrames } from '../../../../../../dist/trace/component/trace/sheet/task/TabPaneTaskFrames.js';
// @ts-ignore
import { FuncStruct } from '../../../../../../dist/trace/database/ui-worker/ProcedureWorkerFunc.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../../dist/trace/component/SpSystemTrace.js';
import { queryTaskListByExecuteTaskIds } from '../../../../../../src/trace/database/SqlLite.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

const sqlite = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneTaskFrames Test', () => {
  let tabPaneTaskFrames = new TabPaneTaskFrames();
  let mockQueryTaskPoolTotalNum = sqlite.queryTaskPoolTotalNum;
  mockQueryTaskPoolTotalNum.mockResolvedValue([
    {
      tid: 1001,
    },
    {
      tid: 1002,
    },
    {
      tid: 1003,
    },
  ]);

  let mockQueryConcurrencyTask = sqlite.queryConcurrencyTask;
  mockQueryConcurrencyTask.mockResolvedValue([
    {
      tid: 28573,
      ipid: 33,
      funName: 'H:Task Perform: taskId : 2, executeId : 2',
      startTs: 26020542893000,
      dur: 4999979000,
      id: 310,
      priority: 1,
      allocationTaskRow: 294,
      executeTaskRow: 310,
      returnTaskRow: 785,
      executeId: 2,
    },
    {
      tid: 28599,
      ipid: 33,
      funName: 'H:Task Perform: taskId : 3, executeId : 3',
      startTs: 26020550419000,
      dur: 5001432000,
      id: 321,
      priority: 1,
      allocationTaskRow: 297,
      executeTaskRow: 321,
      returnTaskRow: 797,
      executeId: 3,
    },
    {
      tid: 28600,
      ipid: 33,
      funName: 'H:Task Perform: taskId : 4, executeId : 4',
      startTs: 26020550826000,
      dur: 5001023000,
      id: 331,
      priority: 1,
      allocationTaskRow: 300,
      executeTaskRow: 331,
      returnTaskRow: 799,
      executeId: 4,
    },
  ]);

  SpSystemTrace.DATA_TASK_POOL_CALLSTACK = new Map();
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(87, {
    id: 87,
    ts: 26015536047000,
    dur: 168000,
    callid: 63,
    cat: null,
    identify: 7,
    name: 'H:Task Allocation: taskId : 1, executeId : 1, priority : 1, executeState : 1',
    depth: 3,
    cookie: null,
    parent_id: 86,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(99, {
    id: 99,
    ts: 26015539134000,
    dur: 5001738000,
    callid: 111,
    cat: null,
    identify: 10,
    name: 'H:Task Perform: taskId : 1, executeId : 1',
    depth: 1,
    cookie: null,
    parent_id: 95,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(292, {
    id: 292,
    ts: 26020541020000,
    dur: 2526000,
    callid: 63,
    cat: null,
    identify: 15,
    name: 'H:Task PerformTask End: taskId : 1, executeId : 1, performResult : IsCanceled',
    depth: 1,
    cookie: null,
    parent_id: 291,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(294, {
    id: 294,
    ts: 26020541698000,
    dur: 44000,
    callid: 63,
    cat: null,
    identify: 7,
    name: 'H:Task Allocation: taskId : 2, executeId : 2, priority : 1, executeState : 1',
    depth: 3,
    cookie: null,
    parent_id: 293,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(297, {
    id: 297,
    ts: 26020541828000,
    dur: 196000,
    callid: 63,
    cat: null,
    identify: 7,
    name: 'H:Task Allocation: taskId : 3, executeId : 3, priority : 1, executeState : 1',
    depth: 3,
    cookie: null,
    parent_id: 295,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(300, {
    id: 300,
    ts: 26020542123000,
    dur: 572000,
    callid: 63,
    cat: null,
    identify: 7,
    name: 'H:Task Allocation: taskId : 4, executeId : 4, priority : 1, executeState : 1',
    depth: 3,
    cookie: null,
    parent_id: 298,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(310, {
    id: 310,
    ts: 26020542893000,
    dur: 4999979000,
    callid: 111,
    cat: null,
    identify: 10,
    name: 'H:Task Perform: taskId : 2, executeId : 2',
    depth: 1,
    cookie: null,
    parent_id: 309,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(321, {
    id: 321,
    ts: 26020550419000,
    dur: 5001432000,
    callid: 152,
    cat: null,
    identify: 10,
    name: 'H:Task Perform: taskId : 3, executeId : 3',
    depth: 1,
    cookie: null,
    parent_id: 320,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(331, {
    id: 331,
    ts: 26020550826000,
    dur: 5001023000,
    callid: 153,
    cat: null,
    identify: 10,
    name: 'H:Task Perform: taskId : 4, executeId : 4',
    depth: 1,
    cookie: null,
    parent_id: 329,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(785, {
    id: 785,
    ts: 26025543022000,
    dur: 30000,
    callid: 63,
    cat: null,
    identify: 4,
    name: 'H:Task PerformTask End: taskId : 2, executeId : 2, performResult : Successful',
    depth: 1,
    cookie: null,
    parent_id: 784,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(797, {
    id: 797,
    ts: 26025551976000,
    dur: 28000,
    callid: 63,
    cat: null,
    identify: 4,
    name: 'H:Task PerformTask End: taskId : 3, executeId : 3, performResult : Successful',
    depth: 1,
    cookie: null,
    parent_id: 794,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });
  SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(799, {
    id: 799,
    ts: 26025552025000,
    dur: 13000,
    callid: 63,
    cat: null,
    identify: 4,
    name: 'H:Task PerformTask End: taskId : 4, executeId : 4, performResult : Successful',
    depth: 1,
    cookie: null,
    parent_id: 798,
    argsetid: null,
    chainId: null,
    spanId: null,
    parentSpanId: null,
    flag: null,
    args: null,
  });

  it('TabPaneTaskFrames Test01', function () {
    TabPaneTaskFrames.TaskArray = [
      {
        startTs: 5628901000,
        dur: 4999979000,
        funName: 'H:Task Perform: taskId : 2, executeId : 2',
        argsetid: null,
        depth: 1,
        id: 310,
        frame: {
          x: 526,
          y: 20,
          width: 469,
          height: 20,
        },
        textMetricsWidth: 185.634765625,
      },
      {
        startTs: 5627706000,
        dur: 44000,
        funName: 'H:Task Allocation: taskId : 2, executeId : 2, priority : 1, executeState : 1',
        argsetid: null,
        depth: 3,
        id: 294,
        frame: {
          x: 526,
          y: 60,
          width: 1,
          height: 20,
        },
      },
      {
        startTs: 10629030000,
        dur: 30000,
        funName: 'H:Task PerformTask End: taskId : 2, executeId : 2, performResult : Successful',
        argsetid: null,
        depth: 1,
        id: 785,
        frame: {
          x: 995,
          y: 20,
          width: 1,
          height: 20,
        },
      },
    ];

    tabPaneTaskFrames.taskFramesTbl = jest.fn(() => true);
    tabPaneTaskFrames.taskFramesTbl!.recycleDataSource = jest.fn(() => true);
    let frameData = {};
    tabPaneTaskFrames.data = frameData;
    expect(tabPaneTaskFrames.data).toBeUndefined();
  });

  it('TabPaneTaskFrames Test02', function () {
    TabPaneTaskFrames.TaskArray = [];
    let mockQueryTaskListByExecuteTaskIds = sqlite.queryTaskListByExecuteTaskIds;
    mockQueryTaskListByExecuteTaskIds.mockResolvedValue([
      {
        ipid: 33,
        allocationTaskRow: 294,
        executeTaskRow: 310,
        returnTaskRow: 785,
        executeId: 2,
        priority: 1,
      },
      {
        ipid: 36,
        allocationTaskRow: 300,
        executeTaskRow: 331,
        returnTaskRow: 799,
        executeId: 4,
        priority: 1,
      },
    ]);
    let frameData1 = {
      recordStartNs: 26014913992000,
      leftNs: 7957675110,
      rightNs: 8705376127,
      taskFramesData: [
        {
          startTs: 5628901000,
          dur: 4999979000,
          funName: 'H:Task Perform: taskId : 2, executeId : 2',
          argsetid: null,
          depth: 1,
          id: 310,
          itid: 111,
          ipid: 33,
          frame: {
            x: 526,
            y: 20,
            width: 469,
            height: 20,
          },
          textMetricsWidth: 185.634765625,
          tid: 28573,
        },
        {
          startTs: 5636834000,
          dur: 5001023000,
          funName: 'H:Task Perform: taskId : 4, executeId : 4',
          argsetid: null,
          depth: 1,
          id: 331,
          itid: 153,
          ipid: 33,
          frame: {
            x: 527,
            y: 20,
            width: 469,
            height: 20,
          },
          textMetricsWidth: 185.634765625,
          tid: 28600,
        },
      ],
    };
    tabPaneTaskFrames.taskFramesTbl = jest.fn(() => true);
    tabPaneTaskFrames.taskFramesTbl!.recycleDataSource = jest.fn(() => true);
    tabPaneTaskFrames.data = frameData1;
    expect(tabPaneTaskFrames.data).toBeUndefined();
  });

  it('TabPaneTaskFrames Test03', function () {
    TabPaneTaskFrames.TaskArray = [];
    let frameData = {
      recordStartNs: 26014913992000,
      leftNs: 96132987,
      rightNs: 2563546344,
      taskFramesData: [
        {
          startTs: 625142000,
          dur: 5001738000,
          funName: 'H:Task Perform: taskId : 1, executeId : 1',
          argsetid: null,
          depth: 1,
          id: 99,
          itid: 111,
          ipid: 33,
          frame: {
            x: 58,
            y: 20,
            width: 469,
            height: 20,
          },
          textMetricsWidth: 185.634765625,
          tid: 28573,
        },
      ],
    };
    let mockQueryTaskListByExecuteTaskIds = sqlite.queryTaskListByExecuteTaskIds;
    mockQueryTaskListByExecuteTaskIds.mockResolvedValue([
      {
        allocationTaskRow: 87,
        executeTaskRow: 99,
        returnTaskRow: 292,
        executeId: 1,
        priority: 1,
      },
    ]);
    tabPaneTaskFrames.taskFramesTbl = jest.fn(() => true);
    tabPaneTaskFrames.taskFramesTbl!.recycleDataSource = jest.fn(() => true);
    tabPaneTaskFrames.data = frameData;
    expect(tabPaneTaskFrames.data).toBeUndefined();
  });

  it('TabPaneTaskFrames Test04', function () {
    TabPaneTaskFrames.TaskArray = [
      {
        startTs: 10628950000,
        dur: 49911000,
        funName: 'H:Task Perform: taskId : 6, executeId : 8',
        argsetid: null,
        depth: 1,
        id: 782,
        frame: {
          x: 710,
          y: 20,
          width: 304,
          height: 20,
        },
        tid: 28573,
        textMetricsWidth: 185.634765625,
      },
      {
        startTs: 6631912000,
        dur: 74000,
        funName: 'H:Task Allocation: taskId : 6, executeId : 8, priority : 0, executeState : 1',
        argsetid: null,
        depth: 2,
        id: 372,
        frame: {
          x: 620,
          y: 40,
          width: 1,
          height: 20,
        },
      },
      {
        startTs: 10679004000,
        dur: 56000,
        funName: 'H:Task PerformTask End: taskId : 6, executeId : 8, performResult : Successful',
        argsetid: null,
        depth: 1,
        id: 806,
        frame: {
          x: 999,
          y: 20,
          width: 1,
          height: 20,
        },
      },
    ];
    TabPaneTaskFrames.IsShowConcurrency = true;
    tabPaneTaskFrames.taskFramesTbl = jest.fn(() => true);
    tabPaneTaskFrames.taskFramesTbl!.recycleDataSource = jest.fn(() => true);
    let frameData = {};
    tabPaneTaskFrames.data = frameData;
    expect(tabPaneTaskFrames.data).toBeUndefined();
  });

  it('TabPaneTaskFrames Test05', function () {
    tabPaneTaskFrames.taskFramesTbl = jest.fn(() => true);
    tabPaneTaskFrames.taskFramesTbl!.recycleDataSource = jest.fn(() => true);
    expect(tabPaneTaskFrames.sortByColumn({ sort: 1, key: 'taskPriority' })).toBeUndefined();
  });
});
