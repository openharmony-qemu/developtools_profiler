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
import {
  queryAbilityExits,
  queryBytesInAbilityData,
  queryBytesOutAbilityData,
  queryBytesReadAbilityData,
  queryBytesWrittenAbilityData,
  queryCachedFilesAbilityData,
  queryCompressedAbilityData,
  queryCpuAbilityData,
  queryCPuAbilityMaxData,
  queryCpuAbilitySystemData,
  queryCpuAbilityUserData,
  queryDiskIoMaxData,
  queryDmaAbilityData,
  queryGpuMemoryAbilityData,
  queryMemoryMaxData,
  queryMemoryUsedAbilityData,
  queryNetWorkMaxData,
  queryPacketsInAbilityData,
  queryPacketsOutAbilityData,
  queryPurgeableSysData,
  queryReadAbilityData,
  queryWrittenAbilityData,
} from '../../database/SqlLite.js';
import { info } from '../../../log/Log.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { Utils } from '../trace/base/Utils.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { ProcessStruct } from '../../database/ui-worker/ProcedureWorkerProcess.js';
import { CpuAbilityMonitorStruct, CpuAbilityRender } from '../../database/ui-worker/ProcedureWorkerCpuAbility.js';
import {
  MemoryAbilityMonitorStruct,
  MemoryAbilityRender,
} from '../../database/ui-worker/ProcedureWorkerMemoryAbility.js';
import {
  DiskAbilityMonitorStruct,
  DiskIoAbilityRender,
} from '../../database/ui-worker/ProcedureWorkerDiskIoAbility.js';
import {
  NetworkAbilityMonitorStruct,
  NetworkAbilityRender,
} from '../../database/ui-worker/ProcedureWorkerNetworkAbility.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { SnapshotRender, SnapshotStruct } from '../../database/ui-worker/ProcedureWorkerSnapshot.js';

