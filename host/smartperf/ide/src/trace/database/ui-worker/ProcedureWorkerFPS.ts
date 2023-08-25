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
  drawFlagLine,
  drawLoading,
  drawSelection,
  isFrameContainPoint,
  ns2x,
  drawLines,
  Rect,
  Render,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class FpsRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
    },
    row: TraceRow<FpsStruct>
  ) {
    let fpsList = row.dataList;
    let fpsFilter = row.dataListCache;
    fps(
      fpsList,
      fpsFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let fpsFind = false;
    for (let re of fpsFilter) {
      FpsStruct.draw(req.context, re);
      if (row.isHover && re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        FpsStruct.hoverFpsStruct = re;
        fpsFind = true;
      }
    }
    if (!fpsFind && row.isHover) FpsStruct.hoverFpsStruct = undefined;
    req.context.closePath();
    let maxFps = FpsStruct.maxFps + 'FPS';
    let textMetrics = req.context.measureText(maxFps);
    req.context.globalAlpha = 0.8;
    req.context.fillStyle = '#f0f0f0';
    req.context.fillRect(0, 5, textMetrics.width + 8, 18);
    req.context.globalAlpha = 1;
    req.context.fillStyle = '#333';
    req.context.textBaseline = 'middle';
    req.context.fillText(maxFps, 4, 5 + 9);
  }

  render(fpsRequest: RequestMessage, list: Array<any>, filter: Array<any>) {
    if (fpsRequest.lazyRefresh) {
      fps(
        list,
        filter,
        fpsRequest.startNS,
        fpsRequest.endNS,
        fpsRequest.totalNS,
        fpsRequest.frame,
        fpsRequest.useCache || !fpsRequest.range.refresh
      );
    } else {
      if (!fpsRequest.useCache) {
        fps(list, filter, fpsRequest.startNS, fpsRequest.endNS, fpsRequest.totalNS, fpsRequest.frame, false);
      }
    }
    if (fpsRequest.canvas) {
      fpsRequest.context.clearRect(0, 0, fpsRequest.frame.width, fpsRequest.frame.height);
      let fpsArr = filter;
      if (fpsArr.length > 0 && !fpsRequest.range.refresh && !fpsRequest.useCache && fpsRequest.lazyRefresh) {
        drawLoading(
          fpsRequest.context,
          fpsRequest.startNS,
          fpsRequest.endNS,
          fpsRequest.totalNS,
          fpsRequest.frame,
          fpsArr[0].startNS,
          fpsArr[fpsArr.length - 1].startNS + fpsArr[fpsArr.length - 1].dur
        );
      }
      fpsRequest.context.beginPath();
      drawLines(fpsRequest.context, fpsRequest.xs, fpsRequest.frame.height, fpsRequest.lineColor);
      FpsStruct.hoverFpsStruct = undefined;
      if (fpsRequest.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            fpsRequest.hoverX >= re.frame.x &&
            fpsRequest.hoverX <= re.frame.x + re.frame.width &&
            fpsRequest.hoverY >= re.frame.y &&
            fpsRequest.hoverY <= re.frame.y + re.frame.height
          ) {
            FpsStruct.hoverFpsStruct = re;
            break;
          }
        }
      } else {
        FpsStruct.hoverFpsStruct = fpsRequest.params.hoverFpsStruct;
      }
      for (let re of filter) {
        FpsStruct.draw(fpsRequest.context, re);
      }
      drawSelection(fpsRequest.context, fpsRequest.params);
      fpsRequest.context.closePath();
      let maxFps = FpsStruct.maxFps + 'FPS';
      let fpsTextMetrics = fpsRequest.context.measureText(maxFps);
      fpsRequest.context.globalAlpha = 0.8;
      fpsRequest.context.fillStyle = '#f0f0f0';
      fpsRequest.context.fillRect(0, 5, fpsTextMetrics.width + 8, 18);
      fpsRequest.context.globalAlpha = 1;
      fpsRequest.context.fillStyle = '#333';
      fpsRequest.context.textBaseline = 'middle';
      fpsRequest.context.fillText(maxFps, 4, 5 + 9);
      drawFlagLine(
        fpsRequest.context,
        fpsRequest.flagMoveInfo,
        fpsRequest.flagSelectedInfo,
        fpsRequest.startNS,
        fpsRequest.endNS,
        fpsRequest.totalNS,
        fpsRequest.frame,
        fpsRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: fpsRequest.id,
      type: fpsRequest.type,
      results: fpsRequest.canvas ? undefined : filter,
      hover: FpsStruct.hoverFpsStruct,
    });
  }
}

