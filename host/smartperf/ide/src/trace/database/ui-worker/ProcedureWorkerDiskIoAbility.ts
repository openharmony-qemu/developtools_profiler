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
  drawLines,
  drawLoading,
  drawSelection,
  isFrameContainPoint,
  ns2x,
  Render,
  drawFlagLine,
  RequestMessage,
  dataFilterHandler,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class DiskIoAbilityRender extends Render {
  renderMainThread(
    req: {
      context: CanvasRenderingContext2D;
      useCache: boolean;
      type: string;
      maxDiskRate: number;
      maxDiskRateName: string;
    },
    diskIoAbilityRow: TraceRow<DiskAbilityMonitorStruct>
  ): void {
    let diskIoAbilityList = diskIoAbilityRow.dataList;
    let diskIoFilter = diskIoAbilityRow.dataListCache;
    dataFilterHandler(diskIoAbilityList, diskIoFilter, {
      startKey: 'startNS',
      durKey: 'dur',
      startNS: TraceRow.range?.startNS ?? 0,
      endNS: TraceRow.range?.endNS ?? 0,
      totalNS: TraceRow.range?.totalNS ?? 0,
      frame: diskIoAbilityRow.frame,
      paddingTop: 5,
      useCache: req.useCache || !(TraceRow.range?.refresh ?? false),
    });
    req.context.beginPath();
    let find = false;
    for (let re of diskIoFilter) {
      DiskAbilityMonitorStruct.draw(req.context, re, req.maxDiskRate, diskIoAbilityRow.isHover);
      if (diskIoAbilityRow.isHover && re.frame &&
        isFrameContainPoint(re.frame, diskIoAbilityRow.hoverX, diskIoAbilityRow.hoverY)
      ) {
        DiskAbilityMonitorStruct.hoverDiskAbilityStruct = re;
        find = true;
      }
    }
    if (!find && diskIoAbilityRow.isHover) {
      DiskAbilityMonitorStruct.hoverDiskAbilityStruct = undefined;
    }
    req.context.closePath();
    let textMetrics = req.context.measureText(req.maxDiskRateName);
    req.context.globalAlpha = 0.8;
    req.context.fillStyle = '#f0f0f0';
    req.context.fillRect(0, 5, textMetrics.width + 8, 18);
    req.context.globalAlpha = 1;
    req.context.fillStyle = '#333';
    req.context.textBaseline = 'middle';
    req.context.fillText(req.maxDiskRateName, 4, 5 + 9);
  }

  render(diskAbilityRequest: RequestMessage, diskIoAbilityList: Array<any>, filter: Array<any>) {
    if (diskAbilityRequest.lazyRefresh) {
      diskIoAbility(
        diskIoAbilityList,
        filter,
        diskAbilityRequest.startNS,
        diskAbilityRequest.endNS,
        diskAbilityRequest.totalNS,
        diskAbilityRequest.frame,
        diskAbilityRequest.useCache || !diskAbilityRequest.range.refresh
      );
    } else {
      if (!diskAbilityRequest.useCache) {
        diskIoAbility(
          diskIoAbilityList,
          filter,
          diskAbilityRequest.startNS,
          diskAbilityRequest.endNS,
          diskAbilityRequest.totalNS,
          diskAbilityRequest.frame,
          false
        );
      }
    }
    if (diskAbilityRequest.canvas) {
      diskAbilityRequest.context.clearRect(0, 0, diskAbilityRequest.frame.width, diskAbilityRequest.frame.height);
      let diskIoArr = filter;
      if (
        diskIoArr.length > 0 &&
        !diskAbilityRequest.range.refresh &&
        !diskAbilityRequest.useCache &&
        diskAbilityRequest.lazyRefresh
      ) {
        drawLoading(
          diskAbilityRequest.context,
          diskAbilityRequest.startNS,
          diskAbilityRequest.endNS,
          diskAbilityRequest.totalNS,
          diskAbilityRequest.frame,
          diskIoArr[0].startNS,
          diskIoArr[diskIoArr.length - 1].startNS + diskIoArr[diskIoArr.length - 1].dur
        );
      }
      diskAbilityRequest.context.beginPath();
      let maxDiskRate = diskAbilityRequest.params.maxDiskRate;
      let maxDiskRateName = diskAbilityRequest.params.maxDiskRateName;
      drawLines(
        diskAbilityRequest.context,
        diskAbilityRequest.xs,
        diskAbilityRequest.frame.height,
        diskAbilityRequest.lineColor
      );
      DiskAbilityMonitorStruct.hoverDiskAbilityStruct = undefined;
      if (diskAbilityRequest.isHover) {
        for (let re of filter) {
          if (
            re.frame &&
            diskAbilityRequest.hoverX >= re.frame.x &&
            diskAbilityRequest.hoverX <= re.frame.x + re.frame.width &&
            diskAbilityRequest.hoverY >= re.frame.y &&
            diskAbilityRequest.hoverY <= re.frame.y + re.frame.height
          ) {
            DiskAbilityMonitorStruct.hoverDiskAbilityStruct = re;
            break;
          }
        }
      }
      DiskAbilityMonitorStruct.selectDiskAbilityStruct = diskAbilityRequest.params.selectDiskAbilityStruct;
      for (let re of filter) {
        DiskAbilityMonitorStruct.draw(diskAbilityRequest.context, re, maxDiskRate, true);
      }
      drawSelection(diskAbilityRequest.context, diskAbilityRequest.params);
      diskAbilityRequest.context.closePath();
      let textMetrics = diskAbilityRequest.context.measureText(maxDiskRateName);
      diskAbilityRequest.context.globalAlpha = 0.8;
      diskAbilityRequest.context.fillStyle = '#f0f0f0';
      diskAbilityRequest.context.fillRect(0, 5, textMetrics.width + 8, 18);
      diskAbilityRequest.context.globalAlpha = 1;
      diskAbilityRequest.context.fillStyle = '#333';
      diskAbilityRequest.context.textBaseline = 'middle';
      diskAbilityRequest.context.fillText(maxDiskRateName, 4, 5 + 9);
      drawFlagLine(
        diskAbilityRequest.context,
        diskAbilityRequest.flagMoveInfo,
        diskAbilityRequest.flagSelectedInfo,
        diskAbilityRequest.startNS,
        diskAbilityRequest.endNS,
        diskAbilityRequest.totalNS,
        diskAbilityRequest.frame,
        diskAbilityRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: diskAbilityRequest.id,
      type: diskAbilityRequest.type,
      results: diskAbilityRequest.canvas ? undefined : filter,
      hover: DiskAbilityMonitorStruct.hoverDiskAbilityStruct,
    });
  }
}

