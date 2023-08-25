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
import { CpuStruct } from '../database/ui-worker/ProcedureWorkerCPU.js';

const padding = 1;

export class ProcessStruct extends BaseStruct {
  startTime: number | undefined;
  cpu: number | undefined;
  dur: number | undefined;
  id: number | undefined;
  utid: number | undefined;
  pid: number | undefined;
  process: string | undefined;
  state: string | undefined;
  thread: string | undefined;
  tid: number | undefined;
  ts: number | undefined;
  type: string | undefined;

  static draw(processBeanCanvasCtx: CanvasRenderingContext2D, processBeanStruct: ProcessStruct) {
    if (processBeanStruct.frame) {
      let width = processBeanStruct.frame.width || 0;
      processBeanCanvasCtx.fillStyle = ColorUtils.colorForTid(processBeanStruct.pid || 0);
      let miniHeight = Math.round(processBeanStruct.frame.height / CpuStruct.cpuCount);
      processBeanCanvasCtx.fillRect(
        processBeanStruct.frame.x,
        processBeanStruct.frame.y + (processBeanStruct.cpu || 0) * miniHeight + padding,
        processBeanStruct.frame.width,
        miniHeight - padding * 2
      );
    }
  }
}
