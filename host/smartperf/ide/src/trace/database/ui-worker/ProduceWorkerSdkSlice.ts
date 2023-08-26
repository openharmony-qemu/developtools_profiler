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
  drawFlagLine,
  drawLines,
  drawLoading,
  drawWakeUp,
  Render,
  RequestMessage,
  BaseStruct,
  isFrameContainPoint,
  ns2x,
  drawSelection,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { CounterStruct } from './ProduceWorkerSdkCounter.js';

export class SdkSliceRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      maxName: string;
      maxValue: number;
    },
    row: TraceRow<SdkSliceStruct>
  ): void {
    let sdkList = row.dataList;
    let sdkFilter = row.dataListCache;
    SdkSliceStruct.maxSdkSlice = req.maxValue;
    SdkSliceStruct.maxSdkSliceName = req.maxName;
    this.sdkSlice(
      sdkList,
      sdkFilter,
      TraceRow.range?.startNS ?? 0,
      TraceRow.range?.endNS ?? 0,
      TraceRow.range?.totalNS ?? 0,
      row.frame,
      req.useCache || (TraceRow.range?.refresh ?? false)
    );
    req.context.beginPath();
    let sdkSliceFind = false;
    for (let re of sdkFilter) {
      if (row.isHover && re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        SdkSliceStruct.hoverSdkSliceStruct = re;
        sdkSliceFind = true;
      }
      SdkSliceStruct.draw(req.context, re);
    }
    if (!sdkSliceFind && row.isHover) {
      SdkSliceStruct.hoverSdkSliceStruct = undefined;
    }
    req.context.closePath();
  }

  render(sdkSliceRequest: RequestMessage, sdkList: Array<any>, filter: Array<any>): void {
    if (sdkSliceRequest.lazyRefresh) {
      this.sdkSlice(
        sdkList,
        filter,
        sdkSliceRequest.startNS,
        sdkSliceRequest.endNS,
        sdkSliceRequest.totalNS,
        sdkSliceRequest.frame,
        sdkSliceRequest.useCache || !sdkSliceRequest.range.refresh
      );
    } else {
      if (!sdkSliceRequest.useCache) {
        this.sdkSlice(
          sdkList,
          filter,
          sdkSliceRequest.startNS,
          sdkSliceRequest.endNS,
          sdkSliceRequest.totalNS,
          sdkSliceRequest.frame,
          false
        );
      }
    }
    if (sdkSliceRequest.canvas) {
      sdkSliceRequest.context.clearRect(0, 0, sdkSliceRequest.canvas.width, sdkSliceRequest.canvas.height);
      let sdkSliceArr = filter;
      if (
        sdkSliceArr.length > 0 &&
        !sdkSliceRequest.range.refresh &&
        !sdkSliceRequest.useCache &&
        sdkSliceRequest.lazyRefresh
      ) {
        drawLoading(
          sdkSliceRequest.context,
          sdkSliceRequest.startNS,
          sdkSliceRequest.endNS,
          sdkSliceRequest.totalNS,
          sdkSliceRequest.frame,
          sdkSliceArr[0].startNS,
          sdkSliceArr[sdkSliceArr.length - 1].startNS + sdkSliceArr[sdkSliceArr.length - 1].dur
        );
      }
      sdkSliceRequest.context.beginPath();
      SdkSliceStruct.maxSdkSlice = sdkSliceRequest.params.maxSdkSlice;
      SdkSliceStruct.maxSdkSliceName = sdkSliceRequest.params.maxSdkSliceName;
      drawLines(sdkSliceRequest.context, sdkSliceRequest.xs, sdkSliceRequest.frame.height, sdkSliceRequest.lineColor);
      SdkSliceStruct.hoverSdkSliceStruct = undefined;
      if (sdkSliceRequest.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            sdkSliceRequest.hoverX >= re.frame.x &&
            sdkSliceRequest.hoverX <= re.frame.x + re.frame.width &&
            sdkSliceRequest.hoverY >= re.frame.y &&
            sdkSliceRequest.hoverY <= re.frame.y + re.frame.height
          ) {
            SdkSliceStruct.hoverSdkSliceStruct = re;
            break;
          }
        }
      }
      SdkSliceStruct.selectSdkSliceStruct = sdkSliceRequest.params.selectSdkSliceStruct;
      for (let re of filter) {
        SdkSliceStruct.draw(sdkSliceRequest.context, re);
      }
      drawSelection(sdkSliceRequest.context, sdkSliceRequest.params);
      sdkSliceRequest.context.closePath();
      sdkSliceRequest.context.globalAlpha = 0.8;
      sdkSliceRequest.context.fillStyle = '#f0f0f0';
      sdkSliceRequest.context.globalAlpha = 1;
      sdkSliceRequest.context.fillStyle = '#333';
      sdkSliceRequest.context.textBaseline = 'middle';
      drawFlagLine(
        sdkSliceRequest.context,
        sdkSliceRequest.flagMoveInfo,
        sdkSliceRequest.flagSelectedInfo,
        sdkSliceRequest.startNS,
        sdkSliceRequest.endNS,
        sdkSliceRequest.totalNS,
        sdkSliceRequest.frame,
        sdkSliceRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: sdkSliceRequest.id,
      type: sdkSliceRequest.type,
      results: sdkSliceRequest.canvas ? undefined : filter,
      hover: SdkSliceStruct.hoverSdkSliceStruct,
    });
  }

  sdkSlice(
    sdkList: Array<any>,
    sdkSliceFilters: Array<any>,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any,
    use: boolean
  ): void {
    if (use && sdkSliceFilters.length > 0) {
      for (let index = 0; index < sdkSliceFilters.length; index++) {
        let item = sdkSliceFilters[index];
        if ((item.end_ts || 0) > (startNS || 0) && (item.start_ts || 0) < (endNS || 0)) {
          SdkSliceStruct.setSdkSliceFrame(sdkSliceFilters[index], 5, startNS || 0, endNS || 0, totalNS || 0, frame);
        } else {
          sdkSliceFilters[index].frame = null;
        }
      }
      return;
    }
    sdkSliceFilters.length = 0;
    if (sdkList) {
      for (let index = 0; index < sdkList.length; index++) {
        let item = sdkList[index];
        if (item.start_ts >= startNS && item.end_ts == 0) {
          item.end_ts = endNS;
        }
        if ((item.end_ts || 0) > (startNS || 0) && (item.start_ts || 0) < (endNS || 0)) {
          SdkSliceStruct.setSdkSliceFrame(sdkList[index], 5, startNS || 0, endNS || 0, totalNS || 0, frame);
          if (
            index > 0 &&
            (sdkList[index - 1].frame?.x || 0) == (sdkList[index].frame?.x || 0) &&
            (sdkList[index - 1].frame?.width || 0) == (sdkList[index].frame?.width || 0)
          ) {
          } else {
            sdkSliceFilters.push(item);
          }
        }
      }
    }
  }
}