export function diskIoAbility(
  diskIoAbilityList: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    for (let i = 0; i < res.length; i++) {
      let diskIoAbilityItem = res[i];
      if (
        (diskIoAbilityItem.startNS || 0) + (diskIoAbilityItem.dur || 0) > (startNS || 0) &&
        (diskIoAbilityItem.startNS || 0) < (endNS || 0)
      ) {
        DiskAbilityMonitorStruct.setDiskIOFrame(diskIoAbilityItem, 5, startNS || 0, endNS || 0, totalNS || 0, frame);
      } else {
        diskIoAbilityItem.frame = null;
      }
    }
    return;
  }
  res.length = 0;
  if (diskIoAbilityList) {
    for (let index = 0; index < diskIoAbilityList.length; index++) {
      let item = diskIoAbilityList[index];
      if (index === diskIoAbilityList.length - 1) {
        item.dur = (endNS || 0) - (item.startNS || 0);
      } else {
        item.dur = (diskIoAbilityList[index + 1].startNS || 0) - (item.startNS || 0);
      }
      if ((item.startNS || 0) + (item.dur || 0) > (startNS || 0) && (item.startNS || 0) < (endNS || 0)) {
        DiskAbilityMonitorStruct.setDiskIOFrame(
          diskIoAbilityList[index],
          5,
          startNS || 0,
          endNS || 0,
          totalNS || 0,
          frame
        );
        if (
          index > 0 &&
          (diskIoAbilityList[index - 1].frame?.x || 0) == (diskIoAbilityList[index].frame?.x || 0) &&
          (diskIoAbilityList[index - 1].frame?.width || 0) == (diskIoAbilityList[index].frame?.width || 0)
        ) {
        } else {
          res.push(item);
        }
      }
    }
  }
}

