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
import { ns2x, Rect, Render } from './ProcedureWorkerCommon.js';

import { BaseStruct } from '../../bean/BaseStruct.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';

const LOG_STRUCT_HEIGHT = 7;
const X_PADDING = 5;
const Y_PADDING = 2;

export class LogRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<LogStruct>
  ): void {
    let logList = row.dataList;
    let logFilter = row.dataListCache;
    filterLogData(
      logList,
      logFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    for (let re of logFilter) {
      LogStruct.draw(req.context, re);
    }
    req.context.closePath();
  }
}

export function filterLogData(
  logList: Array<LogStruct>,
  logFilter: Array<LogStruct>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
): void {
  if (use && logFilter.length > 0) {
    for (let i = 0, len = logFilter.length; i < len; i++) {
      if ((logFilter[i].startTs || 0) + (logFilter[i].dur || 0) >= startNS && (logFilter[i].startTs || 0) <= endNS) {
        LogStruct.setLogFrame(logFilter[i], 0, startNS, endNS, totalNS, frame);
      } else {
        logFilter[i].frame = undefined;
      }
    }
    return;
  }
  logFilter.length = 0;
  if (logList) {
    let allTypeDataMap: Map<number, Array<LogStruct>> = new Map();
    for (let index: number = 0; index < logList.length; index++) {
      let itemLog: LogStruct = logList[index];
      if ((itemLog.startTs ?? 0) + (itemLog.dur ?? 0) >= startNS && (itemLog.startTs ?? 0) <= endNS) {
        let currentDepth = itemLog.depth ?? 0;
        if (allTypeDataMap.has(currentDepth)) {
          let newDataList = allTypeDataMap.get(currentDepth);
          if (newDataList && newDataList.length > 0) {
            let preData = newDataList[newDataList.length - 1];
            let pre = ns2x(preData.startTs ?? 0, startNS, endNS, totalNS, frame);
            let current = ns2x(itemLog.startTs || 0, startNS, endNS, totalNS, frame);
            if (current - pre > 1) {
              LogStruct.setLogFrame(itemLog, 0, startNS, endNS, totalNS, frame);
              newDataList!.push(itemLog);
            }
          }
        } else {
          LogStruct.setLogFrame(itemLog, 0, startNS, endNS, totalNS, frame);
          allTypeDataMap.set(currentDepth, [itemLog]);
        }
      }
    }
    for (let index = 0; index < 5; index++) {
      if (allTypeDataMap.has(index)) {
        let newVar = allTypeDataMap.get(index);
        if (newVar) {
          logFilter.push(...newVar);
        }
      }
    }
  }
}

export class LogStruct extends BaseStruct {
  static hoverLogStruct: LogStruct | undefined;
  static selectLogStruct: LogStruct | undefined;
  //日志等级对应颜色，debug、info、warn、error、fatal
  id: number | undefined;
  pid: number | undefined;
  tid: number | undefined;
  processName: string | undefined;
  startTs: number | undefined;
  level: string | undefined;
  tag: string | undefined;
  context: string | undefined;
  originTime: string | undefined;
  depth: number | undefined;
  dur: number | undefined;

  static setLogFrame(
    logNode: LogStruct,
    padding: number,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any
  ): void {
    let x1: number, x2: number;
    if ((logNode.startTs || 0) > startNS && (logNode.startTs || 0) <= endNS) {
      x1 = ns2x(logNode.startTs || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if ((logNode.startTs || 0) + (logNode.dur || 0) > startNS && (logNode.startTs || 0) + (logNode.dur || 0) <= endNS) {
      x2 = ns2x((logNode.startTs || 0) + (logNode.dur || 0), startNS, endNS, totalNS, frame);
    } else {
      x2 = frame.width;
    }
    if (!logNode.frame) {
      logNode.frame! = new Rect(0, 0, 0, 0);
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    logNode.frame!.x = Math.floor(x1);
    logNode.frame!.y = logNode.depth! * LOG_STRUCT_HEIGHT;
    logNode.frame!.width = Math.ceil(getV);
    logNode.frame!.height = LOG_STRUCT_HEIGHT;
  }

  static draw(ctx: CanvasRenderingContext2D, logData: LogStruct): void {
    if (logData.depth === undefined || logData.depth === null) {
      return;
    }
    if (logData.frame) {
      ctx.globalAlpha = 1;
      ctx.fillStyle = ColorUtils.getHilogColor(logData.level!);
      ctx.fillRect(logData.frame.x, logData.frame.y, logData.frame.width, LOG_STRUCT_HEIGHT - Y_PADDING);
    }
  }

  static isSelected(data: LogStruct): boolean {
    return (
      LogStruct.selectLogStruct !== undefined &&
      LogStruct.selectLogStruct.startTs === data.startTs &&
      LogStruct.selectLogStruct.depth === data.depth
    );
  }
}
