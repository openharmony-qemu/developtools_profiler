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

import { BaseStruct } from './BaseStruct.js';
import { Utils } from '../component/trace/base/Utils.js';
import { drawString } from '../database/ui-worker/ProcedureWorkerCommon.js';

const padding = 1;

export class ThreadStruct extends BaseStruct {
  static runningColor: string = '#467b3b';
  static rColor = '#a0b84d';
  static uninterruptibleSleepColor = '#f19d38';
  static sColor = '#FBFBFB';
  static hoverThreadStruct: ThreadStruct | undefined;
  static selectThreadStruct: ThreadStruct | undefined;
  hasSched: number | undefined;
  pid: number | undefined;
  processName: string | undefined;
  threadName: string | undefined;
  tid: number | undefined;
  upid: number | undefined;
  utid: number | undefined;
  cpu: number | undefined;
  dur: number | undefined;
  end_ts: number | undefined;
  id: number | undefined;
  is_main_thread: number | undefined;
  name: string | undefined;
  startTime: number | undefined;
  start_ts: number | undefined;
  state: string | undefined;
  type: string | undefined;

  static draw(threadBeanCanvasCtx: CanvasRenderingContext2D, threadBeanStructData: ThreadStruct) {
    if (threadBeanStructData.frame) {
      threadBeanCanvasCtx.globalAlpha = 1;
      let stateText = threadBeanStructData.state || '';
      if ('S' == threadBeanStructData.state) {
        threadBeanCanvasCtx.fillStyle = ThreadStruct.sColor;
        threadBeanCanvasCtx.globalAlpha = 0.2; // transparency
        threadBeanCanvasCtx.fillRect(threadBeanStructData.frame.x, threadBeanStructData.frame.y + padding, threadBeanStructData.frame.width, threadBeanStructData.frame.height - padding * 2);
        threadBeanCanvasCtx.globalAlpha = 1; // transparency
      } else if ('R' == threadBeanStructData.state) {
        threadBeanCanvasCtx.fillStyle = ThreadStruct.rColor;
        this.drawRectAndString(threadBeanCanvasCtx, threadBeanStructData);
      } else if ('D' == threadBeanStructData.state) {
        threadBeanCanvasCtx.fillStyle = ThreadStruct.uninterruptibleSleepColor;
        this.drawRectAndString(threadBeanCanvasCtx, threadBeanStructData);
      } else if ('Running' == threadBeanStructData.state) {
        threadBeanCanvasCtx.fillStyle = ThreadStruct.runningColor;
        this.drawRectAndString(threadBeanCanvasCtx, threadBeanStructData);
      } else {
        threadBeanCanvasCtx.fillStyle = ThreadStruct.rColor;
        this.drawRectAndString(threadBeanCanvasCtx, threadBeanStructData);
      }
      if (
        ThreadStruct.selectThreadStruct &&
        ThreadStruct.equals(ThreadStruct.selectThreadStruct, threadBeanStructData) &&
        ThreadStruct.selectThreadStruct.state != 'S'
      ) {
        threadBeanCanvasCtx.strokeStyle = '#232c5d';
        threadBeanCanvasCtx.lineWidth = 2;
        threadBeanCanvasCtx.strokeRect(threadBeanStructData.frame.x, threadBeanStructData.frame.y + padding, threadBeanStructData.frame.width - 2, threadBeanStructData.frame.height - padding * 2);
      }
    }
  }

  private static drawRectAndString(threadBeanCanvasCtx: CanvasRenderingContext2D, threadBeanStructData: ThreadStruct) {
    // @ts-ignore
    threadBeanCanvasCtx.fillRect(threadBeanStructData.frame.x, threadBeanStructData.frame.y + padding, threadBeanStructData.frame.width, threadBeanStructData.frame.height - padding * 2);
    threadBeanCanvasCtx.fillStyle = '#fff';
    // @ts-ignore
    drawString(threadBeanCanvasCtx, ThreadStruct.getEndState(threadBeanStructData.state || ''), 2, threadBeanStructData.frame, threadBeanStructData);
  }

  static getEndState(state: string): string {
    let statusMapElement = Utils.getEndState(state);
    if (statusMapElement) {
      return statusMapElement;
    } else {
      if ('' == statusMapElement || statusMapElement == null) {
        return '';
      }
      return 'Unknown State';
    }
  }

  static equals(d1: ThreadStruct, d2: ThreadStruct): boolean {
    return d1 &&
      d2 &&
      d1.cpu == d2.cpu &&
      d1.tid == d2.tid &&
      d1.state == d2.state &&
      d1.startTime == d2.startTime &&
      d1.dur == d2.dur;
  }
}
