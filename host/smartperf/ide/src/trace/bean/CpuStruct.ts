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
import { WakeupBean } from './WakeupBean.js';

export class CpuStruct extends BaseStruct {
  static cpuCount: number; //最大cpu数量
  static hoverCpuStruct: CpuStruct | undefined;
  static selectCpuStruct: CpuStruct | undefined;
  static wakeupBean: WakeupBean | null | undefined;
  cpu: number | undefined;
  dur: number | undefined;
  end_state: string | undefined;
  id: number | undefined;
  name: string | undefined;
  priority: number | undefined;
  processCmdLine: string | undefined;
  processId: number | undefined;
  processName: string | undefined;
  schedId: number | undefined;
  startTime: number | undefined;
  tid: number | undefined;
  type: string | undefined;

  static draw(cpuBeanStructCanvasCtx: CanvasRenderingContext2D, cpuBeanStruct: CpuStruct) {
    if (cpuBeanStruct.frame) {
      let cpuBeanStructWidth = cpuBeanStruct.frame.width || 0;
      if (cpuBeanStruct.processId === CpuStruct.hoverCpuStruct?.processId || !CpuStruct.hoverCpuStruct) {
        cpuBeanStructCanvasCtx.fillStyle = ColorUtils.colorForTid((cpuBeanStruct.processId || 0) > 0 ? cpuBeanStruct.processId || 0 : cpuBeanStruct.tid || 0);
      } else {
        cpuBeanStructCanvasCtx.fillStyle = '#e0e0e0';
      }
      cpuBeanStructCanvasCtx.fillRect(cpuBeanStruct.frame.x, cpuBeanStruct.frame.y, cpuBeanStructWidth, cpuBeanStruct.frame.height);
      if (cpuBeanStructWidth > textPadding * 2) {
        let cpuBeanProcess = `${cpuBeanStruct.processName || 'Process'} [${cpuBeanStruct.processId}]`;
        let cpuBeanThread = `${cpuBeanStruct.name || 'Thread'} [${cpuBeanStruct.tid}]`;
        let cpuBeanProcessMeasure = cpuBeanStructCanvasCtx.measureText(cpuBeanProcess);
        let cpuBeanThreadMeasure = cpuBeanStructCanvasCtx.measureText(cpuBeanThread);
        let cpuBeanProcessCharWidth = Math.round(cpuBeanProcessMeasure.width / cpuBeanProcess.length);
        let cpuBeanThreadCharWidth = Math.round(cpuBeanThreadMeasure.width / cpuBeanThread.length);
        cpuBeanStructCanvasCtx.fillStyle = '#ffffff';
        let y = cpuBeanStruct.frame.height / 2 + cpuBeanStruct.frame.y;
        if (cpuBeanProcessMeasure.width < cpuBeanStructWidth - textPadding * 2) {
          let x1 = Math.floor(cpuBeanStructWidth / 2 - cpuBeanProcessMeasure.width / 2 + cpuBeanStruct.frame.x + textPadding);
          cpuBeanStructCanvasCtx.textBaseline = 'bottom';
          cpuBeanStructCanvasCtx.fillText(cpuBeanProcess, x1, y, cpuBeanStructWidth - textPadding * 2);
        } else if (cpuBeanStructWidth - textPadding * 2 > cpuBeanProcessCharWidth * 4) {
          let chatNum = (cpuBeanStructWidth - textPadding * 2) / cpuBeanProcessCharWidth;
          let x1 = cpuBeanStruct.frame.x + textPadding;
          cpuBeanStructCanvasCtx.textBaseline = 'bottom';
          cpuBeanStructCanvasCtx.fillText(cpuBeanProcess.substring(0, chatNum - 4) + '...', x1, y, cpuBeanStructWidth - textPadding * 2);
        }
        if (cpuBeanThreadMeasure.width < cpuBeanStructWidth - textPadding * 2) {
          cpuBeanStructCanvasCtx.textBaseline = 'top';
          let x2 = Math.floor(cpuBeanStructWidth / 2 - cpuBeanThreadMeasure.width / 2 + cpuBeanStruct.frame.x + textPadding);
          cpuBeanStructCanvasCtx.fillText(cpuBeanThread, x2, y + 2, cpuBeanStructWidth - textPadding * 2);
        } else if (cpuBeanStructWidth - textPadding * 2 > cpuBeanThreadCharWidth * 4) {
          let chatNum = (cpuBeanStructWidth - textPadding * 2) / cpuBeanThreadCharWidth;
          let x1 = cpuBeanStruct.frame.x + textPadding;
          cpuBeanStructCanvasCtx.textBaseline = 'top';
          cpuBeanStructCanvasCtx.fillText(cpuBeanThread.substring(0, chatNum - 4) + '...', x1, y + 2, cpuBeanStructWidth - textPadding * 2);
        }
      }
      if (CpuStruct.selectCpuStruct && CpuStruct.equals(CpuStruct.selectCpuStruct, cpuBeanStruct)) {
        cpuBeanStructCanvasCtx.strokeStyle = '#232c5d';
        cpuBeanStructCanvasCtx.lineWidth = 2;
        cpuBeanStructCanvasCtx.strokeRect(cpuBeanStruct.frame.x, cpuBeanStruct.frame.y, cpuBeanStructWidth - 2, cpuBeanStruct.frame.height);
      }
    }
  }

  static equals(d1: CpuStruct, d2: CpuStruct): boolean {
    if (
      d1 &&
      d2 &&
      d1.cpu == d2.cpu &&
      d1.tid == d2.tid &&
      d1.processId == d2.processId &&
      d1.startTime == d2.startTime &&
      d1.dur == d2.dur
    ) {
      return true;
    } else {
      return false;
    }
  }
}

const textPadding = 2;