export class SdkSliceStruct extends BaseStruct {
  static maxSdkSlice: number = 0;
  static maxSdkSliceName: string = '';
  static hoverSdkSliceStruct: SdkSliceStruct | undefined;
  static selectSdkSliceStruct: SdkSliceStruct | undefined;

  start_ts: number | undefined;
  end_ts: number | undefined;

  value: number | undefined;
  slice_message: string | undefined;

  static draw(ctx: CanvasRenderingContext2D, data: SdkSliceStruct) {
    if (data.frame) {
      let width = data.frame.width || 0;
      let index = 4;
      ctx.fillStyle = '#6DC0DC';
      ctx.strokeStyle = '#6DC0DC';
      if (data.start_ts === SdkSliceStruct.hoverSdkSliceStruct?.start_ts) {
        ctx.lineWidth = 1;
        ctx.fillRect(data.frame.x, data.frame.y + 4, width, data.frame.height - 10);
        ctx.beginPath();
        ctx.arc(data.frame.x, data.frame.y + 4, 3, 0, 2 * Math.PI, true);
        ctx.fill();
        ctx.globalAlpha = 1.0;
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(data.frame.x + 3, data.frame.y + 4);
        ctx.lineWidth = 3;
        ctx.lineTo(data.frame.x + width, data.frame.y + 4);
        ctx.stroke();
      } else {
        ctx.lineWidth = 1;
        ctx.fillRect(data.frame.x, data.frame.y + 4, width, data.frame.height - 10);
      }
    }
  }

  static setSdkSliceFrame(
    SdkSliceNode: any,
    padding: number,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any
  ): void {
    let sdkSliceStartPointX: number, sdkSliceEndPointX: number;

    if ((SdkSliceNode.start_ts || 0) < startNS) {
      sdkSliceStartPointX = 0;
    } else {
      sdkSliceStartPointX = ns2x(SdkSliceNode.start_ts || 0, startNS, endNS, totalNS, frame);
    }
    if ((SdkSliceNode.end_ts || 0) > endNS) {
      sdkSliceEndPointX = frame.width;
    } else {
      sdkSliceEndPointX = ns2x(SdkSliceNode.end_ts || 0, startNS, endNS, totalNS, frame);
    }
    let frameWidth: number = sdkSliceEndPointX - sdkSliceStartPointX <= 1 ? 1 : sdkSliceEndPointX - sdkSliceStartPointX;
    if (!SdkSliceNode.frame) {
      SdkSliceNode.frame = {};
    }
    SdkSliceNode.frame.x = Math.floor(sdkSliceStartPointX);
    SdkSliceNode.frame.y = frame.y + padding;
    SdkSliceNode.frame.width = Math.ceil(frameWidth);
    SdkSliceNode.frame.height = Math.floor(frame.height - padding * 2);
  }
}
