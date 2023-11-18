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

import {
  BaseStruct,
  dataFilterHandler,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  drawWakeUp,
  Rect,
  Render,
  RequestMessage,
  isFrameContainPoint,
  ns2x,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { HeapStruct as BaseHeapStruct } from '../../bean/HeapStruct.js';
export class NativeMemoryRender extends Render {
  renderMainThread(req: any, row: TraceRow<any>) {}
}
export class HeapRender {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
    },
    row: TraceRow<HeapStruct>
  ) {
    let heapList = row.dataList;
    let heapFilter = row.dataListCache;
    heap(
      heapList,
      heapFilter,
      TraceRow.range?.startNS ?? 0,
      TraceRow.range?.endNS ?? 0,
      TraceRow.range?.totalNS ?? 0,
      row.frame,
      req.useCache || (TraceRow.range?.refresh ?? false)
    );
    if (heapFilter.length >= 2 && heapFilter[heapFilter.length - 1].dur === 0) {
      if (heapFilter[heapFilter.length - 2].frame && heapFilter[heapFilter.length - 1].frame) {
        heapFilter[heapFilter.length - 2].frame!.width = heapFilter[heapFilter.length - 2].frame!.width - 1;
        heapFilter[heapFilter.length - 1].frame!.width = 1;
        heapFilter[heapFilter.length - 1].frame!.x -= 1;
      }
    }
    req.context.beginPath();
    let find = false;
    for (let re of heapFilter) {
      if (row.isHover && re.frame && !find && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        HeapStruct.hoverHeapStruct = re;
        find = true;
      }
    }
    for (let re of heapFilter) {
      HeapStruct.drawHeap(req.context, re, row.drawType);
    }
    if (!find && row.isHover) HeapStruct.hoverHeapStruct = undefined;
    req.context.closePath();
  }

  render(heapRequest: RequestMessage, list: Array<any>, filter: Array<any>) {
    if (heapRequest.lazyRefresh) {
      heap(
        list,
        filter,
        heapRequest.startNS,
        heapRequest.endNS,
        heapRequest.totalNS,
        heapRequest.frame,
        heapRequest.useCache || !heapRequest.range.refresh
      );
    } else {
      if (!heapRequest.useCache) {
        heap(list, filter, heapRequest.startNS, heapRequest.endNS, heapRequest.totalNS, heapRequest.frame, false);
      }
    }
    if (heapRequest.canvas) {
      heapRequest.context.clearRect(0, 0, heapRequest.canvas.width, heapRequest.canvas.height);
      let heapArr = filter;
      if (heapArr.length > 0 && !heapRequest.range.refresh && !heapRequest.useCache && heapRequest.lazyRefresh) {
        drawLoading(
          heapRequest.context,
          heapRequest.startNS,
          heapRequest.endNS,
          heapRequest.totalNS,
          heapRequest.frame,
          heapArr[0].startTime,
          heapArr[heapArr.length - 1].startTime + heapArr[heapArr.length - 1].dur
        );
      }
      heapRequest.context.beginPath();
      drawLines(heapRequest.context, heapRequest.xs, heapRequest.frame.height, heapRequest.lineColor);
      HeapStruct.hoverHeapStruct = undefined;
      if (heapRequest.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            heapRequest.hoverX >= re.frame.x &&
            heapRequest.hoverX <= re.frame.x + re.frame.width &&
            heapRequest.hoverY >= re.frame.y &&
            heapRequest.hoverY <= re.frame.y + re.frame.height
          ) {
            HeapStruct.hoverHeapStruct = re;
            break;
          }
        }
      } else {
        HeapStruct.hoverHeapStruct = heapRequest.params.hoverHeapStruct;
      }
      for (let re of filter) {
        HeapStruct.drawHeap(heapRequest.context, re, heapRequest.params.drawType);
      }
      drawSelection(heapRequest.context, heapRequest.params);
      heapRequest.context.closePath();
      drawFlagLine(
        heapRequest.context,
        heapRequest.flagMoveInfo,
        heapRequest.flagSelectedInfo,
        heapRequest.startNS,
        heapRequest.endNS,
        heapRequest.totalNS,
        heapRequest.frame,
        heapRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: heapRequest.id,
      type: heapRequest.type,
      results: heapRequest.canvas ? undefined : filter,
      hover: HeapStruct.hoverHeapStruct,
    });
  }
}
export function heap(
  heapList: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    for (let i = 0; i < res.length; i++) {
      let it = res[i];
      if ((it.startTime || 0) + (it.dur || 0) > (startNS || 0) && (it.startTime || 0) <= (endNS || 0)) {
        HeapStruct.setFrame(res[i], 5, startNS || 0, endNS || 0, totalNS || 0, frame);
      } else {
        res[i].frame = null;
      }
    }
    return;
  }
  res.length = 0;
  for (let i = 0, len = heapList.length; i < len; i++) {
    let it = heapList[i];
    if ((it.startTime || 0) + (it.dur || 0) > (startNS || 0) && (it.startTime || 0) <= (endNS || 0)) {
      HeapStruct.setFrame(it, 5, startNS || 0, endNS || 0, totalNS || 0, frame);
      if (i > 0) {
        let last = heapList[i - 1];
        if (last.frame?.x != it.frame.x || last.frame.width != it.frame.width) {
          res.push(it);
        }
      } else {
        res.push(it);
      }
    }
  }
}

