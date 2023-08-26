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

import { CpuStruct, WakeupBean } from './ProcedureWorkerCPU.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { TimerShaftElement } from '../../component/trace/TimerShaftElement';

export abstract class Render {
  abstract renderMainThread(req: any, row: TraceRow<any>): void;
}

export abstract class PerfRender {
  abstract render(req: RequestMessage, list: Array<any>, filter: Array<any>, dataList2: Array<any>): void;
}

export class RequestMessage {
  type: string | undefined | null;
  lazyRefresh: boolean | undefined;
  intervalPerf: any;
  canvas: any;
  context: any;
  params: any;
  online: any;
  buf: any;
  isRangeSelect: any;
  isHover: any;
  xs: any;
  frame: any;
  flagMoveInfo: any;
  flagSelectedInfo: any;
  hoverX: any;
  hoverY: any;
  startNS: any;
  endNS: any;
  totalNS: any;
  slicesTime:
    | {
        startTime: number | null;
        endTime: number | null;
        color: string | null;
      }
    | undefined;
  range: any;
  scale: any;
  chartColor: any;
  canvasWidth: any;
  canvasHeight: any;
  useCache: any;
  lineColor: any;
  wakeupBean: WakeupBean | undefined | null;
  id: any;
  postMessage:
    | {
        (message: any, targetOrigin: string, transfer?: Transferable[]): void;
        (message: any, options?: WindowPostMessageOptions): void;
      }
    | undefined;
}

export function ns2s(ns: number): string {
  let second1 = 1_000_000_000; // 1 second
  let millisecond = 1_000_000; // 1 millisecond
  let microsecond = 1_000; // 1 microsecond
  let res;
  if (ns >= second1) {
    res = (ns / 1000 / 1000 / 1000).toFixed(1) + ' s';
  } else if (ns >= millisecond) {
    res = (ns / 1000 / 1000).toFixed(1) + ' ms';
  } else if (ns >= microsecond) {
    res = (ns / 1000).toFixed(1) + ' μs';
  } else if (ns > 0) {
    res = ns.toFixed(1) + ' ns';
  } else {
    res = ns.toFixed(1) + ' s';
  }
  return res;
}

export function isFrameContainPoint(frame: Rect, x: number, y: number): boolean {
  return x >= frame.x && x <= frame.x + frame.width && y >= frame.y && y <= frame.y + frame.height;
}
export const isSurroundingPoint = function (pointX: number, currentRect: Rect, unitPointXRange: number): boolean {
  return (pointX >= currentRect?.x - unitPointXRange) && pointX <= currentRect?.x + unitPointXRange;
};

export const computeUnitWidth = function (
  preTs: number,
  currentTs: number,
  frameWidth: number,
  selectUnitWidth: number
): number {
  let max = 150;
  let unitWidth = ((currentTs - preTs) * frameWidth) / (TraceRow.range!.endNS - TraceRow.range!.startNS);
  if (unitWidth < selectUnitWidth) {
    return unitWidth > max || unitWidth === 0 ? max : unitWidth;
  }
  return selectUnitWidth > max || selectUnitWidth === 0 ? max : selectUnitWidth;
};

class FilterConfig {
  startNS: number = 0;
  endNS: number = 0;
  totalNS: number = 0;
  frame: any = null;
  useCache: boolean = false;
  startKey: string = 'startNS';
  durKey: string = 'dur';
  paddingTop: number = 0;
}

export function fillCacheData(filterList: Array<any>, condition: FilterConfig): boolean {
  if (condition.useCache && filterList.length > 0) {
    let pns = (condition.endNS - condition.startNS) / condition.frame.width;
    let y = condition.frame.y + condition.paddingTop;
    let height = condition.frame.height - condition.paddingTop * 2;
    for (let i = 0, len = filterList.length; i < len; i++) {
      let it = filterList[i];
      if (
        (it[condition.startKey] || 0) + (it[condition.durKey] || 0) > condition.startNS &&
        (it[condition.startKey] || 0) < condition.endNS
      ) {
        if (!filterList[i].frame) {
          filterList[i].frame = {};
          filterList[i].frame.y = y;
          filterList[i].frame.height = height;
        }
        setNodeFrame(
          filterList[i],
          pns,
          condition.startNS,
          condition.endNS,
          condition.frame,
          condition.startKey,
          condition.durKey
        );
      } else {
        filterList[i].frame = null;
      }
    }
    return true;
  }
  return false;
}

export function findRange(fullData: Array<any>, condition: FilterConfig): Array<any> {
  let left = 0,
    right = 0;
  for (let i = 0, j = fullData.length - 1, ib = true, jb = true; i < fullData.length, j >= 0; i++, j--) {
    if (fullData[j][condition.startKey] <= condition.endNS && jb) {
      right = j;
      jb = false;
    }
    if (fullData[i][condition.startKey] + fullData[i][condition.durKey] >= condition.startNS && ib) {
      left = i;
      ib = false;
    }
    if (!ib && !jb) {
      break;
    }
  }
  let slice = fullData.slice(left, right + 1);
  return slice;
}

