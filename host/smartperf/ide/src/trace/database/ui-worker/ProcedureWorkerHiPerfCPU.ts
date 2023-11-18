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
  BaseStruct,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  HiPerfStruct,
  hiPerf,
  PerfRender,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { HiPerfThreadStruct } from './ProcedureWorkerHiPerfThread.js';

export class HiperfCpuRender extends PerfRender {
  renderMainThread(req: any, row: TraceRow<HiPerfCpuStruct>) {
    let list = row.dataList;
    let filter = row.dataListCache;
    let groupBy10MS = req.scale > 30_000_000;
    if (list && row.dataList2.length == 0) {
      row.dataList2 = HiPerfCpuStruct.groupBy10MS(list, req.intervalPerf, req.maxCpu);
    }
    hiPerf(
      list,
      row.dataList2,
      filter,
      TraceRow.range?.startNS ?? 0,
      TraceRow.range?.endNS ?? 0,
      row.frame,
      groupBy10MS,
      req.useCache || (TraceRow.range?.refresh ?? false)
    );
    req.context.beginPath();
    req.context.fillStyle = ColorUtils.FUNC_COLOR[0];
    req.context.strokeStyle = ColorUtils.FUNC_COLOR[0];
    let normalPath = new Path2D();
    let specPath = new Path2D();
    let find = false;
    let offset = groupBy10MS ? 0 : 3;
    for (let re of filter) {
      if (
        row.isHover &&
        re.frame &&
        row.hoverX >= re.frame.x - offset &&
        row.hoverX <= re.frame.x + re.frame.width + offset
      ) {
        HiPerfCpuStruct.hoverStruct = re;
        find = true;
      }
      HiPerfCpuStruct.draw(req.context, normalPath, specPath, re, groupBy10MS);
    }
    if (!find && row.isHover) HiPerfCpuStruct.hoverStruct = undefined;
    if (groupBy10MS) {
      req.context.fill(normalPath);
    } else {
      req.context.stroke(normalPath);
      HiPerfStruct.drawSpecialPath(req.context, specPath);
    }
    req.context.closePath();
  }

  render(hiPerfCpuRequest: RequestMessage, list: Array<any>, filter: Array<any>, dataList2: Array<any>) {}
}

export class HiPerfCpuStruct extends HiPerfStruct {
  static hoverStruct: HiPerfCpuStruct | undefined;
  static selectStruct: HiPerfCpuStruct | undefined;

  cpu: number | undefined;
}
