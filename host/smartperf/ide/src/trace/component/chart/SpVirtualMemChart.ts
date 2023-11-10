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
import { TraceRow } from '../trace/base/TraceRow.js';
import { queryVirtualMemory, queryVirtualMemoryData } from '../../database/SqlLite.js';
import { VirtualMemoryRender, VirtualMemoryStruct } from '../../database/ui-worker/ProcedureWorkerVirtualMemory.js';
import { renders } from '../../database/ui-worker/ProcedureWorker.js';
import { EmptyRender } from '../../database/ui-worker/ProcedureWorkerCPU.js';

export class SpVirtualMemChart {
  private trace: SpSystemTrace;

  constructor(trace: SpSystemTrace) {
    this.trace = trace;
  }

  async init() {
    let array = await queryVirtualMemory();
    if (array.length == 0) {
      return;
    }
    let vmFolder = TraceRow.skeleton();
    vmFolder.rowId = `VirtualMemory`;
    vmFolder.index = 0;
    vmFolder.rowType = TraceRow.ROW_TYPE_VIRTUAL_MEMORY_GROUP;
    vmFolder.rowParentId = '';
    vmFolder.folder = true;
    vmFolder.name = `Virtual Memory`;
    vmFolder.style.height = '40px';
    vmFolder.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    vmFolder.selectChangeHandler = this.trace.selectChangeHandler;
    vmFolder.supplier = () => new Promise<Array<any>>((resolve) => resolve([]));
    vmFolder.onThreadHandler = (useCache) => {
      vmFolder.canvasSave(this.trace.canvasPanelCtx!);
      if (vmFolder.expansion) {
        this.trace.canvasPanelCtx?.clearRect(0, 0, vmFolder.frame.width, vmFolder.frame.height);
      } else {
        (renders['empty'] as EmptyRender).renderMainThread(
          {
            context: this.trace.canvasPanelCtx,
            useCache: useCache,
            type: ``,
          },
          vmFolder
        );
      }
      vmFolder.canvasRestore(this.trace.canvasPanelCtx!);
    };
    this.trace.rowsEL?.appendChild(vmFolder);
    array.forEach((it, idx) => this.initVirtualMemoryRow(vmFolder, it.id, it.name, idx));
  }

  initVirtualMemoryRow(folder: TraceRow<any>, id: number, name: string, idx: number) {
    let virtualMemoryRow = TraceRow.skeleton<VirtualMemoryStruct>();
    virtualMemoryRow.rowId = `${id}`;
    virtualMemoryRow.rowType = TraceRow.ROW_TYPE_VIRTUAL_MEMORY;
    virtualMemoryRow.rowParentId = folder.rowId;
    virtualMemoryRow.rowHidden = !folder.expansion;
    virtualMemoryRow.style.height = '40px';
    virtualMemoryRow.name = `${name.substring(16)}`;
    virtualMemoryRow.setAttribute('children', '');
    virtualMemoryRow.favoriteChangeHandler = this.trace.favoriteChangeHandler;
    virtualMemoryRow.selectChangeHandler = this.trace.selectChangeHandler;
    virtualMemoryRow.supplier = () =>
      queryVirtualMemoryData(id).then((resultVm) => {
        let maxValue = Math.max(...resultVm.map((it) => it.value || 0));
        for (let j = 0; j < resultVm.length; j++) {
          resultVm[j].maxValue = maxValue;
          if (j == resultVm.length - 1) {
            resultVm[j].duration = (TraceRow.range?.totalNS || 0) - (resultVm[j].startTime || 0);
          } else {
            resultVm[j].duration = (resultVm[j + 1].startTime || 0) - (resultVm[j].startTime || 0);
          }
          if (j > 0) {
            resultVm[j].delta = (resultVm[j].value || 0) - (resultVm[j - 1].value || 0);
          } else {
            resultVm[j].delta = 0;
          }
        }
        return resultVm;
      });
    virtualMemoryRow.focusHandler = () => {
      this.trace?.displayTip(
        virtualMemoryRow,
        VirtualMemoryStruct.hoverStruct,
        `<span>value:${VirtualMemoryStruct.hoverStruct?.value}</span>`
      );
    };
    virtualMemoryRow.findHoverStruct = () => {
      VirtualMemoryStruct.hoverStruct = virtualMemoryRow.getHoverStruct();
    };
    virtualMemoryRow.onThreadHandler = (useCache) => {
      let context: CanvasRenderingContext2D;
      if (virtualMemoryRow.currentContext) {
        context = virtualMemoryRow.currentContext;
      } else {
        context = virtualMemoryRow.collect ? this.trace.canvasFavoritePanelCtx! : this.trace.canvasPanelCtx!;
      }
      virtualMemoryRow.canvasSave(context);
      (renders['virtual-memory-cell'] as VirtualMemoryRender).renderMainThread(
        {
          context: context,
          useCache: useCache,
          type: `virtual-memory-cell-${id}`,
        },
        virtualMemoryRow
      );
      virtualMemoryRow.canvasRestore(context);
    };
    folder.addChildTraceRow(virtualMemoryRow);
  }
}
