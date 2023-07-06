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
  drawSelection,
  isFrameContainPoint,
  ns2x,
  Render,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { CpuAbilityMonitorStruct } from './ProcedureWorkerCpuAbility.js';

export class SmapsRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      rowName: string;
      maxValue: number;
    },
    row: TraceRow<SmapsStruct>
  ) {
    let smapsList = row.dataList;
    let smapsFilter = row.dataListCache;
    dataFilterHandler(smapsList, smapsFilter, {
      startKey: 'startNS',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: row.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    req.context.beginPath();
    let drawColor = '#0A59F7';
    if (req.rowName != undefined) {
      switch (req.rowName) {
        case 'dirty':
          drawColor = '#0A59F7';
          break;
        case 'swapper':
          drawColor = '#46B1E3';
          break;
        case 'resident_size':
          drawColor = '#564AF7';
          break;
      }
    }
    let find = false;
    for (let re of smapsFilter) {
      SmapsStruct.draw(req.context, re, req.maxValue, drawColor, row.isHover);
      if (row.isHover && re.frame && isFrameContainPoint(re.frame, row.hoverX, row.hoverY)) {
        SmapsStruct.hoverSmapsStruct = re;
        find = true;
      }
    }
    if (!find && row.isHover) SmapsStruct.hoverSmapsStruct = undefined;

    req.context.closePath();
  }

  render(smapsReq: RequestMessage, list: Array<any>, filter: Array<any>) {
    if (smapsReq.lazyRefresh) {
      smaps(
        list,
        filter,
        smapsReq.startNS,
        smapsReq.endNS,
        smapsReq.totalNS,
        smapsReq.frame,
        smapsReq.useCache || !smapsReq.range.refresh
      );
    } else {
      if (!smapsReq.useCache) {
        smaps(list, filter, smapsReq.startNS, smapsReq.endNS, smapsReq.totalNS, smapsReq.frame, false);
      }
    }
    if (smapsReq.canvas) {
      smapsReq.context.clearRect(0, 0, smapsReq.frame.width, smapsReq.frame.height);
      smapsReq.context.beginPath();
      let maxValue = 0;
      let maxValueName = '';
      if (smapsReq.params.maxValue != undefined || smapsReq.params.maxValueName != undefined) {
        maxValue = smapsReq.params.maxValue;
        maxValueName = smapsReq.params.maxValueName;
      }
      drawLines(smapsReq.context, smapsReq.xs, smapsReq.frame.height, smapsReq.lineColor);
      SmapsStruct.hoverSmapsStruct = undefined;
      if (smapsReq.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            smapsReq.hoverX >= re.frame.x &&
            smapsReq.hoverX <= re.frame.x + re.frame.width &&
            smapsReq.hoverY >= re.frame.y &&
            smapsReq.hoverY <= re.frame.y + re.frame.height
          ) {
            SmapsStruct.hoverSmapsStruct = re;
            break;
          }
        }
      }
      let drawColor = '#0A59F7';
      if (smapsReq.params.rowName != undefined) {
        switch (smapsReq.params.rowName) {
          case 'dirty':
            drawColor = '#0A59F7';
            break;
          case 'swapper':
            drawColor = '#46B1E3';
            break;
          case 'resident_size':
            drawColor = '#564AF7';
            break;
        }
      }
      SmapsStruct.selectSmapsStruct = smapsReq.params.selectSmapsStruct;
      for (let re of filter) {
        SmapsStruct.draw(smapsReq.context, re, maxValue, drawColor, true);
      }
      drawSelection(smapsReq.context, smapsReq.params);
      smapsReq.context.closePath();
      drawFlagLine(
        smapsReq.context,
        smapsReq.flagMoveInfo,
        smapsReq.flagSelectedInfo,
        smapsReq.startNS,
        smapsReq.endNS,
        smapsReq.totalNS,
        smapsReq.frame,
        smapsReq.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: smapsReq.id,
      type: smapsReq.type,
      results: smapsReq.canvas ? undefined : filter,
      hover: SmapsStruct.hoverSmapsStruct,
    });
  }
}