export function dataFilterHandler(fullData: Array<any>, filterData: Array<any>, condition: FilterConfig): void {
  if (fillCacheData(filterData, condition)) {
    return;
  }
  if (fullData) {
    filterData.length = 0;
    let pns = (condition.endNS - condition.startNS) / condition.frame.width; //每个像素多少ns
    let y = condition.frame.y + condition.paddingTop;
    let height = condition.frame.height - condition.paddingTop * 2;
    let slice = findRange(fullData, condition);
    let sum = 0;
    for (let i = 0; i < slice.length; i++) {
      if (!slice[i].frame) {
        slice[i].frame = {};
        slice[i].frame.y = y;
        slice[i].frame.height = height;
      }
      if (i === slice.length - 1) {
        if (slice[i][condition.durKey] === undefined || slice[i][condition.durKey] === null) {
          slice[i][condition.durKey] = (condition.endNS || 0) - (slice[i][condition.startKey] || 0);
        }
      } else {
        if (slice[i][condition.durKey] === undefined || slice[i][condition.durKey] === null) {
          slice[i][condition.durKey] = (slice[i + 1][condition.startKey] || 0) - (slice[i][condition.startKey] || 0);
        }
      }
      if (slice[i][condition.durKey] >= pns || slice.length < 100) {
        slice[i].v = true;
        setNodeFrame(
          slice[i],
          pns,
          condition.startNS,
          condition.endNS,
          condition.frame,
          condition.startKey,
          condition.durKey
        );
      } else {
        if (i > 0) {
          let c = slice[i][condition.startKey] - slice[i - 1][condition.startKey] - slice[i - 1][condition.durKey];
          if (c < pns && sum < pns) {
            sum += c + slice[i - 1][condition.durKey];
            slice[i].v = false;
          } else {
            slice[i].v = true;
            setNodeFrame(
              slice[i],
              pns,
              condition.startNS,
              condition.endNS,
              condition.frame,
              condition.startKey,
              condition.durKey
            );
            sum = 0;
          }
        }
      }
    }
    filterData.push(...slice.filter((it) => it.v));
  }
}

function setNodeFrame(
  node: any,
  pns: number,
  startNS: number,
  endNS: number,
  frame: any,
  startKey: string,
  durKey: string
) {
  if ((node[startKey] || 0) < startNS) {
    node.frame.x = 0;
  } else {
    node.frame.x = Math.floor(((node[startKey] || 0) - startNS) / pns);
  }
  if ((node[startKey] || 0) + (node[durKey] || 0) > endNS) {
    node.frame.width = frame.width - node.frame.x;
  } else {
    node.frame.width = Math.ceil(((node[startKey] || 0) + (node[durKey] || 0) - startNS) / pns - node.frame.x);
  }
  if (node.frame.width < 1) {
    node.frame.width = 1;
  }
}

export function ns2x(ns: number, startNS: number, endNS: number, duration: number, rect: any) {
  // @ts-ignore
  if (endNS == 0) {
    endNS = duration;
  }
  let xSize: number = ((ns - startNS) * rect.width) / (endNS - startNS);
  if (xSize < 0) {
    xSize = 0;
  } else if (xSize > rect.width) {
    xSize = rect.width;
  }
  return xSize;
}

export function ns2xByTimeShaft(ns: number, tse: TimerShaftElement) {
  let startNS = tse.getRange()!.startNS;
  let endNS = tse.getRange()!.endNS;
  let duration = tse.getRange()!.totalNS;
  if (endNS == 0) {
    endNS = duration;
  }
  let width = tse.getBoundingClientRect().width - 258;
  let xSize: number = ((ns - startNS) * width) / (endNS - startNS);
  if (xSize < 0) {
    xSize = 0;
  } else if (xSize > width) {
    xSize = width;
  }
  return xSize;
}

export class Rect {
  x: number = 0;
  y: number = 0;
  width: number = 0;
  height: number = 0;
  constructor(x: number, y: number, width: number, height: number) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
  }

  static intersect(r1: Rect, rect: Rect): boolean {
    let minX = r1.x <= rect.x ? r1.x : rect.x;
    let minY = r1.y <= rect.y ? r1.y : rect.y;
    let maxX = r1.x + r1.width >= rect.x + rect.width ? r1.x + r1.width : rect.x + rect.width;
    let maxY = r1.y + r1.height >= rect.y + rect.height ? r1.y + r1.height : rect.y + rect.height;
    if (maxX - minX <= rect.width + r1.width && maxY - minY <= r1.height + rect.height) {
      return true;
    } else {
      return false;
    }
  }

  static contains(rect: Rect, x: number, y: number): boolean {
    return rect.x <= x && x <= rect.x + rect.width && rect.y <= y && y <= rect.y + rect.height;
  }

  static containsWithMargin(rect: Rect, x: number, y: number, t: number, r: number, b: number, l: number): boolean {
    return rect.x - l <= x && x <= rect.x + rect.width + r && rect.y - t <= y && y <= rect.y + rect.height + b;
  }

  static containsWithPadding(
    rect: Rect,
    x: number,
    y: number,
    paddingLeftRight: number,
    paddingTopBottom: number
  ): boolean {
    return (
      rect.x + paddingLeftRight <= x &&
      rect.y + paddingTopBottom <= y &&
      x <= rect.x + rect.width - paddingLeftRight &&
      y <= rect.y + rect.height - paddingTopBottom
    );
  }

  /**
   * 判断是否相交
   * @param rect
   */
  intersect(rect: Rect): boolean {
    let minX = this.x <= rect.x ? this.x : rect.x;
    let minY = this.y <= rect.y ? this.y : rect.y;
    let maxX = this.x + this.width >= rect.x + rect.width ? this.x + this.width : rect.x + rect.width;
    let maxY = this.y + this.height >= rect.y + rect.height ? this.y + this.height : rect.y + rect.height;
    if (maxX - minX <= rect.width + this.width && maxY - minY <= this.height + rect.height) {
      return true;
    } else {
      return false;
    }
  }

  contains(x: number, y: number): boolean {
    return this.x <= x && x <= this.x + this.width && this.y <= y && y <= this.y + this.height;
  }

  containsWithMargin(x: number, y: number, t: number, r: number, b: number, l: number): boolean {
    return this.x - l <= x && x <= this.x + this.width + r && this.y - t <= y && y <= this.y + this.height + b;
  }

  containsWithPadding(x: number, y: number, paddingLeftRight: number, paddingTopBottom: number): boolean {
    return (
      this.x + paddingLeftRight <= x &&
      x <= this.x + this.width - paddingLeftRight &&
      this.y + paddingTopBottom <= y &&
      y <= this.y + this.height - paddingTopBottom
    );
  }
}

