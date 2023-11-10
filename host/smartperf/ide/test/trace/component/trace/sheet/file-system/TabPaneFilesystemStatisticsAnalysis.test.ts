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
import { TabPaneFilesystemStatisticsAnalysis } from '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFilesystemStatisticsAnalysis.js';
import '../../../../../../dist/trace/component/trace/sheet/file-system/TabPaneFilesystemStatisticsAnalysis.js';
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
  value: {
    getRandomValues: (arr: string | any[]) => crypto.randomBytes(arr.length),
  },
});

describe('TabPaneFilesystemStatisticsAnalysis Test', () => {
  document.body.innerHTML = `<tabpane-file-statistics-analysis id="statistics-analysis"></tabpane-file-statistics-analysis>`;
  let tabPane = document.querySelector<TabPaneFilesystemStatisticsAnalysis>('#statistics-analysis');

  let param = {
    anomalyEnergy: [],
    clockMapData: { size: 193 },
    cpuAbilityIds: [10, 8],
    cpuFreqFilterIds: [56],
    cpuFreqLimitDatas: [],
    cpuStateFilterIds: [12, 98],
    cpus: [],
    diskAbilityIds: [],
    diskIOLatency: false,
    diskIOReadIds: [2, 33],
    diskIOWriteIds: [54, 4, 54, 6],
    diskIOipids: [2, 17, 45, 5, 16],
    fileSysVirtualMemory: false,
    fileSystemType: [],
    fsCount: 3,
    funAsync: [],
    funTids: [],
    hasFps: false,
    irqMapData: { size: 996 },
    jsMemory: [],
    leftNs: 964699125,
    memoryAbilityIds: [12, 34],
    nativeMemory: [],
    nativeMemoryStatistic: [],
    networkAbilityIds: [34, 87],
    perfAll: false,
    perfCpus: [],
    perfProcess: [],
    perfSampleIds: [67, 33],
    perfThread: [],
    powerEnergy: [],
    processTrackIds: [122, 34],
    promiseList: [],
    recordStartNs: 780423788588,
    rightNs: 69267555654,
    sdkCounterIds: [34, 22, 12],
    sdkSliceIds: [221],
    smapsType: [],
    systemEnergy: [],
    threadIds: [12, 45],
    virtualTrackIds: [],
    vmCount: 850,
  };

  let item = {
    durFormat: '194.23ms ',
    duration: 194230478,
    isHover: true,
    percent: '99.00',
    pid: 3744,
    tableName: 'test(3744)',
  };

  let res = [
    {
      durFormat: '1334.23ms ',
      duration: 13230478,
      isHover: true,
      percent: '232.00',
      pid: 34,
      tableName: 'test(3554)',
    },
  ];

  let processData = [
    {
      callChainId: 113,
      dur: 24010,
      libId: 539,
      libName: 'libName.z.so',
      pid: 911,
      processName: 'ksoftirqd/1',
      symbolId: 799,
      symbolName: 'ksoftirqd/1 17',
      threadName: 'ksoftirqd/1',
      tid: 404,
      type: 0,
    },
  ];

  let threadStatisticsData = { durFormat: '194.23ms ', duration: 0, isHover: false, percent: '100.00', tableName: '' };

  it('systemStatisticsAnalysis01', function () {
    let litTable = new LitTable();
    tabPane.appendChild(litTable);
    let filter = new TabPaneFilter();
    tabPane.filter = filter;
    tabPane.loadingList = [];
    tabPane.data = param;
    expect(tabPane.fileStatisticsAnalysisCurrentSelection).not.toBeUndefined();
  });

  it('systemStatisticsAnalysis02', function () {
    expect(tabPane.clearData()).toBeUndefined();
  });

  it('systemStatisticsAnalysis03', function () {
    tabPane.fileStatisticsAnalysisProcessData = processData;
    tabPane.getFilesystemType(item, param);
    expect(tabPane.fileStatisticsAnalysisProgressEL.loading).toBeFalsy();
  });

  it('systemStatisticsAnalysis04', function () {
    tabPane.fileStatisticsAnalysisProcessData = processData;
    tabPane.getFilesystemThread(item, param);
    expect(tabPane.currentLevel).toEqual(2);
  });

  it('systemStatisticsAnalysis05', function () {
    tabPane.fileStatisticsAnalysisProcessData = processData;
    tabPane.getFilesystemSo(item, param);
    expect(tabPane.currentLevel).toEqual(3);
  });

  it('systemStatisticsAnalysis06', function () {
    tabPane.fileStatisticsAnalysisProcessData = processData;
    tabPane.getFilesystemFunction(item, param);
    expect(tabPane.currentLevel).toEqual(4);
  });

  it('systemStatisticsAnalysis07', function () {
    expect(tabPane.typeIdToString(0)).toEqual('OPEN');
  });

  it('systemStatisticsAnalysis08', function () {
    expect(tabPane.typeIdToString(2)).toEqual('READ');
  });

  it('systemStatisticsAnalysis09', function () {
    expect(tabPane.typeIdToString(1)).toEqual('CLOSE');
  });

  it('systemStatisticsAnalysis10', function () {
    expect(tabPane.getFsPieChartData(res).length).toEqual(1);
  });

  it('systemStatisticsAnalysis11', function () {
    tabPane.fileStatisticsAnalysisProcessData = jest.fn(() => true);
    tabPane.fileStatisticsAnalysisProcessData.reMeauseHeight = jest.fn(() => true);
    let parames = [
      {
        type: 0,
        callChainId: 13,
        dur: 240916,
        pid: 911,
        tid: 404,
        threadName: null,
        processName: 'power_host(911)',
        libId: 542,
        symbolId: 802,
        libName: 'libbattery_interface_service_1.0.z.so',
        symbolName:
          'OHOS::HDI::Battery::V1_0::PowerSupplyProvider::ReadBatterySysfsToBuff(char const*, char*, unsigned long) const',
      },
      {
        type: 0,
        callChainId: 17,
        dur: 42000,
        pid: 911,
        tid: 404,
        threadName: null,
        processName: 'power_host(911)',
        libId: 542,
        symbolId: 802,
        libName: 'libbattery_interface_service_1.0.z.so',
        symbolName:
          'OHOS::HDI::Battery::V1_0::PowerSupplyProvider::ReadBatterySysfsToBuff(char const*, char*, unsigned long) const',
      },
    ];
    tabPane.getFilesystemProcess(parames, processData);
    expect(tabPane.fileStatisticsAnalysisProcessData).not.toBeUndefined();
  });
  it('systemStatisticsAnalysis12', function () {
    tabPane.currentLevel = 0;
    let paras = [
      {
        type: 3,
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
        type: 3,
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
  it('systemStatisticsAnalysis13', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.fileProcessLevelClickEvent(it)).toBeUndefined();
  });
  it('systemStatisticsAnalysis14', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.fileTypeLevelClickEvent(it)).toBeUndefined();
  });
  it('systemStatisticsAnalysis15', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.fileThreadLevelClickEvent(it)).toBeUndefined();
  });
  it('systemStatisticsAnalysis16', function () {
    let it = [
      {
        tabName: '',
      },
    ];
    expect(tabPane.fileSoLevelClickEvent(it)).toBeUndefined();
  });
  it('TabPanePerfAnalysisTest17 ', function () {
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
    tabPane.fileStatisticsAnalysisPidData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('TabPanePerfAnalysisTest18 ', function () {
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
    tabPane.fileStatisticsAnalysisThreadData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('TabPanePerfAnalysisTest19 ', function () {
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
    tabPane.fileStatisticsAnalysisSoData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
  it('TabPanePerfAnalysisTest20 ', function () {
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
    tabPane.fileStatisticsAnalysisTypeData = [{}, {}];
    tabPane.back!.dispatchEvent(itemClick);
    expect(tabPane.goBack()).toBeUndefined();
  });
});