export function smaps(
  smapsList: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    for (let i = 0; i < res.length; i++) {
      let smapsItem = res[i];
      if ((smapsItem.startNS || 0) + (smapsItem.dur || 0) > (startNS || 0) && (smapsItem.startNS || 0) < (endNS || 0)) {
        SmapsStruct.setSmapsFrame(smapsItem, 5, startNS || 0, endNS || 0, totalNS || 0, frame);
      } else {
        smapsItem.frame = null;
      }
    }
    return;
  }
  res.length = 0;
  if (smapsList) {
    for (let smapsIndex = 0; smapsIndex < smapsList.length; smapsIndex++) {
      let item = smapsList[smapsIndex];
      if (smapsIndex === smapsList.length - 1) {
        item.dur = (endNS || 0) - (item.startNS || 0);
      } else {
        item.dur = (smapsList[smapsIndex + 1].startNS || 0) - (item.startNS || 0);
      }
      if ((item.startNS || 0) + (item.dur || 0) > (startNS || 0) && (item.startNS || 0) < (endNS || 0)) {
        SmapsStruct.setSmapsFrame(smapsList[smapsIndex], 5, startNS || 0, endNS || 0, totalNS || 0, frame);
        if (
          smapsIndex > 0 &&
          (smapsList[smapsIndex - 1].frame?.x || 0) == (smapsList[smapsIndex].frame?.x || 0) &&
          (smapsList[smapsIndex - 1].frame?.width || 0) == (smapsList[smapsIndex].frame?.width || 0)
        ) {
        } else {
          res.push(item);
        }
      }
    }
  }
}

export class SmapsStruct extends BaseStruct {
  static maxValue: number = 0;
  static maxValueName: string = '0 KB/S';
  static hoverSmapsStruct: SmapsStruct | undefined;
  static selectSmapsStruct: SmapsStruct | undefined;
  value: number | undefined;
  startNS: number | undefined;
  dur: number | undefined;

  static draw(
    smapsContext: CanvasRenderingContext2D,
    data: SmapsStruct,
    maxValue: number,
    drawColor: string,
    isHover: boolean
  ) {
    if (data.frame) {
      let width = data.frame.width || 0;
      smapsContext.fillStyle = drawColor;
      smapsContext.strokeStyle = drawColor;
      if (data.startNS === SmapsStruct.hoverSmapsStruct?.startNS && isHover) {
        smapsContext.lineWidth = 1;
        let smapsDrawHeight: number = Math.floor(((data.value || 0) * (data.frame.height || 0) * 1.0) / maxValue);
        smapsContext.fillRect(
          data.frame.x,
          data.frame.y + data.frame.height - smapsDrawHeight + 4,
          width,
          smapsDrawHeight
        );
        smapsContext.beginPath();
        smapsContext.arc(data.frame.x, data.frame.y + data.frame.height - smapsDrawHeight + 4, 3, 0, 2 * Math.PI, true);
        smapsContext.fill();
        smapsContext.globalAlpha = 1.0;
        smapsContext.stroke();
        smapsContext.beginPath();
        smapsContext.moveTo(data.frame.x + 3, data.frame.y + data.frame.height - smapsDrawHeight + 4);
        smapsContext.lineWidth = 3;
        smapsContext.lineTo(data.frame.x + width, data.frame.y + data.frame.height - smapsDrawHeight + 4);
        smapsContext.stroke();
      } else {
        smapsContext.lineWidth = 1;
        let smapsDrawHeight: number = Math.floor(((data.value || 0) * (data.frame.height || 0)) / maxValue);
        smapsContext.fillRect(
          data.frame.x,
          data.frame.y + data.frame.height - smapsDrawHeight + 4,
          width,
          smapsDrawHeight
        );
      }
    }
    smapsContext.globalAlpha = 1.0;
    smapsContext.lineWidth = 1;
  }

  static setSmapsFrame(smapsNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
    let smapsStartPointX: number, smapsEndPointX: number;

    if ((smapsNode.startNS || 0) < startNS) {
      smapsStartPointX = 0;
    } else {
      smapsStartPointX = ns2x(smapsNode.startNS || 0, startNS, endNS, totalNS, frame);
    }
    if ((smapsNode.startNS || 0) + (smapsNode.dur || 0) > endNS) {
      smapsEndPointX = frame.width;
    } else {
      smapsEndPointX = ns2x((smapsNode.startNS || 0) + (smapsNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let frameWidth: number = smapsEndPointX - smapsStartPointX <= 1 ? 1 : smapsEndPointX - smapsStartPointX;
    if (!smapsNode.frame) {
      smapsNode.frame = {};
    }
    smapsNode.frame.x = Math.floor(smapsStartPointX);
    smapsNode.frame.y = frame.y + padding;
    smapsNode.frame.width = Math.ceil(frameWidth);
    smapsNode.frame.height = Math.floor(frame.height - padding * 2);
  }
}