export class Point {
  x: number = 0;
  y: number = 0;
  isRight: boolean = true;

  constructor(x: number, y: number, isRight: boolean = true) {
    this.x = x;
    this.y = y;
    this.isRight = isRight;
  }
}

export enum LineType {
  brokenLine,
  bezierCurve,
}

export class PairPoint {
  x: number = 0;
  ns: number = 0;
  y: number = 0;
  offsetY: number = 0;
  rowEL: TraceRow<any>;
  isRight: boolean = true;
  lineType?: LineType;
  business: string = '';
  hidden?: boolean = false;
  constructor(
    rowEL: TraceRow<any>,
    x: number,
    y: number,
    ns: number,
    offsetY: number,
    isRight: boolean,
    business: string
  ) {
    this.rowEL = rowEL;
    this.x = x;
    this.y = y;
    this.ns = ns;
    this.offsetY = offsetY;
    this.isRight = isRight;
    this.business = business;
  }
}

export class BaseStruct {
  translateY: number | undefined;
  frame: Rect | undefined;
  isHover: boolean = false;
}

export function drawLines(ctx: CanvasRenderingContext2D, xs: Array<any>, height: number, lineColor: string) {
  if (ctx) {
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = lineColor || '#dadada';
    xs?.forEach((it) => {
      ctx.moveTo(Math.floor(it), 0);
      ctx.lineTo(Math.floor(it), height);
    });
    ctx.stroke();
    ctx.closePath();
  }
}

export function drawFlagLine(
  commonCtx: any,
  hoverFlag: any,
  selectFlag: any,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  slicesTime:
    | {
        startTime: number | null | undefined;
        endTime: number | null | undefined;
        color: string | null | undefined;
      }
    | undefined
) {
  if (commonCtx) {
    if (hoverFlag) {
      commonCtx.beginPath();
      commonCtx.lineWidth = 2;
      commonCtx.strokeStyle = hoverFlag?.color || '#dadada';
      commonCtx.moveTo(Math.floor(hoverFlag.x), 0);
      commonCtx.lineTo(Math.floor(hoverFlag.x), frame.height);
      commonCtx.stroke();
      commonCtx.closePath();
    }
    if (selectFlag) {
      commonCtx.beginPath();
      commonCtx.lineWidth = 2;
      commonCtx.strokeStyle = selectFlag?.color || '#dadada';
      selectFlag.x = ns2x(selectFlag.time, startNS, endNS, totalNS, frame);
      commonCtx.moveTo(Math.floor(selectFlag.x), 0);
      commonCtx.lineTo(Math.floor(selectFlag.x), frame.height);
      commonCtx.stroke();
      commonCtx.closePath();
    }
    if (slicesTime && slicesTime.startTime && slicesTime.endTime) {
      commonCtx.beginPath();
      commonCtx.lineWidth = 1;
      commonCtx.strokeStyle = slicesTime.color || '#dadada';
      let x1 = ns2x(slicesTime.startTime, startNS, endNS, totalNS, frame);
      let x2 = ns2x(slicesTime.endTime, startNS, endNS, totalNS, frame);
      commonCtx.moveTo(Math.floor(x1), 0);
      commonCtx.lineTo(Math.floor(x1), frame.height);
      commonCtx.moveTo(Math.floor(x2), 0);
      commonCtx.lineTo(Math.floor(x2), frame.height);
      commonCtx.stroke();
      commonCtx.closePath();
    }
  }
}

export function drawFlagLineSegment(ctx: any, hoverFlag: any, selectFlag: any, frame: any, tse: TimerShaftElement) {
  if (ctx) {
    if (hoverFlag) {
      ctx.beginPath();
      ctx.lineWidth = 2;
      ctx.strokeStyle = hoverFlag?.color || '#dadada';
      ctx.moveTo(Math.floor(hoverFlag.x), 0);
      ctx.lineTo(Math.floor(hoverFlag.x), frame.height);
      ctx.stroke();
      ctx.closePath();
    }
    if (selectFlag) {
      ctx.beginPath();
      ctx.lineWidth = 2;
      ctx.strokeStyle = selectFlag?.color || '#dadada';
      selectFlag.x = ns2x(
        selectFlag.time,
        TraceRow.range!.startNS,
        TraceRow.range!.endNS,
        TraceRow.range!.totalNS,
        frame
      );
      ctx.moveTo(Math.floor(selectFlag.x), 0);
      ctx.lineTo(Math.floor(selectFlag.x), frame.height);
      ctx.stroke();
      ctx.closePath();
    }
    tse.sportRuler!.slicesTimeList.forEach((slicesTime) => {
      if (slicesTime && slicesTime.startTime && slicesTime.endTime) {
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = slicesTime.color || '#dadada';
        let x1 = ns2x(slicesTime.startTime, TraceRow.range!.startNS, TraceRow.range!.endNS, TraceRow.range!.totalNS, frame);
        let x2 = ns2x(slicesTime.endTime, TraceRow.range!.startNS, TraceRow.range!.endNS, TraceRow.range!.totalNS, frame);
        // 划线逻辑
        ctx.moveTo(Math.floor(x1), 0);
        ctx.lineTo(Math.floor(x1), frame.height); //左边的线
        ctx.moveTo(Math.floor(x2), 0);
        ctx.lineTo(Math.floor(x2), frame.height); // 右边的线
        ctx.stroke();
        ctx.closePath();
      }
    });
  }
}

