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
  isFrameContainPoint,
  ns2x,
  Render,
  RequestMessage,
  drawString,
} from './ProcedureWorkerCommon.js';

export class SoRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<SoStruct>
  ): void {
    let soList = row.dataList;
    let soFilter = row.dataListCache;
    soDataFilter(
      soList,
      soFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let soFind = false;
    for (let so of soFilter) {
      SoStruct.draw(req.context, so);
      if (row.isHover) {
        if (so.dur === 0 || so.dur === null || so.dur === undefined) {
          if (
            so.frame &&
            row.hoverX >= so.frame.x - 5 &&
            row.hoverX <= so.frame.x + 5 &&
            row.hoverY >= so.frame.y &&
            row.hoverY <= so.frame.y + so.frame.height
          ) {
            SoStruct.hoverSoStruct = so;
            soFind = true;
          }
        } else {
          if (so.frame && isFrameContainPoint(so.frame, row.hoverX, row.hoverY)) {
            SoStruct.hoverSoStruct = so;
            soFind = true;
          }
        }
      }
    }
    if (!soFind && row.isHover) SoStruct.hoverSoStruct = undefined;
    req.context.closePath();
  }

  render(req: RequestMessage, list: Array<any>, filter: Array<any>): void {}
}

export function soDataFilter(
  soList: Array<SoStruct>,
  soFilter: Array<SoStruct>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
): void {
  if (use && soFilter.length > 0) {
    for (let i = 0, len = soFilter.length ; i < len ; i++) {
      if ((soFilter[i].startTs || 0) + (soFilter[i].dur || 0) >= startNS && (soFilter[i].startTs || 0) <= endNS) {
        SoStruct.setSoFrame(soFilter[i], 0, startNS, endNS, totalNS, frame);
      } else {
        soFilter[i].frame = undefined;
      }
    }
    return;
  }
  soFilter.length = 0;
  if (soList) {
    let groups = soList
      .filter((it) => (it.startTs ?? 0) + (it.dur ?? 0) >= startNS && (it.startTs ?? 0) <= endNS)
      .map((it) => {
        SoStruct.setSoFrame(it, 0, startNS, endNS, totalNS, frame);
        return it;
      })
      .reduce((pre: any, current, index, arr) => {
        if (current.frame) {
          (pre[`${ current.frame.x }-${ current.depth }`] = pre[`${ current.frame.x }-${ current.depth }`] || []).push(current);
        }
        return pre;
      }, {});
    Reflect.ownKeys(groups).map((kv) => {
      let arr = groups[kv].sort((a: any, b: any) => b.dur - a.dur);
      soFilter.push(arr[0]);
    });
  }
}

export class SoStruct extends BaseStruct {
  static hoverSoStruct: SoStruct | undefined;
  static selectSoStruct: SoStruct | undefined;
  textMetricsWidth: number | undefined;
  depth: number | undefined;
  dur: number | undefined;
  soName: string | undefined;
  process: string | undefined;
  startTs: number | undefined;
  tid: number | undefined;
  pid: number | undefined;
  itid: number | undefined;

  static setSoFrame(soNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any): void {
    let x1: number;
    let x2: number;
    if ((soNode.startTs || 0) > startNS && (soNode.startTs || 0) < endNS) {
      x1 = ns2x(soNode.startTs || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if (
      (soNode.startTs || 0) + (soNode.dur || 0) > startNS &&
      (soNode.startTs || 0) + (soNode.dur || 0) < endNS
    ) {
      x2 = ns2x((soNode.startTs || 0) + (soNode.dur || 0), startNS, endNS, totalNS, frame);
    } else {
      x2 = frame.width;
    }
    if (!soNode.frame) {
      soNode.frame = {};
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    soNode.frame.x = Math.floor(x1);
    soNode.frame.y = soNode.depth * 20;
    soNode.frame.width = Math.ceil(getV);
    soNode.frame.height = 20;
  }

  static draw(ctx: CanvasRenderingContext2D, data: SoStruct): void {
    if (data.frame) {
      if (data.dur === undefined || data.dur === null) {
      } else {

        ctx.globalAlpha = 1;
        ctx.fillStyle = ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.soName || '', 0, ColorUtils.FUNC_COLOR.length)];
        let textColor = ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.soName || '', 0, ColorUtils.FUNC_COLOR.length)];
        let miniHeight = 20;
        if (SoStruct.hoverSoStruct && data.soName === SoStruct.hoverSoStruct.soName) {
          ctx.globalAlpha = 0.7;
        }
        ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
        if (data.frame.width > 10) {
          ctx.strokeStyle = '#fff';
          ctx.lineWidth = 1;
          ctx.strokeRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
          ctx.fillStyle = ColorUtils.funcTextColor(textColor);
          drawString(ctx, `${ data.soName || '' }`, 5, data.frame, data);
        }
        if (data === SoStruct.selectSoStruct) {
          ctx.strokeStyle = '#000';
          ctx.lineWidth = 2;
          ctx.strokeRect(data.frame.x, data.frame.y + 1, data.frame.width, miniHeight - padding * 2 - 2);
        }
      }
    }
  }

}

const padding = 1;
