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
import { Rect } from '../component/trace/timer-shaft/Rect.js';
import { ColorUtils } from '../component/trace/base/ColorUtils.js';
import { drawString } from '../database/ui-worker/ProcedureWorkerCommon.js';

export class FuncStruct extends BaseStruct {
  static hoverFuncStruct: FuncStruct | undefined;
  static selectFuncStruct: FuncStruct | undefined;
  argsetid: number | undefined;
  depth: number | undefined;
  dur: number | undefined;
  funName: string | undefined;
  id: number | undefined;
  is_main_thread: number | undefined;
  parent_id: number | undefined;
  startTs: number | undefined;
  threadName: string | undefined;
  tid: number | undefined;
  itid: number | undefined;
  ipid: number | undefined;
  identify: number | undefined;
  track_id: number | undefined;

  static draw(funcBeanStructCanvasCtx: CanvasRenderingContext2D, funcBeanStruct: FuncStruct) {
    if (funcBeanStruct.frame) {
      if (funcBeanStruct.dur == undefined || funcBeanStruct.dur == null || funcBeanStruct.dur == 0 || FuncStruct.isBinder(funcBeanStruct)) {
      } else {
        funcBeanStructCanvasCtx.fillStyle = ColorUtils.FUNC_COLOR[funcBeanStruct.depth || 0 % ColorUtils.FUNC_COLOR.length];
        let miniHeight = 20;
        funcBeanStructCanvasCtx.fillRect(funcBeanStruct.frame.x, funcBeanStruct.frame.y, funcBeanStruct.frame.width, miniHeight - padding * 2);
        funcBeanStructCanvasCtx.fillStyle = '#fff';
        drawString(funcBeanStructCanvasCtx, funcBeanStruct.funName || '', 5, funcBeanStruct.frame, funcBeanStruct);
        if (FuncStruct.isSelected(funcBeanStruct)) {
          funcBeanStructCanvasCtx.strokeStyle = '#000';
          funcBeanStructCanvasCtx.lineWidth = 1;
          funcBeanStructCanvasCtx.strokeRect(funcBeanStruct.frame.x, funcBeanStruct.frame.y, funcBeanStruct.frame.width, miniHeight - padding * 2);
        }
      }
    }
  }

  static isSelected(data: FuncStruct): boolean {
    return (
      FuncStruct.selectFuncStruct != undefined &&
      FuncStruct.selectFuncStruct.startTs == data.startTs &&
      FuncStruct.selectFuncStruct.dur == data.dur &&
      FuncStruct.selectFuncStruct.funName == data.funName
    );
  }

  static isBinder(data: FuncStruct): boolean {
    if (
      data.funName != null &&
      (data.funName.toLowerCase().startsWith('binder transaction') ||
        data.funName.toLowerCase().startsWith('binder async') ||
        data.funName.toLowerCase().startsWith('binder reply'))
    ) {
      return true;
    } else {
      return false;
    }
  }

  static isBinderAsync(data: FuncStruct): boolean {
    if (data.funName != null && data.funName.toLowerCase().includes('async')) {
      return true;
    } else {
      return false;
    }
  }
}

const padding = 1;
