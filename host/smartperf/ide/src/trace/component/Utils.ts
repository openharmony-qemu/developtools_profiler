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
import { JsCpuProfilerChartFrame } from '../bean/JsStruct.js';
import { SnapshotStruct } from '../database/ui-worker/ProcedureWorkerSnapshot.js';
import { RangeSelectStruct, TraceRow } from './trace/base/TraceRow.js';

export function setSelectState(
  data: JsCpuProfilerChartFrame,
  frameSelectDataIdArr: Array<number>,
  parent?: JsCpuProfilerChartFrame
) {
  if (TraceRow.rangeSelectObject?.startNS && TraceRow.rangeSelectObject?.endNS) {
    let startTime = 0;
    let endTime = 0;
    if (data.startTime < TraceRow.rangeSelectObject?.startNS) {
      startTime = TraceRow.rangeSelectObject?.startNS;
    } else {
      startTime = data.startTime;
    }
    if (data.endTime > TraceRow.rangeSelectObject.endNS) {
      endTime = TraceRow.rangeSelectObject?.endNS;
    } else {
      endTime = data.endTime;
    }
    data.totalTime = endTime - startTime;
    data.selfTime = data.totalTime;
    if (parent) {
      parent.selfTime -= data.totalTime;
    }
  }

  data.isSelect = true;
  if (data.children.length > 0) {
    for (let child of data.children) {
      if (frameSelectDataIdArr.includes(child.id)) {
        setSelectState(child, frameSelectDataIdArr, data);
      }
    }
  }
}

export function intersectData(row:TraceRow<any>) {
  let isIntersect = (snapshotStruct: SnapshotStruct, rangeSelectStruct: RangeSelectStruct) =>
    Math.max(snapshotStruct.startNs! + snapshotStruct.dur!, rangeSelectStruct!.endNS || 0) - Math.min(snapshotStruct.startNs!, rangeSelectStruct!.startNS || 0) <
    snapshotStruct.dur! + (rangeSelectStruct!.endNS || 0) - (rangeSelectStruct!.startNS || 0);
  let intersectData = row.dataList.filter((struct: SnapshotStruct) => {
    return isIntersect(struct, TraceRow.rangeSelectObject!);
  });
  return intersectData;
}
