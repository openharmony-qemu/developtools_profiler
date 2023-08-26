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
  drawLines,
  drawLoading,
  drawSelection,
  isFrameContainPoint,
  ns2x,
  Render,
  RequestMessage,
} from './ProcedureWorkerCommon.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';

export class EnergySystemRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<EnergySystemStruct>
  ) {
    let systemList = row.dataList;
    let systemFilter = row.dataListCache;
    system(
      systemList,
      systemFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let find = false;
    let energySystemData: any = {};
    for (let i = 0; i < systemFilter.length; i++) {
      let energySysStruct = systemFilter[i];

      EnergySystemStruct.draw(req.context, energySysStruct);
      if (row.isHover && energySysStruct.frame && isFrameContainPoint(energySysStruct.frame, row.hoverX, row.hoverY)) {
        EnergySystemStruct.hoverEnergySystemStruct = energySysStruct;
        if (energySysStruct.type == 0) {
          if (energySysStruct.count != undefined) {
            energySystemData.workScheduler = energySysStruct.count;
          } else {
            energySystemData.workScheduler = '0';
          }
        }
        if (energySysStruct.type == 1) {
          if (energySysStruct.count != undefined) {
            energySystemData.power = energySysStruct.count + '';
          } else {
            energySystemData.power = '0';
          }
        }

        if (energySysStruct.type == 2) {
          if (energySysStruct.count != undefined) {
            energySystemData.location = energySysStruct.count + '';
          } else {
            energySystemData.location = '0';
          }
        }
        find = true;
      }
    }
    if (!find && row.isHover) EnergySystemStruct.hoverEnergySystemStruct = undefined;
    if (EnergySystemStruct.hoverEnergySystemStruct) {
      EnergySystemStruct.hoverEnergySystemStruct!.workScheduler = energySystemData.workScheduler == undefined ? '0' : energySystemData.workScheduler;
      EnergySystemStruct.hoverEnergySystemStruct!.power = energySystemData.power == undefined ? '0' : energySystemData.power;
      EnergySystemStruct.hoverEnergySystemStruct!.location = energySystemData.location == undefined ? '0' : energySystemData.location;
    }
    let spApplication = document.getElementsByTagName('sp-application')[0];
    let isDark = spApplication.hasAttribute('dark');
    drawLegend(req, isDark);
    req.context.closePath();
  }

  render(energySysRequest: RequestMessage, list: Array<any>, filter: Array<any>) {
    if (energySysRequest.lazyRefresh) {
      system(
        list,
        filter,
        energySysRequest.startNS,
        energySysRequest.endNS,
        energySysRequest.totalNS,
        energySysRequest.frame,
        energySysRequest.useCache || !energySysRequest.range.refresh
      );
    } else {
      if (!energySysRequest.useCache) {
        system(
          list,
          filter,
          energySysRequest.startNS,
          energySysRequest.endNS,
          energySysRequest.totalNS,
          energySysRequest.frame,
          false
        );
      }
    }
    if (energySysRequest.canvas) {
      energySysRequest.context.clearRect(0, 0, energySysRequest.canvas.width, energySysRequest.canvas.height);
      let energySystemArr = filter;
      if (
        energySystemArr.length > 0 &&
        !energySysRequest.range.refresh &&
        !energySysRequest.useCache &&
        energySysRequest.lazyRefresh
      ) {
        drawLoading(
          energySysRequest.context,
          energySysRequest.startNS,
          energySysRequest.endNS,
          energySysRequest.totalNS,
          energySysRequest.frame,
          energySystemArr[0].startNS,
          energySystemArr[energySystemArr.length - 1].startNS + energySystemArr[energySystemArr.length - 1].dur
        );
      }
      drawLines(
        energySysRequest.context,
        energySysRequest.xs,
        energySysRequest.frame.height,
        energySysRequest.lineColor
      );
      energySysRequest.context.beginPath();
      EnergySystemStruct.hoverEnergySystemStruct = undefined;
      if (energySysRequest.isHover) {
        let a: any = {};
        for (let filterElement of filter) {
          if (
            filterElement.frame &&
            energySysRequest.hoverX >= filterElement.frame.x &&
            energySysRequest.hoverX <= filterElement.frame.x + filterElement.frame.width
          ) {
            EnergySystemStruct.hoverEnergySystemStruct = filterElement;
            if (filterElement.type == 0) {
              if (filterElement.count != undefined) {
                a.workScheduler = filterElement.count;
              } else {
                a.workScheduler = '0';
              }
            }
            if (filterElement.type == 1) {
              if (filterElement.count != undefined) {
                a.power = filterElement.count + '';
              } else {
                a.power = '0';
              }
            }

            if (filterElement.type == 2) {
              if (filterElement.count != undefined) {
                a.location = filterElement.count + '';
              } else {
                a.location = '0';
              }
            }
          }
        }
        if (EnergySystemStruct.hoverEnergySystemStruct) {
          EnergySystemStruct.hoverEnergySystemStruct!.workScheduler =
            a.workScheduler == undefined ? '0' : a.workScheduler;
          EnergySystemStruct.hoverEnergySystemStruct!.power = a.power == undefined ? '0' : a.power;
          EnergySystemStruct.hoverEnergySystemStruct!.location = a.location == undefined ? '0' : a.location;
        }
      }
      EnergySystemStruct.selectEnergySystemStruct = energySysRequest.params.selectEnergySystemStruct;
      for (let re of filter) {
        EnergySystemStruct.draw(energySysRequest.context, re);
      }
      drawLegend(energySysRequest);
      drawSelection(energySysRequest.context, energySysRequest.params);
      energySysRequest.context.closePath();
      drawFlagLine(
        energySysRequest.context,
        energySysRequest.flagMoveInfo,
        energySysRequest.flagSelectedInfo,
        energySysRequest.startNS,
        energySysRequest.endNS,
        energySysRequest.totalNS,
        energySysRequest.frame,
        energySysRequest.slicesTime
      );
    }
    // @ts-ignore
    self.postMessage({
      id: energySysRequest.id,
      type: energySysRequest.type,
      results: energySysRequest.canvas ? undefined : filter,
      hover: EnergySystemStruct.hoverEnergySystemStruct,
    });
  }
}

