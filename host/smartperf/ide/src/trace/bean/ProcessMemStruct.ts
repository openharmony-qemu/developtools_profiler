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
import { ColorUtils } from '../component/trace/base/ColorUtils.js';

export class ProcessMemStruct extends BaseStruct {
  static hoverProcessMemStruct: ProcessMemStruct | undefined;
  trackId: number | undefined;
  processName: string | undefined;
  pid: number | undefined;
  upid: number | undefined;
  trackName: string | undefined;
  type: string | undefined;
  track_id: string | undefined;
  value: number | undefined;
  startTime: number | undefined;
  duration: number | undefined;
  maxValue: number | undefined;
  delta: number | undefined;

  static draw(processMemBeanStructCanvasCtx: CanvasRenderingContext2D, processMemBeanStructData: ProcessMemStruct) {
    if (processMemBeanStructData.frame) {
      let width = processMemBeanStructData.frame.width || 0;
      if (processMemBeanStructData.startTime === ProcessMemStruct.hoverProcessMemStruct?.startTime) {
        processMemBeanStructCanvasCtx.lineWidth = 1;
        processMemBeanStructCanvasCtx.globalAlpha = 0.6;
        let drawHeight: number = Math.floor(
          ((processMemBeanStructData.value || 0) * (processMemBeanStructData.frame.height || 0) * 1.0) / (processMemBeanStructData.maxValue || 0)
        );
        processMemBeanStructCanvasCtx.fillRect(processMemBeanStructData.frame.x, processMemBeanStructData.frame.y + processMemBeanStructData.frame.height - drawHeight, width, drawHeight);
        processMemBeanStructCanvasCtx.beginPath();
        processMemBeanStructCanvasCtx.arc(processMemBeanStructData.frame.x, processMemBeanStructData.frame.y + processMemBeanStructData.frame.height - drawHeight, 3, 0, 2 * Math.PI, true);
        processMemBeanStructCanvasCtx.fill();
        processMemBeanStructCanvasCtx.globalAlpha = 1.0;
        processMemBeanStructCanvasCtx.stroke();
        processMemBeanStructCanvasCtx.beginPath();
        processMemBeanStructCanvasCtx.moveTo(processMemBeanStructData.frame.x + 3, processMemBeanStructData.frame.y + processMemBeanStructData.frame.height - drawHeight);
        processMemBeanStructCanvasCtx.lineWidth = 3;
        processMemBeanStructCanvasCtx.lineTo(processMemBeanStructData.frame.x + width, processMemBeanStructData.frame.y + processMemBeanStructData.frame.height - drawHeight);
        processMemBeanStructCanvasCtx.stroke();
      } else {
        processMemBeanStructCanvasCtx.fillStyle = ColorUtils.colorForTid(processMemBeanStructData.maxValue || 0);
        processMemBeanStructCanvasCtx.strokeStyle = ColorUtils.colorForTid(processMemBeanStructData.maxValue || 0);
        processMemBeanStructCanvasCtx.globalAlpha = 0.6;
        processMemBeanStructCanvasCtx.lineWidth = 1;
        let drawHeight: number = ((processMemBeanStructData.value || 0) * (processMemBeanStructData.frame.height || 0) * 1.0) / (processMemBeanStructData.maxValue || 1);
        processMemBeanStructCanvasCtx.fillRect(processMemBeanStructData.frame.x, processMemBeanStructData.frame.y + processMemBeanStructData.frame.height - drawHeight, width, drawHeight);
      }
    }
    processMemBeanStructCanvasCtx.globalAlpha = 1.0;
    processMemBeanStructCanvasCtx.lineWidth = 1;
  }
}
