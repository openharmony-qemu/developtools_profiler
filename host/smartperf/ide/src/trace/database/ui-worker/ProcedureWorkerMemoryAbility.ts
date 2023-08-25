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
  dataFilterHandler,
  drawFlagLine,
  drawLines,
  drawLoading,
  drawSelection,
  Render,
  RequestMessage,
  isFrameContainPoint,
  ns2x,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class MemoryAbilityRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      maxMemoryByte: number;
      maxMemoryByteName: string;
    },
    memoryAbilityRow: TraceRow<MemoryAbilityMonitorStruct>
  ): void {
    let memoryAbilityList = memoryAbilityRow.dataList;
    let memoryAbilityFilter = memoryAbilityRow.dataListCache;
    dataFilterHandler(memoryAbilityList, memoryAbilityFilter, {
      startKey: 'startNS',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: memoryAbilityRow.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    req.context.beginPath();
    let find = false;
    for (let re of memoryAbilityFilter) {
      MemoryAbilityMonitorStruct.draw(req.context, re, req.maxMemoryByte, memoryAbilityRow.isHover);
      if (memoryAbilityRow.isHover && re.frame &&
        isFrameContainPoint(re.frame, memoryAbilityRow.hoverX, memoryAbilityRow.hoverY)
      ) {
        MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = re;
        find = true;
      }
    }
    if (!find && memoryAbilityRow.isHover) {
      MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = undefined;
    }
    req.context.closePath();
    let textMetrics = req.context.measureText(req.maxMemoryByteName);
    req.context.globalAlpha = 0.8;
    req.context.fillStyle = '#f0f0f0';
    req.context.fillRect(0, 5, textMetrics.width + 8, 18);
    req.context.globalAlpha = 1;
    req.context.fillStyle = '#333';
    req.context.textBaseline = 'middle';
    req.context.fillText(req.maxMemoryByteName, 4, 5 + 9);
  }

  render(memoryAbilityRequest: RequestMessage, list: Array<any>, filter: Array<any>): void {
    if (memoryAbilityRequest.lazyRefresh) {
      memoryAbility(
        list,
        filter,
        memoryAbilityRequest.startNS,
        memoryAbilityRequest.endNS,
        memoryAbilityRequest.totalNS,
        memoryAbilityRequest.frame,
        memoryAbilityRequest.useCache || !memoryAbilityRequest.range.refresh
      );
    } else {
      if (!memoryAbilityRequest.useCache) {
        memoryAbility(
          list,
          filter,
          memoryAbilityRequest.startNS,
          memoryAbilityRequest.endNS,
          memoryAbilityRequest.totalNS,
          memoryAbilityRequest.frame,
          false
        );
      }
    }
    if (memoryAbilityRequest.canvas) {
      memoryAbilityRequest.context.clearRect(0, 0, memoryAbilityRequest.frame.width, memoryAbilityRequest.frame.height);
      let memoryAbilityArr = filter;
      if (
        memoryAbilityArr.length > 0 &&
        !memoryAbilityRequest.range.refresh &&
        !memoryAbilityRequest.useCache &&
        memoryAbilityRequest.lazyRefresh
      ) {
        drawLoading(
          memoryAbilityRequest.context,
          memoryAbilityRequest.startNS,
          memoryAbilityRequest.endNS,
          memoryAbilityRequest.totalNS,
          memoryAbilityRequest.frame,
          memoryAbilityArr[0].startNS,
          memoryAbilityArr[memoryAbilityArr.length - 1].startNS + memoryAbilityArr[memoryAbilityArr.length - 1].dur
        );
      }
      memoryAbilityRequest.context.beginPath();
      MemoryAbilityMonitorStruct.maxMemoryByte = memoryAbilityRequest.params.maxMemoryByte;
      MemoryAbilityMonitorStruct.maxMemoryByteName = memoryAbilityRequest.params.maxMemoryByteName;
      drawLines(
        memoryAbilityRequest.context,
        memoryAbilityRequest.xs,
        memoryAbilityRequest.frame.height,
        memoryAbilityRequest.lineColor
      );
      MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = undefined;
      if (memoryAbilityRequest.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            memoryAbilityRequest.hoverX >= re.frame.x &&
            memoryAbilityRequest.hoverX <= re.frame.x + re.frame.width &&
            memoryAbilityRequest.hoverY >= re.frame.y &&
            memoryAbilityRequest.hoverY <= re.frame.y + re.frame.height
          ) {
            MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct = re;
            break;
          }
        }
      }
      MemoryAbilityMonitorStruct.selectMemoryAbilityStruct = memoryAbilityRequest.params.selectMemoryAbilityStruct;
      for (let re of filter) {
        MemoryAbilityMonitorStruct.draw(
          memoryAbilityRequest.context,
          re,
          MemoryAbilityMonitorStruct.maxMemoryByte,
          true
        );
      }
      drawSelection(memoryAbilityRequest.context, memoryAbilityRequest.params);
      memoryAbilityRequest.context.closePath();
      let s = MemoryAbilityMonitorStruct.maxMemoryByteName;
      let textMetrics = memoryAbilityRequest.context.measureText(s);
      memoryAbilityRequest.context.globalAlpha = 0.8;
      memoryAbilityRequest.context.fillStyle = '#f0f0f0';
      memoryAbilityRequest.context.fillRect(0, 5, textMetrics.width + 8, 18);
      memoryAbilityRequest.context.globalAlpha = 1;
      memoryAbilityRequest.context.fillStyle = '#333';
      memoryAbilityRequest.context.textBaseline = 'middle';
      memoryAbilityRequest.context.fillText(s, 4, 5 + 9);
      drawFlagLine(
        memoryAbilityRequest.context,
        memoryAbilityRequest.flagMoveInfo,
        memoryAbilityRequest.flagSelectedInfo,
        memoryAbilityRequest.startNS,
        memoryAbilityRequest.endNS,
        memoryAbilityRequest.totalNS,
        memoryAbilityRequest.frame,
        memoryAbilityRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: memoryAbilityRequest.id,
      type: memoryAbilityRequest.type,
      results: memoryAbilityRequest.canvas ? undefined : filter,
      hover: MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct,
    });
  }
}

