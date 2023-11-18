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
import { ns2x } from '../component/trace/TimerShaftElement.js';
import { Rect } from '../component/trace/timer-shaft/Rect.js';

export class HeapStruct extends BaseStruct {
  static hoverHeapStruct: HeapStruct | undefined;
  startTime: number | undefined;
  endTime: number | undefined;
  dur: number | undefined;
  eventType: string | undefined;
  heapsize: number | undefined;
  density: number | undefined;
  maxHeapSize: number = 0;
  minHeapSize: number = 0;
  static draw(heapBeanStructCanvasCtx: CanvasRenderingContext2D, heapBeanData: HeapStruct) {
    if (heapBeanData.frame) {
      let width = heapBeanData.frame.width || 0;
      heapBeanStructCanvasCtx.fillStyle = '#2db3aa';
      heapBeanStructCanvasCtx.strokeStyle = '#2db3aa';
      if (heapBeanData.startTime === HeapStruct.hoverHeapStruct?.startTime) {
        heapBeanStructCanvasCtx.lineWidth = 1;
        heapBeanStructCanvasCtx.globalAlpha = 0.6;
        let drawHeight: number = Math.ceil(((heapBeanData.heapsize || 0) * (heapBeanData.frame.height || 0)) / heapBeanData.maxHeapSize);
        heapBeanStructCanvasCtx.fillRect(heapBeanData.frame.x, heapBeanData.frame.y + heapBeanData.frame.height - drawHeight, width, drawHeight);
        heapBeanStructCanvasCtx.beginPath();
        heapBeanStructCanvasCtx.arc(heapBeanData.frame.x, heapBeanData.frame.y + heapBeanData.frame.height - drawHeight, 3, 0, 2 * Math.PI, true);
        heapBeanStructCanvasCtx.fill();
        heapBeanStructCanvasCtx.globalAlpha = 1.0;
        heapBeanStructCanvasCtx.stroke();
        heapBeanStructCanvasCtx.beginPath();
        heapBeanStructCanvasCtx.moveTo(heapBeanData.frame.x + 3, heapBeanData.frame.y + heapBeanData.frame.height - drawHeight);
        heapBeanStructCanvasCtx.lineWidth = 3;
        heapBeanStructCanvasCtx.lineTo(heapBeanData.frame.x + width, heapBeanData.frame.y + heapBeanData.frame.height - drawHeight);
        heapBeanStructCanvasCtx.stroke();
      } else {
        heapBeanStructCanvasCtx.globalAlpha = 0.6;
        heapBeanStructCanvasCtx.lineWidth = 1;
        let drawHeight: number = Math.ceil(((heapBeanData.heapsize || 0) * (heapBeanData.frame.height || 0)) / heapBeanData.maxHeapSize);
        heapBeanStructCanvasCtx.fillRect(heapBeanData.frame.x, heapBeanData.frame.y + heapBeanData.frame.height - drawHeight, width, drawHeight);
      }
    }
    heapBeanStructCanvasCtx.globalAlpha = 1.0;
    heapBeanStructCanvasCtx.lineWidth = 1;
  }
}
