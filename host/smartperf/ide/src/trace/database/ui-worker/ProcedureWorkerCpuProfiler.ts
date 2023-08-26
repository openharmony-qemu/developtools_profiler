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

import { BaseStruct, Rect, Render, drawString, isFrameContainPoint, ns2x } from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import { JsCpuProfilerChartFrame } from '../../bean/JsStruct.js';

export class JsCpuProfilerRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    jsCpuProfilerRow: TraceRow<JsCpuProfilerStruct>
  ) {
    let list = jsCpuProfilerRow.dataList;
    let filter = jsCpuProfilerRow.dataListCache;
    jsCpuProfiler(
      list,
      filter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      jsCpuProfilerRow.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let jsCpuProfilerFind = false;
    for (let re of filter) {
      JsCpuProfilerStruct.draw(req.context, re);
      if (jsCpuProfilerRow.isHover) {
        if (
          re.endTime - re.startTime == 0 ||
          re.endTime - re.startTime == null ||
          re.endTime - re.startTime == undefined
        ) {
          if (
            re.frame &&
            jsCpuProfilerRow.hoverX >= re.frame.x - 5 &&
            jsCpuProfilerRow.hoverX <= re.frame.x + 5 &&
            jsCpuProfilerRow.hoverY >= re.frame.y &&
            jsCpuProfilerRow.hoverY <= re.frame.y + re.frame.height
          ) {
            JsCpuProfilerStruct.hoverJsCpuProfilerStruct = re;
            jsCpuProfilerFind = true;
          }
        } else {
          if (re.frame && isFrameContainPoint(re.frame, jsCpuProfilerRow.hoverX, jsCpuProfilerRow.hoverY)) {
            JsCpuProfilerStruct.hoverJsCpuProfilerStruct = re;
            jsCpuProfilerFind = true;
          }
        }
      }
    }
    if (!jsCpuProfilerFind && jsCpuProfilerRow.isHover) JsCpuProfilerStruct.hoverJsCpuProfilerStruct = undefined;
    req.context.closePath();
  }
}
export function jsCpuProfiler(
  list: Array<any>,
  filter: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: Rect,
  use: boolean
) {
  if (use && filter.length > 0) {
    for (let i = 0, len = filter.length; i < len; i++) {
      if ((filter[i].startTime || 0) + (filter[i].totalTime || 0) >= startNS && (filter[i].startTime || 0) <= endNS) {
        JsCpuProfilerStruct.setJsCpuProfilerFrame(filter[i], startNS, endNS, totalNS, frame);
      } else {
        filter[i].frame = null;
      }
    }
    return;
  }
  filter.length = 0;
  if (list) {
    let groups = list
      .filter((it) => (it.startTime ?? 0) + (it.totalTime ?? 0) >= startNS && (it.startTime ?? 0) <= endNS)
      .map((it) => {
        JsCpuProfilerStruct.setJsCpuProfilerFrame(it, startNS, endNS, totalNS, frame);
        return it;
      })
      .reduce((pre, current) => {
        (pre[`${current.frame.x}-${current.depth}`] = pre[`${current.frame.x}-${current.depth}`] || []).push(current);
        return pre;
      }, {});
    Reflect.ownKeys(groups).map((kv) => {
      let arr = groups[kv].sort((a: JsCpuProfilerChartFrame, b: JsCpuProfilerChartFrame) => b.totalTime - a.totalTime);
      filter.push(arr[0]);
    });
  }
}

const padding = 1;
export class JsCpuProfilerStruct extends BaseStruct {
  static lastSelectJsCpuProfilerStruct: JsCpuProfilerStruct | undefined;
  static selectJsCpuProfilerStruct: JsCpuProfilerStruct | undefined;
  static hoverJsCpuProfilerStruct: JsCpuProfilerStruct | undefined;
  id: number = 0;
  name: string = '';
  startTime: number = 0;
  endTime: number = 0;
  selfTime: number = 0;
  totalTime: number = 0;
  url: string = '';
  depth: number = 0;
  parentId: number = 0;
  children!: Array<JsCpuProfilerChartFrame>;
  isSelect: boolean = false;

  static setJsCpuProfilerFrame(jsCpuProfilerNode: any, startNS: number, endNS: number, totalNS: number, frame: Rect) {
    let x1: number, x2: number;
    if ((jsCpuProfilerNode.startTime || 0) > startNS && (jsCpuProfilerNode.startTime || 0) < endNS) {
      x1 = ns2x(jsCpuProfilerNode.startTime || 0, startNS, endNS, totalNS, frame);
    } else {
      x1 = 0;
    }
    if (
      (jsCpuProfilerNode.startTime || 0) + (jsCpuProfilerNode.totalTime || 0) > startNS &&
      (jsCpuProfilerNode.startTime || 0) + (jsCpuProfilerNode.totalTime || 0) < endNS
    ) {
      x2 = ns2x(
        (jsCpuProfilerNode.startTime || 0) + (jsCpuProfilerNode.totalTime || 0),
        startNS,
        endNS,
        totalNS,
        frame
      );
    } else {
      x2 = frame.width;
    }
    if (!jsCpuProfilerNode.frame) {
      jsCpuProfilerNode.frame = {};
    }
    let getV: number = x2 - x1 < 1 ? 1 : x2 - x1;
    jsCpuProfilerNode.frame.x = Math.floor(x1);
    jsCpuProfilerNode.frame.y = jsCpuProfilerNode.depth * 20;
    jsCpuProfilerNode.frame.width = Math.ceil(getV);
    jsCpuProfilerNode.frame.height = 20;
  }

  static draw(jsCpuProfilerCtx: CanvasRenderingContext2D, data: JsCpuProfilerStruct) {
    if (data.frame) {
      if (data.endTime - data.startTime == undefined || data.endTime - data.startTime == null) {
      } else {
        jsCpuProfilerCtx.globalAlpha = 1;
        if (data.name === '(program)') {
          jsCpuProfilerCtx.fillStyle = '#ccc';
        } else if (data.name === '(idle)') {
          jsCpuProfilerCtx.fillStyle = '#f0f0f0';
        } else {
          jsCpuProfilerCtx.fillStyle =
            ColorUtils.FUNC_COLOR[ColorUtils.hashFunc(data.name || '', 0, ColorUtils.FUNC_COLOR.length)];
        }
        let miniHeight = 20;
        if (JsCpuProfilerStruct.hoverJsCpuProfilerStruct && data == JsCpuProfilerStruct.hoverJsCpuProfilerStruct) {
          jsCpuProfilerCtx.globalAlpha = 0.7;
        }
        jsCpuProfilerCtx.fillRect(data.frame.x, data.frame.y, data.frame.width, miniHeight - padding * 2);
        if (data.frame.width > 8) {
          jsCpuProfilerCtx.lineWidth = 1;
          jsCpuProfilerCtx.fillStyle = '#fff';
          jsCpuProfilerCtx.textBaseline = 'middle';
          drawString(jsCpuProfilerCtx, `${data.name || ''}`, 4, data.frame, data);
        }
        if (data === JsCpuProfilerStruct.selectJsCpuProfilerStruct) {
          jsCpuProfilerCtx.strokeStyle = '#000';
          jsCpuProfilerCtx.lineWidth = 2;
          jsCpuProfilerCtx.strokeRect(
            data.frame.x + 1,
            data.frame.y + 1,
            data.frame.width - 2,
            miniHeight - padding * 2 - 2
          );
        }
      }
    }
  }
}
