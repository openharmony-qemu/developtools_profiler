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

import {
  BaseStruct,
  dataFilterHandler,
  isFrameContainPoint,
  Rect,
  Render,
  RequestMessage,
  drawString,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { Utils } from '../../component/trace/base/Utils.js';
import { ThreadStruct as BaseThreadStruct } from '../../bean/ThreadStruct.js';
export class ThreadRender extends Render {
  renderMainThread(
    threadReq: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      translateY: number;
    },
    row: TraceRow<ThreadStruct>
  ) {
    let threadList = row.dataList;
    let threadFilter = row.dataListCache;
    dataFilterHandler(threadList, threadFilter, {
      startKey: 'startTime',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: row.frame,
      paddingTop: 5,
      useCache: threadReq.useCache || !(TraceRow.range?.refresh ?? false),
    });
    threadReq.context.beginPath();
    for (let re of threadFilter) {
      re.translateY = threadReq.translateY;
      ThreadStruct.drawThread(threadReq.context, re);
      if (row.isHover && re.frame && isFrameContainPoint(re.frame!, row.hoverX, row.hoverY)) {
        ThreadStruct.hoverThreadStruct = re;
      }
    }
    threadReq.context.closePath();
  }

  render(threadReq: RequestMessage, threadList: Array<any>, threadFilter: Array<any>) {}
}

const padding = 3;

export class ThreadStruct extends BaseThreadStruct {
  static otherColor = '#673ab7';
  static uninterruptibleSleepColor = '#f19d38';
  static uninterruptibleSleepNonIOColor = '#795548';
  static traceColor = '#0d47a1';
  static sColor = '#FBFBFB';
  static hoverThreadStruct: ThreadStruct | undefined;
  static selectThreadStruct: ThreadStruct | undefined;
  argSetID: number | undefined;
  translateY: number | undefined;
  textMetricsWidth: number | undefined;

  static drawThread(threadContext: CanvasRenderingContext2D, data: ThreadStruct) {
    if (data.frame) {
      threadContext.globalAlpha = 1;
      let stateText = ThreadStruct.getEndState(data.state || '');
      threadContext.fillStyle = Utils.getStateColor(data.state || '');
      if ('S' === data.state) {
        threadContext.globalAlpha = 0.2;
      }
      threadContext.fillRect(data.frame.x, data.frame.y + padding, data.frame.width, data.frame.height - padding * 2);
      threadContext.fillStyle = '#fff';
      threadContext.textBaseline = 'middle';
      threadContext.font = '8px sans-serif';
      if ('S' !== data.state) {
        data.frame.width > 7 && drawString(threadContext, stateText, 2, data.frame, data);
      }
      if (
        ThreadStruct.selectThreadStruct &&
        ThreadStruct.equals(ThreadStruct.selectThreadStruct, data) &&
        ThreadStruct.selectThreadStruct.state != 'S'
      ) {
        threadContext.strokeStyle = '#232c5d';
        threadContext.lineWidth = 2;
        threadContext.strokeRect(
          data.frame.x,
          data.frame.y + padding,
          data.frame.width - 2,
          data.frame.height - padding * 2
        );
      }
    }
  }
}