export function drawLegend(req: RequestMessage | any, isDark?: boolean) {
  let textList = ['WORKSCHEDULER', 'POWER_RUNNINGLOCK', 'LOCATION'];
  for (let index = 0; index < textList.length; index++) {
    let text = req.context.measureText(textList[index]);
    req.context.fillStyle = EnergySystemStruct.getColor(index);
    let canvasEndX = req.context.canvas.clientWidth - EnergySystemStruct.OFFSET_WIDTH;
    let textColor = isDark ? '#FFFFFF' : '#333';
    if (textList[index] == 'WORKSCHEDULER') {
      req.context.fillRect(canvasEndX - EnergySystemStruct.itemNumber * 120, 12, 8, 8);
      req.context.globalAlpha = 1;
      req.context.textBaseline = 'middle';
      req.context.fillStyle = textColor;
      req.context.fillText(textList[index], canvasEndX - EnergySystemStruct.itemNumber * 120 + 10, 18);
      EnergySystemStruct.currentTextWidth = canvasEndX - EnergySystemStruct.itemNumber * 120 + 40 + text.width;
    } else {
      req.context.fillRect(EnergySystemStruct.currentTextWidth, 12, 8, 8);
      req.context.globalAlpha = 1;
      req.context.fillStyle = textColor;
      req.context.textBaseline = 'middle';
      req.context.fillText(textList[index], EnergySystemStruct.currentTextWidth + 12, 18);
      EnergySystemStruct.currentTextWidth = EnergySystemStruct.currentTextWidth + 40 + text.width;
    }
  }
  req.context.fillStyle = '#333';
}