export function drawSelection(ctx: any, params: any) {
  if (params.isRangeSelect && params.rangeSelectObject) {
    params.rangeSelectObject!.startX = Math.floor(
      ns2x(params.rangeSelectObject!.startNS!, params.startNS, params.endNS, params.totalNS, params.frame)
    );
    params.rangeSelectObject!.endX = Math.floor(
      ns2x(params.rangeSelectObject!.endNS!, params.startNS, params.endNS, params.totalNS, params.frame)
    );
    if (ctx) {
      ctx.globalAlpha = 0.5;
      ctx.fillStyle = '#666666';
      ctx.fillRect(
        params.rangeSelectObject!.startX!,
        params.frame.y,
        params.rangeSelectObject!.endX! - params.rangeSelectObject!.startX!,
        params.frame.height
      );
      ctx.globalAlpha = 1;
    }
  }
}

// draw range select
export function drawSelectionRange(context: any, params: TraceRow<any>) {
  if (params.rangeSelect && TraceRow.rangeSelectObject) {
    TraceRow.rangeSelectObject!.startX = Math.floor(
      ns2x(
        TraceRow.rangeSelectObject!.startNS!,
        TraceRow.range?.startNS ?? 0,
        TraceRow.range?.endNS ?? 0,
        TraceRow.range?.totalNS ?? 0,
        params.frame
      )
    );
    TraceRow.rangeSelectObject!.endX = Math.floor(
      ns2x(
        TraceRow.rangeSelectObject!.endNS!,
        TraceRow.range?.startNS ?? 0,
        TraceRow.range?.endNS ?? 0,
        TraceRow.range?.totalNS ?? 0,
        params.frame
      )
    );
    if (context) {
      context.globalAlpha = 0.5;
      context.fillStyle = '#666666';
      context.fillRect(
        TraceRow.rangeSelectObject!.startX!,
        params.frame.y,
        TraceRow.rangeSelectObject!.endX! - TraceRow.rangeSelectObject!.startX!,
        params.frame.height
      );
      context.globalAlpha = 1;
    }
  }
}

export function drawWakeUp(
  wakeUpContext: CanvasRenderingContext2D | any,
  wake: WakeupBean | undefined | null,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: Rect,
  selectCpuStruct: CpuStruct | undefined = undefined,
  wakeUpCurrentCpu: number | undefined = undefined,
  noVerticalLine = false
) {
  if (wake) {
    let x1 = Math.floor(ns2x(wake.wakeupTime || 0, startNS, endNS, totalNS, frame));
    wakeUpContext.beginPath();
    wakeUpContext.lineWidth = 2;
    wakeUpContext.fillStyle = '#000000';
    if (x1 > 0 && x1 < frame.x + frame.width) {
      if (!noVerticalLine) {
        wakeUpContext.moveTo(x1, frame.y);
        wakeUpContext.lineTo(x1, frame.y + frame.height);
      }
      if (wakeUpCurrentCpu == wake.cpu) {
        let centerY = Math.floor(frame.y + frame.height / 2);
        wakeUpContext.moveTo(x1, centerY - 6);
        wakeUpContext.lineTo(x1 + 4, centerY);
        wakeUpContext.lineTo(x1, centerY + 6);
        wakeUpContext.lineTo(x1 - 4, centerY);
        wakeUpContext.lineTo(x1, centerY - 6);
        wakeUpContext.fill();
      }
    }
    if (selectCpuStruct) {
      let x2 = Math.floor(ns2x(selectCpuStruct.startTime || 0, startNS, endNS, totalNS, frame));
      let y = frame.y + frame.height - 10;
      wakeUpContext.moveTo(x1, y);
      wakeUpContext.lineTo(x2, y);

      let s = ns2s((selectCpuStruct.startTime || 0) - (wake.wakeupTime || 0));
      let distance = x2 - x1;
      if (distance > 12) {
        wakeUpContext.moveTo(x1, y);
        wakeUpContext.lineTo(x1 + 6, y - 3);
        wakeUpContext.moveTo(x1, y);
        wakeUpContext.lineTo(x1 + 6, y + 3);
        wakeUpContext.moveTo(x2, y);
        wakeUpContext.lineTo(x2 - 6, y - 3);
        wakeUpContext.moveTo(x2, y);
        wakeUpContext.lineTo(x2 - 6, y + 3);
        let measure = wakeUpContext.measureText(s);
        let tHeight = measure.actualBoundingBoxAscent + measure.actualBoundingBoxDescent;
        let xStart = x1 + Math.floor(distance / 2 - measure.width / 2);
        if (distance > measure.width + 4) {
          wakeUpContext.fillStyle = '#ffffff';
          wakeUpContext.fillRect(xStart - 2, y - 4 - tHeight, measure.width + 4, tHeight + 4);
          wakeUpContext.font = '10px solid';
          wakeUpContext.fillStyle = '#000000';
          wakeUpContext.textBaseline = 'bottom';
          wakeUpContext.fillText(s, xStart, y - 2);
        }
      }
    }
    wakeUpContext.strokeStyle = '#000000';
    wakeUpContext.stroke();
    wakeUpContext.closePath();
  }
}

