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

import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import {
  BaseStruct,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  drawWakeUp,
  isFrameContainPoint,
  ns2x,
  PerfRender,
  Render,
  RequestMessage,
  mem,
} from './ProcedureWorkerCommon.js';
import { CpuStruct } from './ProcedureWorkerCPU.js';
import { ProcessMemStruct as BaseProcessMemStruct } from '../../bean/ProcessMemStruct.js';
export class MemRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<ProcessMemStruct>
  ) {
    let memList = row.dataList;
    let memFilter = row.dataListCache;
    mem(
      memList,
      memFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let memFind = false;
    for (let re of memFilter) {
      ProcessMemStruct.draw(req.context, re);
      if (row.isHover) {
        if (re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
          ProcessMemStruct.hoverProcessMemStruct = re;
          memFind = true;
        }
      }
    }
    if (!memFind && row.isHover) ProcessMemStruct.hoverProcessMemStruct = undefined;
    req.context.closePath();
  }

  render(memRequest: RequestMessage, memList: Array<any>, filter: Array<any>) {
    if (memRequest.lazyRefresh) {
      mem(
        memList,
        filter,
        memRequest.startNS,
        memRequest.endNS,
        memRequest.totalNS,
        memRequest.frame,
        memRequest.useCache || !memRequest.range.refresh
      );
    } else {
      if (!memRequest.useCache) {
        mem(memList, filter, memRequest.startNS, memRequest.endNS, memRequest.totalNS, memRequest.frame, false);
      }
    }
    if (memRequest.canvas) {
      memRequest.context.clearRect(0, 0, memRequest.frame.width, memRequest.frame.height);
      let arr = filter;
      if (arr.length > 0 && !memRequest.range.refresh && !memRequest.useCache && memRequest.lazyRefresh) {
        drawLoading(
          memRequest.context,
          memRequest.startNS,
          memRequest.endNS,
          memRequest.totalNS,
          memRequest.frame,
          arr[0].startTime,
          arr[arr.length - 1].startTime + arr[arr.length - 1].dur
        );
      }
      memRequest.context.beginPath();
      drawLines(memRequest.context, memRequest.xs, memRequest.frame.height, memRequest.lineColor);
      ProcessMemStruct.hoverProcessMemStruct = undefined;
      if (memRequest.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            memRequest.hoverX >= re.frame.x &&
            memRequest.hoverX <= re.frame.x + re.frame.width &&
            memRequest.hoverY >= re.frame.y &&
            memRequest.hoverY <= re.frame.y + re.frame.height
          ) {
            ProcessMemStruct.hoverProcessMemStruct = re;
            break;
          }
        }
      } else {
        ProcessMemStruct.hoverProcessMemStruct = memRequest.params.hoverProcessMemStruct;
      }
      for (let re of filter) {
        ProcessMemStruct.draw(memRequest.context, re);
      }
      drawSelection(memRequest.context, memRequest.params);
      memRequest.context.closePath();
      drawFlagLine(
        memRequest.context,
        memRequest.flagMoveInfo,
        memRequest.flagSelectedInfo,
        memRequest.startNS,
        memRequest.endNS,
        memRequest.totalNS,
        memRequest.frame,
        memRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: memRequest.id,
      type: memRequest.type,
      results: memRequest.canvas ? undefined : filter,
      hover: ProcessMemStruct.hoverProcessMemStruct,
    });
  }
}

export class ProcessMemStruct extends BaseProcessMemStruct {
  static draw(memContext: CanvasRenderingContext2D, data: ProcessMemStruct) {
    if (data.frame) {
      let width = data.frame.width || 0;
      memContext.fillStyle = ColorUtils.colorForTid(data.maxValue || 0);
      memContext.strokeStyle = ColorUtils.colorForTid(data.maxValue || 0);
      if (data === ProcessMemStruct.hoverProcessMemStruct) {
        memContext.lineWidth = 1;
        memContext.globalAlpha = 0.6;
        let memDrawHeight: number = Math.floor(
          ((data.value || 0) * (data.frame.height || 0) * 1.0) / (data.maxValue || 1)
        );
        memDrawHeight = memDrawHeight > 0 ? memDrawHeight : 1;
        memContext.fillRect(data.frame.x, data.frame.y + data.frame.height - memDrawHeight, width, memDrawHeight);
        memContext.beginPath();
        memContext.arc(data.frame.x, data.frame.y + data.frame.height - memDrawHeight, 3, 0, 2 * Math.PI, true);
        memContext.fill();
        memContext.globalAlpha = 1.0;
        memContext.stroke();
        memContext.closePath();
        memContext.beginPath();
        memContext.moveTo(data.frame.x + 3, data.frame.y + data.frame.height - memDrawHeight);
        memContext.lineWidth = 3;
        memContext.lineTo(data.frame.x + width, data.frame.y + data.frame.height - memDrawHeight);
        memContext.stroke();
        memContext.closePath();
      } else {
        memContext.globalAlpha = 0.6;
        memContext.lineWidth = 1;
        let drawHeight: number = ((data.value || 0) * (data.frame.height || 0) * 1.0) / (data.maxValue || 1);
        drawHeight = drawHeight > 0 ? drawHeight : 1;
        memContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
        if (width > 2) {
          memContext.lineWidth = 1;
          memContext.globalAlpha = 1.0;
          memContext.strokeRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
        }
      }
    }
    memContext.globalAlpha = 1.0;
    memContext.lineWidth = 1;
  }
}