export function system(
  systemList: Array<any>,
  res: Array<any>,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: any,
  use: boolean
) {
  if (use && res.length > 0) {
    for (let i = 0; i < res.length; i++) {
      let systemItem = res[i];
      if (
        (systemItem.startNs || 0) + (systemItem.dur || 0) > (startNS || 0) &&
        (systemItem.startNs || 0) < (endNS || 0)
      ) {
        EnergySystemStruct.setSystemFrame(systemItem, 10, startNS || 0, endNS || 0, totalNS || 0, frame);
      } else {
        systemItem.frame = null;
      }
    }
    return;
  }
  res.length = 0;
  if (systemList) {
    for (let i = 0; i < 3; i++) {
      let arr = systemList[i];
      if (arr) {
        for (let index = 0; index < arr.length; index++) {
          let item = arr[index];
          if (index === arr.length - 1) {
            item.dur = (endNS || 0) - (item.startNs || 0);
          } else {
            item.dur = (arr[index + 1].startNs || 0) - (item.startNs || 0);
          }
          if (item.count == 0) {
            item.dur = 0;
          }
          if ((item.startNs || 0) + (item.dur || 0) > (startNS || 0) && (item.startNs || 0) < (endNS || 0)) {
            EnergySystemStruct.setSystemFrame(item, 10, startNS || 0, endNS || 0, totalNS || 0, frame);
            res.push(item);
          }
        }
      }
    }
  }
}

export class EnergySystemStruct extends BaseStruct {
  static hoverEnergySystemStruct: EnergySystemStruct | undefined;
  static selectEnergySystemStruct: EnergySystemStruct | undefined;
  static itemNumber: number = 3;
  static currentTextWidth: number = 0;
  static OFFSET_WIDTH: number = 266;
  type: number | undefined;
  startNs: number | undefined;
  dur: number | undefined;
  count: number | undefined;
  workScheduler: string | undefined;
  power: string | undefined;
  location: string | undefined;

  static draw(energySystemContext: CanvasRenderingContext2D, data: EnergySystemStruct) {
    if (data.frame) {
      let width = data.frame.width || 0;
      energySystemContext.globalAlpha = 1.0;
      energySystemContext.lineWidth = 1;
      energySystemContext.fillStyle = this.getColor(data.type!);
      energySystemContext.strokeStyle = this.getColor(data.type!);
      energySystemContext.fillRect(data.frame.x, data.frame.y + 4, width, data.frame.height);
    }
    energySystemContext.globalAlpha = 1.0;
    energySystemContext.lineWidth = 1;
  }

  static setSystemFrame(systemNode: any, padding: number, startNS: number, endNS: number, totalNS: number, frame: any) {
    let systemStartPointX: number;
    let systemEndPointX: number;
    if ((systemNode.startNs || 0) < startNS) {
      systemStartPointX = 0;
    } else {
      systemStartPointX = ns2x(systemNode.startNs || 0, startNS, endNS, totalNS, frame);
    }
    if ((systemNode.startNs || 0) + (systemNode.dur || 0) > endNS) {
      systemEndPointX = frame.width;
    } else {
      systemEndPointX = ns2x((systemNode.startNs || 0) + (systemNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let frameWidth: number = systemEndPointX - systemStartPointX <= 1 ? 1 : systemEndPointX - systemStartPointX;
    if (!systemNode.frame) {
      systemNode.frame = {};
    }
    systemNode.frame.x = Math.floor(systemStartPointX);
    if (systemNode.type === 0) {
      systemNode.frame.y = frame.y + padding * 2.5;
    } else if (systemNode.type === 1) {
      systemNode.frame.y = frame.y + padding * 4.5;
    } else if (systemNode.type === 2) {
      systemNode.frame.y = frame.y + padding * 6.5;
    }
    systemNode.frame.width = Math.ceil(frameWidth);
    systemNode.frame.height = Math.floor(padding);
  }

  static getColor(textItem: number): string {
    switch (textItem) {
      case 0:
        return '#E64566';
      case 1:
        return '#FFC880';
      default:
        return '#564AF7';
    }
  }
}
