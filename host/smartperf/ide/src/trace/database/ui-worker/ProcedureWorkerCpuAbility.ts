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
  isFrameContainPoint,
  ns2x,
  RequestMessage,
  Render,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { DiskAbilityMonitorStruct } from './ProcedureWorkerDiskIoAbility.js';

export class CpuAbilityRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      maxCpuUtilization: number;
      maxCpuUtilizationName: string;
    },
    cpuAbilityRow: TraceRow<CpuAbilityMonitorStruct>
  ): void {
    let cpuAbilityList = cpuAbilityRow.dataList;
    let cpuAbilityFilter = cpuAbilityRow.dataListCache;
    dataFilterHandler(cpuAbilityList, cpuAbilityFilter, {
      startKey: 'startNS',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: cpuAbilityRow.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    let find = false;
    req.context.beginPath();
    for (let re of cpuAbilityFilter) {
      CpuAbilityMonitorStruct.draw(req.context, re, req.maxCpuUtilization, cpuAbilityRow.isHover);
      if (cpuAbilityRow.isHover && re.frame &&
        isFrameContainPoint(re.frame, cpuAbilityRow.hoverX, cpuAbilityRow.hoverY)
      ) {
        CpuAbilityMonitorStruct.hoverCpuAbilityStruct = re;
        find = true;
      }
    }
    if (!find && cpuAbilityRow.isHover) {
      CpuAbilityMonitorStruct.hoverCpuAbilityStruct = undefined;
    }
    req.context.closePath();
    let textMetrics = req.context.measureText(req.maxCpuUtilizationName);
    req.context.globalAlpha = 0.8;
    req.context.fillStyle = '#f0f0f0';
    req.context.fillRect(0, 5, textMetrics.width + 8, 18);
    req.context.globalAlpha = 1;
    req.context.fillStyle = '#333';
    req.context.textBaseline = 'middle';
    req.context.fillText(req.maxCpuUtilizationName, 4, 5 + 9);
  }

  render(req: RequestMessage, list: Array<any>, filter: Array<any>) {}
}

export function cpuAbility(
  cpuAbilityList: Array<any>,
  cpuAbilityFilters: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && cpuAbilityFilters.length > 0) {
    for (let index = 0; index < cpuAbilityFilters.length; index++) {
      let item = cpuAbilityFilters[index];
      if ((item.startNS || 0) + (item.dur || 0) > (startNS || 0) && (item.startNS || 0) < (endNS || 0)) {
        CpuAbilityMonitorStruct.setCpuAbilityFrame(cpuAbilityFilters[index], 5, startNS || 0, endNS || 0, totalNS || 0, frame);
      } else {
        cpuAbilityFilters[index].frame = null;
      }
    }
    return;
  }
  cpuAbilityFilters.length = 0;
  if (cpuAbilityList) {
    for (let cpuAbilityIndex = 0; cpuAbilityIndex < cpuAbilityList.length; cpuAbilityIndex++) {
      let item = cpuAbilityList[cpuAbilityIndex];
      if (cpuAbilityIndex === cpuAbilityList.length - 1) {
        item.dur = (endNS || 0) - (item.startNS || 0);
      } else {
        item.dur = (cpuAbilityList[cpuAbilityIndex + 1].startNS || 0) - (item.startNS || 0);
      }
      if ((item.startNS || 0) + (item.dur || 0) > (startNS || 0) && (item.startNS || 0) < (endNS || 0)) {
        CpuAbilityMonitorStruct.setCpuAbilityFrame(
          cpuAbilityList[cpuAbilityIndex],
          5,
          startNS || 0,
          endNS || 0,
          totalNS || 0,
          frame
        );
        if (
          cpuAbilityIndex > 0 &&
          (cpuAbilityList[cpuAbilityIndex - 1].frame?.x || 0) == (cpuAbilityList[cpuAbilityIndex].frame?.x || 0) &&
          (cpuAbilityList[cpuAbilityIndex - 1].frame?.width || 0) == (cpuAbilityList[cpuAbilityIndex].frame?.width || 0)
        ) {
        } else {
          cpuAbilityFilters.push(item);
        }
      }
    }
  }
}

export class CpuAbilityMonitorStruct extends BaseStruct {
  static maxCpuUtilization: number = 0;
  static maxCpuUtilizationName: string = '0 %';
  static hoverCpuAbilityStruct: CpuAbilityMonitorStruct | undefined;
  static selectCpuAbilityStruct: CpuAbilityMonitorStruct | undefined;

