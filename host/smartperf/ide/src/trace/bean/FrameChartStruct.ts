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

import { SpApplication } from '../SpApplication.js';
import { Rect } from '../component/trace/timer-shaft/Rect.js';
import { warn } from '../../log/Log.js';
import { BaseStruct, drawString } from '../database/ui-worker/ProcedureWorkerCommon.js';

const padding: number = 1;
const rectHeight = 20;
const lightBlue = {
  r: 82,
  g: 145,
  b: 255,
  a: 0.9,
};

export class ChartStruct extends BaseStruct {
  static hoverFuncStruct: ChartStruct | undefined;
  static selectFuncStruct: ChartStruct | undefined;
  static lastSelectFuncStruct: ChartStruct | undefined;
  isDraw = false; // 是否绘制，太小的不绘制
  depth: number = 0;
  symbol: string = '';
  lib: string = '';

  size: number = 0; // 实际size
  count: number = 0; // 实际count
  dur: number = 0; // 实际dur
  //搜索后会根据搜索匹配的函数的值赋值给parent
  searchSize: number = 0; //
  searchCount: number = 0;
  searchDur: number = 0;
  //点击绘制的size在搜索的基础上，赋值给parent
  drawSize: number = 0;
  drawCount: number = 0;
  drawDur: number = 0;

  parent: ChartStruct | undefined;
  children: Array<ChartStruct> = [];
  percent: number = 0; // 0 - 1 该node所占整体的百分比
  addr: string = '';
  isSearch: boolean = false;
  isChartSelect: boolean = false; // 是否为点选的调用链
  isChartSelectParent: boolean = false; // 用来显示灰色
  tsArray: Array<number> = [];
  countArray: Array<number> = [];
  durArray: Array<number> = [];
}

export enum ChartMode {
  Byte, // Native Memory
  Count, // Perf
  Duration, // eBpf
}

export function setFuncFrame(node: ChartStruct, canvasFrame: Rect, total: number, mode: ChartMode): void {
  if (!node.frame) {
    node.frame = new Rect(0, 0, 0, 0);
  }
  // filter depth is 0
  if (node.parent) {
    let idx = node.parent.children.indexOf(node);
    if (idx === 0) {
      node.frame!.x = node.parent.frame!.x;
    } else {
      // set x by left frame. left frame is parent.children[idx - 1]
      node.frame.x = node.parent.children[idx - 1].frame!.x + node.parent.children[idx - 1].frame!.width;
    }
    if (node.parent?.isChartSelect && !node.isChartSelect) {
      node.frame!.width = 0;
    } else {
      switch (mode) {
        case ChartMode.Byte:
          node.frame!.width = Math.floor(((node.drawSize || node.size) / total) * canvasFrame.width);
          break;
        case ChartMode.Count:
          node.frame!.width = Math.floor(((node.drawCount || node.count) / total) * canvasFrame.width);
          break;
        case ChartMode.Duration:
          node.frame!.width = Math.floor(((node.drawDur || node.dur) / total) * canvasFrame.width);
          break;
        default:
          warn('not match ChartMode');
      }
    }

    node.frame!.y = node.parent.frame!.y + rectHeight;
    node.frame!.height = rectHeight;
  }
}

/**
 * draw rect
 * @param canvasCtx CanvasRenderingContext2D
 * @param node rect which is need draw
 * @param percent function size or count / total size or count
 */
export function draw(canvasCtx: CanvasRenderingContext2D, node: ChartStruct): void {
  let spApplication = <SpApplication>document.getElementsByTagName('sp-application')[0];
  if (!node.frame) {
    return;
  }
  //主体
  const drawHeight = rectHeight - padding * 2; //绘制方块上下留一个像素
  if (node.depth === 0 || (node.isChartSelectParent && node !== ChartStruct.selectFuncStruct)) {
    canvasCtx.fillStyle = `rgba(${lightBlue.g}, ${lightBlue.g}, ${lightBlue.g}, ${lightBlue.a})`;
  } else {
    if (node.isSearch) {
      canvasCtx.fillStyle = `rgba(${lightBlue.r}, ${lightBlue.g}, ${lightBlue.b}, ${lightBlue.a})`;
    } else {
      canvasCtx.fillStyle = getHeatColor(node.percent);
    }
  }
  canvasCtx.fillRect(node.frame.x, node.frame.y, node.frame.width, drawHeight);
  //边框
  canvasCtx.lineWidth = 0.4;
  if (isHover(node)) {
    if (spApplication.dark) {
      canvasCtx.strokeStyle = '#fff';
    } else {
      canvasCtx.strokeStyle = '#000';
    }
  } else {
    if (spApplication.dark) {
      canvasCtx.strokeStyle = '#000';
    } else {
      canvasCtx.strokeStyle = '#fff';
    }
  }
  canvasCtx.strokeRect(node.frame.x, node.frame.y, node.frame.width, drawHeight);
  //文字
  if (node.frame.width > 10) {
    if (node.percent > 0.6 || node.isSearch) {
      canvasCtx.fillStyle = '#fff';
    } else {
      canvasCtx.fillStyle = '#000';
    }
    drawString(canvasCtx, node.symbol || '', 5, node.frame, node);
  }
  node.isDraw = true;
}

/**
 * 火焰图颜色计算，根据每个node占总大小的百分比调整
 * @param widthPercentage 百分比
 * @returns rbg
 */
function getHeatColor(widthPercentage: number): string {
  return `rgba(
    ${Math.floor(245 + 10 * (1 - widthPercentage))},
    ${Math.floor(110 + 105 * (1 - widthPercentage))},
    ${100},
    0.9)`;
}

function isHover(data: ChartStruct): boolean {
  return ChartStruct.hoverFuncStruct === data;
}
