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

import { CpuRender, EmptyRender } from './ProcedureWorkerCPU.js';
import { RequestMessage } from './ProcedureWorkerCommon.js';
import { FreqRender } from './ProcedureWorkerFreq.js';
import { ProcessRender } from './ProcedureWorkerProcess.js';
import { MemRender } from './ProcedureWorkerMem.js';
import { ThreadRender } from './ProcedureWorkerThread.js';
import { FuncRender } from './ProcedureWorkerFunc.js';
import { FpsRender } from './ProcedureWorkerFPS.js';
import { HeapRender, NativeMemoryRender } from './ProcedureWorkerHeap.js';
import { CpuAbilityRender } from './ProcedureWorkerCpuAbility.js';
import { MemoryAbilityRender } from './ProcedureWorkerMemoryAbility.js';
import { DiskIoAbilityRender } from './ProcedureWorkerDiskIoAbility.js';
import { NetworkAbilityRender } from './ProcedureWorkerNetworkAbility.js';
import { HiperfCpuRender } from './ProcedureWorkerHiPerfCPU.js';
import { HiperfProcessRender } from './ProcedureWorkerHiPerfProcess.js';
import { HiperfThreadRender } from './ProcedureWorkerHiPerfThread.js';
import { HiperfEventRender } from './ProcedureWorkerHiPerfEvent.js';
import { HiperfReportRender } from './ProcedureWorkerHiPerfReport.js';
import { VirtualMemoryRender } from './ProcedureWorkerVirtualMemory.js';
import { FileSystemRender } from './ProcedureWorkerFileSystem.js';
import { info } from '../../../log/Log.js';
import { SdkSliceRender } from './ProduceWorkerSdkSlice.js';
import { SdkCounterRender } from './ProduceWorkerSdkCounter.js';
import { CpuStateRender } from './ProcedureWorkerCpuState.js';
import { EnergyAnomalyRender } from './ProcedureWorkerEnergyAnomaly.js';
import { EnergySystemRender } from './ProcedureWorkerEnergySystem.js';
import { EnergyPowerRender } from './ProcedureWorkerEnergyPower.js';
import { EnergyStateRender } from './ProcedureWorkerEnergyState.js';
import { CpuFreqLimitRender } from './ProcedureWorkerCpuFreqLimits.js';
import { ClockRender } from './ProcedureWorkerClock.js';
import { IrqRender } from './ProcedureWorkerIrq.js';
import { JankRender } from './ProcedureWorkerJank.js';
import { HeapTimelineRender } from './ProcedureWorkerHeapTimeline.js';
import { HeapSnapshotRender } from './ProcedureWorkerHeapSnapshot.js';
import { translateJsonString } from '../logic-worker/ProcedureLogicWorkerCommon.js';
import { AppStartupRender } from './ProcedureWorkerAppStartup.js';
import { SoRender } from './ProcedureWorkerSoInit.js';
import { FrameDynamicRender } from './ProcedureWorkerFrameDynamic.js';
import { FrameAnimationRender } from './ProcedureWorkerFrameAnimation.js';
import { FrameSpacingRender } from './ProcedureWorkerFrameSpacing.js';
import { JsCpuProfilerRender } from './ProcedureWorkerCpuProfiler.js';
import { SnapshotRender } from './ProcedureWorkerSnapshot.js';

let dataList: any = {};
let dataList2: any = {};
let dataFilter: any = {};
let canvasList: any = {};
let contextList: any = {};
export let renders: any = {
  'cpu-data': new CpuRender(),
  'cpu-state': new CpuStateRender(),
  'cpu-limit-freq': new CpuFreqLimitRender(),
  fps: new FpsRender(),
  freq: new FreqRender(),
  empty: new EmptyRender(),
  'virtual-memory-folder': new EmptyRender(),
  'virtual-memory-cell': new VirtualMemoryRender(),
  'file-system-group': new EmptyRender(),
  'file-system-cell': new FileSystemRender(),
  process: new ProcessRender(),
  'app-start-up': new AppStartupRender(),
  'app-so-init': new SoRender(),
  heap: new HeapRender(),
  'heap-timeline': new HeapTimelineRender(),
  'heap-snapshot': new HeapSnapshotRender(),
  mem: new MemRender(),
  thread: new ThreadRender(),
  func: new FuncRender(),
  native: new NativeMemoryRender(),
  'HiPerf-Group': new EmptyRender(),
  monitorGroup: new EmptyRender(),
  'HiPerf-Cpu': new HiperfCpuRender(),
  'HiPerf-Process': new HiperfProcessRender(),
  'HiPerf-Thread': new HiperfThreadRender(),
  'HiPerf-Report-Event': new HiperfEventRender(),
  'HiPerf-Report-Fold': new HiperfReportRender(),
  monitorCpu: new CpuAbilityRender(),
  monitorMemory: new MemoryAbilityRender(),
  monitorDiskIo: new DiskIoAbilityRender(),
  monitorNetwork: new NetworkAbilityRender(),
  'sdk-slice': new SdkSliceRender(),
  'sdk-counter': new SdkCounterRender(),
  energyAnomaly: new EnergyAnomalyRender(),
  energySystem: new EnergySystemRender(),
  energyPower: new EnergyPowerRender(),
  energyState: new EnergyStateRender(),
  clock: new ClockRender(),
  irq: new IrqRender(),
  jank: new JankRender(),
  frameDynamicCurve: new FrameDynamicRender(),
  frameAnimation: new FrameAnimationRender(),
  frameSpacing: new FrameSpacingRender(),
  'js-cpu-profiler': new JsCpuProfilerRender(),
  snapshot: new SnapshotRender(),
};

