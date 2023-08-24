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
  Rect,
  Render,
  RequestMessage,
  drawString,
} from './ProcedureWorkerCommon.js';
import { FuncStruct as BaseFuncStruct } from '../../bean/FuncStruct.js';
import { FlagsConfig } from '../../component/SpFlags.js';
export class FuncRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<FuncStruct>
  ) {
    let funcList = row.dataList;
    let funcFilter = row.dataListCache;
    func(
      funcList,
      funcFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let funcFind = false;
    for (let re of funcFilter) {
      FuncStruct.draw(req.context, re);
      if (row.isHover) {
        if (re.dur == 0 || re.dur == null || re.dur == undefined) {
          if (
            re.frame &&
            row.hoverX >= re.frame.x - 5 &&
            row.hoverX <= re.frame.x + 5 &&
            row.hoverY >= re.frame.y &&
            row.hoverY <= re.frame.y + re.frame.height
          ) {
            FuncStruct.hoverFuncStruct = re;
            funcFind = true;
          }
        } else {
          if (re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
            FuncStruct.hoverFuncStruct = re;
            funcFind = true;
          }
        }
      }
    }
    if (!funcFind && row.isHover) FuncStruct.hoverFuncStruct = undefined;
    req.context.closePath();
  }

  render(req: RequestMessage, list: Array<any>, filter: Array<any>) {}
}

export function func(
  funcList: Array<any>,
  funcFilter: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && funcFilter.length > 0) {
    for (let i = 0, len = funcFilter.length; i < len; i++) {
      if ((funcFilter[i].startTs || 0) + (funcFilter[i].dur || 0) >= startNS && (funcFilter[i].startTs || 0) <= endNS) {
        FuncStruct.setFuncFrame(funcFilter[i], 0, startNS, endNS, totalNS, frame);
      } else {
        funcFilter[i].frame = null;
      }
    }
    return;
  }
  funcFilter.length = 0;
  if (funcList) {
    let groups = funcList
      .filter((it) => (it.startTs ?? 0) + (it.dur ?? 0) >= startNS && (it.startTs ?? 0) <= endNS)
      .map((it) => {
        FuncStruct.setFuncFrame(it, 0, startNS, endNS, totalNS, frame);
        return it;
      })
      .reduce((pre, current, index, arr) => {
        (pre[`${current.frame.x}-${current.depth}`] = pre[`${current.frame.x}-${current.depth}`] || []).push(current);
        return pre;
      }, {});
    Reflect.ownKeys(groups).map((kv) => {
      let arr = groups[kv].sort((a: any, b: any) => b.dur - a.dur);
      funcFilter.push(arr[0]);
    });
  }
}

export class FuncStruct extends BaseFuncStruct {
  static hoverFuncStruct: FuncStruct | undefined;
  static selectFuncStruct: FuncStruct | undefined;
  flag: string | undefined; // 570000
  textMetricsWidth: number | undefined;

  static setFuncFrame(funcNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
    let x1: number, x2: number;
    if ((funcNode.startTs || 0) > startNS && (funcNode.startTs || 0) < endNS) {
      x1 = ns2x(funcNode.startTs || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if (
      (funcNode.startTs || 0) + (funcNode.dur || 0) > startNS &&
      (funcNode.startTs || 0) + (funcNode.dur || 0) < endNS
    ) {
      x2 = ns2x((funcNode.startTs || 0) + (funcNode.dur || 0), startNS, endNS, totalNS, frame);
    } else {
      x2 = frame.width;
    }
    if (!funcNode.frame) {
      funcNode.frame = {};
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    funcNode.frame.x = Math.floor(x1);
    funcNode.frame.y = funcNode.depth * 20;
    funcNode.frame.width = Math.ceil(getV);
    funcNode.frame.height = 20;
  }

  static draw(ctx: CanvasRenderingContext2D, data: FuncStruct) {
    if (data.frame) {
      let isBinder = FuncStruct.isBinder(data);
      if (data.dur == undefined || data.dur == null) {
      } else {
        ctx.globalAlpha = 1;
        ctx.fillStyle = ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.funName || '', 0, ColorUtils.FUNC_COLOR.length)];
        let textColor = ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.funName || '', 0, ColorUtils.FUNC_COLOR.length)];
        let miniHeight = 20;
        if (FuncStruct.hoverFuncStruct && data.funName == FuncStruct.hoverFuncStruct.funName) {
          ctx.globalAlpha = 0.7;
        }
        ctx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
        if (data.frame.width > 10) {
          ctx.strokeStyle = '#fff';
          ctx.lineWidth = 1;
          ctx.strokeRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
          ctx.fillStyle = ColorUtils.funcTextColor(textColor);
          drawString(ctx, `${data.funName || ''}`, 5, data.frame, data);
        }
        if (data === FuncStruct.selectFuncStruct) {
          ctx.strokeStyle = '#000';
          ctx.lineWidth = 2;
          ctx.strokeRect(data.frame.x, data.frame.y + 1, data.frame.width, miniHeight - padding * 2 - 2);
        }
        let flagConfig = FlagsConfig.getFlagsConfig('TaskPool');
        if (
          flagConfig!.TaskPool === 'Enabled' &&
          data.funName!.indexOf('H:Task PerformTask End:') >= 0 &&
          data.funName!.indexOf('Successful') < 0
        ) {
          if (data.frame!.width < 10) {
            FuncStruct.drawTaskPoolUnSuccessFlag(ctx, data.frame!.x, (data.depth! + 0.5) * 20, 3, data!);
          } else {
            FuncStruct.drawTaskPoolUnSuccessFlag(ctx, data.frame!.x, (data.depth! + 0.5) * 20, 6, data!);
          }
        }
        if (flagConfig!.TaskPool === 'Enabled' && data.funName!.indexOf('H:Thread Timeout Exit') >= 0) {
          FuncStruct.drawTaskPoolTimeOutFlag(ctx, data.frame!.x, (data.depth! + 0.5) * 20, 10, data!);
        }
      }
    }
  }

  static drawTaskPoolUnSuccessFlag(
    ctx: CanvasRenderingContext2D,
    x: number,
    y: number,
    radius: number,
    data: FuncStruct
  ) {
    ctx.strokeStyle = '#FFC880';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.arc(x + data.frame!.width, y, radius, 0, Math.PI * 2);
    ctx.closePath();
    ctx.fillStyle = '#E64566';
    ctx.fill();
    ctx.stroke();
  }

  static drawTaskPoolTimeOutFlag(
    canvas: CanvasRenderingContext2D,
    x: number,
    y: number,
    radius: number,
    data: FuncStruct
  ) {
    canvas.strokeStyle = '#FFC880';
    canvas.lineWidth = 1;
    canvas.beginPath();
    canvas.arc(x + data.frame!.width + 20, y, radius, 0, Math.PI * 2);
    canvas.closePath();
    canvas.fillStyle = '#FFC880';
    canvas.fill();
    canvas.stroke();
    canvas.font = '18px Arial';
    canvas.fillStyle = ColorUtils.GREY_COLOR;
    canvas.textAlign = 'center';
    canvas.fillText('¡', x + data.frame!.width + 20, y);
  }

  static isSelected(data: FuncStruct): boolean {
    return (
      FuncStruct.selectFuncStruct != undefined &&
      FuncStruct.selectFuncStruct.startTs == data.startTs &&
      FuncStruct.selectFuncStruct.depth == data.depth
    );
  }
}

const padding = 1;
