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

import { ColorUtils } from '../component/trace/base/ColorUtils.js';
import { BaseStruct } from './BaseStruct.js';

export class CpuFreqStruct extends BaseStruct {
  static maxFreq: number = 0;
  static maxFreqName: string = '0 GHz';
  static hoverCpuFreqStruct: CpuFreqStruct | undefined;
  static selectCpuFreqStruct: CpuFreqStruct | undefined;
  cpu: number | undefined;
  value: number | undefined;
  startNS: number | undefined;
  dur: number | undefined; // Self-supplementing, the database is not returned

  static draw(freqBeanStructCanvasCtx: any, freqBeanStruct: CpuFreqStruct) {
    if (freqBeanStruct.frame) {
      let freqBeanStructWidth = freqBeanStruct.frame.width || 0;
      let freqBeanStructIndex = freqBeanStruct.cpu || 0;
      freqBeanStructIndex += 2;
      freqBeanStructCanvasCtx.fillStyle = ColorUtils.colorForTid(freqBeanStructIndex);
      freqBeanStructCanvasCtx.strokeStyle = ColorUtils.colorForTid(freqBeanStructIndex);
      if (freqBeanStruct.startNS === CpuFreqStruct.hoverCpuFreqStruct?.startNS) {
        freqBeanStructCanvasCtx.lineWidth = 1;
        freqBeanStructCanvasCtx.globalAlpha = 0.6;
        let freqBeanStructDrawHeight: number = Math.floor(
          ((freqBeanStruct.value || 0) * (freqBeanStruct.frame.height || 0) * 1.0) /
          CpuFreqStruct.maxFreq
        );
        freqBeanStructCanvasCtx.fillRect(
          freqBeanStruct.frame.x,
          freqBeanStruct.frame.y + freqBeanStruct.frame.height - freqBeanStructDrawHeight,
          freqBeanStructWidth,
          freqBeanStructDrawHeight
        );
        freqBeanStructCanvasCtx.beginPath();
        freqBeanStructCanvasCtx.arc(
          freqBeanStruct.frame.x,
          freqBeanStruct.frame.y + freqBeanStruct.frame.height - freqBeanStructDrawHeight,
          3,
          0,
          2 * Math.PI,
          true
        );
        freqBeanStructCanvasCtx.fill();
        freqBeanStructCanvasCtx.globalAlpha = 1.0;
        freqBeanStructCanvasCtx.stroke();
        freqBeanStructCanvasCtx.beginPath();
        freqBeanStructCanvasCtx.moveTo(
          freqBeanStruct.frame.x + 3,
          freqBeanStruct.frame.y + freqBeanStruct.frame.height - freqBeanStructDrawHeight
        );
        freqBeanStructCanvasCtx.lineWidth = 3;
        freqBeanStructCanvasCtx.lineTo(
          freqBeanStruct.frame.x + freqBeanStructWidth,
          freqBeanStruct.frame.y + freqBeanStruct.frame.height - freqBeanStructDrawHeight
        );
        freqBeanStructCanvasCtx.stroke();
      } else {
        freqBeanStructCanvasCtx.globalAlpha = 0.6;
        freqBeanStructCanvasCtx.lineWidth = 1;
        let drawHeight: number = Math.floor(
          ((freqBeanStruct.value || 0) * (freqBeanStruct.frame.height || 0)) /
          CpuFreqStruct.maxFreq
        );
        freqBeanStructCanvasCtx.fillRect(
          freqBeanStruct.frame.x,
          freqBeanStruct.frame.y + freqBeanStruct.frame.height - drawHeight,
          freqBeanStructWidth,
          drawHeight
        );
      }
    }
    freqBeanStructCanvasCtx.globalAlpha = 1.0;
    freqBeanStructCanvasCtx.lineWidth = 1;
  }

}

const textPadding = 2;
