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
  ProcedureLogicWorkerPerf,
  PerfCountSample,
  PerfCallChainMerageData,
  PerfStack,
  PerfCmdLine,
  PerfCall,
  timeMsFormat2p,
  PerfFile,
  PerfThread,
  PerfCallChain,
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerPerf.js';

describe('ProcedureLogicWorkerPerf Test', () => {
  it('ProcedureLogicWorkerPerfTest', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    expect(procedureLogicWorkerPerf).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest01', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-init',
    };
    procedureLogicWorkerPerf.initPerfFiles = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest02', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-queryPerfFiles',
    };
    procedureLogicWorkerPerf.initPerfThreads = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest03', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-queryPerfThread',
    };
    procedureLogicWorkerPerf.initPerfCalls = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest04', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-queryPerfCalls',
    };
    procedureLogicWorkerPerf.initPerfCallchains = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest05', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-queryPerfCallchains',
    };
    window.postMessage = jest.fn(() => true);
    procedureLogicWorkerPerf.initCallChainTopDown = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest06', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-queryCallchainsGroupSample',
    };
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest07', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let data = {
      id: 1,
      params: [
        {
          list: '',
        },
      ],
      action: '',
      type: 'perf-action',
    };
    procedureLogicWorkerPerf.resolvingAction = jest.fn(() => true);
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.handle(data)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest08', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    expect(procedureLogicWorkerPerf.clearAll()).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest14', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let callChain = {
      sampleId: '',
      depth: 0,
      canCharge: false,
      name: '',
      tid: '',
      fileName: '',
      threadName: '',
    };
    expect(procedureLogicWorkerPerf.addPerfCallData(callChain)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest19', function () {
    let perfCountSample = new PerfCountSample();
    perfCountSample = {
      sampleId: 0,
      count: 0,
      pid: 0,
      tid: 0,
      threadState: '',
    };
    expect(perfCountSample).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest20', function () {
    let perfStack = new PerfStack();
    perfStack = {
      sample: '',
      path: '',
      fileId: 0,
      type: 0,
      vaddrInFile: 0,
    };
    expect(perfStack).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest21', function () {
    let perfCmdLine = new PerfCmdLine();
    perfCmdLine = {
      report_value: '',
    };
    expect(perfCmdLine).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest21', function () {
    let perfCmdLine = new PerfCmdLine();
    perfCmdLine = {
      report_value: '',
    };
    expect(perfCmdLine).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest21', function () {
    let perfCall = new PerfCall();
    perfCall = {
      sampleId: 0,
      name: '',
      depth: 0,
    };
    expect(perfCall).not.toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest22', function () {
    expect(timeMsFormat2p('')).toBe('0s');
  });

  it('ProcedureLogicWorkerPerfTest23', function () {
    expect(timeMsFormat2p(3600_000)).toBe('1.00h');
  });

  it('ProcedureLogicWorkerPerfTest24', function () {
    expect(timeMsFormat2p(60_000)).toBe('1.00min');
  });

  it('ProcedureLogicWorkerPerfTest25', function () {
    expect(timeMsFormat2p(1_000)).toBe('1.00s');
  });

  it('ProcedureLogicWorkerPerfTest26', function () {
    expect(timeMsFormat2p(100)).toBe('100.00ms');
  });

  it('ProcedureLogicWorkerPerfTest31', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.recursionChargeInitTree = jest.fn(() => undefined);
    let node = [
      {
        symbolName: '',
        libName: '',
        length: 1,
        initChildren: {
          length: 1,
        },
      },
    ];
    expect(procedureLogicWorkerPerf.recursionChargeInitTree(node, '', true)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest33', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.recursionPruneInitTree = jest.fn(() => undefined);
    let node = {
      symbolName: '',
      libName: '',
      length: 1,
      initChildren: {
        length: 1,
      },
    };
    expect(procedureLogicWorkerPerf.recursionPruneInitTree(node, '', true)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest34', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.recursionChargeByRule = jest.fn(() => undefined);
    let node = {
      initChildren: [
        {
          length: 1,
        },
      ],
    };
    let rule = {
      child: {
        isStore: 1,
      },
    };
    expect(procedureLogicWorkerPerf.recursionChargeByRule(node, '', rule)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest35', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    window.postMessage = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.queryData('', '', '')).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest36', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.queryData = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.initPerfFiles()).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest37', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.queryData = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.initPerfThreads()).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest38', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.queryData = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.initPerfCalls()).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest39', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.queryData = jest.fn(() => true);
    expect(procedureLogicWorkerPerf.initPerfCallchains()).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest40', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    procedureLogicWorkerPerf.queryData = jest.fn(() => true);
    let selectionParam = {
      perfAll: '',
      perfCpus: [1],
      perfProcess: [2],
      perfThread: [1],
      leftNs: 0,
      rightNs: 0,
    };
    expect(procedureLogicWorkerPerf.getCurrentDataFromDb(selectionParam)).toBeUndefined();
  });

  it('ProcedureLogicWorkerPerfTest41', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let currentNode = {
      children: [],
      initChildren: [],
    };
    let list = [
      {
        length: 1,
        name: '',
      },
    ];
    expect(procedureLogicWorkerPerf.merageChildren(currentNode, list, true)).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest42', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let sampleIds = {
      length: 1,
    };
    let isTopDown = {};
    expect(procedureLogicWorkerPerf.groupNewTreeNoId(sampleIds, isTopDown)).toStrictEqual([]);
  });
  it('ProcedureLogicWorkerPerfTest46', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let node = {
      children: {
        length: 1,
        forEach: jest.fn(() => true),
      },
      libName: 1,
    };
    let symbolName = 1;

    let isSymbol = true;
    expect(procedureLogicWorkerPerf.recursionPruneTree(node, symbolName, isSymbol)).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest47', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let node = {
      initChildren: {
        length: 1,
        forEach: jest.fn(() => true),
      },
    };
    let ruleName = 1;

    let rule = true;
    expect(procedureLogicWorkerPerf.recursionChargeByRule(node, ruleName, rule)).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest48', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let node = {
      initChildren: {
        length: 1,
        forEach: jest.fn(() => true),
      },
    };
    let symbolName = 1;
    expect(procedureLogicWorkerPerf.pruneChildren(node, symbolName)).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest49', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    expect(procedureLogicWorkerPerf.clearSplitMapData()).toBeUndefined();
  });
  it('PerfFileTest01', function () {
    let perfFile = new PerfFile();
    expect(perfFile.constructor()).toBeUndefined();
  });
  it('PerfFileTest02', function () {
    let perfFile = new PerfFile();
    let path = true;
    expect(perfFile.setFileName(path)).toBeUndefined();
  });
  it('PerfThreadTest01', function () {
    let perfThread = new PerfThread();
    expect(perfThread.constructor()).toBeUndefined();
  });
  it('PerfCallChainTest01', function () {
    let perfCallChain = new PerfCallChain();
    expect(perfCallChain.constructor()).toBeUndefined();
  });
  it('PerfCallChainMerageDataTest01', function () {
    let perfCallChainMerageData = new PerfCallChainMerageData();
    expect(perfCallChainMerageData.constructor()).toEqual({
      addr: '',
      canCharge: true,
      children: [],
      count: 0,
      currentTreeParentNode: undefined,
      depth: 0,
      dur: 0,
      id: '',
      initChildren: [],
      isSearch: false,
      isSelected: false,
      isStore: 0,
      lib: '',
      libName: '',
      parentId: '',
      path: '',
      pid: 0,
      searchShow: true,
      self: '0s',
      selfDur: 0,
      size: 0,
      symbol: '',
      symbolName: '',
      tid: 0,
      type: 0,
      vaddrInFile: 0,
      weight: '',
      weightPercent: '',
    });
  });
  it('PerfCallChainMerageDataTest03', function () {
    let perfCallChainMerageData = new PerfCallChainMerageData();
    perfCallChainMerageData.parentNode = true;
    expect(perfCallChainMerageData.parentNode).toBeTruthy();
  });
  it('PerfCallChainMerageDataTest04', function () {
    let perfCallChainMerageData = new PerfCallChainMerageData();
    perfCallChainMerageData.total = true;
    expect(perfCallChainMerageData.total).toBeTruthy();
  });
  it('ProcedureLogicWorkerPerfTest53', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    expect(procedureLogicWorkerPerf.hideSystemLibrary()).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest54', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    expect(procedureLogicWorkerPerf.hideNumMaxAndMin(1,'∞')).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest55', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    let sampleArray = [
      {
        addr: "",
        canCharge: true,
        children: [],
        count: 4,
        currentTreeParentNode: {
          addr: "",
          canCharge: true,
          children: [],
          count: 4,
          currentTreeParentNode: {},
          depth: 0,
          dur: 1,
          id: "11758",
          initChildren: [],
          isSearch: false,
          isSelected: false,
          isStore: 0,
          lib: "",
          libName: "ld-musl-aarch64.so.1",
          parentId: "977",
          path: "/system/lib/ld-musl-aarch64.so.1",
          pid: 28917,
          searchShow: false,
          self: "0ms",
          selfDur: 4,
          size: 0,
          symbol: "fopen64  (ld-musl-aarch64.so.1)",
          symbolName: "fopen64",
          tid: 28922,
          type: 0,
          vaddrInFile: 730108,
          weight: "1.00ms",
          weightPercent: "0.0%"
        },
        depth: 0,
        dur: 4,
        id: "2791",
        initChildren: [],
        isSearch: false,
        isSelected: false,
        isStore: 0,
        lib: "",
        libName: "[kernel.kallsyms]",
        parentId: "2790",
        path: "[kernel.kallsyms]",
        pid: 28917,
        searchShow: false,
        self: "4.00ms",
        selfDur: 4,
        size: 0,
        symbol: "perf_trace_sched_switch  ([kernel.kallsyms])",
        symbolName: "perf_trace_sched_switch",
        tid: 28922,
        type: 0,
        vaddrInFile: -274609073904,
        weight: "4.00ms",
        weightPercent: "0.0%"
      }
    ];
    expect(procedureLogicWorkerPerf.findSearchNode(sampleArray,'da',false)).toBeUndefined();
  });
  it('ProcedureLogicWorkerPerfTest56', function () {
    let procedureLogicWorkerPerf = new ProcedureLogicWorkerPerf();
    expect(procedureLogicWorkerPerf.splitAllProcess([])).toBeUndefined();
  });
});
