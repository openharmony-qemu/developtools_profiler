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
  queryDmaSampsData,
  queryGpuMemoryData,
  querySmapsData,
  querySmapsExits,
  queryVmTrackerShmData,
  queryPurgeableProcessData,
  queryGpuGLData,
  queryGpuTotalData,
  queryGpuTotalType,
  queryGpuWindowData,
  queryGpuWindowType,
} from '../../database/SqlLite.js';
import { TraceRow } from '../trace/base/TraceRow.js';
import { BaseStruct } from '../../bean/BaseStruct.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { Utils } from '../trace/base/Utils.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { info } from '../../../log/Log.js';
import { SnapshotRender, SnapshotStruct } from '../../database/ui-worker/ProcedureWorkerSnapshot.js';
import { TreeItemData } from '../../../base-ui/tree/LitTree.js';
import { MemoryConfig } from '../../bean/MemoryConfig.js';

export class VmTrackerChart {
  private trace: SpSystemTrace;
  private rowFolder!: TraceRow<BaseStruct>;
  private sMapsFolder!: TraceRow<BaseStruct>;
  private gpuFolder!: TraceRow<BaseStruct>;
  private memoryConfig: MemoryConfig = MemoryConfig.getInstance();
  static gpuTotalModule: number | null = null; //ns
  static gpuWindow: number | null = null; //ns
  static gpuWindowModule: number | null = null; //ns
  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init(): Promise<void> {
    const result = await querySmapsExits();
    if (result.length <= 0) {
      return;
    }
    await this.initVmTrackerFolder();
    await this.initSMapsFolder();
    const rowNameList: Array<string> = ['Dirty', 'Swapped', 'RSS', 'PSS', 'USS'];
    for (const rowName of rowNameList) {
      await this.initSmapsRows(rowName);
    }
    await this.initShmRows();
    await this.initPurgeableVM();
    await this.initDmaRow();
    const gpuMemoryData = await queryGpuMemoryData(this.memoryConfig.iPid);
    const glArr = await queryGpuGLData(MemoryConfig.getInstance().iPid).then((res) => {
      res.forEach((gl, index) => {
        (gl as any).name = `SnapShot ${index}`;
      });
      return res as SnapshotStruct[];
    });
    if (gpuMemoryData.length > 0 || glArr.length > 0) {
      await this.initGpuFolder();
      if (gpuMemoryData.length > 0) {
        await this.initGpuMemoryRow(gpuMemoryData);
      }
      await this.addGpuGLRow(glArr);
      if (glArr.length > 0) {
        await this.addGpuTotalRow();
        await this.addGpuWindowRow();
      }
    }
  }