  type: number | undefined;
  value: number | undefined;
  startNS: number | undefined;
  dur: number | undefined; //自补充，数据库没有返回

  static draw(
    cpuAbilityContext2D: CanvasRenderingContext2D,
    cpuAbilityData: CpuAbilityMonitorStruct,
    maxCpuUtilization: number,
    isHover: boolean
  ) {
    if (cpuAbilityData.frame) {
      let width = cpuAbilityData.frame.width || 0;
      let index = 2;
      cpuAbilityContext2D.fillStyle = ColorUtils.colorForTid(index);
      cpuAbilityContext2D.strokeStyle = ColorUtils.colorForTid(index);
      if (cpuAbilityData.startNS === CpuAbilityMonitorStruct.hoverCpuAbilityStruct?.startNS && isHover) {
        cpuAbilityContext2D.lineWidth = 1;
        cpuAbilityContext2D.globalAlpha = 0.6;
        let drawHeight: number = Math.floor(((cpuAbilityData.value || 0) * (cpuAbilityData.frame.height || 0) * 1.0) / maxCpuUtilization);
        cpuAbilityContext2D.fillRect(
          cpuAbilityData.frame.x,
          cpuAbilityData.frame.y + cpuAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
        cpuAbilityContext2D.beginPath();
        cpuAbilityContext2D.arc(
          cpuAbilityData.frame.x,
          cpuAbilityData.frame.y + cpuAbilityData.frame.height - drawHeight + 4,
          3,
          0,
          2 * Math.PI,
          true
        );
        cpuAbilityContext2D.fill();
        cpuAbilityContext2D.globalAlpha = 1.0;
        cpuAbilityContext2D.stroke();
        cpuAbilityContext2D.beginPath();
        cpuAbilityContext2D.moveTo(cpuAbilityData.frame.x + 3, cpuAbilityData.frame.y + cpuAbilityData.frame.height - drawHeight + 4);
        cpuAbilityContext2D.lineWidth = 3;
        cpuAbilityContext2D.lineTo(cpuAbilityData.frame.x + width, cpuAbilityData.frame.y + cpuAbilityData.frame.height - drawHeight + 4);
        cpuAbilityContext2D.stroke();
      } else {
        cpuAbilityContext2D.globalAlpha = 0.6;
        cpuAbilityContext2D.lineWidth = 1;
        let drawHeight: number = Math.floor(((cpuAbilityData.value || 0) * (cpuAbilityData.frame.height || 0)) / maxCpuUtilization);
        cpuAbilityContext2D.fillRect(
          cpuAbilityData.frame.x,
          cpuAbilityData.frame.y + cpuAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
      }
    }
    cpuAbilityContext2D.globalAlpha = 1.0;
    cpuAbilityContext2D.lineWidth = 1;
  }

  static setCpuAbilityFrame(
    cpuAbilityNode: any,
    padding: number,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: any
  ) {
    let cpuAbilityStartPointX: number, cpuAbilityEndPointX: number;

    if ((cpuAbilityNode.startNS || 0) < startNS) {
      cpuAbilityStartPointX = 0;
    } else {
      cpuAbilityStartPointX = ns2x(cpuAbilityNode.startNS || 0, startNS, endNS, totalNS, frame);
    }
    if ((cpuAbilityNode.startNS || 0) + (cpuAbilityNode.dur || 0) > endNS) {
      cpuAbilityEndPointX = frame.width;
    } else {
      cpuAbilityEndPointX = ns2x(
        (cpuAbilityNode.startNS || 0) + (cpuAbilityNode.dur || 0),
        startNS,
        endNS,
        totalNS,
        frame
      );
    }
    let frameWidth: number =
      cpuAbilityEndPointX - cpuAbilityStartPointX <= 1 ? 1 : cpuAbilityEndPointX - cpuAbilityStartPointX;
    if (!cpuAbilityNode.frame) {
      cpuAbilityNode.frame = {};
    }
    cpuAbilityNode.frame.x = Math.floor(cpuAbilityStartPointX);
    cpuAbilityNode.frame.y = frame.y + padding;
    cpuAbilityNode.frame.width = Math.ceil(frameWidth);
    cpuAbilityNode.frame.height = Math.floor(frame.height - padding * 2);
  }
}

export class CpuAbility {
  context: any;
  params: any;
}
