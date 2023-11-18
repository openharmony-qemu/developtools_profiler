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

import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { BaseStruct, ns2x, Rect, Render } from './ProcedureWorkerCommon.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';

export class HiSysEventRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<HiSysEventStruct>
  ): void {
    let hiSysEventList = row.dataList;
    let hiSysEventFilter = row.dataListCache;
    hiSysEvent(
      hiSysEventList,
      hiSysEventFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let find = false;
    for (let re of hiSysEventFilter) {
      HiSysEventStruct.draw(req.context, re);
    }
    if (!find && row.isHover) {
      HiSysEventStruct.hoverHiSysEventStruct = undefined;
    }
    req.context.closePath();
  }
}

export function hiSysEvent(
  hiSysEventList: Array<HiSysEventStruct>,
  hiSysEventFilter: Array<HiSysEventStruct>,
  startNS: number,
  endNS: number,
  totalNS: number,
  row: TraceRow<HiSysEventStruct>,
  use: boolean
): void {
  if (use && hiSysEventFilter.length > 0) {
    for (let i = 0, len = hiSysEventFilter.length; i < len; i++) {
      let item = hiSysEventFilter[i];
      if ((item.ts || 0) + (item.dur || 0) >= startNS && (item.ts || 0) <= endNS) {
        HiSysEventStruct.setSysEventFrame(item, startNS, endNS, totalNS, row.frame);
      } else {
        item.frame = undefined;
      }
    }
    return;
  }
  hiSysEventFilter.length = 0;
  if (hiSysEventList) {
    for (let index = 0; index < hiSysEventList.length; index++) {
      let item = hiSysEventList[index];
      if ((item.ts || 0) + (item.dur || 0) >= startNS && (item.ts || 0) <= endNS) {
        HiSysEventStruct.setSysEventFrame(item, startNS, endNS, totalNS, row.frame);
        if (
          index > 0 &&
          (hiSysEventList[index - 1].frame?.x || 0) === (item.frame?.x || 0) &&
          (hiSysEventList[index - 1].frame?.width || 0) === (item.frame?.width || 0) &&
          (hiSysEventList[index - 1].depth === item.depth)
        ) {
        } else {
          hiSysEventFilter.push(item);
        }
      }
    }
  }
}

export class HiSysEventStruct extends BaseStruct{
  static hoverHiSysEventStruct: HiSysEventStruct | undefined;
  static selectHiSysEventStruct: HiSysEventStruct | undefined;
  id: number | undefined;
  domain: string | undefined;
  eventName: string | undefined;
  eventType: string | undefined;
  ts: number | undefined;
  tz: string | undefined;
  pid: number | undefined;
  tid: number | undefined;
  uid: number | undefined;
  info: string | undefined;
  level: string | undefined;
  seq: string | undefined;
  contents: string | undefined;
  dur: number | undefined;
  depth: number | undefined;
  static setSysEventFrame(
    sysEventNode: HiSysEventStruct,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: Rect
  ): void {
    let x1: number, x2: number;
    if ((sysEventNode.ts || 0) >= startNS && (sysEventNode.ts || 0) <= endNS) {
      x1 = ns2x(sysEventNode.ts || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if ((sysEventNode.ts || 0) + (sysEventNode.dur || 0) >= startNS && (sysEventNode.ts || 0) +
      (sysEventNode.dur || 0) <= endNS) {
      x2 = ns2x((sysEventNode.ts || 0) + (sysEventNode.dur || 0), startNS, endNS, totalNS, frame);
    } else {
      x2 = frame.width;
    }
    if (!sysEventNode.frame) {
      sysEventNode.frame = new Rect(0, 0, 0, 0);
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    sysEventNode.frame.x = Math.floor(x1);
    sysEventNode.frame.y = sysEventNode.depth! * rectHeight + padding * 2;
    sysEventNode.frame.width = Math.ceil(getV);
    sysEventNode.frame.height = 20;
  }

  static draw(ctx: CanvasRenderingContext2D, data: HiSysEventStruct): void {
    if (data.depth === undefined || data.depth === null) {
      return;
    }
    if (data.frame) {
      ctx.globalAlpha = 1;
      ctx.fillStyle = ColorUtils.getHisysEventColor(data.level!);
      ctx.fillRect(data.frame.x, data.frame.y + padding * data.depth, data.frame.width, rectHeight);
    }
  }
}
const padding = 5;
const rectHeight = 10;

