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
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));
// @ts-ignore
import { TabPaneIOTierStatisticsAnalysis } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIOTierStatisticsAnalysis.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneIOTierStatisticsAnalysis.js';
// @ts-ignore
import { LitTable } from '../../../../../../dist/base-ui/table/lit-table.js';
import crypto from 'crypto';
// @ts-ignore
import { TabPaneFilter } from '../../../../../../dist/trace/component/trace/sheet/TabPaneFilter.js';
Object.defineProperty(global.self, 'crypto', {
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});

describe('TabPaneIOTierStatisticsAnalysis Test', () => {
  document.body.innerHTML = `<tabpane-tb-vm-statistics id="statistics-analysis"></tabpane-tb-vm-statistics>`;
  let tabPane = document.querySelector<TabPaneIOTierStatisticsAnalysis>('#statistics-analysis');
  let param = {
    anomalyEnergy: [],
    clockMapData: { size: 985 },
    cpuAbilityIds: [12, 45, 7],
    cpuFreqFilterIds: [10, 67, 9],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [23, 4],
    cpus: [],
    diskAbilityIds: [43, 45, 4],
    diskIOLatency: false,
    diskIOReadIds: [24, 17, 71],
    diskIOWriteIds: [2, 14, 5, 36],
    diskIOipids: [42, 7, 65],
    fileSysVirtualMemory: false,
    fileSystemType: [],
    fsCount: 44,
    funAsync: [],
    funTids: [23, 78],
    hasFps: false,
    irqMapData: { size: 966 },
    jsMemory: [],
    leftNs: 964696659,
    memoryAbilityIds: [65, 67, 89, 11],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [12, 56, 88],
    perfAll: false,
    perfCpus: [0, 2],
    perfProcess: [],
    perfSampleIds: [11, 53, 110],
    perfThread: [],
    powerEnergy: [],
    processTrackIds: [12, 110, 889],
    promiseList: [],
    recordStartNs: 780423782222,
    rightNs: 24267332224,
    sdkCounterIds: [12, 4, 55, 1],
    sdkSliceIds: [45, 12],
    smapsType: [],
    systemEnergy: [],
    threadIds: [66, 3, 10],
    virtualTrackIds: [],
    vmCount: 17,
  };
  let processData = [
    {
      callChainId: 213,
      dur: 295432,
      libId: 511,
      libName: 'libName.z.so',
      pid: 911,
      processName: 'mdss_fb0 8544',
      symbolId: 799,
      symbolName: 'mdss_fb0',
      threadName: 'mdss_fb0',
      tid: 30,
      type: 90,
    },
  ];
  let item = {
    durFormat: '183.23ms ',
    duration: 19965478,
    isHover: true,
    percent: '502.00',
    pid: 320,
    tableName: 'sumb(3745)',
  };
  let res = [
    {
      durFormat: '201.33ms ',
      duration: 365130478,
      isHover: true,
      percent: '3599.00',
      pid: 301,
      tableName: 'test(3204)',
    },
  ];
  let itemClick = new CustomEvent('click', <CustomEventInit>{
    detail: {
      ...{},
      data: {},
    },
    composed: true,
  });
  it('tabPaneIOTierStatisticsAnalysis01', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    expect(tabPane.ioTierStatisticsAnalysisSelection).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis02', function () {
    expect(tabPane.clearData()).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis03', function () {
    tabPane.processData = jest.fn(() => true);
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
    tabPane.getIOTierProcess(paras, processData);
    expect(tabPane.processData).not.toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis04', function () {
    tabPane.processData = processData;
    tabPane.getIOTierType(item, param);
    expect(tabPane.progressEL.loading).toBeFalsy();
  });
  it('tabPaneIOTierStatisticsAnalysis05', function () {
    tabPane.processData = processData;
    tabPane.getIOTierThread(item, param);
    expect(tabPane.currentLevel).toEqual(2);
  });
  it('tabPaneIOTierStatisticsAnalysis06', function () {
    tabPane.processData = processData;
    tabPane.getIOTierSo(item, param);
    expect(tabPane.currentLevel).toEqual(3);
  });
  it('tabPaneIOTierStatisticsAnalysis07', function () {
    tabPane.processData = processData;
    tabPane.getIOTierFunction(item, param);
    expect(tabPane.currentLevel).toEqual(4);
  });
  it('tabPaneIOTierStatisticsAnalysis08', function () {
    expect(tabPane.typeIdToString(1)).toEqual('DATA_READ');
  });

  it('tabPaneIOTierStatisticsAnalysis09', function () {
    expect(tabPane.typeIdToString(2)).toEqual('DATA_WRITE');
  });

  it('tabPaneIOTierStatisticsAnalysis10', function () {
    expect(tabPane.typeIdToString(3)).toEqual('METADATA_READ');
  });
  it('tabPaneIOTierStatisticsAnalysis11', function () {
    expect(tabPane.typeIdToString(4)).toEqual('METADATA_WRITE');
  });
  it('tabPaneIOTierStatisticsAnalysis12', function () {
    expect(tabPane.getIOTierPieChartData(res).length).toEqual(1);
  });

  it('tabPaneIOTierStatisticsAnalysis13', function () {
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
  it('tabPaneIOTierStatisticsAnalysis14', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.ioTierProcessLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis15', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.ioTierTypeLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis16', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.ioTierThreadLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis17', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.ioTierSoLevelClickEvent(it)).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis18', function () {
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
    tabPane.pidData = [{}, {}];
    tabPane.iOTierStatisticsAnalysisBack!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis19', function () {
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
    tabPane.typeData = [{}, {}];
    tabPane.iOTierStatisticsAnalysisBack!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis20', function () {
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
    tabPane.threadData = [{}, {}];
    tabPane.iOTierStatisticsAnalysisBack!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('tabPaneIOTierStatisticsAnalysis21', function () {
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
    tabPane.soData = [{}, {}];
    tabPane.iOTierStatisticsAnalysisBack!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
});
