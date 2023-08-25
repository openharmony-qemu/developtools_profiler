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

import {
  BaseStruct,
  dataFilterHandler,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  drawWakeUp,
  ns2x,
  PerfRender,
  Render,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import { convertJSON } from '../logic-worker/ProcedureLogicWorkerCommon.js';

export class CpuStateRender extends PerfRender {
  renderMainThread(
    req: {
      useCache: boolean;
      cpuStateContext: CanvasRenderingContext2D;
      type: string;
      cpu: number;
    },
    cpuStateRow: TraceRow<CpuStateStruct>
  ): void {
    let list = cpuStateRow.dataList;
    let filter = cpuStateRow.dataListCache;
    dataFilterHandler(list, filter, {
      startKey: 'startTs',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: cpuStateRow.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    let chartColor = ColorUtils.colorForTid(req.cpu);
    req.cpuStateContext.beginPath();
    req.cpuStateContext.font = '11px sans-serif';
    req.cpuStateContext.fillStyle = chartColor;
    req.cpuStateContext.strokeStyle = chartColor;
    req.cpuStateContext.globalAlpha = 0.6;
    let path = new Path2D();
    let find = false;
    let offset = 3;
    let heights = [4, 12, 21, 30];
    for (let re of filter) {
      re.height = heights[(re as any).value];
      CpuStateStruct.draw(req.cpuStateContext, path, re);
      if (cpuStateRow.isHover) {
        if (re.frame && cpuStateRow.hoverX >= re.frame.x - offset &&
          cpuStateRow.hoverX <= re.frame.x + re.frame.width + offset
        ) {
          CpuStateStruct.hoverStateStruct = re;
          find = true;
        }
      }
    }
    if (!find && cpuStateRow.isHover) {
      CpuStateStruct.hoverStateStruct = undefined;
    }
    req.cpuStateContext.fill(path);
  }

  render(cpuStateReq: RequestMessage, list: Array<any>, filter: Array<any>, dataList2: Array<any>) {
    if (cpuStateReq.lazyRefresh || !cpuStateReq.useCache) {
      let cpuStateUse = false;
      if (cpuStateReq.lazyRefresh) {
        cpuStateUse = cpuStateReq.useCache || !cpuStateReq.range.refresh;
      }
      this.cpuState(
        list,
        dataList2,
        cpuStateReq.type!,
        filter,
        cpuStateReq.params.cpu,
        cpuStateReq.startNS,
        cpuStateReq.endNS,
        cpuStateReq.totalNS,
        cpuStateReq.frame,
        cpuStateUse
      );
    }
    CpuStateStruct.hoverStateStruct = undefined;
    if (cpuStateReq.canvas) {
      cpuStateReq.context.clearRect(0, 0, cpuStateReq.frame.width, cpuStateReq.frame.height);
      if (filter.length > 0 && !cpuStateReq.range.refresh && !cpuStateReq.useCache && cpuStateReq.lazyRefresh) {
        drawLoading(
          cpuStateReq.context,
          cpuStateReq.startNS,
          cpuStateReq.endNS,
          cpuStateReq.totalNS,
          cpuStateReq.frame,
          filter[0].startTs,
          filter[filter.length - 1].startTs + filter[filter.length - 1].dur
        );
      }
      cpuStateReq.context.beginPath();
      drawLines(cpuStateReq.context, cpuStateReq.xs, cpuStateReq.frame.height, cpuStateReq.lineColor);
      if (cpuStateReq.isHover) {
        let offset = 3;
        for (let re of filter) {
          if (
            re.frame &&
            cpuStateReq.hoverX >= re.frame.x - offset &&
            cpuStateReq.hoverX <= re.frame.x + re.frame.width + offset
          ) {
            CpuStateStruct.hoverStateStruct = re;
            break;
          }
        }
      }
      CpuStateStruct.selectStateStruct = cpuStateReq.params.selectStateStruct;
      cpuStateReq.context.font = '11px sans-serif';
      cpuStateReq.context.fillStyle = cpuStateReq.chartColor;
      cpuStateReq.context.strokeStyle = cpuStateReq.chartColor;
      cpuStateReq.context.globalAlpha = 0.6;
      let path = new Path2D();
      for (let re of filter) {
        CpuStateStruct.draw(cpuStateReq.context, path, re);
      }
      cpuStateReq.context.fill(path);
      drawSelection(cpuStateReq.context, cpuStateReq.params);
      drawFlagLine(
        cpuStateReq.context,
        cpuStateReq.flagMoveInfo,
        cpuStateReq.flagSelectedInfo,
        cpuStateReq.startNS,
        cpuStateReq.endNS,
        cpuStateReq.totalNS,
        cpuStateReq.frame,
        cpuStateReq.slicesTime
      );
    }
    let msg = {
      id: cpuStateReq.id,
      type: cpuStateReq.type,
      results: cpuStateReq.canvas ? undefined : filter,
      hover: CpuStateStruct.hoverStateStruct,
    };
    self.postMessage(msg);
  }

  cpuState(
    arr: Array<any>,
    arr2: Array<any>,
    type: string,
    cpuStateRes: Array<any>,
    cpu: number,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any,
    use: boolean
  ) {
    if (use && cpuStateRes.length > 0) {
      for (let i = 0, len = cpuStateRes.length; i < len; i++) {
        if (
          (cpuStateRes[i].startTs || 0) + (cpuStateRes[i].dur || 0) >= startNS &&
          (cpuStateRes[i].startTs || 0) <= endNS
        ) {
          CpuStateStruct.setFrame(cpuStateRes[i], 5, startNS, endNS, totalNS, frame);
        } else {
          cpuStateRes[i].frame = null;
        }
      }
      return;
    }
    cpuStateRes.length = 0;
    if (arr) {
      let list: Array<any> = arr2;
      cpuStateRes.length = 0;
      let pns = (endNS - startNS) / frame.width; //每个像素多少ns
      let y = frame.y + 5;
      let frameHeight = frame.height - 10;
      let left = 0,
        right = 0;
      for (let i = 0, j = list.length - 1, ib = true, jb = true; i < list.length, j >= 0; i++, j--) {
        if (list[j].startTs <= endNS && jb) {
          right = j;
          jb = false;
        }
        if (list[i].startTs + list[i].dur >= startNS && ib) {
          left = i;
          ib = false;
        }
        if (!ib && !jb) {
          break;
        }
      }
      let slice = list.slice(left, right + 1);
      let sum = 0;
      for (let i = 0; i < slice.length; i++) {
        if (!slice[i].frame) {
          slice[i].frame = {};
          slice[i].frame.y = y;
          slice[i].frame.height = frameHeight;
        }
        if (slice[i].dur >= pns) {
          slice[i].v = true;
          CpuStateStruct.setFrame(slice[i], 5, startNS, endNS, totalNS, frame);
        } else {
          if (i > 0) {
            let c = slice[i].startTs - slice[i - 1].startTs - slice[i - 1].dur;
            if (c < pns && sum < pns) {
              sum += c + slice[i - 1].dur;
              slice[i].v = false;
            } else {
              slice[i].v = true;
              CpuStateStruct.setFrame(slice[i], 5, startNS, endNS, totalNS, frame);
              sum = 0;
            }
          }
        }
      }
      cpuStateRes.push(...slice.filter((it) => it.v));
    }
  }
}

export class CpuStateStruct extends BaseStruct {
  static hoverStateStruct: CpuStateStruct | undefined;
  static selectStateStruct: CpuStateStruct | undefined;
  dur: number | undefined;
  value: string | undefined;
  startTs: number | undefined;
  height: number | undefined;
  cpu: number | undefined;

  static draw(ctx: CanvasRenderingContext2D, path: Path2D, data: CpuStateStruct) {
    if (data.frame) {
      if (data === CpuStateStruct.hoverStateStruct || data === CpuStateStruct.selectStateStruct) {
        path.rect(data.frame.x, 35 - (data.height || 0), data.frame.width, data.height || 0);
        ctx.lineWidth = 1;
        ctx.globalAlpha = 1.0;
        ctx.beginPath();
        ctx.arc(data.frame.x, 35 - (data.height || 0), 3, 0, 2 * Math.PI, true);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(data.frame.x + 3, 35 - (data.height || 0));
        ctx.lineWidth = 3;
        ctx.lineTo(data.frame.x + data.frame.width, 35 - (data.height || 0));
        ctx.stroke();
        ctx.lineWidth = 1;
        ctx.globalAlpha = 0.6;
        ctx.fillRect(data.frame.x, 35 - (data.height || 0), data.frame.width, data.height || 0);
      } else {
        ctx.globalAlpha = 0.6;
        path.rect(data.frame.x, 35 - (data.height || 0), data.frame.width, data.height || 0);
      }
    }
  }

  static setCpuFrame(cpuStateNode: any, pns: number, startNS: number, endNS: number, frame: any) {
    if ((cpuStateNode.startTime || 0) < startNS) {
      cpuStateNode.frame.x = 0;
    } else {
      cpuStateNode.frame.x = Math.floor(((cpuStateNode.startTs || 0) - startNS) / pns);
    }
    if ((cpuStateNode.startTime || 0) + (cpuStateNode.dur || 0) > endNS) {
      cpuStateNode.frame.width = frame.width - cpuStateNode.frame.x;
    } else {
      cpuStateNode.frame.width = Math.ceil(
        ((cpuStateNode.startTs || 0) + (cpuStateNode.dur || 0) - startNS) / pns - cpuStateNode.frame.x
      );
    }
    if (cpuStateNode.frame.width < 1) {
      cpuStateNode.frame.width = 1;
    }
  }
  static setFrame(cpuStateNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
    let x1: number, x2: number;
    if ((cpuStateNode.startTs || 0) < startNS) {
      x1 = 0;
    } else {
      x1 = ns2x(cpuStateNode.startTs || 0, startNS, endNS, totalNS, frame);
    }
    if ((cpuStateNode.startTs || 0) + (cpuStateNode.dur || 0) > endNS) {
      x2 = frame.width;
    } else {
      x2 = ns2x((cpuStateNode.startTs || 0) + (cpuStateNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let cpuStateGetV: number = x2 - x1 <= 1 ? 1 : x2 - x1;
    if (!cpuStateNode.frame) {
      cpuStateNode.frame = {};
    }
    cpuStateNode.frame.x = Math.ceil(x1);
    cpuStateNode.frame.y = frame.y + padding;
    cpuStateNode.frame.width = Math.floor(cpuStateGetV);
    cpuStateNode.frame.height = cpuStateNode.height;
  }
}