export class HeapStruct extends BaseHeapStruct {
  static hoverHeapStruct: HeapStruct | undefined;
  static selectHeapStruct: HeapStruct | undefined;
  maxDensity: number = 0;
  minDensity: number = 0;

  static setFrame(node: HeapStruct, padding: number, startNS: number, endNS: number, totalNS: number, frame: Rect) {
    let x1: number, x2: number;
    if ((node.startTime || 0) < startNS) {
      x1 = 0;
    } else {
      x1 = ns2x(node.startTime || 0, startNS, endNS, totalNS, frame);
    }
    if ((node.startTime || 0) + (node.dur || 0) > endNS) {
      x2 = frame.width;
    } else {
      x2 = ns2x(
        // @ts-ignore
        node.startTime + node.dur,
        startNS,
        endNS,
        totalNS,
        frame
      );
    }
    let getV: number = x2 - x1 <= 1 ? 1 : x2 - x1;
    let rectangle: Rect = new Rect(
      Math.floor(x1),
      Math.ceil(frame.y + padding),
      Math.ceil(getV),
      Math.floor(frame.height - padding * 2)
    );
    node.frame = rectangle;
  }

  static drawHeap(heapContext: CanvasRenderingContext2D, data: HeapStruct, drawType: number) {
    if (data.frame) {
      let width = data.frame.width || 0;
      heapContext.fillStyle = '#2db3aa';
      heapContext.strokeStyle = '#2db3aa';
      let drawHeight: number = 0;
      if (drawType == 0) {
        if (data.minHeapSize < 0) {
          drawHeight = Math.ceil(
            (((data.heapsize || 0) - data.minHeapSize) * (data.frame.height || 0)) /
              (data.maxHeapSize - data.minHeapSize)
          );
        } else {
          drawHeight = Math.ceil(((data.heapsize || 0) * (data.frame.height || 0)) / data.maxHeapSize);
        }
      } else {
        if (data.minDensity < 0) {
          drawHeight = Math.ceil(
            (((data.density || 0) - data.minDensity) * (data.frame.height || 0)) / (data.maxDensity - data.minDensity)
          );
        } else {
          drawHeight = Math.ceil(((data.density || 0) * (data.frame.height || 0)) / data.maxDensity);
        }
      }
      if (data == HeapStruct.hoverHeapStruct || data == HeapStruct.selectHeapStruct) {
        heapContext.lineWidth = 1;
        heapContext.globalAlpha = 0.6;
        heapContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
        heapContext.beginPath();
        heapContext.arc(data.frame.x, data.frame.y + data.frame.height - drawHeight, 3, 0, 2 * Math.PI, true);
        heapContext.fill();
        heapContext.globalAlpha = 1.0;
        heapContext.stroke();
        heapContext.beginPath();
        heapContext.moveTo(data.frame.x + 3, data.frame.y + data.frame.height - drawHeight);
        heapContext.lineWidth = 3;
        heapContext.lineTo(data.frame.x + width, data.frame.y + data.frame.height - drawHeight);
        heapContext.stroke();
      } else {
        heapContext.globalAlpha = 0.6;
        heapContext.lineWidth = 1;
        heapContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
      }
    }
    heapContext.globalAlpha = 1.0;
    heapContext.lineWidth = 1;
  }
}