  private initVmTrackerFolder = async (): Promise<void> => {
    let VmTrackerRow = TraceRow.skeleton();
    VmTrackerRow.rowId = 'VmTrackerRow';
    VmTrackerRow.rowType = TraceRow.ROW_TYPE_VM_TRACKER;
    VmTrackerRow.rowParentId = '';
    VmTrackerRow.style.height = '40px';
    VmTrackerRow.index = 0;
    VmTrackerRow.folder = true;
    VmTrackerRow.name = `VM Tracker (${this.memoryConfig.processName} ${this.memoryConfig.pid})`;
    VmTrackerRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    VmTrackerRow.selectChangeHandler = this.trace.selectChangeHandler;
    VmTrackerRow.supplier = (): Promise<Array<SnapshotStruct>> =>
      new Promise<Array<SnapshotStruct>>((resolve) => resolve([]));
    VmTrackerRow.onThreadHandler = (useCache): void => {
      VmTrackerRow.canvasSave(this.trace.canvasPanelCtx!);
      if (VmTrackerRow.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, VmTrackerRow.frame.width, VmTrackerRow.frame.height);
      } else {
        (renders.empty as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: '',
          },
          VmTrackerRow
        );
      }
      VmTrackerRow.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.rowFolder = VmTrackerRow;
    this.trace.rowsEL?.appendChild(VmTrackerRow);
  };

  private initSMapsFolder = async (): Promise<void> => {
    let sMapsRow = TraceRow.skeleton<SnapshotStruct>();
    sMapsRow.rowId = 'smapsRow';
    sMapsRow.rowParentId = 'VmTrackerRow';
    sMapsRow.rowHidden = !this.rowFolder.expansion;
    sMapsRow.rowType = TraceRow.ROW_TYPE_VM_TRACKER_SMAPS;
    sMapsRow.folder = true;
    sMapsRow.name = 'Smaps';
    sMapsRow.folderPaddingLeft = 20;
    sMapsRow.style.height = '40px';
    sMapsRow.style.width = '100%';
    sMapsRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    sMapsRow.selectChangeHandler = this.trace.selectChangeHandler;
    sMapsRow.supplier = (): Promise<Array<SnapshotStruct>> =>
      new Promise<Array<SnapshotStruct>>((resolve) => resolve([]));
    sMapsRow.onThreadHandler = (useCache): void => {
      sMapsRow.canvasSave(this.trace.canvasPanelCtx!);
      if (sMapsRow.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, sMapsRow.frame.width, sMapsRow.frame.height);
      } else {
        (renders.empty as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: '',
          },
          sMapsRow
        );
      }
      sMapsRow.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.sMapsFolder = sMapsRow;
    this.rowFolder?.addChildTraceRow(sMapsRow);
  };

  private initGpuFolder = async (): Promise<TraceRow<SnapshotStruct>> => {
    let gpuTraceRow = TraceRow.skeleton<SnapshotStruct>();
    gpuTraceRow.rowId = 'skiaGpuTraceRow';
    gpuTraceRow.rowType = TraceRow.ROW_TYPE_SYS_MEMORY_GPU;
    gpuTraceRow.rowParentId = 'VmTrackerRow';
    gpuTraceRow.style.height = '40px';
    gpuTraceRow.folder = true;
    gpuTraceRow.folderPaddingLeft = 20;
    gpuTraceRow.rowHidden = !this.rowFolder.expansion;
    gpuTraceRow.name = 'GPU';
    gpuTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    gpuTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    gpuTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
      new Promise<Array<SnapshotStruct>>((resolve) => resolve([]));
    gpuTraceRow.onThreadHandler = (useCache): void => {
      gpuTraceRow.canvasSave(this.trace.canvasPanelCtx!);
      if (gpuTraceRow.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, gpuTraceRow.frame.width, gpuTraceRow.frame.height);
      } else {
        (renders.empty as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: '',
          },
          gpuTraceRow
        );
      }
      gpuTraceRow.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.gpuFolder = gpuTraceRow;
    this.rowFolder.addChildTraceRow(gpuTraceRow);
    return gpuTraceRow;
  };

  private getSmapsKeyName(rowName: string): string {
    let columnName = rowName.toLowerCase();
    let keyName = '';
    switch (rowName) {
      case 'USS':
        keyName = 'private_clean + private_dirty';
        break;
      case 'RSS':
        keyName = 'resident_size';
        break;
      default:
        keyName = columnName;
    }
    return keyName;
  }

  private initSmapsRows = async (rowName: string): Promise<void> => {
    let sMapsTraceRow = this.initTraceRow(rowName, TraceRow.ROW_TYPE_VM_TRACKER_SMAPS, 'smapsRow');
    sMapsTraceRow.rowHidden = !this.sMapsFolder.expansion;
    sMapsTraceRow.folderTextLeft = 40;
    let sMapsData = await querySmapsData(this.getSmapsKeyName(rowName));
    for (let i = 0; i < sMapsData.length; i++) {
      sMapsData[i].name = `Snapshot${i}`;
    }
    sMapsTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
      new Promise<Array<SnapshotStruct>>((resolve) => resolve(sMapsData));
    this.sMapsFolder.addChildTraceRow(sMapsTraceRow);
  };

  private initShmRows = async (): Promise<void> => {
    let shmTraceRow = this.initTraceRow('SHM', TraceRow.ROW_TYPE_VMTRACKER_SHM, 'VmTrackerRow');
    let shmData = await queryVmTrackerShmData(this.memoryConfig.iPid);
    for (let i = 0; i < shmData.length; i++) {
      shmData[i].name = `Snapshot${i}`;
    }
    shmTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
      new Promise<Array<SnapshotStruct>>((resolve) => resolve(shmData));
    if (shmData.length > 0) {
      this.rowFolder.addChildTraceRow(shmTraceRow);
    }
  };

  private async initPurgeableTotal(): Promise<void> {
    let purgeableTotalData = await queryPurgeableProcessData(this.memoryConfig.iPid);
    if (purgeableTotalData.length > 0) {
      for (let i = 0; i < purgeableTotalData.length; i++) {
        purgeableTotalData[i].name = `Snapshot${i}`;
      }
      let totalTraceRow = this.initTraceRow('Purgeable Total', TraceRow.ROW_TYPE_PURGEABLE_TOTAL_VM, 'VmTrackerRow');
      totalTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
        new Promise<Array<SnapshotStruct>>((resolve) => resolve(purgeableTotalData));
      this.rowFolder.addChildTraceRow(totalTraceRow);
    }
  }

  private async initPurgeablePin(): Promise<void> {
    let purgeablePinData = await queryPurgeableProcessData(this.memoryConfig.iPid, true);
    if (purgeablePinData.length > 0) {
      for (let i = 0; i < purgeablePinData.length; i++) {
        purgeablePinData[i].name = `Snapshot${i}`;
      }
      let pinTraceRow = this.initTraceRow('Purgeable Pin', TraceRow.ROW_TYPE_PURGEABLE_PIN_VM, 'VmTrackerRow');
      pinTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
        new Promise<Array<SnapshotStruct>>((resolve) => resolve(purgeablePinData));
      this.rowFolder.addChildTraceRow(pinTraceRow);
    }
  }

  private initPurgeableVM = async (): Promise<void> => {
    let time = new Date().getTime();
    await this.initPurgeableTotal();
    await this.initPurgeablePin();
    let durTime = new Date().getTime() - time;
    info('The time to load the VM Purgeable is: ', durTime);
  };

  private initDmaRow = async (): Promise<void> => {
    let dmaData = await queryDmaSampsData(this.memoryConfig.iPid);
    if (dmaData.length > 0) {
      for (let i = 0; i < dmaData.length; i++) {
        dmaData[i].name = `Snapshot${i}`;
      }
      let dmaTraceRow = this.initTraceRow('DMA', TraceRow.ROW_TYPE_DMA_VMTRACKER, 'VmTrackerRow');
      dmaTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
        new Promise<Array<SnapshotStruct>>((resolve) => resolve(dmaData));
      this.rowFolder.addChildTraceRow(dmaTraceRow);
    }
  };

  private initGpuMemoryRow = async (gpuMemoryData: Array<SnapshotStruct>): Promise<void> => {
    for (let i = 0; i < gpuMemoryData.length; i++) {
      gpuMemoryData[i].name = `Snapshot${i}`;
    }
    let gpuMemoryTraceRow = this.initTraceRow(
      'Skia Gpu Memory',
      TraceRow.ROW_TYPE_GPU_MEMORY_VMTRACKER,
      'skiaGpuTraceRow'
    );
    gpuMemoryTraceRow.rowHidden = !this.gpuFolder.expansion;
    gpuMemoryTraceRow.folderTextLeft = 40;
    gpuMemoryTraceRow.supplier = (): Promise<Array<SnapshotStruct>> =>
      new Promise<Array<SnapshotStruct>>((resolve) => resolve(gpuMemoryData));
    this.gpuFolder.addChildTraceRow(gpuMemoryTraceRow);
  };

  private async addGpuGLRow(glArr: Array<SnapshotStruct>): Promise<void> {
    if (glArr.length > 0) {
      let glRow = this.initTraceRow('GL', TraceRow.ROW_TYPE_SYS_MEMORY_GPU_GL, this.gpuFolder.rowId!);
      glRow.addTemplateTypes('sys-memory');
      glRow.folderTextLeft = 40;
      glRow.supplier = () => new Promise((resolve) => resolve(glArr));
      this.gpuFolder.addChildTraceRow(glRow);
    }
  }

  private async addGpuTotalRow(): Promise<void> {
    let types = await queryGpuTotalType();
    let gpuTotalRow = this.initTraceRow(
      'Skia Gpu Dump Total',
      TraceRow.ROW_TYPE_SYS_MEMORY_GPU_TOTAL,
      this.gpuFolder.rowId!
    );
    gpuTotalRow.folderTextLeft = 40;
    gpuTotalRow.addTemplateTypes('sys-memory');
    gpuTotalRow.rowSetting = 'enable';
    gpuTotalRow.rowSettingList = [
      {
        key: 'total',
        title: 'Total',
        checked: true,
      },
      ...types.map((it) => {
        return {
          key: `${it.id}`,
          title: it.data,
        };
      }),
    ];
    gpuTotalRow.onRowSettingChangeHandler = (setting) => {
      if (setting && setting.length > 0) {
        gpuTotalRow.dataListCache = [];
        gpuTotalRow.dataList = [];
        gpuTotalRow.isComplete = false;
        VmTrackerChart.gpuTotalModule = setting[0] === 'total' ? null : parseInt(setting[0]);
        this.trace.refreshCanvas(false);
      }
    };
    gpuTotalRow.supplier = (): Promise<Array<SnapshotStruct>> => {
      return queryGpuTotalData(VmTrackerChart.gpuTotalModule).then((res) => {
        res.forEach((it, index) => {
          (it as any).name = `SnapShot ${index}`;
        });
        return res as SnapshotStruct[];
      });
    };
    this.gpuFolder.addChildTraceRow(gpuTotalRow);
  }

  private async addGpuWindowRow(): Promise<void> {
    let types = await queryGpuWindowType();
    let settings: TreeItemData[] = types
      .filter((it) => it.pid === null)
      .map((it) => {
        return {
          key: `${it.id}`,
          title: it.data,
          children: [],
        };
      });
    settings.forEach((it) => {
      it.children = types
        .filter((child) => `${child.pid}` === it.key)
        .map((item) => {
          return {
            key: `${it.key}-${item.id}`,
            title: item.data,
          };
        });
    });
    settings[0].checked = true;
    VmTrackerChart.gpuWindow = parseInt(settings[0].key);
    VmTrackerChart.gpuWindowModule = null;
    let gpuWindowRow = this.initTraceRow(
      'Skia Gpu Dump Window',
      TraceRow.ROW_TYPE_SYS_MEMORY_GPU_WINDOW,
      this.gpuFolder.rowId!
    );
    gpuWindowRow.folderTextLeft = 40;
    gpuWindowRow.rowSetting = 'enable';
    gpuWindowRow.rowSettingList = settings;
    gpuWindowRow.addTemplateTypes('sys-memory');
    gpuWindowRow.onRowSettingChangeHandler = (setting) => {
      if (setting && setting.length > 0) {
        let split = setting[0].split('-');
        VmTrackerChart.gpuWindow = parseInt(split[0]);
        VmTrackerChart.gpuWindowModule = split.length > 1 ? parseInt(split[1]) : null;
        gpuWindowRow.dataListCache = [];
        gpuWindowRow.dataList = [];
        gpuWindowRow.isComplete = false;
        this.trace.refreshCanvas(false);
      }
    };
    gpuWindowRow.supplier = () => {
      return queryGpuWindowData(VmTrackerChart.gpuWindow!, VmTrackerChart.gpuWindowModule).then((res) => {
        res.forEach((window, index) => {
          (window as any).name = `SnapShot ${index}`;
        });
        return res as SnapshotStruct[];
      });
    };
    this.gpuFolder.addChildTraceRow(gpuWindowRow);
  }

  private initTraceRow(rowName: string, type: string, rowParentId: string): TraceRow<SnapshotStruct> {
    let vmTrackerTraceRow = TraceRow.skeleton<SnapshotStruct>();
    vmTrackerTraceRow.rowParentId = rowParentId;
    vmTrackerTraceRow.rowId = rowName;
    vmTrackerTraceRow.rowType = type;
    vmTrackerTraceRow.folderTextLeft = 20;
    vmTrackerTraceRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    vmTrackerTraceRow.selectChangeHandler = this.trace.selectChangeHandler;
    vmTrackerTraceRow.style.height = '40px';
    vmTrackerTraceRow.style.width = '100%';
    vmTrackerTraceRow.setAttribute('children', '');
    vmTrackerTraceRow.name = rowName;
    vmTrackerTraceRow.focusHandler = (): void => {
      this.showTip(vmTrackerTraceRow);
    };
    vmTrackerTraceRow.onThreadHandler = (useCache): void => {
      let context = vmTrackerTraceRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      vmTrackerTraceRow.canvasSave(context);
      (renders.snapshot as SnapshotRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: 'snapshot',
        },
        vmTrackerTraceRow
      );
      vmTrackerTraceRow.canvasRestore(context);
    };
    return vmTrackerTraceRow;
  }

  private showTip(traceRow: TraceRow<SnapshotStruct>): void {
    this.trace?.displayTip(
      traceRow,
      SnapshotStruct.hoverSnapshotStruct,
      `<span>Name: ${SnapshotStruct.hoverSnapshotStruct?.name || ''}</span>
      <span>Size: ${Utils.getBinaryByteWithUnit(SnapshotStruct.hoverSnapshotStruct?.value || 0)}</span>`
    );
  }
}
