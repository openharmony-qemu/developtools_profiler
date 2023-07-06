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
  ProcedureLogicWorkerFileSystem,
  FileCallChain,
  FileSample,
  Stack,
  FileSysEvent,
  FileMerageBean,
  IoCompletionTimes,
  VirtualMemoryEvent,
  //@ts-ignore
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerFileSystem.js';

describe('ProcedureLogicWorkerFileSystem Test', () => {
  let procedureLogicWorkerF = new ProcedureLogicWorkerFileSystem();

  procedureLogicWorkerF.queryData = jest.fn();
  procedureLogicWorkerF.initCallchains();
  it('procedureLogicWorkerFileSystemTest', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    expect(procedureLogicWorkerFileSystem).not.toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest01', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    window.postMessage = jest.fn(() => true);
    let data = {
      type: 'fileSystem-init',
    };
    expect(procedureLogicWorkerFileSystem.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest35', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    window.postMessage = jest.fn(() => true);
    let data = {
      type: 'fileSystem-queryCallchains',
      params: {
        list: [],
      },
    };
    expect(procedureLogicWorkerFileSystem.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest36', function () {
    window.postMessage = jest.fn(() => true);
    let data = {
      type: 'fileSystem-queryFileSamples',
      params: {
        list: [],
      },
    };
    expect(procedureLogicWorkerF.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest37', function () {
    window.postMessage = jest.fn(() => true);
    let data = {
      type: 'fileSystem-action',
      length: 0,
      params: {
        callType: 'fileSystem',
        args: [],
        list: [],
        filter: () => {
          return [];
        },
      },
    };
    expect(procedureLogicWorkerF.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest38', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    window.postMessage = jest.fn(() => true);
    let data = {
      type: 'fileSystem-queryStack',
      params: {
        list: [],
      },
    };
    expect(procedureLogicWorkerFileSystem.handle(data)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest39', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    window.postMessage = jest.fn(() => true);
    let data = {
      type: 'fileSystem-queryFileSysEvents',
      params: {
        list: [],
      },
    };
    expect(procedureLogicWorkerFileSystem.handle(data)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest07', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    expect(handlerMap.clearSplitMapData([true])).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest08', function () {
    let fileCallChain = {
      callChainId: 0,
      depth: 0,
      symbolsId: 0,
      pathId: 0,
      ip: '',
    };
    expect(fileCallChain).not.toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest09', function () {
    let stack = new Stack();
    stack = {
      type: 0,
      symbol: '',
      path: '',
    };
    expect(stack).not.toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest10', function () {
    let fileSysEvent = new FileSysEvent();
    fileSysEvent = {
      id: 0,
      callchainId: 0,
      startTs: 0,
      startTsStr: '',
      durStr: '',
      dur: 0,
      process: 0,
      type: 0,
      typeStr: '',
      fd: '',
      size: 0,
      depth: 0,
      firstArg: '',
      secondArg: '',
      thirdArg: '',
      fourthArg: '',
      returnValue: '',
      error: '',
      path: '',
      symbol: '',
      backtrace: [],
    };
    expect(fileSysEvent).not.toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest11', function () {
    let fileMerageBean = new FileMerageBean();
    expect(fileMerageBean).not.toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest13', function () {
    let fileSample = {
      callChainId: 0,
      dur: 0,
      pid: 0,
      processName: '',
    };
    expect(fileSample).not.toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest14', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    expect(handlerMap.clearAll()).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest15', function () {
    window.postMessage = jest.fn(() => true);
    let map = new Map();
    map.set('symbol', '');
    map.set('path', '');
    map.set('libName', '');
    map.set('symbolName', '');
    let data = {
      type: 'fileSystem-init',
      params: map,
    };
    procedureLogicWorkerF.handle(data);
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let currentNode = {
      symbol: 'symbol',
      path: 'path',
      libName: 'libName',
      symbolName: 'symbolName',
    };
    expect(handlerMap.setMerageName(currentNode)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest33', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let currentNode = {
      pathId: -1,
      symbol: '',
      path: '',
      libName: '',
      symbolName: '',
    };
    expect(handlerMap.setMerageName(currentNode)).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest18', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    expect(procedureLogicWorkerFileSystem.initCallChainTopDown([])).toBeUndefined();
  });

  it('procedureLogicWorkerFileSystemTest19', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    expect(procedureLogicWorkerFileSystem.supplementFileSysEvents([], 'events'));
  });

  it('procedureLogicWorkerFileSystemTest21', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    expect(procedureLogicWorkerFileSystem.supplementFileSysEvents([], ''));
  });
  it('procedureLogicWorkerFileSystemTest22', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    expect(procedureLogicWorkerFileSystem.supplementFileSysEvents([]));
  });

  it('procedureLogicWorkerFileSystemTest20', function () {
    let procedureLogicWorkerFileSystem = new ProcedureLogicWorkerFileSystem();
    expect(procedureLogicWorkerFileSystem.getStacksByCallchainId([]));
  });

  it('procedureLogicWorkerFileSystemTest23', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'getCallChainsBySampleIds',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest24', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'getCurrentDataFromDb',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest25', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'hideSystemLibrary',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest26', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'hideNumMaxAndMin',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest27', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'splitAllProcess',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest28', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'resetAllNode',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest29', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'resotreAllNode',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest30', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'clearSplitMapData',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest31', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'splitTree',
    };
    expect(handlerMap.resolvingAction(params));
  });

  it('procedureLogicWorkerFileSystemTest32', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      funcName: 'setSearchValue',
    };
    expect(handlerMap.resolvingAction(params));
  });
  it('procedureLogicWorkerFileSystemTest34', function () {
    let currentNode = {
      processName: '',
      ip: '',
      pid: 1,
      pathId: '',
      symbolsId: '',
      selfDur: 1,
      self: 1,
      count: 1,
    };
    let callChain = {
      ip: '',
      pid: 1,
      pathId: '',
      symbolsId: '',
    };
    let sample = {
      processName: '',
      dur: 1,
    };
    let isEnd = true;
    expect(FileMerageBean.merageCallChainSample(currentNode, callChain, sample, isEnd));
  });
  it('procedureLogicWorkerFileSystemTest40', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let selectionParam = {
      diskIOipids: {
        length: 1,
        join: jest.fn(() => true),
      },
      fileSystemType: {
        length: 1,
        join: jest.fn(() => true),
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(handlerMap.queryFileSamples(selectionParam)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest41', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let sample = {
      callChainId: 1,
      type: {
        toString,
      },
    };
    window.postMessage = jest.fn(() => true);
    expect(handlerMap.createThreadAndType(sample)).toBeTruthy();
  });
  it('procedureLogicWorkerFileSystemTest42', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let currentNode = {
      initChildren: {
        filter: jest.fn(() => -1),
      },
    };
    let callChainDataList = {
      length: 1,
    };
    let index = 0;
    let sample = {};
    let isTopDown = {};
    window.postMessage = jest.fn(() => true);
    expect(handlerMap.merageChildrenByIndex(currentNode, callChainDataList, index, sample, isTopDown)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest43', function () {
    let handlerMap = procedureLogicWorkerF.handlerMap.get('fileSystem');
    let params = {
      length: 1,
      forEach: jest.fn(() => true),
    };
    window.postMessage = jest.fn(() => true);
    expect(handlerMap.resolvingAction(params)).toStrictEqual([]);
  });

  it('procedureLogicWorkerFileSystemTest45', function () {
    let ioCompletionTimes = new IoCompletionTimes();
    expect(ioCompletionTimes).toBeDefined();
  });

  it('procedureLogicWorkerFileSystemTest46', function () {
    let virtualMemoryEvent = new VirtualMemoryEvent();
    expect(virtualMemoryEvent).toBeDefined();
  });
  it('procedureLogicWorkerFileSystemTest47', function () {
    let procedureLogicWorkerFileSystem   = new ProcedureLogicWorkerFileSystem ();
    expect(procedureLogicWorkerFileSystem.queryFileSysEvents(1,0,[1],true)).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest48', function () {
    let procedureLogicWorkerFileSystem   = new ProcedureLogicWorkerFileSystem ();
    expect(procedureLogicWorkerFileSystem.queryVMEvents(1,0,[1])).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest49', function () {
    let procedureLogicWorkerFileSystem   = new ProcedureLogicWorkerFileSystem ();
    expect(procedureLogicWorkerFileSystem.queryVMEvents(1,0,[1])).toBeUndefined();
  });
  it('procedureLogicWorkerFileSystemTest50', function () {
    let procedureLogicWorkerFileSystem   = new ProcedureLogicWorkerFileSystem ();
    expect(procedureLogicWorkerFileSystem.queryIOEvents(1,0,[1])).toBeUndefined();
  });
});
