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

import { JanksStruct } from '../../bean/JanksStruct.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import {
  BaseStruct,
  isFrameContainPoint,
  ns2x,
  Render,
  RequestMessage,
  drawString,
} from './ProcedureWorkerCommon.js';

export class JankRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<JankStruct>
  ): void {
    let jankList = row.dataList;
    let jankFilter = row.dataListCache;
    jank(
      jankList,
      jankFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let find = false;
    let nsScale = ((TraceRow.range!.endNS || 0) - (TraceRow.range!.startNS || 0)) / (TraceRow.range!.totalNS * 9);
    for (let re of jankFilter) {
      JankStruct.draw(req.context, re, nsScale);
      if (row.isHover) {
        if (re.dur == 0 || re.dur == null || re.dur == undefined) {
          if (
            re.frame &&
            row.hoverX >= re.frame.x - 5 &&
            row.hoverX <= re.frame.x + 5 &&
            row.hoverY >= re.frame.y &&
            row.hoverY <= re.frame.y + re.frame.height
          ) {
            JankStruct.hoverJankStruct = re;
            find = true;
          }
        } else {
          if (re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
            JankStruct.hoverJankStruct = re;
            find = true;
          }
        }
      }
    }
    if (!find && row.isHover) JankStruct.hoverJankStruct = undefined;
    req.context.closePath();
  }

  render(req: RequestMessage, list: Array<any>, filter: Array<any>): void {}
}

export function jank(
  jankList: Array<any>,
  jankFilter: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
): void {
  if (use && jankFilter.length > 0) {
    for (let i = 0, len = jankFilter.length ; i < len ; i++) {
      if ((jankFilter[i].ts || 0) + (jankFilter[i].dur || 0) >= startNS && (jankFilter[i].ts || 0) <= endNS) {
        JankStruct.setJankFrame(jankFilter[i], 0, startNS, endNS, totalNS, frame);
      } else {
        jankFilter[i].frame = null;
      }
    }
    return;
  }
  jankFilter.length = 0;
  if (jankList) {
    let groups = jankList
      .filter((it) => (it.ts ?? 0) + (it.dur ?? 0) >= startNS && (it.ts ?? 0) <= endNS)
      .map((it) => {
        JankStruct.setJankFrame(it, 0, startNS, endNS, totalNS, frame);
        return it;
      })
      .reduce((pre, current, index, arr) => {
        (pre[`${ current.frame.x }-${ current.depth }`] = pre[`${ current.frame.x }-${ current.depth }`] || []).push(current);
        return pre;
      }, {});
    Reflect.ownKeys(groups).map((kv) => {
      let arr = groups[kv].sort((a: any, b: any) => b.dur - a.dur);
      jankFilter.push(arr[0]);
    });
  }
}

export class JankStruct extends JanksStruct {
  static hoverJankStruct: JankStruct | undefined;
  static selectJankStruct: JankStruct | undefined;
  static selectJankStructList: Array<JankStruct> = new Array<JankStruct>();

  static setJankFrame(jankNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any): void {
    let x1: number, x2: number;
    if ((jankNode.ts || 0) > startNS && (jankNode.ts || 0) < endNS) {
      x1 = ns2x(jankNode.ts || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if ((jankNode.ts || 0) + (jankNode.dur || 0) > startNS && (jankNode.ts || 0) + (jankNode.dur || 0) < endNS) {
      x2 = ns2x((jankNode.ts || 0) + (jankNode.dur || 0), startNS, endNS, totalNS, frame);
    } else {
      x2 = frame.width;
    }
    if (!jankNode.frame) {
      jankNode.frame = {};
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    jankNode.frame.x = Math.floor(x1);
    jankNode.frame.y = jankNode.depth * 20;
    jankNode.frame.width = Math.ceil(getV);
    jankNode.frame.height = 20;
  }

  static draw(ctx: CanvasRenderingContext2D, data: JankStruct, nsScale: number): void {
    if (data.frame) {
      if (data.dur == undefined || data.dur == null || data.dur == 0) {
      } else {
        ctx.globalAlpha = 1;
        ctx.fillStyle = ColorUtils.JANK_COLOR[0];
        if (data.jank_tag === 1) {
          ctx.fillStyle = ColorUtils.JANK_COLOR[2];
        } else if (data.jank_tag === 3) {
          ctx.fillStyle = ColorUtils.JANK_COLOR[3];
        }
        let miniHeight = 20;
        if (
          JankStruct.hoverJankStruct &&
          data.name == JankStruct.hoverJankStruct.name &&
          JankStruct.hoverJankStruct.type == data.type &&
          JankStruct.hoverJankStruct.pid == data.pid &&
          JankStruct.hoverJankStruct.frame_type == data.frame_type
        ) {
          ctx.globalAlpha = 0.7;
        }
        if (data.type == '0') {
          this.drawActualFrame(ctx, data, miniHeight);
        } else {
          this.drawExpectedFrame(data, nsScale, ctx, miniHeight);
        }
        if (data.frame.width > 10) {
          ctx.fillStyle = '#fff';
          drawString(ctx, `${ data.name || '' }`, 5, data.frame, data);
        }
        if (JankStruct.isSelected(data)) {
          ctx.strokeStyle = '#000';
          ctx.lineWidth = 2;
          ctx.strokeRect(data.frame.x, data.frame.y + 1, data.frame.width, miniHeight - padding * 2 - 2);
        }
      }
    }
  }

  private static drawExpectedFrame(
    data: JankStruct,
    nsScale: number,
    ctx: CanvasRenderingContext2D,
    miniHeight: number
  ): void {
    if (data.frame && data.frame.width * nsScale < 1.5) {
      ctx.fillStyle = '#FFFFFF';
      ctx.fillRect(data.frame.x, data.frame.y, data.frame.width * nsScale, miniHeight - padding * 2);
      ctx.fillStyle = ColorUtils.JANK_COLOR[0];
      ctx.fillRect(
        data.frame.x + data.frame.width * nsScale,
        data.frame.y,
        data.frame.width - nsScale * 2,
        miniHeight - padding * 2
      );
      ctx.fillStyle = '#FFFFFF';
      ctx.fillRect(
        data.frame.x + data.frame.width * nsScale + data.frame.width - nsScale * 2,
        data.frame.y,
        data.frame.width * nsScale,
        miniHeight - padding * 2
      );
    } else {
      ctx.fillStyle = ColorUtils.JANK_COLOR[0];
      if (data.frame) {
        ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
      }
    }
  }
  private static drawActualFrame(ctx: CanvasRenderingContext2D, data: JankStruct, miniHeight: number): void {
    ctx.fillStyle = ColorUtils.JANK_COLOR[0];
    if (data.jank_tag === 1) {
      ctx.fillStyle = ColorUtils.JANK_COLOR[2];
    } else if (data.jank_tag === 3) {
      ctx.fillStyle = ColorUtils.JANK_COLOR[3];
    }
    if (data.frame) {
      ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
    }
  }
  static isSelected(data: JankStruct): boolean {
    return (
      JankStruct.selectJankStruct != undefined &&
      JankStruct.selectJankStruct.ts == data.ts &&
      JankStruct.selectJankStruct.depth == data.depth &&
      JankStruct.selectJankStruct.type == data.type &&
      JankStruct.selectJankStruct.pid == data.pid &&
      JankStruct.selectJankStruct.frame_type == data.frame_type
    );
  }
}

const padding = 1;
