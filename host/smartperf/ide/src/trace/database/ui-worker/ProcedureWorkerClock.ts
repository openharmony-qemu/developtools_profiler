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

import { BaseStruct, dataFilterHandler, isFrameContainPoint, Render } from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';

export class ClockRender extends Render {
  renderMainThread(
    clockReq: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      maxValue: number;
      index: number;
      maxName: string;
    },
    row: TraceRow<ClockStruct>
  ) {
    ClockStruct.index = clockReq.index;
    let clockList = row.dataList;
    let clockFilter = row.dataListCache;
    dataFilterHandler(clockList, clockFilter, {
      startKey: 'startNS',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: row.frame,
      paddingTop: 5,
      useCache: clockReq.useCache || !(TraceRow.range?.refresh ?? false),
    });
    clockReq.context.beginPath();
    let find = false;
    for (let re of clockFilter) {
      ClockStruct.draw(clockReq.context, re, clockReq.maxValue);
      if (row.isHover && re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        ClockStruct.hoverClockStruct = re;
        find = true;
      }
    }
    if (!find && row.isHover) ClockStruct.hoverClockStruct = undefined;
    clockReq.context.closePath();
    let s = clockReq.maxName;
    let textMetrics = clockReq.context.measureText(s);
    clockReq.context.globalAlpha = 0.8;
    clockReq.context.fillStyle = '#f0f0f0';
    clockReq.context.fillRect(0, 5, textMetrics.width + 8, 18);
    clockReq.context.globalAlpha = 1;
    clockReq.context.fillStyle = '#333';
    clockReq.context.textBaseline = 'middle';
    clockReq.context.fillText(s, 4, 5 + 9);
  }
}

export class ClockStruct extends BaseStruct {
  static maxValue: number = 0;
  static maxName: string = '';
  static hoverClockStruct: ClockStruct | undefined;
  static selectClockStruct: ClockStruct | undefined;
  static index = 0;
  filterId: number | undefined;
  value: number | undefined;
  startNS: number | undefined;
  dur: number | undefined; //自补充，数据库没有返回
  delta: number | undefined; //自补充，数据库没有返回

  static draw(clockContext: CanvasRenderingContext2D, data: ClockStruct, maxValue: number) {
    if (data.frame) {
      let width = data.frame.width || 0;
      clockContext.fillStyle = ColorUtils.colorForTid(ClockStruct.index);
      clockContext.strokeStyle = ColorUtils.colorForTid(ClockStruct.index);
      let drawHeight: number = Math.floor(((data.value || 0) * (data.frame.height || 0) * 1.0) / maxValue);
      if (drawHeight === 0) {
        drawHeight = 1;
      }
      if (ClockStruct.isHover(data)) {
        clockContext.lineWidth = 1;
        clockContext.globalAlpha = 0.6;
        clockContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
        clockContext.beginPath();
        clockContext.arc(data.frame.x, data.frame.y + data.frame.height - drawHeight, 3, 0, 2 * Math.PI, true);
        clockContext.fill();
        clockContext.globalAlpha = 1.0;
        clockContext.stroke();
        clockContext.beginPath();
        clockContext.moveTo(data.frame.x + 3, data.frame.y + data.frame.height - drawHeight);
        clockContext.lineWidth = 3;
        clockContext.lineTo(data.frame.x + width, data.frame.y + data.frame.height - drawHeight);
        clockContext.stroke();
      } else {
        clockContext.lineWidth = 1;
        clockContext.globalAlpha = 1.0;
        clockContext.strokeRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight)
        clockContext.globalAlpha = 0.6;
        clockContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
      }
    }
    clockContext.globalAlpha = 1.0;
    clockContext.lineWidth = 1;
  }

  static isHover(clock: ClockStruct) {
    return clock === ClockStruct.hoverClockStruct || clock === ClockStruct.selectClockStruct;
  }
}