function match(type: string, req: RequestMessage) {
  Reflect.ownKeys(renders).filter((it) => {
    if (type.startsWith(it as string)) {
      if (dataList[type]) {
        req.lazyRefresh = dataList[type].length > 20000;
      }
      renders[it].render(req, dataList[type], dataFilter[type], dataList2);
    }
  });
}

let dec = new TextDecoder();
let convertJSON = (arr: any) => {
  if (arr instanceof ArrayBuffer) {
    let jsonArr = [];
    let str = dec.decode(new Uint8Array(arr));
    str = str.substring(str.indexOf('\n') + 1);
    if (!str) {
    } else {
      let parsed = JSON.parse(translateJsonString(str));
      let columns = parsed.columns;
      let values = parsed.values;
      for (let i = 0; i < values.length; i++) {
        let obj: any = {};
        for (let j = 0; j < columns.length; j++) {
          obj[columns[j]] = values[i][j];
        }
        jsonArr.push(obj);
      }
    }
    return jsonArr;
  } else {
    return arr;
  }
};
self.onmessage = function (e: any) {
  if ((e.data.type as string).startsWith('clear')) {
    dataList = {};
    dataList2 = {};
    dataFilter = {};
    canvasList = {};
    contextList = {};
    // @ts-ignore
    self.postMessage({
      id: e.data.id,
      type: e.data.type,
      results: null,
    });
    return;
  }
  if (e.data.params.list) {
    dataList[e.data.type] = convertJSON(e.data.params.list);
    if (e.data.params.offscreen) {
      canvasList[e.data.type] = e.data.params.offscreen;
      contextList[e.data.type] = e.data.params.offscreen!.getContext('2d');
      contextList[e.data.type].scale(e.data.params.dpr, e.data.params.dpr);
    }
  }
  if (!dataFilter[e.data.type]) {
    dataFilter[e.data.type] = [];
  }
  let req = new RequestMessage();
  req.canvas = canvasList[e.data.type];
  req.context = contextList[e.data.type];
  req.type = e.data.type as string;
  req.params = e.data.params;
  req.online = e.data.params.online;
  req.buf = e.data.params.buf;
  req.isRangeSelect = e.data.params.isRangeSelect;
  req.isHover = e.data.params.isHover;
  req.xs = e.data.params.xs;
  req.frame = e.data.params.frame;
  req.flagMoveInfo = e.data.params.flagMoveInfo;
  req.flagSelectedInfo = e.data.params.flagSelectedInfo;
  req.hoverX = e.data.params.hoverX;
  req.hoverY = e.data.params.hoverY;
  req.startNS = e.data.params.startNS;
  req.endNS = e.data.params.endNS;
  req.totalNS = e.data.params.totalNS;
  req.slicesTime = e.data.params.slicesTime;
  req.range = e.data.params.range;
  req.scale = e.data.params.scale;
  req.canvasWidth = e.data.params.canvasWidth;
  req.canvasHeight = e.data.params.canvasHeight;
  req.useCache = e.data.params.useCache;
  req.lineColor = e.data.params.lineColor;
  req.chartColor = e.data.params.chartColor;
  req.wakeupBean = e.data.params.wakeupBean;
  req.intervalPerf = e.data.params.intervalPerf;
  req.id = e.data.id;
  if (!req.frame) {
    info(req.frame);
    return;
  }
  if (req.canvas) {
    if (req.canvas.width !== req.canvasWidth || req.canvas.height !== req.canvasHeight) {
      req.canvas.width = req.canvasWidth;
      req.canvas.height = req.canvasHeight;
      req.context.scale(e.data.params.dpr, e.data.params.dpr);
    }
  }
  match(req.type, req);
};
self.onmessageerror = function (e: any) {};
