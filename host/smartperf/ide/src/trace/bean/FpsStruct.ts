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

import { Rect } from '../component/trace/timer-shaft/Rect.js';
import { BaseStruct } from './BaseStruct.js';

import { ns2x } from '../component/trace/TimerShaftElement.js';

export class FpsStruct extends BaseStruct {
  static maxFps: number = 0;
  static maxFpsName: string = '0 FPS';
  static hoverFpsStruct: FpsStruct | undefined;
  static selectFpsStruct: FpsStruct | undefined;
  fps: number | undefined;
  startNS: number | undefined = 0;
  dur: number | undefined; //自补充，数据库没有返回

  static draw(fpsBeanStructCanvasCtx: CanvasRenderingContext2D, fpsBeanStructData: FpsStruct) {
    if (fpsBeanStructData.frame) {
      let fpsBeanWidth = fpsBeanStructData.frame.width || 0;
      fpsBeanStructCanvasCtx.fillStyle = '#535da6';
      fpsBeanStructCanvasCtx.strokeStyle = '#535da6';
      if (fpsBeanStructData.startNS === FpsStruct.hoverFpsStruct?.startNS) {
        fpsBeanStructCanvasCtx.lineWidth = 1;
        fpsBeanStructCanvasCtx.globalAlpha = 0.6;
        let drawHeight: number = ((fpsBeanStructData.fps || 0) * (fpsBeanStructData.frame.height || 0) * 1.0) / FpsStruct.maxFps;
        fpsBeanStructCanvasCtx.fillRect(fpsBeanStructData.frame.x, fpsBeanStructData.frame.y + fpsBeanStructData.frame.height - drawHeight, fpsBeanWidth, drawHeight);
        fpsBeanStructCanvasCtx.beginPath();
        fpsBeanStructCanvasCtx.arc(fpsBeanStructData.frame.x, fpsBeanStructData.frame.y + fpsBeanStructData.frame.height - drawHeight, 3, 0, 2 * Math.PI, true);
        fpsBeanStructCanvasCtx.fill();
        fpsBeanStructCanvasCtx.globalAlpha = 1.0;
        fpsBeanStructCanvasCtx.stroke();
        fpsBeanStructCanvasCtx.beginPath();
        fpsBeanStructCanvasCtx.moveTo(fpsBeanStructData.frame.x + 3, fpsBeanStructData.frame.y + fpsBeanStructData.frame.height - drawHeight);
        fpsBeanStructCanvasCtx.lineWidth = 3;
        fpsBeanStructCanvasCtx.lineTo(fpsBeanStructData.frame.x + fpsBeanWidth, fpsBeanStructData.frame.y + fpsBeanStructData.frame.height - drawHeight);
        fpsBeanStructCanvasCtx.stroke();
      } else {
        fpsBeanStructCanvasCtx.globalAlpha = 0.6;
        fpsBeanStructCanvasCtx.lineWidth = 1;
        let drawHeight: number = ((fpsBeanStructData.fps || 0) * (fpsBeanStructData.frame.height || 0) * 1.0) / FpsStruct.maxFps;
        fpsBeanStructCanvasCtx.fillRect(fpsBeanStructData.frame.x, fpsBeanStructData.frame.y + fpsBeanStructData.frame.height - drawHeight, fpsBeanWidth, drawHeight);
      }
    }
    fpsBeanStructCanvasCtx.globalAlpha = 1.0;
    fpsBeanStructCanvasCtx.lineWidth = 1;
  }

  static setFrame(fpsBeanStructNode: FpsStruct, fpsBeanFramePadding: number, startNS: number, endNS: number, totalNS: number, frame: Rect) {
    let fpsBeanStructX1: number, fpsBeanStructX2: number;
    if ((fpsBeanStructNode.startNS || 0) < startNS) {
      fpsBeanStructX1 = 0;
    } else {
      fpsBeanStructX1 = ns2x(fpsBeanStructNode.startNS || 0, startNS, endNS, totalNS, frame);
    }
    if ((fpsBeanStructNode.startNS || 0) + (fpsBeanStructNode.dur || 0) > endNS) {
      fpsBeanStructX2 = frame.width;
    } else {
      fpsBeanStructX2 = ns2x((fpsBeanStructNode.startNS || 0) + (fpsBeanStructNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let getV: number = fpsBeanStructX2 - fpsBeanStructX1 <= 1 ? 1 : fpsBeanStructX2 - fpsBeanStructX1;
    let rectangle: Rect = new Rect(
      Math.floor(fpsBeanStructX1),
      Math.ceil(frame.y + fpsBeanFramePadding),
      Math.ceil(getV),
      Math.floor(frame.height - fpsBeanFramePadding * 2)
    );
    fpsBeanStructNode.frame = rectangle;
  }
}

const textPadding = 2;
