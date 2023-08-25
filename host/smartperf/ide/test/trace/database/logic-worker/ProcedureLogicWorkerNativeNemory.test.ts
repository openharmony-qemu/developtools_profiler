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

//@ts-ignore
import {
  HeapStruct,
  NativeEvent,
  NativeHookCallInfo,
  NativeHookStatistics,
  NativeMemory,
  ProcedureLogicWorkerNativeMemory,
  StatisticsSelection,
  //@ts-ignore
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerNativeNemory.js';

//@ts-ignore
import {
  HeapTreeDataBean,
  //@ts-ignore
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerCommon.js';

describe('ProcedureLogicWorkerNativeNemory Test', () => {
  let procedureLogicWorkerNativeMemory = new ProcedureLogicWorkerNativeMemory();
  procedureLogicWorkerNativeMemory.resolvingActionCallInfo = jest.fn(() => true);
  procedureLogicWorkerNativeMemory.resolvingActionCallInfo.dataSource = jest.fn(() => true);
  procedureLogicWorkerNativeMemory.resolvingActionCallInfo.dataSource.map = jest.fn(() => true);
  it('ProcedureLogicWorkerNativeNemoryTest01', function () {
    expect(procedureLogicWorkerNativeMemory).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest02', function () {
    let nativeHookStatistics = new NativeHookStatistics();
    nativeHookStatistics = {
      eventId: 0,
      eventType: '',
      subType: '',
      subTypeId: 0,
      heapSize: 0,
      addr: '',
      startTs: 0,
      endTs: 0,
      sumHeapSize: 0,
      max: 0,
      count: 0,
      tid: 0,
      threadName: '',
      sSelected: false,
    };
    expect(nativeHookStatistics).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest03', function () {
    let nativeHookCallInfo = new NativeHookCallInfo();
    expect(nativeHookCallInfo).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest04', function () {
    let heapTreeDataBean = new HeapTreeDataBean();
    heapTreeDataBean = {
      symbolId: 0,
      fileId: 0,
      startTs: 0,
      endTs: 0,
      depth: 0,
      heapSize: 0,
      eventId: '',
    };
    expect(heapTreeDataBean).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest05', function () {
    let nativeMemory = new NativeMemory();
    nativeMemory = {
      index: 0,
      eventId: 0,
      eventType: '',
      subType: '',
      addr: '',
      startTs: 0,
      endTs: 0,
      timestamp: '',
      heapSize: 0,
      heapSizeUnit: '',
      symbol: '',
      library: '',
      isSelected: false,
      state: '',
      threadId: 0,
      threadName: '',
    };
    expect(nativeMemory).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest06', function () {
    let heapStruct = new HeapStruct();
    expect(heapStruct).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest07', function () {
    let nativeEvent = new NativeEvent();
    expect(nativeEvent).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest08', function () {
    let statisticsSelection = new StatisticsSelection();
    expect(statisticsSelection).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest09', function () {
    expect(procedureLogicWorkerNativeMemory.clearAll()).toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest11', function () {
    expect(procedureLogicWorkerNativeMemory.getTypeFromIndex(-1, '', '')).toBeFalsy();
  });

  it('ProcedureLogicWorkerNativeNemoryTest12', function () {
    expect(procedureLogicWorkerNativeMemory.getTypeFromIndex(0, '', '')).toBeTruthy();
  });

  it('ProcedureLogicWorkerNativeNemoryTest13', function () {
    let item = {
      eventType: 'AllocEvent',
    };
    expect(procedureLogicWorkerNativeMemory.getTypeFromIndex(1, item, '')).toBeTruthy();
  });

  it('ProcedureLogicWorkerNativeNemoryTest14', function () {
    let item = {
      eventType: 'MmapEvent',
    };
    expect(procedureLogicWorkerNativeMemory.getTypeFromIndex(2, item, '')).toBeTruthy();
  });

  it('ProcedureLogicWorkerNativeNemoryTest15', function () {
    let stack = {
      children: [],
      count: 123,
      countValue: '',
      countPercent: '69%',
      size: 32,
      threadId: 3221,
      threadName: 'AsyncTask',
      heapSizeStr: '',
      heapPercent: '56%',
      tsArray: [],
    };
    expect(procedureLogicWorkerNativeMemory.traverseTree(stack, 1)).toBeUndefined();
  });

  it('ProcedureLogicWorkerNativeNemoryTest16', function () {
    let stack = {
      children: [],
      count: 32,
      countValue: '',
      countPercent: '87%',
      size: 23,
      threadId: 332,
      threadName: 'ssioncontroller',
      heapSizeStr: '',
      heapPercent: '12%',
      tsArray: [],
    };
    expect(procedureLogicWorkerNativeMemory.traverseSampleTree(stack, 1)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest18', function () {
    window.postMessage = jest.fn(() => true);
    let data = {
      params: [
        {
          list: '',
        },
      ],
      action: '',
      id: 2,
      type: 'native-memory-init',
    };
    procedureLogicWorkerNativeMemory.clearAll = jest.fn(() => true);
    procedureLogicWorkerNativeMemory.initDataDict = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest19', function () {
    let data = {
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'native-memory-queryDataDICT',
      id: 1,
    };
    procedureLogicWorkerNativeMemory.initNMChartData = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest20', function () {
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'native-memory-queryNMChartData',
    };
    procedureLogicWorkerNativeMemory.initNMFrameData = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest21', function () {
    let data = {
      id: 3,
      params: [
        {
          list: '',
        },
      ],
      action: 'a',
      type: 'native-memory-queryNMFrameData',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerNativeMemory.initNMStack = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest22', function () {
    let data = {
      id: 4,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'native-memory-action',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerNativeMemory.resolvingAction = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest23', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.queryData()).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest25', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.initNMChartData()).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest26', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.initNMFrameData()).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest27', function () {
    window.postMessage = jest.fn(() => true);
    let frameArr = {
      map: jest.fn(() => true),
    };
    expect(procedureLogicWorkerNativeMemory.initNMStack(frameArr)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest28', function () {
    let paramMap = {
      get: jest.fn(() => 'call-info'),
    };
    expect(procedureLogicWorkerNativeMemory.resolvingAction(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest29', function () {
    let paramMap = new Map();
    paramMap.set('actionType', 'info');
    expect(procedureLogicWorkerNativeMemory.resolvingAction(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest30', function () {
    let paramMap = {
      get: jest.fn(() => 'memory-stack'),
    };
    expect(procedureLogicWorkerNativeMemory.resolvingAction(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest31', function () {
    let paramMap = {
      get: jest.fn(() => 'memory-chart'),
    };
    expect(procedureLogicWorkerNativeMemory.resolvingAction(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest32', function () {
    let paramMap = {
      get: jest.fn(() => true),
    };
    expect(procedureLogicWorkerNativeMemory.resolvingAction(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest33', function () {
    let paramMap = {
      get: jest.fn(() => 0),
    };
    expect(procedureLogicWorkerNativeMemory.resolvingActionNativeMemoryChartData(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest34', function () {
    let paramMap = {
      get: jest.fn(() => 1),
    };
    expect(procedureLogicWorkerNativeMemory.resolvingActionNativeMemoryChartData(paramMap)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest35', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.queryCallchainsSamples('', 1, 1, [''])).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest37', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.queryStatisticCallchainsSamples('', 1, 1, [''])).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest38', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.getFilterLevel(1000001)).toBe(100000);
  });
  it('procedureLogicWorkerFileSystemTest39', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.getFilterLevel(5000001)).toBe(500000);
  });
  it('procedureLogicWorkerFileSystemTest40', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.getFilterLevel(3000001)).toBe(500000);
  });
  it('procedureLogicWorkerFileSystemTest41', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.getFilterLevel(150000)).toBe(0);
  });
  it('procedureLogicWorkerFileSystemTest42', function () {
    window.postMessage = jest.fn(() => true);
    let params = [
      {
        length: 1,
      },
    ];
    expect(procedureLogicWorkerNativeMemory.resolvingNMCallAction(params)).toStrictEqual([]);
  });
  it('procedureLogicWorkerFileSystemTest43', function () {
    window.postMessage = jest.fn(() => true);
    let params = {
      symbol: '',
      path: '',
      symbolId: 1,
      fileId: 1,
      lib: '',
      symbolName: '',
      type: 0,
    };
    procedureLogicWorkerNativeMemory.dataCache.dataDict = new Map();
    procedureLogicWorkerNativeMemory.dataCache.dataDict.set(0, '');
    procedureLogicWorkerNativeMemory.dataCache.dataDict.set(1, 'H:RSMainThread::DoComposition');
    procedureLogicWorkerNativeMemory.dataCache.dataDict.set(2, 'H:RSUniRender::Process:[EntryView]');
    procedureLogicWorkerNativeMemory.dataCache.dataDict.set(3, 'iowait');
    expect(procedureLogicWorkerNativeMemory.setMerageName(params)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest44', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.clearSplitMapData('')).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest49', function () {
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'native-memory-calltree-action',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerNativeMemory.initNMStack = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest50', function () {
    let data = {
      id: 2,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'native-memory-init-responseType',
    };
    procedureLogicWorkerNativeMemory.initNMStack = jest.fn(() => true);
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest51', function () {
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: 'a',
      type: 'native-memory-get-responseType',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerNativeMemory.initNMStack = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest52', function () {
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: 'b',
      type: 'native-memory-queryNativeHookStatistic',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerNativeMemory.initNMStack = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest53', function () {
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerNativeMemory.queryNativeHookStatistic(1)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest54', function () {
    expect(
      procedureLogicWorkerNativeMemory.statisticDataHandler([
        {
          callchainId: 4,
          ts: 0,
          applyCount: 1,
          applySize: 24,
          releaseCount: 0,
          releaseSize: 0,
        },
        {
          callchainId: 3,
          ts: 0,
          applyCount: 1,
          applySize: 64,
          releaseCount: 0,
          releaseSize: 0,
        },
        {
          callchainId: 2,
          ts: 0,
          applyCount: 1,
          applySize: 32,
          releaseCount: 1,
          releaseSize: 32,
        },
        {
          callchainId: 1,
          ts: 0,
          applyCount: 1,
          applySize: 32,
          releaseCount: 0,
          releaseSize: 0,
        },
      ])
    ).toStrictEqual([{ density: 3, dur: 0, heapsize: 120, startTime: 0 }]);
  });

  it('procedureLogicWorkerFileSystemTest55', function () {
    expect(
      procedureLogicWorkerNativeMemory.handleNativeHookStatisticData([
        { density: 3, dur: 0, heapsize: 120, startTime: 0 },
      ])
    ).toStrictEqual([
      {
        density: 3,
        dur: NaN,
        heapsize: 120,
        maxDensity: 3,
        maxHeapSize: 120,
        minDensity: 0,
        minHeapSize: 0,
        startTime: 0,
      },
    ]);
  });
});