export class DiskAbilityMonitorStruct extends BaseStruct {
  static maxDiskRate: number = 0;
  static maxDiskRateName: string = '0 KB/S';
  static hoverDiskAbilityStruct: DiskAbilityMonitorStruct | undefined;
  static selectDiskAbilityStruct: DiskAbilityMonitorStruct | undefined;
  value: number | undefined;
  startNS: number | undefined;
  dur: number | undefined; //自补充，数据库没有返回

  static draw(
    diskIoAbilityContext: CanvasRenderingContext2D,
    diskIoAbilityData: DiskAbilityMonitorStruct,
    maxDiskRate: number,
    isHover: boolean
  ) {
    if (diskIoAbilityData.frame) {
      let width = diskIoAbilityData.frame.width || 0;
      let index = 2;
      diskIoAbilityContext.fillStyle = ColorUtils.colorForTid(index);
      diskIoAbilityContext.strokeStyle = ColorUtils.colorForTid(index);
      if (diskIoAbilityData.startNS === DiskAbilityMonitorStruct.hoverDiskAbilityStruct?.startNS && isHover) {
        diskIoAbilityContext.lineWidth = 1;
        diskIoAbilityContext.globalAlpha = 0.6;
        let drawHeight: number = Math.floor(((diskIoAbilityData.value || 0) * (diskIoAbilityData.frame.height || 0) * 1.0) / maxDiskRate);
        diskIoAbilityContext.fillRect(
          diskIoAbilityData.frame.x,
          diskIoAbilityData.frame.y + diskIoAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
        diskIoAbilityContext.beginPath();
        diskIoAbilityContext.arc(
          diskIoAbilityData.frame.x,
          diskIoAbilityData.frame.y + diskIoAbilityData.frame.height - drawHeight + 4,
          3,
          0,
          2 * Math.PI,
          true
        );
        diskIoAbilityContext.fill();
        diskIoAbilityContext.globalAlpha = 1.0;
        diskIoAbilityContext.stroke();
        diskIoAbilityContext.beginPath();
        diskIoAbilityContext.moveTo(diskIoAbilityData.frame.x + 3, diskIoAbilityData.frame.y + diskIoAbilityData.frame.height - drawHeight + 4);
        diskIoAbilityContext.lineWidth = 3;
        diskIoAbilityContext.lineTo(diskIoAbilityData.frame.x + width, diskIoAbilityData.frame.y + diskIoAbilityData.frame.height - drawHeight + 4);
        diskIoAbilityContext.stroke();
      } else {
        diskIoAbilityContext.globalAlpha = 0.6;
        diskIoAbilityContext.lineWidth = 1;
        let drawHeight: number = Math.floor(((diskIoAbilityData.value || 0) * (diskIoAbilityData.frame.height || 0)) / maxDiskRate);
        diskIoAbilityContext.fillRect(
          diskIoAbilityData.frame.x,
          diskIoAbilityData.frame.y + diskIoAbilityData.frame.height - drawHeight + 4,
          width,
          drawHeight
        );
      }
    }
    diskIoAbilityContext.globalAlpha = 1.0;
    diskIoAbilityContext.lineWidth = 1;
  }

  static setDiskIOFrame(diskIONode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
    let diskIOStartPointX: number, diskIOEndPointX: number;

    if ((diskIONode.startNS || 0) < startNS) {
      diskIOStartPointX = 0;
    } else {
      diskIOStartPointX = ns2x(diskIONode.startNS || 0, startNS, endNS, totalNS, frame);
    }
    if ((diskIONode.startNS || 0) + (diskIONode.dur || 0) > endNS) {
      diskIOEndPointX = frame.width;
    } else {
      diskIOEndPointX = ns2x((diskIONode.startNS || 0) + (diskIONode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let frameWidth: number = diskIOEndPointX - diskIOStartPointX <= 1 ? 1 : diskIOEndPointX - diskIOStartPointX;
    if (!diskIONode.frame) {
      diskIONode.frame = {};
    }
    diskIONode.frame.x = Math.floor(diskIOStartPointX);
    diskIONode.frame.y = frame.y + padding;
    diskIONode.frame.width = Math.ceil(frameWidth);
    diskIONode.frame.height = Math.floor(frame.height - padding * 2);
  }
}
