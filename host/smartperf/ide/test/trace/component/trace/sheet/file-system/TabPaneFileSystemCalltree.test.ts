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
import { TabpaneFilesystemCalltree } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabpaneFilesystemCalltree.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabpaneFilesystemCalltree.js';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
import { FrameChart } from '../../../../../../dist/trace/component/chart/FrameChart.js';

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

import crypto from 'crypto';

window.ResizeObserver =
    window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
      disconnect: jest.fn(),
      observe: jest.fn(),
      unobserve: jest.fn(),
    }));

Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});

describe('TabpaneFilesystemCalltree Test', () => {
  document.body.innerHTML = `<sp-application dark></sp-application><tabpane-filesystem-calltree id="tree"></tabpane-filesystem-calltree>`;
  let tabpaneFilesystemCalltree = document.querySelector<TabpaneFilesystemCalltree>('#tree');
  let val = {
    anomalyEnergy: [],
    clockMapData: { size: 0 },
    cpuAbilityIds: [],
    cpuFreqFilterIds: [],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [],
    cpus: [],
    diskAbilityIds: [],
    diskIOLatency: false,
    diskIOReadIds: [2, 7, 1, 3, 4, 5, 6],
    diskIOWriteIds: [2, 7, 1, 3, 4, 5, 6],
    diskIOipids: [2, 7, 1, 3, 4, 5, 6],
    fileSysVirtualMemory: false,
    fileSystemType: [],
    fsCount: 0,
    funAsync: [],
    funTids: [],
    hasFps: false,
    irqMapData: { size: 0 },
    jsMemory: [],
    leftNs: 964699689,
    memoryAbilityIds: [],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [],
    perfAll: false,
    perfCpus: [],
    perfProcess: [],
    perfSampleIds: [],
    perfThread: [],
    powerEnergy: [],
    processTrackIds: [],
    promiseList: [],
    recordStartNs: 780423789228,
    rightNs: 24267556624,
    sdkCounterIds: [],
    sdkSliceIds: [],
    smapsType: [],
    systemEnergy: [],
    threadIds: [],
    virtualTrackIds: [],
    vmCount: 0,
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
        addr: '',
        canCharge: false,
        count: 67,
        depth: 0,
        drawCount: 0,
        drawDur: 0,
        drawSize: 0,
        dur: 43334510310,
        frame: { x: 0, y: 30, width: 594, height: 20 },
        id: '38',
        ip: '',
        isDraw: false,
        isSearch: false,
        isSelected: false,
        isStore: 0,
        lib: '',
        libName: '',
        parentId: '',
        path: '',
        pathId: 0,
        percent: 0.3642222150324375,
        pid: 0,
        processName: '',
        searchShow: true,
        self: '0s',
        selfDur: 0,
        size: 0,
        symbol: 'symbol',
        symbolName: 'symbolName',
        symbolsId: 0,
        textMetricsWidth: 62.7783203125,
        type: 0,
        weight: '43.33s ',
        weightPercent: '36.4%',
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
    let switchData = {
      firstSelect: '',
      icon: 'block',
      inputValue: 'kk',
      mark: false,
      secondSelect: '',
      thirdSelect: '',
      type: 'inputValue',
    };
    tabpaneFilesystemCalltree.fsCallTreeTbl.reMeauseHeight = jest.fn(() => true);
    let frameChart = new FrameChart();
    tabpaneFilesystemCalltree.frameChart = frameChart;
    tabpaneFilesystemCalltree.switchFlameChart(switchData);
    expect(tabpaneFilesystemCalltree.isChartShow).toBeTruthy();
  });

  it('TabpaneFilesystemCalltreeTest11', function () {
    let call = {
      id: '1',
      dur: 1,
      children: [],
    };
    expect(tabpaneFilesystemCalltree.setRightTableData(call)).toBeUndefined();
  });
});