const wid = 5;
const linkLineColor = '#ff0000';
export function drawLinkLines(
  context: CanvasRenderingContext2D,
  nodes: PairPoint[][],
  tm: TimerShaftElement,
  isFavorite: boolean
) {
  let percentage =
    (tm.getRange()!.totalNS - Math.abs(tm.getRange()!.endNS - tm.getRange()!.startNS)) / tm.getRange()!.totalNS;
  let maxWidth = tm.getBoundingClientRect().width - 268;
  for (let i = 0; i < nodes.length; i++) {
    let it = nodes[i];
    if (it[0].hidden) {
      continue;
    }
    if (isFavorite) {
      if (!it[0].rowEL.collect && !it[1].rowEL.collect) {
        continue;
      }
    }
    switch (it[0].lineType) {
      case LineType.brokenLine:
        drawBrokenLine(it, maxWidth, context);
        break;
      case LineType.bezierCurve:
        drawBezierCurve(it, maxWidth, context, percentage);
        break;
      default:
        drawBezierCurve(it, maxWidth, context, percentage);
    }
  }
}

function drawBezierCurve(it: PairPoint[], maxWidth: number, context: CanvasRenderingContext2D, percentage: number) {
  let bezierCurveStart = it[0].x > it[1].x ? it[1] : it[0];
  let bezierCurveEnd = it[0].x > it[1].x ? it[0] : it[1];
  if (bezierCurveStart && bezierCurveEnd) {
    //左移到边界，不画线
    if (bezierCurveStart.x <= 0) {
      bezierCurveStart.x = -100;
    }
    if (bezierCurveEnd.x <= 0) {
      bezierCurveEnd.x = -100;
    }
    //右移到边界，不画线
    if (bezierCurveStart.x >= maxWidth) {
      bezierCurveStart.x = maxWidth + 100;
    }
    if (bezierCurveEnd.x >= maxWidth) {
      bezierCurveEnd.x = maxWidth + 100;
    }
    context.beginPath();
    context.lineWidth = 2;
    context.fillStyle = linkLineColor;
    context.strokeStyle = linkLineColor;
    let x0;
    let y0;
    let x1;
    let x2;
    let y1;
    let y2;
    let x3;
    let y3;
    x0 = bezierCurveStart.x ?? 0;
    y0 = bezierCurveStart.y ?? 0;
    x3 = bezierCurveEnd.x ?? 0;
    y3 = bezierCurveEnd.y ?? 0;
    if (bezierCurveEnd.isRight) {
      x2 = x3 - 100 * percentage;
    } else {
      x2 = x3 + 100 * percentage;
    }
    y2 = y3 - 40 * percentage;
    if (bezierCurveStart.isRight) {
      x1 = x0 - 100 * percentage;
    } else {
      x1 = x0 + 100 * percentage;
    }
    y1 = y0 + 40 * percentage;
    //向右箭头终点在x轴正向有偏移
    if (!bezierCurveStart.isRight) {
      x0 -= 5;
    }
    context.moveTo(x0, y0);
    //箭头向左还是向右
    if (bezierCurveStart.isRight) {
      context.lineTo(x0 - wid, y0 + wid);
      context.moveTo(x0, y0);
      context.lineTo(x0 - wid, y0 - wid);
    } else {
      context.lineTo(x0 + wid, y0 + wid);
      context.moveTo(x0, y0);
      context.lineTo(x0 + wid, y0 - wid);
    }
    context.moveTo(x0, y0);
    context.bezierCurveTo(x1, y1, x2, y2, x3, y3);
    context.moveTo(x3, y3);
    //箭头向左还是向右
    if (bezierCurveEnd.isRight) {
      context.lineTo(x3 - wid, y3 + wid);
      context.moveTo(x3, y3);
      context.lineTo(x3 - wid, y3 - wid);
    } else {
      context.lineTo(x3 + wid, y3 + wid);
      context.moveTo(x3, y3);
      context.lineTo(x3 + wid, y3 - wid);
    }
    context.moveTo(x3, y3);
    context.stroke();
    context.closePath();
  }
}