export function memoryAbility(
  memoryAbilityList: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    for (let i = 0; i < res.length; i++) {
      let memoryAbilityItem = res[i];
      if (
        (memoryAbilityItem.startNS || 0) + (memoryAbilityItem.dur || 0) > (startNS || 0) &&
        (memoryAbilityItem.startNS || 0) < (endNS || 0)
      ) {
        MemoryAbilityMonitorStruct.setMemoryFrame(memoryAbilityItem, 5, startNS || 0, endNS || 0, totalNS || 0, frame);
      } else {
        memoryAbilityItem.frame = null;
      }
    }
    return;
  }
  res.length = 0;
  if (memoryAbilityList) {
    for (let memoryAbilityIndex = 0; memoryAbilityIndex < memoryAbilityList.length; memoryAbilityIndex++) {
      let item = memoryAbilityList[memoryAbilityIndex];
      if (memoryAbilityIndex === memoryAbilityList.length - 1) {
        item.dur = (endNS || 0) - (item.startNS || 0);
      } else {
        item.dur = (memoryAbilityList[memoryAbilityIndex + 1].startNS || 0) - (item.startNS || 0);
      }
      if ((item.startNS || 0) + (item.dur || 0) > (startNS || 0) && (item.startNS || 0) < (endNS || 0)) {
        MemoryAbilityMonitorStruct.setMemoryFrame(
          memoryAbilityList[memoryAbilityIndex],
          5,
          startNS || 0,
          endNS || 0,
          totalNS || 0,
          frame
        );
        if (
          memoryAbilityIndex > 0 &&
          (memoryAbilityList[memoryAbilityIndex - 1].frame?.x || 0) ==
            (memoryAbilityList[memoryAbilityIndex].frame?.x || 0) &&
          (memoryAbilityList[memoryAbilityIndex - 1].frame?.width || 0) ==
            (memoryAbilityList[memoryAbilityIndex].frame?.width || 0)
        ) {
        } else {
          res.push(item);
        }
      }
    }
  }
}

