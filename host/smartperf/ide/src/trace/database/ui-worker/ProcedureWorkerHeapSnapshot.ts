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

import { BaseStruct, Rect, Render, isFrameContainPoint } from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { Utils } from '../../component/trace/base/Utils.js';
export class HeapSnapshotRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
    },
    row: TraceRow<HeapSnapshotStruct>
  ) {
    let list = row.dataList;
    let filter = row.dataListCache;
    HeapSnapshot(
      list,
      filter,
      TraceRow.range?.startNS ?? 0,
      TraceRow.range?.endNS ?? 0,
      (TraceRow.range?.endNS ?? 0) - (TraceRow.range?.startNS! ?? 0),
      row.frame
    );
    req.context!.beginPath();
    for (let re of filter) {
      HeapSnapshotStruct.draw(req.context, re);
    }
    for (let re of filter) {
      if (re.frame && !isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        HeapSnapshotStruct.hoverSnapshotStruct = undefined;
      }
      if (re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        HeapSnapshotStruct.hoverSnapshotStruct = re;
        break;
      }
    }
    req.context!.closePath();
  }
}
export function HeapSnapshot(
  list: Array<HeapSnapshotStruct>,
  filter: Array<HeapSnapshotStruct>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: Rect
) {
  for (let file of list) {
    HeapSnapshotStruct.setFrame(file, startNS || 0, endNS || 0, totalNS || 0, frame);
  }
  filter.length = 0;
  for (let i = 0, len = list.length; i < len; i++) {
    if (list[i].frame) {
      filter.push(list[i]);
    }
  }
}
const padding = 3;
export class HeapSnapshotStruct extends BaseStruct {
  startTs: number = 0;
  endTs: number = 0;
  id: number = 0;
  pid: number = 0;
  name: string | undefined;
  textWidth: number | undefined;
  size: number = 0;
  static hoverSnapshotStruct: HeapSnapshotStruct | undefined;
  static selectSnapshotStruct: HeapSnapshotStruct | undefined;

  static setFrame(node: HeapSnapshotStruct, startNS: number, endNS: number, totalNS: number, frame: Rect) {
    node.frame = undefined;
    if ((node.startTs - startNS || node.startTs - startNS === 0) && node.endTs - node.startTs) {
      let rectangle: Rect = new Rect(
        Math.floor(((node.startTs - startNS) / totalNS) * frame.width),
        0,
        Math.ceil(((node.endTs - node.startTs) / totalNS) * frame.width),
        40
      );
      node.frame = rectangle;
    }
  }

  static draw(ctx: CanvasRenderingContext2D, data: HeapSnapshotStruct): void {
    if (data.frame) {
      ctx.fillStyle = 'rgb(86,192,197)';
      ctx!.fillRect(data.frame!.x, data.frame!.y + padding, data.frame!.width, data.frame!.height - padding * 2);
      if (data.frame!.width > 7) {
        ctx.globalAlpha = 1.0;
        ctx.lineWidth = 1;
        ctx.fillStyle = '#fff';
        ctx.textBaseline = 'middle';
        ctx.font = '12px sans-serif';
        HeapSnapshotStruct.drawString(ctx, data.name || '', 3, data.frame!, data, 4);
        HeapSnapshotStruct.drawString(ctx, Utils.getBinaryByteWithUnit(data.size) || '', 9, data.frame!, data, 2);
      }
      if (
        HeapSnapshotStruct.selectSnapshotStruct &&
        HeapSnapshotStruct.equals(HeapSnapshotStruct.selectSnapshotStruct, data)
      ) {
        ctx.strokeStyle = '#232c5d';
        ctx.lineWidth = 2;
        ctx.strokeRect(data.frame!.x, data.frame!.y + padding, data.frame!.width - 2, data.frame!.height - padding * 2);
      }
    }
  }

  /**
   *
   * @param ctx current context
   * @param str text
   * @param textPadding padding
   * @param frame rectangle
   * @param data HeapSnapshotStruct
   * @param location the position of the string, the bigger the numerical value, the higher the position on the canvas
   */
  static drawString(
    ctx: CanvasRenderingContext2D,
    str: string,
    textPadding: number,
    frame: Rect,
    data: HeapSnapshotStruct,
    location: number
  ) {
    if (data.textWidth === undefined) {
      data.textWidth = ctx.measureText(str).width;
    }
    let textWidth = Math.round(data.textWidth / str.length);
    let fillTextWidth = frame.width - textPadding * 2;
    if (data.textWidth < fillTextWidth) {
      let x = Math.floor(frame.width / 2 - data.textWidth / 2 + frame.x + textPadding);
      ctx.fillText(str, x, Math.floor(frame.y + frame.height / location + textPadding), fillTextWidth);
    } else {
      if (fillTextWidth >= textWidth) {
        let characterNum = fillTextWidth / textWidth;
        let x = frame.x + textPadding;
        if (characterNum < 2) {
          ctx.fillText(
            str.substring(0, 1),
            x,
            Math.floor(frame.y + frame.height / location + textPadding),
            fillTextWidth
          );
        } else {
          ctx.fillText(
            str.substring(0, characterNum - 1) + '...',
            x,
            Math.floor(frame.y + frame.height / location + textPadding),
            fillTextWidth
          );
        }
      }
    }
  }

  static equals(d1: HeapSnapshotStruct, d2: HeapSnapshotStruct): boolean {
    return (
      d1 &&
      d2 &&
      d1.name == d2.name &&
      d1.id == d2.id &&
      d1.pid == d2.pid &&
      d1.startTs == d2.startTs &&
      d1.endTs == d2.endTs
    );
  }
}
