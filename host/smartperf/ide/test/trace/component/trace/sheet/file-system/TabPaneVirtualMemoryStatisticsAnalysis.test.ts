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
import { TabPaneVirtualMemoryStatisticsAnalysis } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneVirtualMemoryStatisticsAnalysis.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneVirtualMemoryStatisticsAnalysis.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    unobserve: jest.fn(),
    observe: jest.fn(),
  }));
Object.defineProperty(global.self, 'crypto', {
  value: { getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length) },
});

describe('TabPaneVirtualMemoryStatisticsAnalysis Test', () => {
  document.body.innerHTML = `<tabpane-virtual-memory-statistics-analysis id="statistics-analysis"></tabpane-virtual-memory-statistics-analysis>`;
  let tabPane = document.querySelector<TabPaneVirtualMemoryStatisticsAnalysis>('#statistics-analysis');
  let param = {
    anomalyEnergy: [],
    clockMapData: { size: 623 },
    cpuAbilityIds: [234, 56],
    cpuFreqFilterIds: [46, 87, 9],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [77, 97, 3],
    cpus: [2, 1],
    diskAbilityIds: [43, 98],
    diskIOLatency: false,
    diskIOReadIds: [1],
    diskIOWriteIds: [12, 4, 55, 6],
    diskIOipids: [6, 88, 3, 551, 6],
    fileSysVirtualMemory: true,
    fileSystemType: [],
    fsCount: 56,
    funAsync: [],
    funTids: [6, 87, 2],
    hasFps: false,
    irqMapData: { size: 158 },
    jsMemory: [],
    leftNs: 964699699,
    memoryAbilityIds: [25, 87, 8],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [67, 12, 9],
    perfAll: false,
    perfCpus: [],
    perfProcess: [],
    perfSampleIds: [12, 66, 6],
    perfThread: [],
    powerEnergy: [],
    processTrackIds: [42, 87, 43],
    promiseList: [],
    recordStartNs: 780621789228,
    rightNs: 24269616624,
    sdkCounterIds: [33, 87, 21],
    sdkSliceIds: [2, 7, 2],
    smapsType: [],
    systemEnergy: [],
    threadIds: [2, 9, 1],
    virtualTrackIds: [],
    vmCount: 450,
  };
  let processData = [
    {
      callChainId: 154,
      dur: 240768,
      libId: 102,
      libName: 'libName.com',
      pid: 91,
      processName: 'ssioncontroller 3675',
      symbolId: 320,
      symbolName: 'ssioncontroller',
      threadName: 'ssioncontroller',
      tid: 920,
      type: 20,
    },
  ];
  let item = {
    durFormat: '133.23ms ',
    duration: 265930478,
    isHover: true,
    percent: '8023.00',
    pid: 1520,
    tableName: 'test(6124)',
  };
  let res = [
    {
      durFormat: '1964.3ms ',
      duration: 210230478,
      isHover: true,
      percent: '1099.00',
      pid: 5214,
      tableName: 'test(5624)',
    },
  ];
  it('tabPaneVirtualMemoryStatisticsAnalysis01', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    expect(tabPane.vmStatisticsAnalysisSelection).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis02', function () {
    expect(tabPane.clearData()).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis03', function () {
    tabPane.vmStatisticsAnalysisProcessData = jest.fn(() => true);
    let data = [
      {
        type: 7,
        callChainId: 12,
        dur: 30625,
        pid: 1374,
        tid: 1374,
        threadName: 'com.ohos.mms',
        processName: 'com.ohos.mms(1374)',
        libId: 344,
        symbolId: 727,
        libName: 'libmmi-util.z.so',
        symbolName: 'OHOS::MMI::GetThisThreadIdOfString()',
      },
      {
        type: 7,
        callChainId: 24,
        dur: 42000,
        pid: 1818,
        tid: 1374,
        threadName: null,
        processName: 'RSRenderThread(1818)',
        libId: null,
        symbolId: null,
        libName: '',
        symbolName: '0x7f7380e670 ()',
      },
    ];
    tabPane.getVirtualMemoryProcess(data, processData);
    expect(tabPane.vmStatisticsAnalysisProcessData).not.toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis04', function () {
    tabPane.vmStatisticsAnalysisProcessData = processData;
    tabPane.getVirtualMemoryType(item, param);
    expect(tabPane.vmStatisticsAnalysisProgressEL.loading).toBeFalsy();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis05', function () {
    tabPane.vmStatisticsAnalysisProcessData = processData;
    tabPane.getVirtualMemoryThread(item, param);
    expect(tabPane.currentLevel).toEqual(2);
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis06', function () {
    tabPane.vmStatisticsAnalysisProcessData = processData;
    tabPane.getVirtualMemorySo(item, param);
    expect(tabPane.currentLevel).toEqual(3);
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis07', function () {
    tabPane.vmStatisticsAnalysisProcessData = processData;
    tabPane.getVirtualMemoryFunction(item, param);
    expect(tabPane.currentLevel).toEqual(4);
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis08', function () {
    expect(tabPane.typeIdToString(1)).toEqual('File Backed In');
  });

  it('tabPaneVirtualMemoryStatisticsAnalysis09', function () {
    expect(tabPane.typeIdToString(7)).toEqual('Copy On Writer');
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis10', function () {
    expect(tabPane.getVmPieChartData(res).length).toEqual(1);
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis11', function () {
    tabPane.currentLevel = 0;
    let paras = [
      {
        type: 2,
        callChainId: 1,
        dur: 4757959,
        pid: 237,
        tid: 237,
        threadName: 'jbd2/mmcblk0p11',
        processName: 'jbd2/mmcblk0p11(237)',
        libId: 263,
        symbolId: 12560,
        libName: 'kallsyms',
        symbolName: 'submit_bh',
      },
      {
        type: 2,
        callChainId: 1,
        dur: 4673084,
        pid: 237,
        tid: 237,
        threadName: 'jbd2/mmcblk0p11',
        processName: 'jbd2/mmcblk0p11(237)',
        libId: 263,
        symbolId: 12560,
        libName: 'kallsyms',
        symbolName: 'submit_bh',
      },
    ];
    tabPane.currentLevelData = paras;
    expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
    tabPane.currentLevel = 1;
    expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
    tabPane.currentLevel = 2;
    expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
    tabPane.currentLevel = 3;
    expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
    tabPane.currentLevel = 4;
    expect(tabPane.sortByColumn('tableName', 0)).toBeUndefined();
    tabPane.currentLevel = 0;
    expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
    tabPane.currentLevel = 1;
    expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
    tabPane.currentLevel = 2;
    expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
    tabPane.currentLevel = 3;
    expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
    tabPane.currentLevel = 4;
    expect(tabPane.sortByColumn('tableName', 1)).toBeUndefined();
    tabPane.currentLevel = 0;
    expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
    tabPane.currentLevel = 1;
    expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
    tabPane.currentLevel = 2;
    expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
    tabPane.currentLevel = 3;
    expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
    tabPane.currentLevel = 4;
    expect(tabPane.sortByColumn('durFormat', 1)).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis12', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.vmProcessLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis13', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.vmTypeLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis14', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.vmThreadLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis15', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.vmSoLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis16', function () {
    let itemClick = new CustomEvent('click', <CustomEventInit>{
      detail: {
        ...{},
        data: {},
      },
      composed: true,
    });
    tabPane.tabName!.textContent = 'Statistic By type AllDuration';
    tabPane.processStatisticsData = jest.fn(() => true);
    tabPane.processStatisticsData.allDuration = jest.fn(() => true);
    tabPane.vmStatisticsAnalysisPidData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis17', function () {
    let itemClick = new CustomEvent('click', <CustomEventInit>{
      detail: {
        ...{},
        data: {},
      },
      composed: true,
    });
    tabPane.tabName!.textContent = 'Statistic By Thread AllDuration';
    tabPane.typeStatisticsData = jest.fn(() => true);
    tabPane.typeStatisticsData.allDuration = jest.fn(() => true);
    tabPane.vmStatisticsAnalysisTypeData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis18', function () {
    let itemClick = new CustomEvent('click', <CustomEventInit>{
      detail: {
        ...{},
        data: {},
      },
      composed: true,
    });
    tabPane.tabName!.textContent = 'Statistic By Library AllDuration';
    tabPane.threadStatisticsData = jest.fn(() => true);
    tabPane.threadStatisticsData.allDuration = jest.fn(() => true);
    tabPane.vmStatisticsAnalysisThreadData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('tabPaneVirtualMemoryStatisticsAnalysis19', function () {
    let itemClick = new CustomEvent('click', <CustomEventInit>{
      detail: {
        ...{},
        data: {},
      },
      composed: true,
    });
    tabPane.tabName!.textContent = 'Statistic By Function AllDuration';
    tabPane.libStatisticsData = jest.fn(() => true);
    tabPane.libStatisticsData.allDuration = jest.fn(() => true);
    tabPane.vmStatisticsAnalysisSoData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
});
