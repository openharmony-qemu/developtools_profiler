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

import { SpSystemTrace } from '../SpSystemTrace.js';
import { SpHiPerf } from './SpHiPerf.js';
import { SpCpuChart } from './SpCpuChart.js';
import { SpFreqChart } from './SpFreqChart.js';
import { SpFpsChart } from './SpFpsChart.js';
import {
  getCpuUtilizationRate, queryAppStartupProcessIds,
  queryDataDICT,
  queryMemoryConfig,
  queryTaskPoolCallStack,
  queryThreadAndProcessName,
  queryTotalTime,
} from '../../database/SqlLite.js';
import { info } from '../../../log/Log.js';
import { SpNativeMemoryChart } from './SpNativeMemoryChart.js';
import { SpAbilityMonitorChart } from './SpAbilityMonitorChart.js';
import { SpProcessChart } from './SpProcessChart.js';
import { perfDataQuery } from './PerfDataQuery.js';
import { SpVirtualMemChart } from './SpVirtualMemChart.js';
import { SpFileSystemChart } from './SpFileSystemChart.js';
import { SpSdkChart } from './SpSdkChart.js';
import { SpHiSysEventChart } from './SpHiSysEventChart.js';
import { VmTrackerChart } from './SpVmTrackerChart.js';
import { SpClockChart } from './SpClockChart.js';
import { SpIrqChart } from './SpIrqChart.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { SpFrameTimeChart } from './SpFrameTimeChart.js';
import { Utils } from '../trace/base/Utils.js';
import { SpArkTsChart } from './SpArkTsChart.js';
import { MemoryConfig } from '../../bean/MemoryConfig.js';
import { FlagsConfig } from '../SpFlags.js';

export class SpChartManager {
  static APP_STARTUP_PID_ARR: Array<number> = [];

