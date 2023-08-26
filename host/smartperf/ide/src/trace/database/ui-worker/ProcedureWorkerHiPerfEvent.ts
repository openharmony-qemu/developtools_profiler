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
  hiPerf,
  HiPerfStruct,
  PerfRender,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class HiperfEventRender extends PerfRender {
  renderMainThread(hiPerfEventReq: any, row: TraceRow<HiPerfEventStruct>): void {
    let list = row.dataList;
    let list2 = row.dataList2;
    let filter = row.dataListCache;
    let groupBy10MS = hiPerfEventReq.scale > 30_000_000;
    if (list && row.dataList2.length === 0) {
      row.dataList2 = HiPerfEventStruct.eventGroupBy10MS(list, hiPerfEventReq.intervalPerf, hiPerfEventReq.type);
    }
    hiPerf(
      list,
      list2,
      filter,
      TraceRow.range?.startNS ?? 0,
      TraceRow.range?.endNS ?? 0,
      row.frame,
      groupBy10MS,
      hiPerfEventReq.useCache || (TraceRow.range?.refresh ?? false)
    );
    hiPerfEventReq.context.beginPath();
    hiPerfEventReq.context.fillStyle = ColorUtils.FUNC_COLOR[0];
    hiPerfEventReq.context.strokeStyle = ColorUtils.FUNC_COLOR[0];
    let offset = groupBy10MS ? 0 : 3;
    let normalPath = new Path2D();
    let specPath = new Path2D();
    let find = false;
    for (let re of filter) {
      HiPerfEventStruct.draw(hiPerfEventReq.context, normalPath, specPath, re, groupBy10MS);
      if (row.isHover) {
        if (re.frame && row.hoverX >= re.frame.x - offset && row.hoverX <= re.frame.x + re.frame.width + offset) {
          HiPerfEventStruct.hoverStruct = re;
          find = true;
        }
      }
    }
    if (!find && row.isHover) {
      HiPerfEventStruct.hoverStruct = undefined;
    }
    if (groupBy10MS) {
      hiPerfEventReq.context.fill(normalPath);
    } else {
      hiPerfEventReq.context.stroke(normalPath);
      HiPerfStruct.drawSpecialPath(hiPerfEventReq.context, specPath);
    }
    let maxEvent = HiPerfEventStruct.maxEvent!.get(hiPerfEventReq.type!) || 0;
    let textMetrics = hiPerfEventReq.context.measureText(maxEvent);
    hiPerfEventReq.context.globalAlpha = 0.8;
    hiPerfEventReq.context.fillStyle = '#f0f0f0';
    hiPerfEventReq.context.fillRect(0, 5, textMetrics.width + 8, 18);
    hiPerfEventReq.context.globalAlpha = 1;
    hiPerfEventReq.context.fillStyle = '#333';
    hiPerfEventReq.context.textBaseline = 'middle';
    hiPerfEventReq.context.fillText(maxEvent, 4, 5 + 9);
    hiPerfEventReq.context.stroke();
    hiPerfEventReq.context.closePath();
  }

  render(hiPerfEventRequest: RequestMessage, list: Array<any>, filter: Array<any>, dataList2: Array<any>): void {}
}

export class HiPerfEventStruct extends HiPerfStruct {
  static hoverStruct: HiPerfEventStruct | undefined;
  static selectStruct: HiPerfEventStruct | undefined;

  static maxEvent: Map<string, number> | undefined = new Map();
  sum: number | undefined;
  max: number | undefined;

  static eventGroupBy10MS(array: Array<any>, intervalPerf: number, type: string): Array<any> {
    let obj = array
      .map((hiPerfDataItem) => {
        hiPerfDataItem.timestamp_group = Math.trunc(hiPerfDataItem.startNS / 1_000_000_0) * 1_000_000_0;
        return hiPerfDataItem;
      })
      .reduce((pre, current) => {
        (pre[current['timestamp_group']] = pre[current['timestamp_group']] || []).push(current);
        return pre;
      }, {});
    let eventArr: any[] = [];
    let max = 0;
    for (let aKey in obj) {
      let sum = obj[aKey].reduce((pre: any, cur: any) => {
        return pre + cur.event_count;
      }, 0);
      if (sum > max) max = sum;
      let ns = parseInt(aKey);
      eventArr.push({
        startNS: ns,
        dur: 1_000_000_0,
        height: 0,
        sum: sum,
      });
    }
    if (typeof HiPerfEventStruct.maxEvent!.get(type) === 'undefined') {
      HiPerfEventStruct.maxEvent!.set(type, max);
    }
    eventArr.map((it) => {
      it.height = Math.floor((40 * it.sum) / max);
      it.max = max;
      return it;
    });
    return eventArr;
  }
}