function drawBrokenLine(it: PairPoint[], maxWidth: number, context: CanvasRenderingContext2D) {
  let brokenLineStart = it[0].x > it[1].x ? it[1] : it[0];
  let brokenLineEnd = it[0].x > it[1].x ? it[0] : it[1];
  if (brokenLineStart && brokenLineEnd) {
    if (brokenLineStart.x <= 0) {
      brokenLineStart.x = -100;
    }
    if (brokenLineEnd.x <= 0) {
      brokenLineEnd.x = -100;
    }
    //右移到边界，不画线
    if (brokenLineStart.x >= maxWidth) {
      brokenLineStart.x = maxWidth + 100;
    }
    if (brokenLineEnd.x >= maxWidth) {
      brokenLineEnd.x = maxWidth + 100;
    }
    context.beginPath();
    context.lineWidth = 2;
    context.fillStyle = '#46B1E3';
    context.strokeStyle = '#46B1E3';
    let x0;
    let y0;
    let x1;
    let y1;
    let x2;
    let y2;
    x0 = brokenLineStart.x ?? 0;
    y0 = brokenLineStart.y ?? 0;
    y2 = brokenLineEnd.y ?? 0;
    x2 = brokenLineEnd.x ?? 0;
    let leftEndpointX;
    let leftEndpointY;
    let rightEndpointX;
    let rightEndpointY;

    if (brokenLineStart.y < brokenLineEnd.y) {
      x1 = brokenLineStart.x ?? 0;
      y1 = brokenLineEnd.y ?? 0;
      leftEndpointX = x2 - wid;
      leftEndpointY = y2 - wid;
      rightEndpointX = x2 - wid;
      rightEndpointY = y2 + wid;
    } else {
      x2 = brokenLineEnd.x - wid ?? 0;
      x1 = brokenLineEnd.x - wid ?? 0;
      y1 = brokenLineStart.y ?? 0;
      leftEndpointX = x2 - wid;
      leftEndpointY = y2 + wid;
      rightEndpointX = x2 + wid;
      rightEndpointY = y2 + wid;
    }
    context.moveTo(x0, y0);
    context.lineTo(x1, y1);
    context.lineTo(x2, y2);
    context.stroke();
    context.closePath();
    context.beginPath();
    context.lineWidth = 2;
    context.fillStyle = '#46B1E3';
    context.strokeStyle = '#46B1E3';
    context.moveTo(x2, y2);
    context.lineTo(leftEndpointX, leftEndpointY);
    context.lineTo(rightEndpointX, rightEndpointY);
    context.lineTo(x2, y2);
    context.fill();
    context.closePath();
  }
}

export function drawLoading(
  ctx: CanvasRenderingContext2D,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  left: number,
  right: number
) {}

export function drawString(ctx: CanvasRenderingContext2D, str: string, textPadding: number, frame: Rect, data: any) {
  if (data.textMetricsWidth === undefined) {
    data.textMetricsWidth = ctx.measureText(str).width;
  }
  let charWidth = Math.round(data.textMetricsWidth / str.length);
  let fillTextWidth = frame.width - textPadding * 2;
  if (data.textMetricsWidth < fillTextWidth) {
    let x2 = Math.floor(frame.width / 2 - data.textMetricsWidth / 2 + frame.x + textPadding);
    ctx.fillText(str, x2, Math.floor(frame.y + frame.height / 2), fillTextWidth);
  } else {
    if (fillTextWidth >= charWidth) {
      let chatNum = fillTextWidth / charWidth;
      let x1 = frame.x + textPadding;
      if (chatNum < 2) {
        ctx.fillText(str.substring(0, 1), x1, Math.floor(frame.y + frame.height / 2), fillTextWidth);
      } else {
        ctx.fillText(str.substring(0, chatNum - 1) + '...', x1, Math.floor(frame.y + frame.height / 2), fillTextWidth);
      }
    }
  }
}

export function drawString2Line(
  ctx: CanvasRenderingContext2D,
  str1: string,
  str2: string,
  textPadding: number,
  frame: Rect,
  data: any
) {
  if (frame.height < 30) return;
  if (data.textMetrics1Width === undefined) {
    data.textMetrics1Width = ctx.measureText(str1).width;
    data.textMetrics2Width = ctx.measureText(str2).width;
  }
  let charWidth = Math.round(data.textMetrics1Width / str1.length);
  let fillTextWidth = frame.width - textPadding * 2;
  let y1 = frame.y + 12;
  let y2 = y1 + 14;
  if (data.textMetrics1Width < fillTextWidth) {
    let x = Math.floor(frame.width / 2 - data.textMetrics1Width / 2 + frame.x + textPadding - 1);
    ctx.fillText(str1, x, y1, fillTextWidth);
  } else {
    if (fillTextWidth >= charWidth) {
      let chatNum = fillTextWidth / charWidth;
      let x = frame.x + textPadding - 1;
      if (chatNum < 2) {
        ctx.fillText(str1.substring(0, 1), x, y1, fillTextWidth);
      } else {
        ctx.fillText(str1.substring(0, chatNum - 1) + '...', x, y1, fillTextWidth);
      }
    }
  }
  if (data.textMetrics2Width < fillTextWidth) {
    let x = Math.floor(frame.width / 2 - data.textMetrics2Width / 2 + frame.x + textPadding - 1);
    ctx.fillText(str2, x, y2, fillTextWidth);
  } else {
    if (fillTextWidth >= charWidth) {
      let chatNum = fillTextWidth / charWidth;
      let x = frame.x + textPadding - 1;
      if (chatNum < 2) {
        ctx.fillText(str2.substring(0, 1), x, y2, fillTextWidth);
      } else {
        ctx.fillText(str2.substring(0, chatNum - 1) + '...', x, y2, fillTextWidth);
      }
    }
  }
}