  private trace: SpSystemTrace;
  public perf: SpHiPerf;
  private cpu: SpCpuChart;
  private freq: SpFreqChart;
  private virtualMemChart: SpVirtualMemChart;
  private fps: SpFpsChart;
  private nativeMemory: SpNativeMemoryChart;
  private abilityMonitor: SpAbilityMonitorChart;
  private process: SpProcessChart;
  private fileSystem: SpFileSystemChart;
  private sdkChart: SpSdkChart;
  private hiSyseventChart: SpHiSysEventChart;
  private smapsChart: VmTrackerChart;
  private clockChart: SpClockChart;
  private irqChart: SpIrqChart;
  frameTimeChart: SpFrameTimeChart;
  public arkTsChart: SpArkTsChart;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
    this.perf = new SpHiPerf(trace);
    this.fileSystem = new SpFileSystemChart(trace);
    this.cpu = new SpCpuChart(trace);
    this.freq = new SpFreqChart(trace);
    this.virtualMemChart = new SpVirtualMemChart(trace);
    this.fps = new SpFpsChart(trace);
    this.nativeMemory = new SpNativeMemoryChart(trace);
    this.abilityMonitor = new SpAbilityMonitorChart(trace);
    this.process = new SpProcessChart(trace);
    this.sdkChart = new SpSdkChart(trace);
    this.hiSyseventChart = new SpHiSysEventChart(trace);
    this.smapsChart = new VmTrackerChart(trace);
    this.clockChart = new SpClockChart(trace);
    this.irqChart = new SpIrqChart(trace);
    this.frameTimeChart = new SpFrameTimeChart(trace);
    this.arkTsChart = new SpArkTsChart(trace);
  }

  async init(progress: Function) {
    progress('load data dict', 50);
    SpSystemTrace.DATA_DICT.clear();
    SpChartManager.APP_STARTUP_PID_ARR = [];
    let dict = await queryDataDICT();
    if (FlagsConfig.getFlagsConfigEnableStatus('AppStartup')) {
      let appStartUpPids = await queryAppStartupProcessIds();
      appStartUpPids.forEach(it => SpChartManager.APP_STARTUP_PID_ARR.push(it.pid));
    }
    await this.initTraceConfig();
    dict.map((d) => SpSystemTrace.DATA_DICT.set(d['id'], d['data']));
    SpSystemTrace.DATA_TASK_POOL_CALLSTACK.clear();
    let taskPoolCallStack = await queryTaskPoolCallStack();
    taskPoolCallStack.map((d) => SpSystemTrace.DATA_TASK_POOL_CALLSTACK.set(d.id, d));
    progress('time range', 65);
    await this.initTotalTime();
    let ptArr = await queryThreadAndProcessName();
    this.handleProcessThread(ptArr);
    info('timerShaftEL Data initialized');
    progress('cpu', 70);
    await this.cpu.init();
    info('cpu Data initialized');
    progress('process/thread state', 73);
    await this.cpu.initProcessThreadStateData(progress);
    if (FlagsConfig.getFlagsConfigEnableStatus('SchedulingAnalysis')) {
      await this.cpu.initCpuIdle0Data(progress);
      await this.cpu.initSchedulingPTData(progress);
      await this.cpu.initSchedulingFreqData(progress);
    }
    info('ProcessThreadState Data initialized');
    progress('cpu rate', 75);
    await this.initCpuRate();
    info('Cpu Rate Data initialized');
    progress('cpu freq', 80);
    await this.freq.init();
    progress('Clock init', 82);
    await this.clockChart.init();
    progress('Irq init', 84);
    await this.irqChart.init();
    info('Cpu Freq Data initialized');
    await this.virtualMemChart.init();
    progress('fps', 85);
    await this.fps.init();
    info('FPS Data initialized');
    progress('native memory', 87);
    await this.nativeMemory.initChart();
    info('Native Memory Data initialized');
    progress('ability monitor', 88);
    await this.abilityMonitor.init();
    progress('hiSysevent', 88.2);
    await this.hiSyseventChart.init();
    info('Perf Files Data initialized');
    progress('vm tracker', 88.4);
    await this.smapsChart.init();
    progress('sdk', 88.6);
    await this.sdkChart.init();
    progress('perf', 88.8);
    await this.perf!.init();
    progress('file system', 89);
    await this.fileSystem!.init();
    info('Ability Monitor Data initialized');
    await perfDataQuery.initPerfCache();
    info('HiPerf Data initialized');
    progress('ark ts', 90);
    await this.arkTsChart.initFolder();
    info('ark ts initialized');
    await this.frameTimeChart.init();
    progress('process', 92);
    await this.process.initAsyncFuncData();
    await this.process.initDeliverInputEvent();
    await this.process.init();
    info('Process Data initialized');
    progress('display', 95);
  }

  async importSoFileUpdate() {
    SpSystemTrace.DATA_DICT.clear();
    let dict = await queryDataDICT();
    dict.map((d) => SpSystemTrace.DATA_DICT.set(d['id'], d['data']));
    await perfDataQuery.initPerfCache();
    await this.nativeMemory.initNativeMemory();
    await this.fileSystem.initFileCallchain();
    this.perf.updateChartData();
  }

  handleProcessThread(arr: { id: number; name: string; type: string }[]) {
    Utils.PROCESS_MAP.clear();
    Utils.THREAD_MAP.clear();
    for (let pt of arr) {
      if (pt.type === 'p') {
        Utils.PROCESS_MAP.set(pt.id, pt.name);
      } else {
        Utils.THREAD_MAP.set(pt.id, pt.name);
      }
    }
  }

  initTotalTime = async () => {
    let res = await queryTotalTime();
    if (this.trace.timerShaftEL) {
      let total = res[0].total;
      let startNS = res[0].recordStartNS;
      let endNS = res[0].recordEndNS;
      if (total === 0 && startNS === endNS) {
        total = 1;
        endNS = startNS + 1;
      }
      this.trace.timerShaftEL.totalNS = total;
      this.trace.timerShaftEL.getRangeRuler()!.drawMark = true;
      this.trace.timerShaftEL.setRangeNS(0, total);
      (window as any).recordStartNS = startNS;
      (window as any).recordEndNS = endNS;
      (window as any).totalNS = total;
      this.trace.timerShaftEL.loadComplete = true;
    }
  };

  initCpuRate = async () => {
    let rates = await getCpuUtilizationRate(0, this.trace.timerShaftEL?.totalNS || 0);
    if (this.trace.timerShaftEL) this.trace.timerShaftEL.cpuUsage = rates;
    info('Cpu UtilizationRate data size is: ', rates.length);
  };

  initTraceConfig = async (): Promise<void> => {
    queryMemoryConfig().then((result) => {
      if (result && result.length > 0) {
        const config = result[0];
        MemoryConfig.getInstance().updateConfig(config.pid, config.iPid, config.processName, config.interval);
      }
    });
  };
}

export const FolderSupplier = () => {
  return () => new Promise<Array<any>>((resolve) => resolve([]));
};
export const FolderThreadHandler = (row: TraceRow<any>, trace: SpSystemTrace) => {
  return (useCache: boolean) => {
    row.canvasSave(trace.canvasPanelCtx!);
    if (row.expansion) {
      trace.canvasPanelCtx?.clearRect(0, 0, row.frame.width, row.frame.height);
    } else {
      (renders['empty'] as EmptyRender).renderMainThread(
        {
          context: trace.canvasPanelCtx,
          useCache: useCache,
          type: ``,
        },
        row
      );
    }
    row.canvasRestore(trace.canvasPanelCtx!);
  };
};
