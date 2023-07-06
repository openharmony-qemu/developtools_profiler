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
import {
  BaseStruct,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  drawWakeUp,
  ns2x,
  Render,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { CpuStruct } from './ProcedureWorkerCPU.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class ProcessRender extends Render {
  renderMainThread(req: any, row: TraceRow<ProcessStruct>) {
    let list = row.dataList;
    let filter = row.dataListCache;
    proc(
      list,
      filter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let path = new Path2D();
    let miniHeight: number = 0;
    miniHeight = Math.round((row.frame.height - CpuStruct.cpuCount * 2) / CpuStruct.cpuCount);
    req.context.fillStyle = ColorUtils.colorForTid(req.pid || 0);
    for (let re of filter) {
      ProcessStruct.draw(req.context, path, re, miniHeight);
    }
    req.context.fill(path);
    req.context.closePath();
  }

  render(processReq: RequestMessage, list: Array<any>, filter: Array<any>) {
    if (processReq.lazyRefresh) {
      proc(
        list,
        filter,
        processReq.startNS,
        processReq.endNS,
        processReq.totalNS,
        processReq.frame,
        processReq.useCache || !processReq.range.refresh
      );
    } else {
      if (!processReq.useCache) {
        proc(list, filter, processReq.startNS, processReq.endNS, processReq.totalNS, processReq.frame, false);
      }
    }
    if (processReq.canvas) {
      processReq.context.clearRect(0, 0, processReq.frame.width, processReq.frame.height);
      let arr = filter;
      if (arr.length > 0 && !processReq.range.refresh && !processReq.useCache && processReq.lazyRefresh) {
        drawLoading(
          processReq.context,
          processReq.startNS,
          processReq.endNS,
          processReq.totalNS,
          processReq.frame,
          arr[0].startTime,
          arr[arr.length - 1].startTime + arr[arr.length - 1].dur
        );
      }
      processReq.context.beginPath();
      CpuStruct.cpuCount = processReq.params.cpuCount;
      drawLines(processReq.context, processReq.xs, processReq.frame.height, processReq.lineColor);
      let path = new Path2D();
      let miniHeight: number = 0;
      miniHeight = Math.round((processReq.frame.height - CpuStruct.cpuCount * 2) / CpuStruct.cpuCount);
      processReq.context.fillStyle = ColorUtils.colorForTid(processReq.params.pid || 0);
      for (let re of filter) {
        ProcessStruct.draw(processReq.context, path, re, miniHeight);
      }
      processReq.context.fill(path);
      drawSelection(processReq.context, processReq.params);
      processReq.context.closePath();
      drawFlagLine(
        processReq.context,
        processReq.flagMoveInfo,
        processReq.flagSelectedInfo,
        processReq.startNS,
        processReq.endNS,
        processReq.totalNS,
        processReq.frame,
        processReq.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: processReq.id,
      type: processReq.type,
      results: processReq.canvas ? undefined : filter,
      hover: undefined,
    });
  }
}
export function proc(
  processList: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    res.forEach((it) => ProcessStruct.setProcessFrame(it, 5, startNS || 0, endNS || 0, totalNS || 0, frame));
    return;
  }
  res.length = 0;
  if (processList) {
    for (let i = 0, len = processList.length; i < len; i++) {
      let it = processList[i];
      if ((it.startTime || 0) + (it.dur || 0) > (startNS || 0) && (it.startTime || 0) < (endNS || 0)) {
        ProcessStruct.setProcessFrame(processList[i], 5, startNS || 0, endNS || 0, totalNS || 0, frame);
        if (
          i > 0 &&
          (processList[i - 1].frame?.x || 0) == (processList[i].frame?.x || 0) &&
          (processList[i - 1].frame?.width || 0) == (processList[i].frame?.width || 0)
        ) {
        } else {
          res.push(processList[i]);
        }
      }
    }
  }
}

const padding = 1;

export class ProcessStruct extends BaseStruct {
  cpu: number | undefined;
  dur: number | undefined;
  id: number | undefined;
  pid: number | undefined;
  process: string | undefined;
  startTime: number | undefined;
  state: string | undefined;
  thread: string | undefined;
  tid: number | undefined;
  ts: number | undefined;
  type: string | undefined;
  utid: number | undefined;

  static draw(ctx: CanvasRenderingContext2D, path: Path2D, data: ProcessStruct, miniHeight: number) {
    if (data.frame) {
      path.rect(data.frame.x, data.frame.y + (data.cpu || 0) * miniHeight + padding, data.frame.width, miniHeight);
    }
  }

  static setFrame(processNode: any, pns: number, startNS: number, endNS: number, frame: any) {
    if ((processNode.startTime || 0) < startNS) {
      processNode.frame.x = 0;
    } else {
      processNode.frame.x = Math.floor(((processNode.startTime || 0) - startNS) / pns);
    }
    if ((processNode.startTime || 0) + (processNode.dur || 0) > endNS) {
      processNode.frame.width = frame.width - processNode.frame.x;
    } else {
      processNode.frame.width = Math.ceil(
        ((processNode.startTime || 0) + (processNode.dur || 0) - startNS) / pns - processNode.frame.x
      );
    }
    if (processNode.frame.width < 1) {
      processNode.frame.width = 1;
    }
  }

  static setProcessFrame(
    processNode: any,
    padding: number,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any
  ) {
    let x1: number;
    let x2: number;
    if ((processNode.startTime || 0) < startNS) {
      x1 = 0;
    } else {
      x1 = ns2x(processNode.startTime || 0, startNS, endNS, totalNS, frame);
    }
    if ((processNode.startTime || 0) + (processNode.dur || 0) > endNS) {
      x2 = frame.width;
    } else {
      x2 = ns2x((processNode.startTime || 0) + (processNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let processGetV: number = x2 - x1 <= 1 ? 1 : x2 - x1;
    if (!processNode.frame) {
      processNode.frame = {};
    }
    processNode.frame.x = Math.floor(x1);
    processNode.frame.y = Math.floor(frame.y + 2);
    processNode.frame.width = Math.ceil(processGetV);
    processNode.frame.height = Math.floor(frame.height - padding * 2);
  }
}
