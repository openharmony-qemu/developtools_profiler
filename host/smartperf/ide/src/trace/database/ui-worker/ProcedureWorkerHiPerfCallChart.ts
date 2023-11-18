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
  type Rect,
  ns2x,
  drawString,
  Render,
  isFrameContainPoint
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { HiPerfChartFrame } from '../../bean/PerfStruct.js';

export class HiPerfCallChartRender extends Render {
  renderMainThread(req: any, row: TraceRow<HiPerfCallChartStruct>): void {
    let list = row.dataList;
    let filter = row.dataListCache;
    hiperf(
      list,
      filter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh,
      row.funcExpand
    );
    req.context.beginPath();;
    let find = false;
    let offset = 5;
    for (let re of filter) {
      HiPerfCallChartStruct.draw(req.context, re);
      if (row.isHover) {
        if (re.endTime - re.startTime === 0 ||
          re.endTime - re.startTime == null ||
          re.endTime - re.startTime === undefined) {
          if (re.frame &&
            row.hoverX >= re.frame.x - offset &&
            row.hoverX <= re.frame.x + re.frame.width + offset &&
            row.hoverY >= re.frame.y &&
            row.hoverY <= re.frame.y + re.frame.height) {
            HiPerfCallChartStruct.hoverPerfCallCutStruct = re;
            find = true;
          }
        } else {
          if (re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
            HiPerfCallChartStruct.hoverPerfCallCutStruct = re;
            find = true
          }
        }
      }
      if (!find && row.isHover) {
        HiPerfCallChartStruct.hoverPerfCallCutStruct = undefined
      }
    }
    req.context.closePath();
  }
}

// 火焰图数据模板
let padding = 1;
export class HiPerfCallChartStruct extends BaseStruct {
  static selectStruct: HiPerfCallChartStruct | undefined;
  static hoverPerfCallCutStruct: HiPerfCallChartStruct | undefined;
  id: number = 0;
  name: string = '';
  startTime: number = 0;
  endTime: number = 0;
  eventCount: number = 0;
  depth: number = 0;
  fileId: number = 0;
  symbolId: number = 0;
  children!: Array<HiPerfChartFrame>;
  isSelect: boolean = false;
  totalTime: number = 0;
  callchain_id:number = 0;

  static setPerfFrame(
    hiPerfNode: any,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: Rect
  ): void {
    let x1: number, x2: number;
    if ((hiPerfNode.startTime || 0) > startNS && (hiPerfNode.startTime || 0) < endNS) {
      x1 = ns2x(hiPerfNode.startTime || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if (
      (hiPerfNode.startTime || 0) + (hiPerfNode.totalTime || 0) > startNS &&
      (hiPerfNode.startTime || 0) + (hiPerfNode.totalTime || 0) < endNS
    ) {
      x2 = ns2x(
        (hiPerfNode.startTime || 0) + (hiPerfNode.totalTime || 0),
        startNS,
        endNS,
        totalNS,
        frame
      );
    } else {
      x2 = frame.width;
    }
    if (!hiPerfNode.frame) {
      hiPerfNode.frame = {};
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    hiPerfNode.frame.x = Math.floor(x1);
    hiPerfNode.frame.y = hiPerfNode.depth * 20;
    hiPerfNode.frame.width = Math.ceil(getV);
    hiPerfNode.frame.height = 20;
  }

  static draw(ctx: CanvasRenderingContext2D, data: HiPerfCallChartStruct): void {
    if (data.frame) {
      if (data.endTime - data.startTime === undefined || data.endTime - data.startTime === null) {
      } else {
        ctx.globalAlpha = 1;
        if (data.name === '(program)') {
          ctx.fillStyle = '#ccc';
        } else if (data.name === '(idle)') {
          ctx.fillStyle = '#f0f0f0';
        } else {
          ctx.fillStyle =
            ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.name || '', 0, ColorUtils.FUNC_COLOR.length)];
        }
        let miniHeight = 20;
        if (HiPerfCallChartStruct.hoverPerfCallCutStruct && data === HiPerfCallChartStruct.hoverPerfCallCutStruct) {
          ctx.globalAlpha = 0.7;
        }
        ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
        if (data.frame.width > 8) {
          ctx.lineWidth = 1;
          ctx.fillStyle = '#fff';
          ctx.textBaseline = 'middle';
          drawString(ctx, `${data.name || ''}`, 4, data.frame, data);
        }
        if (data === HiPerfCallChartStruct.selectStruct) {
          ctx.strokeStyle = '#000';
          ctx.lineWidth = 2;
          ctx.strokeRect(
            data.frame.x + 1,
            data.frame.y + 1,
            data.frame.width - 2,
            miniHeight - padding * 2 - 2
          );
        }
      }
    }
  }
}

export function hiperf(
  list: Array<any>,
  filter: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: Rect,
  use: boolean,
  expand: boolean
): void {
  if (use && filter.length > 0) {
    for (let i = 0, len = filter.length; i < len; i++) {
      if (filter[i].totalTime > 0
        && (filter[i].startTime || 0) + (filter[i].totalTime || 0) >= startNS
        && (filter[i].startTime || 0) <= endNS) {
        HiPerfCallChartStruct.setPerfFrame(filter[i], startNS, endNS, totalNS, frame);
      } else {
        filter[i].frame = null;
      }
    }
    return;
  }
  filter.length = 0;
  if (list) {
    let groups = list
      .filter((it) => it.totalTime > 0
        && (it.startTime ?? 0) + (it.totalTime ?? 0) >= startNS
        && (it.startTime ?? 0) <= endNS
        && ((!expand && it.depth === 0) || expand))
      .map((it) => {
        HiPerfCallChartStruct.setPerfFrame(it, startNS, endNS, totalNS, frame);
        return it;
      })
      .reduce((pre, current) => {
        (pre[`${current.frame.x}-${current.depth}`] = pre[`${current.frame.x}-${current.depth}`] || []).push(current);
        return pre;
      }, {});
    Reflect.ownKeys(groups).map((kv) => {
      // 从小到大排序
      let arr = groups[kv].sort((a: HiPerfChartFrame, b: HiPerfChartFrame) => b.totalTime - a.totalTime);
      filter.push(arr[0]);
    });
  }
}