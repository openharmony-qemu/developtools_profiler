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

import '../../../../../../dist/trace/component/trace/sheet/file-system/TabpaneFilesystemCalltree.js';
// @ts-ignore
import { TabpaneFilesystemCalltree } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabpaneFilesystemCalltree.js';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
import { FrameChart } from '../../../../../../dist/trace/component/chart/FrameChart.js';
// @ts-ignore
import { NativeHookStatisticsTableData } from '../../../../../../dist/trace/database/ui-worker/ProcedureWorkerCPU.js';

jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});
jest.mock('../../../../../../dist/trace/database/ui-worker/ProcedureWorkerCPU.js', () => {
  return {
    cpuCount: 1,
    CpuRender: Object,
    EmptyRender: Object,
  };
});

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

import crypto from 'crypto';
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    observe: jest.fn(),
  }));

Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});
describe('TabpaneFilesystemCalltree Test', () => {
  document.body.innerHTML = `<tabpane-filesystem-calltree id="tree"></tabpane-filesystem-calltree>`;
  let tabpaneFilesystemCalltree = document.querySelector<TabpaneFilesystemCalltree>('#tree');
  let val = {
    anomalyEnergy: [],
    clockMapData: { size: 16 },
    cpuAbilityIds: [12, 155, 223],
    cpuFreqFilterIds: [52, 122, 22],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [52, 566, 115],
    cpus: [1, 2],
    diskAbilityIds: [451, 22],
    diskIOLatency: false,
    diskIOReadIds: [522, 4, 5, 6],
    diskIOWriteIds: [2, 5621, 5, 6],
    diskIOipids: [2, 120, 5, 6],
    fileSysVirtualMemory: true,
    fileSystemType: [],
    fsCount: 32,
    funAsync: [],
    funTids: [120, 55],
    hasFps: false,
    irqMapData: { size: 85 },
    jsMemory: [],
    leftNs: 555123,
    memoryAbilityIds: [],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [],
    perfAll: true,
    perfCpus: [1, 2],
    perfProcess: [],
    perfSampleIds: [],
    perfThread: ['hiprofiler_cmd', ''],
    powerEnergy: [],
    processTrackIds: [552, 123],
    promiseList: [],
    recordStartNs: 780423789228,
    rightNs: 966666666,
    sdkCounterIds: [120, 451, 52],
    sdkSliceIds: [154],
    smapsType: [],
    systemEnergy: [],
    threadIds: [101, 2122, 4],
    virtualTrackIds: [412, 85],
    vmCount: 3,
  };

  it('TabpaneFilesystemCalltreeTest02', function () {
    tabpaneFilesystemCalltree!.showButtonMenu = jest.fn(() => '');
    tabpaneFilesystemCalltree.fsCallTreeFilter = jest.fn(() => '');
    tabpaneFilesystemCalltree.fsCallTreeFilter.setAttribute = jest.fn(() => '');
    expect(tabpaneFilesystemCalltree.showButtonMenu(true)).toBe('');
  });

  it('TabpaneFilesystemCalltreeTest03', function () {
    TabpaneFilesystemCalltree.getParentTree = jest.fn(() => true);
    let call = {
      id: '1',
      children: [],
    };
    let target = {
      id: '1',
    };
    expect(tabpaneFilesystemCalltree.getParentTree([call], { target }, [])).toBeFalsy();
  });

  it('TabpaneFilesystemCalltreeTest04', function () {
    TabpaneFilesystemCalltree.getParentTree = jest.fn(() => true);
    let call = {
      children: [],
    };
    expect(tabpaneFilesystemCalltree.getParentTree([call], '', [])).not.toBeUndefined();
  });

  it('TabpaneFilesystemCalltreeTest05', function () {
    TabpaneFilesystemCalltree.getChildTree = jest.fn(() => true);
    let call = {
      id: '1',
      children: [],
    };
    let id = '1';
    expect(tabpaneFilesystemCalltree.getChildTree([call], { id }, [])).not.toBeUndefined();
  });

  it('TabpaneFilesystemCalltreeTest06', function () {
    TabpaneFilesystemCalltree.getChildTree = jest.fn(() => true);
    let call = {
      children: [],
    };
    expect(tabpaneFilesystemCalltree.getChildTree([call], '', [])).not.toBeUndefined();
  });

  it('TabpaneFilesystemCalltreeTest07', function () {
    let filter = new TabPaneFilter();
    tabpaneFilesystemCalltree.fsCallTreeFilter = filter;
    tabpaneFilesystemCalltree.data = val;
    expect(tabpaneFilesystemCalltree.currentSelection).not.toBeUndefined();
  });

  it('TabpaneFilesystemCalltreeTest08', function () {
    let resultData = [
      {
        addr: 'SpSystemTrace.js',
        canCharge: false,
        count: 56,
        depth: 1,
        drawCount: 23,
        drawDur: 32,
        drawSize: 33,
        dur: 43331564310,
        frame: { x: 10, y: 32, width: 522, height: 10 },
        id: '1',
        ip: '12',
        isDraw: false,
        isSearch: true,
        isSelected: false,
        isStore: 1,
        lib: '',
        libName: 'file',
        parentId: '12',
        path: '',
        pathId: 2,
        percent: 0.36425651150324375,
        pid: 23,
        processName: '',
        searchShow: true,
        self: '5s',
        selfDur: 54,
        size: 443,
        symbol: 'symsdbl',
        symbolName: 'symbols',
        symbolsId: 210,
        textMetricsWidth: 62.7712033125,
        type: 10,
        weight: '66.33s ',
        weightPercent: '23.4%',
        children: [],
      },
    ];
    tabpaneFilesystemCalltree.setLTableData(resultData);
    expect(tabpaneFilesystemCalltree.fsCallTreeDataSource.length).toEqual(1);
  });

  it('TabpaneFilesystemCalltreeTest09', function () {
    let switchData = {
      firstSelect: '',
      icon: 'tree',
      inputValue: 'kk',
      mark: false,
      secondSelect: '',
      thirdSelect: '',
      type: 'inputValue',
    };
    tabpaneFilesystemCalltree.fsCallTreeTbl.reMeauseHeight = jest.fn(() => true);
    tabpaneFilesystemCalltree.switchFlameChart(switchData);
    expect(tabpaneFilesystemCalltree.isChartShow).toBeFalsy();
  });

  it('TabpaneFilesystemCalltreeTest10', function () {
    let call = {
      id: '1',
      dur: 1,
      children: [],
    };
    expect(tabpaneFilesystemCalltree.setRightTableData(call)).toBeUndefined();
  });
  it('TabpaneFilesystemCalltreeTest11', function () {
    let filterData = {
      callTree: [{}, {}],
      dataMining: {
        concat: jest.fn(() => true),
      },
      callTreeConstraints: {
        checked: false,
      },
    };
    expect(tabpaneFilesystemCalltree.refreshAllNode(filterData)).toBeUndefined();
  });
});