export function hiPerf(
  arr: Array<any>,
  arr2: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  frame: any,
  groupBy10MS: boolean,
  use: boolean
) {
  if (use && res.length > 0) {
    let pns = (endNS - startNS) / frame.width;
    let y = frame.y;
    for (let i = 0; i < res.length; i++) {
      let it = res[i];
      if ((it.startNS || 0) + (it.dur || 0) > startNS && (it.startNS || 0) < endNS) {
        if (!it.frame) {
          it.frame = {};
          it.frame.y = y;
        }
        it.frame.height = it.height;
        HiPerfStruct.setFrame(it, pns, startNS, endNS, frame);
      } else {
        it.frame = null;
      }
    }
    return;
  }
  res.length = 0;
  if (arr) {
    let list = groupBy10MS ? arr2 : arr;
    let pns = (endNS - startNS) / frame.width;
    let y = frame.y;
    for (let i = 0, len = list.length; i < len; i++) {
      let it = list[i];
      if ((it.startNS || 0) + (it.dur || 0) > startNS && (it.startNS || 0) < endNS) {
        if (!list[i].frame) {
          list[i].frame = {};
          list[i].frame.y = y;
        }
        list[i].frame.height = it.height;
        HiPerfStruct.setFrame(list[i], pns, startNS, endNS, frame);
        if (groupBy10MS) {
          if (
            i > 0 &&
            (list[i - 1].frame?.x || 0) == (list[i].frame?.x || 0) &&
            (list[i - 1].frame?.width || 0) == (list[i].frame?.width || 0) &&
            (list[i - 1].frame?.height || 0) == (list[i].frame?.height || 0)
          ) {
          } else {
            res.push(list[i]);
          }
        } else {
          if (i > 0 && Math.abs((list[i - 1].frame?.x || 0) - (list[i].frame?.x || 0)) < 4) {
          } else {
            res.push(list[i]);
          }
        }
      }
    }
  }
}

export class HiPerfStruct extends BaseStruct {
  static hoverStruct: HiPerfStruct | undefined;
  static selectStruct: HiPerfStruct | undefined;
  id: number | undefined;
  callchain_id: number | undefined;
  timestamp: number | undefined;
  thread_id: number | undefined;
  event_count: number | undefined;
  event_type_id: number | undefined;
  cpu_id: number | undefined;
  thread_state: string | undefined;
  startNS: number | undefined;
  endNS: number | undefined;
  dur: number | undefined;
  height: number | undefined;

  static drawRoundRectPath(cxt: Path2D, x: number, y: number, width: number, height: number, radius: number) {
    cxt.arc(x + width - radius, y + height - radius, radius, 0, Math.PI / 2);
    cxt.lineTo(x + radius, y + height);
    cxt.arc(x + radius, y + height - radius, radius, Math.PI / 2, Math.PI);
    cxt.lineTo(x + 0, y + radius);
    cxt.arc(x + radius, y + radius, radius, Math.PI, (Math.PI * 3) / 2);
    cxt.lineTo(x + width - radius, y + 0);
    cxt.arc(x + width - radius, y + radius, radius, (Math.PI * 3) / 2, Math.PI * 2);
    cxt.lineTo(x + width, y + height - radius);
    cxt.moveTo(x + width / 3, y + height / 5);
    cxt.lineTo(x + width / 3, y + (height / 5) * 4);
    cxt.moveTo(x + width / 3, y + height / 5);
    cxt.bezierCurveTo(
      x + width / 3 + 7,
      y + height / 5 - 2,
      x + width / 3 + 7,
      y + height / 5 + 6,
      x + width / 3,
      y + height / 5 + 4
    );
  }

  static draw(ctx: CanvasRenderingContext2D, normalPath: Path2D, specPath: Path2D, data: any, groupBy10MS: boolean) {
    if (data.frame) {
      if (groupBy10MS) {
        let width = data.frame.width;
        normalPath.rect(data.frame.x, 40 - (data.height || 0), width, data.height || 0);
      } else {
        let path = data.callchain_id === -1 ? specPath : normalPath;
        path.moveTo(data.frame.x + 7, 20);
        HiPerfStruct.drawRoundRectPath(path, data.frame.x - 7, 20 - 7, 14, 14, 3);
        path.moveTo(data.frame.x, 27);
        path.lineTo(data.frame.x, 33);
      }
    }
  }

  static drawSpecialPath(ctx: CanvasRenderingContext2D, specPath: Path2D) {
    ctx.strokeStyle = '#9fafc4';
    ctx.globalAlpha = 0.5;
    ctx.stroke(specPath);
    ctx.globalAlpha = 1;
  }

  static setFrame(node: any, pns: number, startNS: number, endNS: number, frame: any) {
    if ((node.startNS || 0) < startNS) {
      node.frame.x = 0;
    } else {
      node.frame.x = Math.floor(((node.startNS || 0) - startNS) / pns);
    }
    if ((node.startNS || 0) + (node.dur || 0) > endNS) {
      node.frame.width = frame.width - node.frame.x;
    } else {
      node.frame.width = Math.ceil(((node.startNS || 0) + (node.dur || 0) - startNS) / pns - node.frame.x);
    }
    if (node.frame.width < 1) {
      node.frame.width = 1;
    }
  }

  static groupBy10MS(groupArray: Array<any>, intervalPerf: number, maxCpu?: number | undefined): Array<any> {
    let obj = groupArray
      .map((it) => {
        it.timestamp_group = Math.trunc(it.startNS / 1_000_000_0) * 1_000_000_0;
        return it;
      })
      .reduce((pre, current) => {
        (pre[current['timestamp_group']] = pre[current['timestamp_group']] || []).push(current);
        return pre;
      }, {});
    let arr = [];
    for (let aKey in obj) {
      let ns = parseInt(aKey);
      let height: number = 0;
      if (maxCpu != undefined) {
        height = Math.floor((obj[aKey].length / (10 / intervalPerf) / maxCpu) * 40);
      } else {
        height = Math.floor((obj[aKey].length / (10 / intervalPerf)) * 40);
      }
      arr.push({
        startNS: ns,
        dur: 1_000_000_0,
        height: height,
      });
    }
    return arr;
  }
}