export class MemoryAbilityMonitorStruct extends BaseStruct {
  static maxMemoryByte: number = 0;
  static maxMemoryByteName: string = '0 MB';
  static hoverMemoryAbilityStruct: MemoryAbilityMonitorStruct | undefined;
  static selectMemoryAbilityStruct: MemoryAbilityMonitorStruct | undefined;
  cpu: number | undefined;
  value: number | undefined;
  startNS: number | undefined;
  dur: number | undefined;

  static draw(
    memoryAbilityContext2D: CanvasRenderingContext2D,
    memoryAbilityData: MemoryAbilityMonitorStruct,
    maxMemoryByte: number,
    isHover: boolean
  ) {
    if (memoryAbilityData.frame) {
      let width = memoryAbilityData.frame.width || 0;
      let index = 2;
      memoryAbilityContext2D.fillStyle = ColorUtils.colorForTid(index);
      memoryAbilityContext2D.strokeStyle = ColorUtils.colorForTid(index);
      if (memoryAbilityData.startNS === MemoryAbilityMonitorStruct.hoverMemoryAbilityStruct?.startNS && isHover) {
        memoryAbilityContext2D.lineWidth = 1;
        memoryAbilityContext2D.globalAlpha = 0.6;
        let drawHeight: number = Math.floor(((memoryAbilityData.value || 0) * (memoryAbilityData.frame.height || 0) * 1.0) / maxMemoryByte);
        memoryAbilityContext2D.fillRect(
          memoryAbilityData.frame.x,
          memoryAbilityData.frame.y + memoryAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
        memoryAbilityContext2D.beginPath();
        memoryAbilityContext2D.arc(
          memoryAbilityData.frame.x,
          memoryAbilityData.frame.y + memoryAbilityData.frame.height - drawHeight + 4,
          3,
          0,
          2 * Math.PI,
          true
        );
        memoryAbilityContext2D.fill();
        memoryAbilityContext2D.globalAlpha = 1.0;
        memoryAbilityContext2D.stroke();
        memoryAbilityContext2D.beginPath();
        memoryAbilityContext2D.moveTo(memoryAbilityData.frame.x + 3, memoryAbilityData.frame.y + memoryAbilityData.frame.height - drawHeight + 4);
        memoryAbilityContext2D.lineWidth = 3;
        memoryAbilityContext2D.lineTo(memoryAbilityData.frame.x + width, memoryAbilityData.frame.y + memoryAbilityData.frame.height - drawHeight + 4);
        memoryAbilityContext2D.stroke();
      } else {
        memoryAbilityContext2D.globalAlpha = 0.6;
        memoryAbilityContext2D.lineWidth = 1;
        let drawHeight: number = Math.floor(((memoryAbilityData.value || 0) * (memoryAbilityData.frame.height || 0)) / maxMemoryByte);
        memoryAbilityContext2D.fillRect(
          memoryAbilityData.frame.x,
          memoryAbilityData.frame.y + memoryAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
      }
    }
    memoryAbilityContext2D.globalAlpha = 1.0;
    memoryAbilityContext2D.lineWidth = 1;
  }

  static setMemoryFrame(memoryNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
    let memoryStartPointX: number, memoryEndPointX: number;

    if ((memoryNode.startNS || 0) < startNS) {
      memoryStartPointX = 0;
    } else {
      memoryStartPointX = ns2x(memoryNode.startNS || 0, startNS, endNS, totalNS, frame);
    }
    if ((memoryNode.startNS || 0) + (memoryNode.dur || 0) > endNS) {
      memoryEndPointX = frame.width;
    } else {
      memoryEndPointX = ns2x((memoryNode.startNS || 0) + (memoryNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let frameWidth: number = memoryEndPointX - memoryStartPointX <= 1 ? 1 : memoryEndPointX - memoryStartPointX;
    if (!memoryNode.frame) {
      memoryNode.frame = {};
    }
    memoryNode.frame.x = Math.floor(memoryStartPointX);
    memoryNode.frame.y = frame.y + padding;
    memoryNode.frame.width = Math.ceil(frameWidth);
    memoryNode.frame.height = Math.floor(frame.height - padding * 2);
  }
}

const textPadding = 2;