export class SpAbilityMonitorChart {
  private trace: SpSystemTrace;
  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }
  memoryMath = (maxByte: number): string => {
    let maxByteName = '';
    if (maxByte > 0) {
      maxByteName = Utils.getBinaryKBWithUnit(maxByte);
    }
    return maxByteName;
  };

  diskIOMath = (maxByte: number): string => {
    let maxByteName = '';
    if (maxByte > 0) {
      maxByteName = `${maxByte}KB/S`;
    }
    return maxByteName;
  };

  networkMath = (maxValue: number): string => {
    let maxByteName = '';
    if (maxValue > 0) {
      maxByteName = Utils.getBinaryByteWithUnit(maxValue);
    }
    return maxByteName;
  };

  async init() {
    let time = new Date().getTime();
    let result = await queryAbilityExits();
    info('Ability Monitor Exits Tables size is: ', result!.length);
    if (result.length <= 0) return;
    let processRow = this.initAbilityRow();
    if (this.hasTable(result, 'trace_cpu_usage')) {
      await this.initCpuAbility(processRow);
    }
    if (this.hasTable(result, 'sys_memory')) {
      await this.initMemoryAbility(processRow);
    }
    if (this.hasTable(result, 'trace_diskio')) {
      await this.initDiskAbility(processRow);
    }
    if (this.hasTable(result, 'trace_network')) {
      await this.initNetworkAbility(processRow);
    }
    // 初始化PurgeableToTal和PurgeablePin泳道图
    await this.initPurgeableAbility(processRow);
    await this.initDmaAbility(processRow);
    await this.initGpuMemoryAbility(processRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the AbilityMonitor data is: ', durTime);
  }

  private hasTable(result: Array<any>, tableName: string) {
    return result.find((o) => {
      return o.event_name === tableName;
    });
  }

  private initAbilityRow = () => {
    let abilityRow = TraceRow.skeleton<ProcessStruct>();
    abilityRow.rowId = 'abilityMonitor';
    abilityRow.rowType = TraceRow.ROW_TYPE_MONITOR;
    abilityRow.style.height = '40px';
    abilityRow.rowParentId = '';
    abilityRow.folder = true;
    abilityRow.name = 'Ability Monitor';
    abilityRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    abilityRow.selectChangeHandler = this.trace.selectChangeHandler;
    abilityRow.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
    abilityRow.onThreadHandler = (useCache) => {
      abilityRow.canvasSave(this.trace.canvasPanelCtx!);
      if (abilityRow.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, abilityRow.frame.width, abilityRow.frame.height);
      } else {
        (renders.empty as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: '',
          },
          abilityRow
        );
      }
      abilityRow.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.trace.rowsEL?.appendChild(abilityRow);
    return abilityRow;
  };

  private initCpuAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let time = new Date().getTime();
    let cpuMaxData = await queryCPuAbilityMaxData();
    let hasTotal = false;
    let hasUserLoad = false;
    let hasSystemLoad = false;
    let userLoad = cpuMaxData[0].userLoad;
    if (userLoad > 0) {
      hasUserLoad = true;
    }
    let systemLoad = cpuMaxData[0].systemLoad;
    if (systemLoad > 0) {
      hasSystemLoad = true;
    }
    let totalLoad = cpuMaxData[0].totalLoad;
    if (totalLoad > 0) {
      hasTotal = true;
    }
    let cpuNameList: Array<string> = ['Total', 'User', 'System'];
    let traceRow = TraceRow.skeleton<CpuAbilityMonitorStruct>();
    traceRow.rowParentId = `abilityMonitor`;
    traceRow.rowHidden = !processRow.expansion;
    traceRow.rowId = cpuNameList[0];
    traceRow.rowType = TraceRow.ROW_TYPE_CPU_ABILITY;
    traceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    traceRow.selectChangeHandler = this.trace.selectChangeHandler;
    traceRow.style.height = '40px';
    traceRow.style.width = `100%`;
    traceRow.setAttribute('children', '');
    traceRow.name = `CPU ${cpuNameList[0]} Load`;
    traceRow.supplier = () => queryCpuAbilityData();
    traceRow.focusHandler = (ev) => {
      let monitorCpuTip = (CpuAbilityMonitorStruct.hoverCpuAbilityStruct?.value || 0).toFixed(2) + '%';
      this.trace?.displayTip(traceRow, CpuAbilityMonitorStruct.hoverCpuAbilityStruct, `<span>${monitorCpuTip}</span>`);
    };
    traceRow.onThreadHandler = (useCache) => {
      let context = traceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      traceRow.canvasSave(context);
      (renders['monitorCpu'] as CpuAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorCpu0`,
          maxCpuUtilization: 100,
          maxCpuUtilizationName: hasTotal ? '100%' : '0%',
        },
        traceRow
      );
      traceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(traceRow);
    let userTraceRow = TraceRow.skeleton<CpuAbilityMonitorStruct>();
    userTraceRow.rowParentId = `abilityMonitor`;
    userTraceRow.rowHidden = !processRow.expansion;
    userTraceRow.rowId = cpuNameList[1];
    userTraceRow.rowType = TraceRow.ROW_TYPE_CPU_ABILITY;
    userTraceRow.style.height = '40px';
    userTraceRow.style.width = `100%`;
    userTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    userTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    userTraceRow.setAttribute('children', '');
    userTraceRow.name = `CPU ${cpuNameList[1]} Load`;
    userTraceRow.supplier = () => queryCpuAbilityUserData();
    userTraceRow.focusHandler = (ev) => {
      let monitorCpuTip = (CpuAbilityMonitorStruct.hoverCpuAbilityStruct?.value || 0).toFixed(2) + '%';
      this.trace?.displayTip(
        userTraceRow,
        CpuAbilityMonitorStruct.hoverCpuAbilityStruct,
        `<span>${monitorCpuTip}</span>`
      );
    };
    userTraceRow.onThreadHandler = (useCache) => {
      let context = userTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      userTraceRow.canvasSave(context);
      (renders['monitorCpu'] as CpuAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorCpu1`,
          maxCpuUtilization: 100,
          maxCpuUtilizationName: hasUserLoad ? '100%' : '0%',
        },
        userTraceRow
      );
      userTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(userTraceRow);
    let sysTraceRow = TraceRow.skeleton<CpuAbilityMonitorStruct>();
    sysTraceRow.rowParentId = `abilityMonitor`;
    sysTraceRow.rowHidden = !processRow.expansion;
    sysTraceRow.rowId = cpuNameList[2];
    sysTraceRow.rowType = TraceRow.ROW_TYPE_CPU_ABILITY;
    sysTraceRow.style.height = '40px';
    sysTraceRow.style.width = `100%`;
    sysTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    sysTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    sysTraceRow.setAttribute('children', '');
    sysTraceRow.name = `CPU ${cpuNameList[2]} Load`;
    sysTraceRow.supplier = () => queryCpuAbilitySystemData();
    sysTraceRow.focusHandler = (ev) => {
      let monitorCpuTip = (CpuAbilityMonitorStruct.hoverCpuAbilityStruct?.value || 0).toFixed(2) + '%';
      this.trace?.displayTip(
        sysTraceRow,
        CpuAbilityMonitorStruct.hoverCpuAbilityStruct,
        `<span>${monitorCpuTip}</span>`
      );
    };
    sysTraceRow.onThreadHandler = (useCache) => {
      let context = sysTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      sysTraceRow.canvasSave(context);
      (renders['monitorCpu'] as CpuAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorCpu2`,
          maxCpuUtilization: 100,
          maxCpuUtilizationName: hasSystemLoad ? '100%' : '0%',
        },
        sysTraceRow
      );
      sysTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(sysTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Cpu is: ', durTime);
  };

  private initMemoryAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let time = new Date().getTime();
    // sys.mem.total  sys.mem.cached  sys.mem.swap.total
    let memoryNameList: Array<string> = ['MemoryTotal', 'Cached', 'SwapTotal'];
    let memoryTotal = await queryMemoryMaxData('sys.mem.total');
    let memoryTotalValue = memoryTotal[0].maxValue;
    let memoryTotalId = memoryTotal[0].filter_id;
    let memoryTotalValueName = this.memoryMath(memoryTotalValue);
    let memoryUsedTraceRow = TraceRow.skeleton<MemoryAbilityMonitorStruct>();
    memoryUsedTraceRow.rowParentId = `abilityMonitor`;
    memoryUsedTraceRow.rowHidden = !processRow.expansion;
    memoryUsedTraceRow.rowId = memoryNameList[0];
    memoryUsedTraceRow.rowType = TraceRow.ROW_TYPE_MEMORY_ABILITY;
    memoryUsedTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    memoryUsedTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    memoryUsedTraceRow.style.height = '40px';
    memoryUsedTraceRow.style.width = `100%`;
    memoryUsedTraceRow.setAttribute('children', '');
    memoryUsedTraceRow.name = memoryNameList[0];
    memoryUsedTraceRow.supplier = () => queryMemoryUsedAbilityData(memoryTotalId);
    memoryUsedTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        memoryUsedTraceRow,
        MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct,
        `<span>${Utils.getBinaryKBWithUnit(MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct?.value || 0)}</span>`
      );
    };
    memoryUsedTraceRow.onThreadHandler = (useCache) => {
      let context = memoryUsedTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      memoryUsedTraceRow.canvasSave(context);
      (renders['monitorMemory'] as MemoryAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorMemory0`,
          maxMemoryByte: memoryTotalValue,
          maxMemoryByteName: memoryTotalValueName,
        },
        memoryUsedTraceRow
      );
      memoryUsedTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(memoryUsedTraceRow);
    let cached = await queryMemoryMaxData('sys.mem.cached');
    let cachedValue = cached[0].maxValue;
    let cachedValueName = this.memoryMath(cachedValue);
    let cachedId = cached[0].filter_id;
    let cachedFilesTraceRow = TraceRow.skeleton<MemoryAbilityMonitorStruct>();
    cachedFilesTraceRow.rowParentId = `abilityMonitor`;
    cachedFilesTraceRow.rowHidden = !processRow.expansion;
    cachedFilesTraceRow.rowId = memoryNameList[1];
    cachedFilesTraceRow.rowType = TraceRow.ROW_TYPE_MEMORY_ABILITY;
    cachedFilesTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    cachedFilesTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    cachedFilesTraceRow.style.height = '40px';
    cachedFilesTraceRow.style.width = `100%`;
    cachedFilesTraceRow.setAttribute('children', '');
    cachedFilesTraceRow.name = memoryNameList[1];
    cachedFilesTraceRow.supplier = () => queryCachedFilesAbilityData(cachedId);
    cachedFilesTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        cachedFilesTraceRow,
        MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct,
        `<span>${Utils.getBinaryKBWithUnit(MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct?.value || 0)}</span>`
      );
    };
    cachedFilesTraceRow.onThreadHandler = (useCache) => {
      let context = cachedFilesTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      cachedFilesTraceRow.canvasSave(context);
      (renders['monitorMemory'] as MemoryAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorMemory1`,
          maxMemoryByte: cachedValue,
          maxMemoryByteName: cachedValueName,
        },
        cachedFilesTraceRow
      );
      cachedFilesTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(cachedFilesTraceRow);
    let swap = await queryMemoryMaxData('sys.mem.swap.total');
    let swapValue = swap[0].maxValue;
    let swapValueName = this.memoryMath(swapValue);
    let swapId = swap[0].filter_id;
    let compressedTraceRow = TraceRow.skeleton<MemoryAbilityMonitorStruct>();
    compressedTraceRow.rowParentId = `abilityMonitor`;
    compressedTraceRow.rowHidden = !processRow.expansion;
    compressedTraceRow.rowId = memoryNameList[2];
    compressedTraceRow.rowType = TraceRow.ROW_TYPE_MEMORY_ABILITY;
    compressedTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    compressedTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    compressedTraceRow.style.height = '40px';
    compressedTraceRow.style.width = `100%`;
    compressedTraceRow.setAttribute('children', '');
    compressedTraceRow.name = memoryNameList[2];
    compressedTraceRow.supplier = () => queryCompressedAbilityData(swapId);
    compressedTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        compressedTraceRow,
        MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct,
        `<span>${Utils.getBinaryKBWithUnit(MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct?.value || 0)}</span>`
      );
    };
    compressedTraceRow.onThreadHandler = (useCache) => {
      let context = compressedTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      compressedTraceRow.canvasSave(context);
      (renders['monitorMemory'] as MemoryAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorMemory2`,
          maxMemoryByte: swapValue,
          maxMemoryByteName: swapValueName,
        },
        compressedTraceRow
      );
      compressedTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(compressedTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Memory is: ', durTime);
  };

  private initDiskAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let time = new Date().getTime();
    let maxList = await queryDiskIoMaxData();
    let maxBytesRead = maxList[0].bytesRead;
    let maxBytesReadName = this.diskIOMath(maxBytesRead);
    let diskIONameList: Array<string> = ['Bytes Read/Sec', 'Bytes Written/Sec', 'Read Ops/Sec', 'Written Ops/Sec'];
    let bytesReadTraceRow = TraceRow.skeleton<DiskAbilityMonitorStruct>();
    bytesReadTraceRow.rowParentId = `abilityMonitor`;
    bytesReadTraceRow.rowHidden = !processRow.expansion;
    bytesReadTraceRow.rowId = diskIONameList[0];
    bytesReadTraceRow.rowType = TraceRow.ROW_TYPE_DISK_ABILITY;
    bytesReadTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    bytesReadTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    bytesReadTraceRow.style.height = '40px';
    bytesReadTraceRow.style.width = `100%`;
    bytesReadTraceRow.setAttribute('children', '');
    bytesReadTraceRow.name = 'Disk ' + diskIONameList[0];
    bytesReadTraceRow.supplier = () => queryBytesReadAbilityData();
    bytesReadTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        bytesReadTraceRow,
        DiskAbilityMonitorStruct.hoverDiskAbilityStruct,
        `<span>${DiskAbilityMonitorStruct.hoverDiskAbilityStruct?.value || '0'} KB/S</span>`
      );
    };
    bytesReadTraceRow.onThreadHandler = (useCache) => {
      let context = bytesReadTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      bytesReadTraceRow.canvasSave(context);
      (renders['monitorDiskIo'] as DiskIoAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorDiskIo0`,
          maxDiskRate: maxBytesRead,
          maxDiskRateName: maxBytesReadName,
        },
        bytesReadTraceRow
      );
      bytesReadTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(bytesReadTraceRow);
    let maxBytesWrite = maxList[0].bytesWrite;
    let maxBytesWriteName = this.diskIOMath(maxBytesWrite);
    let bytesWrittenTraceRow = TraceRow.skeleton<DiskAbilityMonitorStruct>();
    bytesWrittenTraceRow.rowParentId = `abilityMonitor`;
    bytesWrittenTraceRow.rowHidden = !processRow.expansion;
    bytesWrittenTraceRow.rowId = diskIONameList[1];
    bytesWrittenTraceRow.rowType = TraceRow.ROW_TYPE_DISK_ABILITY;
    bytesWrittenTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    bytesWrittenTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    bytesWrittenTraceRow.style.height = '40px';
    bytesWrittenTraceRow.style.width = `100%`;
    bytesWrittenTraceRow.setAttribute('children', '');
    bytesWrittenTraceRow.name = 'Disk ' + diskIONameList[1];
    bytesWrittenTraceRow.supplier = () => queryBytesWrittenAbilityData();
    bytesWrittenTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        bytesWrittenTraceRow,
        DiskAbilityMonitorStruct.hoverDiskAbilityStruct,
        `<span>${DiskAbilityMonitorStruct.hoverDiskAbilityStruct?.value || '0'} KB/S</span>`
      );
    };
    bytesWrittenTraceRow.onThreadHandler = (useCache) => {
      let context = bytesWrittenTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      bytesWrittenTraceRow.canvasSave(context);
      (renders['monitorDiskIo'] as DiskIoAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorDiskIo1`,
          maxDiskRate: maxBytesWrite,
          maxDiskRateName: maxBytesWriteName,
        },
        bytesWrittenTraceRow
      );
      bytesWrittenTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(bytesWrittenTraceRow);
    let maxReadOps = maxList[0].readOps;
    let maxReadOpsName = this.diskIOMath(maxReadOps);
    let readOpsTraceRow = TraceRow.skeleton<DiskAbilityMonitorStruct>();
    readOpsTraceRow.rowParentId = `abilityMonitor`;
    readOpsTraceRow.rowHidden = !processRow.expansion;
    readOpsTraceRow.rowId = diskIONameList[2];
    readOpsTraceRow.rowType = TraceRow.ROW_TYPE_DISK_ABILITY;
    readOpsTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    readOpsTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    readOpsTraceRow.style.height = '40px';
    readOpsTraceRow.style.width = `100%`;
    readOpsTraceRow.setAttribute('children', '');
    readOpsTraceRow.name = 'Disk ' + diskIONameList[2];
    readOpsTraceRow.supplier = () => queryReadAbilityData();
    readOpsTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        readOpsTraceRow,
        DiskAbilityMonitorStruct.hoverDiskAbilityStruct,
        `<span>${DiskAbilityMonitorStruct.hoverDiskAbilityStruct?.value || '0'} KB/S</span>`
      );
    };
    readOpsTraceRow.onThreadHandler = (useCache) => {
      let context = readOpsTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      readOpsTraceRow.canvasSave(context);
      (renders['monitorDiskIo'] as DiskIoAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorDiskIo2`,
          maxDiskRate: maxReadOps,
          maxDiskRateName: maxReadOpsName,
        },
        readOpsTraceRow
      );
      readOpsTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(readOpsTraceRow);
    let maxWriteOps = maxList[0].writeOps;
    let maxWriteOpsName = this.diskIOMath(maxWriteOps);
    let writtenOpsTraceRow = TraceRow.skeleton<DiskAbilityMonitorStruct>();
    writtenOpsTraceRow.rowParentId = `abilityMonitor`;
    writtenOpsTraceRow.rowHidden = !processRow.expansion;
    writtenOpsTraceRow.rowId = diskIONameList[3];
    writtenOpsTraceRow.rowType = TraceRow.ROW_TYPE_DISK_ABILITY;
    writtenOpsTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    writtenOpsTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    writtenOpsTraceRow.style.height = '40px';
    writtenOpsTraceRow.style.width = `100%`;
    writtenOpsTraceRow.setAttribute('children', '');
    writtenOpsTraceRow.name = 'Disk ' + diskIONameList[3];
    writtenOpsTraceRow.supplier = () => queryWrittenAbilityData();
    writtenOpsTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        writtenOpsTraceRow,
        DiskAbilityMonitorStruct.hoverDiskAbilityStruct,
        `<span>${DiskAbilityMonitorStruct.hoverDiskAbilityStruct?.value || '0'} KB/S</span>`
      );
    };
    writtenOpsTraceRow.onThreadHandler = (useCache) => {
      let context = writtenOpsTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      writtenOpsTraceRow.canvasSave(context);
      (renders['monitorDiskIo'] as DiskIoAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorDiskIo3`,
          maxDiskRate: maxWriteOps,
          maxDiskRateName: maxWriteOpsName,
        },
        writtenOpsTraceRow
      );
      writtenOpsTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(writtenOpsTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability DiskIO is: ', durTime);
  };

  private initNetworkAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let time = new Date().getTime();
    let maxList = await queryNetWorkMaxData();
    let maxBytesIn = maxList[0].maxIn;
    let maxInByteName = this.networkMath(maxBytesIn);
    let networkNameList: Array<string> = ['Bytes In/Sec', 'Bytes Out/Sec', 'Packets In/Sec', 'Packets Out/Sec'];
    let bytesInTraceRow = TraceRow.skeleton<NetworkAbilityMonitorStruct>();
    bytesInTraceRow.rowParentId = `abilityMonitor`;
    bytesInTraceRow.rowHidden = !processRow.expansion;
    bytesInTraceRow.rowId = networkNameList[0];
    bytesInTraceRow.rowType = TraceRow.ROW_TYPE_NETWORK_ABILITY;
    bytesInTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    bytesInTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    bytesInTraceRow.style.height = '40px';
    bytesInTraceRow.style.width = `100%`;
    bytesInTraceRow.setAttribute('children', '');
    bytesInTraceRow.name = 'Network ' + networkNameList[0];
    bytesInTraceRow.supplier = () => queryBytesInAbilityData();
    bytesInTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        bytesInTraceRow,
        NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct,
        `<span>${Utils.getBinaryByteWithUnit(NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct?.value || 0)}</span>`
      );
    };
    bytesInTraceRow.onThreadHandler = (useCache) => {
      let context = bytesInTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      bytesInTraceRow.canvasSave(context);
      (renders['monitorNetwork'] as NetworkAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorNetwork0`,
          maxNetworkRate: maxBytesIn,
          maxNetworkRateName: maxInByteName,
        },
        bytesInTraceRow
      );
      bytesInTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(bytesInTraceRow);
    let bytesOutTraceRow = TraceRow.skeleton<NetworkAbilityMonitorStruct>();
    let maxBytesOut = maxList[0].maxOut;
    let maxOutByteName = this.networkMath(maxBytesOut);
    bytesOutTraceRow.rowParentId = `abilityMonitor`;
    bytesOutTraceRow.rowHidden = !processRow.expansion;
    bytesOutTraceRow.rowId = networkNameList[1];
    bytesOutTraceRow.rowType = TraceRow.ROW_TYPE_NETWORK_ABILITY;
    bytesOutTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    bytesOutTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    bytesOutTraceRow.style.height = '40px';
    bytesOutTraceRow.style.width = `100%`;
    bytesOutTraceRow.setAttribute('children', '');
    bytesOutTraceRow.name = 'Network ' + networkNameList[1];
    bytesOutTraceRow.supplier = () => queryBytesOutAbilityData();
    bytesOutTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        bytesOutTraceRow,
        NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct,
        `<span>${Utils.getBinaryByteWithUnit(NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct?.value || 0)}</span>`
      );
    };
    bytesOutTraceRow.onThreadHandler = (useCache) => {
      let context = bytesOutTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      bytesOutTraceRow.canvasSave(context);
      (renders['monitorNetwork'] as NetworkAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorNetwork1`,
          maxNetworkRate: maxBytesOut,
          maxNetworkRateName: maxOutByteName,
        },
        bytesOutTraceRow
      );
      bytesOutTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(bytesOutTraceRow);
    let packetInTraceRow = TraceRow.skeleton<NetworkAbilityMonitorStruct>();
    let maxPacketIn = maxList[0].maxPacketIn;
    let maxInPacketName = this.networkMath(maxPacketIn);
    packetInTraceRow.rowParentId = `abilityMonitor`;
    packetInTraceRow.rowHidden = !processRow.expansion;
    packetInTraceRow.rowId = networkNameList[2];
    packetInTraceRow.rowType = TraceRow.ROW_TYPE_NETWORK_ABILITY;
    packetInTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    packetInTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    packetInTraceRow.style.height = '40px';
    packetInTraceRow.style.width = `100%`;
    packetInTraceRow.setAttribute('children', '');
    packetInTraceRow.name = 'Network ' + networkNameList[2];
    packetInTraceRow.supplier = () => queryPacketsInAbilityData();
    packetInTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        packetInTraceRow,
        NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct,
        `<span>${Utils.getBinaryByteWithUnit(NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct?.value || 0)}</span>`
      );
    };
    packetInTraceRow.onThreadHandler = (useCache) => {
      let context = packetInTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      packetInTraceRow.canvasSave(context);
      (renders['monitorNetwork'] as NetworkAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorNetwork-Packet2`,
          maxNetworkRate: maxPacketIn,
          maxNetworkRateName: maxInPacketName,
        },
        packetInTraceRow
      );
      packetInTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(packetInTraceRow);
    let packetOutTraceRow = TraceRow.skeleton<NetworkAbilityMonitorStruct>();
    let maxPacketOut = maxList[0].maxPacketOut;
    let maxOutPacketName = this.networkMath(maxPacketOut);
    packetOutTraceRow.rowParentId = `abilityMonitor`;
    packetOutTraceRow.rowHidden = !processRow.expansion;
    packetOutTraceRow.rowId = networkNameList[3];
    packetOutTraceRow.rowType = TraceRow.ROW_TYPE_NETWORK_ABILITY;
    packetOutTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    packetOutTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    packetOutTraceRow.style.height = '40px';
    packetOutTraceRow.style.width = `100%`;
    packetOutTraceRow.setAttribute('children', '');
    packetOutTraceRow.name = 'Network ' + networkNameList[3];
    packetOutTraceRow.supplier = () => queryPacketsOutAbilityData();
    packetOutTraceRow.focusHandler = (ev) => {
      if (NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct) {
        this.trace?.displayTip(
          packetOutTraceRow,
          NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct,
          `<span>${Utils.getBinaryByteWithUnit(NetworkAbilityMonitorStruct.hoverNetworkAbilityStruct!.value!)}</span>`
        );
      }
    };
    packetOutTraceRow.onThreadHandler = (useCache) => {
      let context = packetOutTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      packetOutTraceRow.canvasSave(context);
      (renders['monitorNetwork'] as NetworkAbilityRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `monitorNetwork3`,
          maxNetworkRate: maxPacketOut,
          maxNetworkRateName: maxOutPacketName,
        },
        packetOutTraceRow
      );
      packetOutTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(packetOutTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Network is: ', durTime);
  };

  private initPurgeableAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let time = new Date().getTime();
    let purgeableTotalData = await queryPurgeableSysData();
    if (purgeableTotalData.length > 0) {
      for (let i = 0; i < purgeableTotalData.length; i++) {
        purgeableTotalData[i].name = 'Snapshot' + i;
      }
      let totalTraceRow = TraceRow.skeleton<SnapshotStruct>();
      totalTraceRow.rowParentId = `abilityMonitor`;
      totalTraceRow.rowHidden = !processRow.expansion;
      totalTraceRow.rowId = 'System Purgeable Total';
      totalTraceRow.rowType = TraceRow.ROW_TYPE_PURGEABLE_TOTAL_ABILITY;
      totalTraceRow.style.height = '40px';
      totalTraceRow.style.width = `100%`;
      totalTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      totalTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
      totalTraceRow.setAttribute('children', '');
      totalTraceRow.name = `Purgeable Total`;
      totalTraceRow.supplier = () => new Promise<Array<any>>((resolve) => resolve(purgeableTotalData));
      totalTraceRow.focusHandler = (ev) => {
        this.trace?.displayTip(
          totalTraceRow,
          SnapshotStruct.hoverSnapshotStruct,
          `<span>Name: ${SnapshotStruct.hoverSnapshotStruct?.name || ''}</span>
			   <span>Value: ${Utils.getBinaryByteWithUnit(SnapshotStruct.hoverSnapshotStruct?.value || 0)}</span>`
        );
      };
      totalTraceRow.onThreadHandler = (useCache) => {
        let context = totalTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        totalTraceRow.canvasSave(context);
        (renders['snapshot'] as SnapshotRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: `snapshot`,
          },
          totalTraceRow
        );
        totalTraceRow.canvasRestore(context);
      };
      processRow.addChildTraceRow(totalTraceRow);
    }
    let purgeablePinData = await queryPurgeableSysData(true);
    if (purgeablePinData.length > 0) {
      for (let i = 0; i < purgeablePinData.length; i++) {
        purgeablePinData[i].name = 'Snapshot' + i;
      }
      let pinTraceRow = TraceRow.skeleton<SnapshotStruct>();
      pinTraceRow.rowParentId = `abilityMonitor`;
      pinTraceRow.rowHidden = !processRow.expansion;
      pinTraceRow.rowId = 'System Purgeable Pin';
      pinTraceRow.rowType = TraceRow.ROW_TYPE_PURGEABLE_PIN_ABILITY;
      pinTraceRow.style.height = '40px';
      pinTraceRow.style.width = `100%`;
      pinTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
      pinTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
      pinTraceRow.setAttribute('children', '');
      pinTraceRow.name = `Purgeable Pin`;
      pinTraceRow.supplier = () => new Promise<Array<any>>((resolve) => resolve(purgeablePinData));
      pinTraceRow.focusHandler = (ev) => {
        this.trace?.displayTip(
          pinTraceRow,
          SnapshotStruct.hoverSnapshotStruct,
          `<span>Name: ${SnapshotStruct.hoverSnapshotStruct?.name || ''}</span>
         <span>Value: ${Utils.getBinaryByteWithUnit(SnapshotStruct.hoverSnapshotStruct?.value || 0)}</span>`
        );
      };
      pinTraceRow.onThreadHandler = (useCache) => {
        let context = pinTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
        pinTraceRow.canvasSave(context);
        (renders['snapshot'] as SnapshotRender).renderMainThread(
          {
            context: context,
            useCache: useCache,
            type: `snapshot`,
          },
          pinTraceRow
        );
        pinTraceRow.canvasRestore(context);
      };
      processRow.addChildTraceRow(pinTraceRow);
    }

    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Purgeable is: ', durTime);
  };

  /**
   * DMA
   * @param processRow
   */
  private initDmaAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let dmaAbilityData = await queryDmaAbilityData();
    for (let i = 0; i < dmaAbilityData.length; i++) {
      dmaAbilityData[i].name = 'snapshot' + i;
    }
    let time = new Date().getTime();
    let dmaTraceRow = TraceRow.skeleton<SnapshotStruct>();
    dmaTraceRow.rowParentId = `abilityMonitor`;
    dmaTraceRow.rowHidden = !processRow.expansion;
    dmaTraceRow.rowId = 'abilityMonitorDma';
    dmaTraceRow.rowType = TraceRow.ROW_TYPE_DMA_ABILITY;
    dmaTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    dmaTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    dmaTraceRow.style.height = '40px';
    dmaTraceRow.style.width = `100%`;
    dmaTraceRow.setAttribute('children', '');
    dmaTraceRow.name = 'DMA';
    dmaTraceRow.supplier = () => new Promise<Array<any>>((resolve) => resolve(dmaAbilityData));
    //文字悬浮提示
    dmaTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        dmaTraceRow,
        SnapshotStruct.hoverSnapshotStruct,
        `<span>${SnapshotStruct.hoverSnapshotStruct?.name || ''}</span>
         <span>${Utils.getBinaryByteWithUnit(SnapshotStruct.hoverSnapshotStruct?.value || 0)}</span>`
      );
    };
    dmaTraceRow.onThreadHandler = (useCache) => {
      let context = dmaTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      dmaTraceRow.canvasSave(context);
      (renders.snapshot as SnapshotRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'snapshot',
        },
        dmaTraceRow!
      );
      dmaTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(dmaTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Dma is: ', durTime);
  };

  /**
   * Skia Gpu Memory
   * @param processRow
   */
  private initGpuMemoryAbility = async (processRow: TraceRow<ProcessStruct>) => {
    let gpuMemoryAbilityData = await queryGpuMemoryAbilityData();
    for (let i = 0; i < gpuMemoryAbilityData.length; i++) {
      gpuMemoryAbilityData[i].name = 'snapshot' + i;
    }
    let time = new Date().getTime();
    let gpuMemoryTraceRow = TraceRow.skeleton<SnapshotStruct>();
    gpuMemoryTraceRow.rowParentId = `abilityMonitor`;
    gpuMemoryTraceRow.rowHidden = !processRow.expansion;
    gpuMemoryTraceRow.rowId = 'abilityMonitorGpuMemory';
    gpuMemoryTraceRow.rowType = TraceRow.ROW_TYPE_GPU_MEMORY_ABILITY;
    gpuMemoryTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    gpuMemoryTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    gpuMemoryTraceRow.style.height = '40px';
    gpuMemoryTraceRow.style.width = `100%`;
    gpuMemoryTraceRow.setAttribute('children', '');
    gpuMemoryTraceRow.name = 'Skia Gpu Memory';
    gpuMemoryTraceRow.supplier = () => new Promise<Array<any>>((resolve) => resolve(gpuMemoryAbilityData));
    //文字悬浮提示
    gpuMemoryTraceRow.focusHandler = (ev) => {
      this.trace?.displayTip(
        gpuMemoryTraceRow,
        SnapshotStruct.hoverSnapshotStruct,
        `<span>${SnapshotStruct.hoverSnapshotStruct?.name || ''}</span>
        <span>${Utils.getBinaryByteWithUnit(SnapshotStruct.hoverSnapshotStruct?.value || 0)}</span>`
      );
    };
    gpuMemoryTraceRow.onThreadHandler = (useCache) => {
      let context = gpuMemoryTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      gpuMemoryTraceRow.canvasSave(context);
      (renders.snapshot as SnapshotRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'snapshot',
        },
        gpuMemoryTraceRow!
      );
      gpuMemoryTraceRow.canvasRestore(context);
    };
    processRow.addChildTraceRow(gpuMemoryTraceRow);
    let durTime = new Date().getTime() - time;
    info('The time to load the Ability Dma is: ', durTime);
  };
}