export function fps(
  list: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    res.forEach((it) => FpsStruct.setFrame(it, 5, startNS, endNS, totalNS, frame));
    return;
  }
  FpsStruct.maxFps = 0;
  res.length = 0;
  if (list) {
    for (let i = 0, len = list.length; i < len; i++) {
      let it = list[i];
      if ((it.fps || 0) > FpsStruct.maxFps) {
        FpsStruct.maxFps = it.fps || 0;
      }
      if (i === list.length - 1) {
        it.dur = (endNS || 0) - (it.startNS || 0);
      } else {
        it.dur = (list[i + 1].startNS || 0) - (it.startNS || 0);
      }
      if ((it.startNS || 0) + (it.dur || 0) > startNS && (it.startNS || 0) < endNS) {
        FpsStruct.setFrame(list[i], 5, startNS, endNS, totalNS, frame);
        if (
          i > 0 &&
          (list[i - 1].frame?.x || 0) == (list[i].frame?.x || 0) &&
          (list[i - 1].frame?.width || 0) == (list[i].frame?.width || 0)
        ) {
        } else {
          res.push(list[i]);
        }
      }
    }
  }
}

export class FpsStruct extends BaseStruct {
  static maxFps: number = 0;
  static maxFpsName: string = '0 FPS';
  static hoverFpsStruct: FpsStruct | undefined;
  static selectFpsStruct: FpsStruct | undefined;
  fps: number | undefined;
  startNS: number | undefined = 0;
  dur: number | undefined; //自补充，数据库没有返回

  static draw(fpsContext: CanvasRenderingContext2D, data: FpsStruct) {
    if (data.frame) {
      let width = data.frame.width || 0;
      fpsContext.fillStyle = '#535da6';
      fpsContext.strokeStyle = '#535da6';
      if (data === FpsStruct.hoverFpsStruct) {
        fpsContext.lineWidth = 1;
        fpsContext.globalAlpha = 0.6;
        let drawHeight: number = ((data.fps || 0) * (data.frame.height || 0) * 1.0) / FpsStruct.maxFps;
        fpsContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
        fpsContext.beginPath();
        fpsContext.arc(data.frame.x, data.frame.y + data.frame.height - drawHeight, 3, 0, 2 * Math.PI, true);
        fpsContext.fill();
        fpsContext.globalAlpha = 1.0;
        fpsContext.stroke();
        fpsContext.beginPath();
        fpsContext.moveTo(data.frame.x + 3, data.frame.y + data.frame.height - drawHeight);
        fpsContext.lineWidth = 3;
        fpsContext.lineTo(data.frame.x + width, data.frame.y + data.frame.height - drawHeight);
        fpsContext.stroke();
      } else {
        fpsContext.globalAlpha = 0.6;
        fpsContext.lineWidth = 1;
        let drawHeight: number = ((data.fps || 0) * (data.frame.height || 0) * 1.0) / FpsStruct.maxFps;
        fpsContext.fillRect(data.frame.x, data.frame.y + data.frame.height - drawHeight, width, drawHeight);
      }
    }
    fpsContext.globalAlpha = 1.0;
    fpsContext.lineWidth = 1;
  }

  static setFrame(fpsNode: FpsStruct, padding: number, startNS: number, endNS: number, totalNS: number, frame: Rect) {
    let fpsLeftPointX: number, fpsRightPointX: number;
    if ((fpsNode.startNS || 0) < startNS) {
      fpsLeftPointX = 0;
    } else {
      fpsLeftPointX = ns2x(fpsNode.startNS || 0, startNS, endNS, totalNS, frame);
    }
    if ((fpsNode.startNS || 0) + (fpsNode.dur || 0) > endNS) {
      fpsRightPointX = frame.width;
    } else {
      fpsRightPointX = ns2x((fpsNode.startNS || 0) + (fpsNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let getV: number = fpsRightPointX - fpsLeftPointX <= 1 ? 1 : fpsRightPointX - fpsLeftPointX;
    let rectangle: Rect = new Rect(
      Math.floor(fpsLeftPointX),
      Math.ceil(frame.y + padding),
      Math.ceil(getV),
      Math.floor(frame.height - padding * 2)
    );
    fpsNode.frame = rectangle;
  }
}

const textPadding = 2;