function setMemFrame(node: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
  let x1: number;
  let x2: number;
  if ((node.startTime || 0) <= startNS) {
    x1 = 0;
  } else {
    x1 = ns2x(node.startTime || 0, startNS, endNS, totalNS, frame);
  }
  if ((node.startTime || 0) + (node.duration || 0) >= endNS) {
    x2 = frame.width;
  } else {
    x2 = ns2x((node.startTime || 0) + (node.duration || 0), startNS, endNS, totalNS, frame);
  }
  let getV: number = x2 - x1 <= 1 ? 1 : x2 - x1;
  if (!node.frame) {
    node.frame = {};
  }
  node.frame.x = Math.floor(x1);
  node.frame.y = Math.floor(frame.y + padding);
  node.frame.width = Math.ceil(getV);
  node.frame.height = Math.floor(frame.height - padding * 2);
}

export function mem(
  list: Array<any>,
  memFilter: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && memFilter.length > 0) {
    for (let i = 0, len = memFilter.length; i < len; i++) {
      if (
        (memFilter[i].startTime || 0) + (memFilter[i].duration || 0) > startNS &&
        (memFilter[i].startTime || 0) < endNS
      ) {
        setMemFrame(memFilter[i], 5, startNS, endNS, totalNS, frame);
      } else {
        memFilter[i].frame = null;
      }
    }
    return;
  }
  memFilter.length = 0;
  if (list) {
    for (let i = 0, len = list.length; i < len; i++) {
      let it = list[i];
      if ((it.startTime || 0) + (it.duration || 0) > startNS && (it.startTime || 0) < endNS) {
        setMemFrame(list[i], 5, startNS, endNS, totalNS, frame);
        if (
          i > 0 &&
          (list[i - 1].frame?.x || 0) == (list[i].frame?.x || 0) &&
          (list[i - 1].frame?.width || 0) == (list[i].frame?.width || 0)
        ) {
        } else {
          memFilter.push(list[i]);
        }
      }
    }
  }
}

export function drawWakeUpList(
  wakeUpListContext: CanvasRenderingContext2D | any,
  wake: WakeupBean | undefined | null,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: Rect,
  wakeup: WakeupBean | undefined = undefined,
  currentCpu: number | undefined | null = undefined,
  noVerticalLine = false,
) {
  if (wake) {
    let x1 = Math.floor(
      ns2x(wake.wakeupTime || 0, startNS, endNS, totalNS, frame)
    );
    wakeUpListContext.beginPath();
    wakeUpListContext.lineWidth = 2;
    wakeUpListContext.fillStyle = '#000000';
    if (x1 > 0 && x1 < frame.x + frame.width) {
      if (!noVerticalLine) {
        wakeUpListContext.moveTo(x1, frame.y);
        wakeUpListContext.lineTo(x1, frame.y + frame.height);
      }
      if (currentCpu == wake.cpu) {
        let centerY = Math.floor(frame.y + frame.height / 2);
        wakeUpListContext.moveTo(x1, centerY - 6);
        wakeUpListContext.lineTo(x1 + 4, centerY);
        wakeUpListContext.lineTo(x1, centerY + 6);
        wakeUpListContext.lineTo(x1 - 4, centerY);
        wakeUpListContext.lineTo(x1, centerY - 6);
        wakeUpListContext.fill();
      }
    }
    if (wakeup) {
      let x2 = Math.floor(
        ns2x(
          wakeup.ts || 0,
          startNS,
          endNS,
          totalNS,
          frame
        )
      );
      let y = frame.y + frame.height - 10;
      wakeUpListContext.moveTo(x1, y);
      wakeUpListContext.lineTo(x2, y);
      wakeUpListContext.moveTo(x2, y - 25);
      wakeUpListContext.lineTo(x2, y + 5);

      let s = ns2s(
        (wakeup.ts || 0) - (wake.wakeupTime || 0)
      );
      let wakeUpListDistance = x2 - x1;
      if (wakeUpListDistance > 12) {
        wakeUpListContext.moveTo(x1, y);
        wakeUpListContext.lineTo(x1 + 6, y - 3);
        wakeUpListContext.moveTo(x1, y);
        wakeUpListContext.lineTo(x1 + 6, y + 3);
        wakeUpListContext.moveTo(x2, y);
        wakeUpListContext.lineTo(x2 - 6, y - 3);
        wakeUpListContext.moveTo(x2, y);
        wakeUpListContext.lineTo(x2 - 6, y + 3);
        let measure = wakeUpListContext.measureText(s);
        let tHeight =
          measure.actualBoundingBoxAscent +
          measure.actualBoundingBoxDescent;
        let xStart = x1 + Math.floor(wakeUpListDistance / 2 - measure.width / 2);
        if (wakeUpListDistance > measure.width + 4) {
          wakeUpListContext.fillStyle = '#ffffff';
          wakeUpListContext.fillRect(
            xStart - 2,
            y - 4 - tHeight,
            measure.width + 4,
            tHeight + 4
          );
          wakeUpListContext.font = '10px solid';
          wakeUpListContext.fillStyle = '#000000';
          wakeUpListContext.textBaseline = 'bottom';
          wakeUpListContext.fillText(s, xStart, y - 2);
        }
      }
    }
    wakeUpListContext.strokeStyle = '#000000';
    wakeUpListContext.stroke();
    wakeUpListContext.closePath();
  }
}
