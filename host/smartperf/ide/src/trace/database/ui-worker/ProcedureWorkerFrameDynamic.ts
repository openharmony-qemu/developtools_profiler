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

import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { BaseStruct, computeUnitWidth, isSurroundingPoint, ns2x, Rect, Render } from './ProcedureWorkerCommon.js';
import { AnimationRanges } from '../../bean/FrameComponentBean.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';

export class FrameDynamicRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
      animationRanges: AnimationRanges[];
    },
    row: TraceRow<FrameDynamicStruct>
  ): void {
    let frameDynamicList: FrameDynamicStruct[] = row.dataList;
    let frameDynamicFilter: FrameDynamicStruct[] = row.dataListCache;
    this.frameDynamic(frameDynamicList, frameDynamicFilter, row, req.animationRanges, req.useCache);
    if (req.animationRanges.length > 0 && req.animationRanges[0] && frameDynamicFilter.length > 0) {
      let modelType: string = row.getAttribute('model-type') || 'x';
      let [minValue, maxValue] = this.getMinAndMaxData(frameDynamicFilter, modelType);
      let preDynamic: FrameDynamicStruct = frameDynamicFilter[0];
      let isDraw = false;
      let selectUnitWidth: number = 0;
      for (let index: number = 0; index < frameDynamicFilter.length; index++) {
        let currDynamic: FrameDynamicStruct = frameDynamicFilter[index];
        selectUnitWidth = computeUnitWidth(preDynamic.ts, currDynamic.ts, row.frame.width, selectUnitWidth);
        this.refreshPointY(currDynamic, row, modelType, minValue, maxValue);
        if (currDynamic.groupId === 0) {
          if (currDynamic.ts > TraceRow.range!.startNS && currDynamic.ts < TraceRow.range!.endNS) {
            isDraw = true;
            this.drawSinglePoint(req.context, currDynamic, row, modelType, minValue, maxValue);
          }
        } else if (currDynamic.groupId !== invalidGroupId && index > 0 && currDynamic.groupId === preDynamic!.groupId) {
          isDraw = true;
          FrameDynamicStruct.draw(req.context, preDynamic, currDynamic, row, modelType);
        }
        FrameDynamicStruct.drawSelect(req.context, currDynamic, row);
        preDynamic = currDynamic;
      }
      if (isDraw) {
        this.drawDynamicPointYStr(req.context, frameDynamicFilter, row.frame, minValue, maxValue);
      }
      if (!this.setHoverFrameDynamic(row, frameDynamicFilter, selectUnitWidth) && row.isHover) {
        FrameDynamicStruct.hoverFrameDynamicStruct = undefined;
      }
    }
  }

  private setHoverFrameDynamic(
    row: TraceRow<FrameDynamicStruct>,
    frameDynamicFilter: FrameDynamicStruct[],
    selectUnitWidth: number
  ): boolean {
    let find: boolean = false;
    let findStructList = frameDynamicFilter.filter(filter =>
      row.isHover && isSurroundingPoint(row.hoverX, filter.frame!, selectUnitWidth / multiple));
    if (findStructList.length > 0) {
      find = true;
      let hoverIndex: number = 0;
      if (findStructList.length > unitIndex) {
        hoverIndex = Math.ceil(findStructList.length / multiple);
      }
      FrameDynamicStruct.hoverFrameDynamicStruct = findStructList[hoverIndex];
    }
    return find;
  }

  private frameDynamic(
    frameDynamicList: FrameDynamicStruct[],
    frameDynamicFilter: FrameDynamicStruct[],
    row: TraceRow<FrameDynamicStruct>,
    animationRanges: AnimationRanges[],
    use: boolean
  ): void {
    let startNS: number = TraceRow.range!.startNS;
    let endNS: number = TraceRow.range!.endNS;
    let totalNS: number = TraceRow.range!.totalNS;
    let frame: Rect = row.frame;
    let modelName: string | undefined | null = row.getAttribute('model-name');
    if ((use || !TraceRow.range!.refresh) && frameDynamicFilter.length > 0) {
      this.refreshDynamicFrame(frameDynamicFilter, frame, startNS, endNS, totalNS, modelName!);
      return;
    }
    frameDynamicFilter.length = 0;
    if (frameDynamicList) {
      let groupIdList: number[] = [];
      for (let dataIndex: number = 0; dataIndex < frameDynamicList.length; dataIndex++) {
        let currentDynamic: FrameDynamicStruct = frameDynamicList[dataIndex];
        if (currentDynamic.appName === modelName) {
          currentDynamic.groupId = invalidGroupId;
          for (let rangeIndex = 0; rangeIndex < animationRanges.length; rangeIndex++) {
            let currentRange = animationRanges[rangeIndex];
            if (currentDynamic.ts >= currentRange.start && currentDynamic.ts <= currentRange.end) {
              currentDynamic.groupId = currentRange.start;
              break;
            }
          }
          if (currentDynamic.ts < startNS && (dataIndex + unitIndex) < frameDynamicList.length &&
            frameDynamicList[dataIndex + unitIndex].ts >= startNS && currentDynamic.groupId !== invalidGroupId) {
            this.refreshFilterDynamicFrame(frameDynamicFilter, currentDynamic, row.frame, startNS,
              endNS, totalNS, groupIdList);
          }
          if (currentDynamic.ts >= startNS && currentDynamic.ts <= endNS && currentDynamic.groupId !== invalidGroupId) {
            this.refreshFilterDynamicFrame(frameDynamicFilter, currentDynamic, row.frame, startNS,
              endNS, totalNS, groupIdList);
          }
          if (currentDynamic.ts >= endNS && currentDynamic.groupId !== invalidGroupId) {
            this.refreshFilterDynamicFrame(frameDynamicFilter, currentDynamic, row.frame, startNS,
              endNS, totalNS, groupIdList);
            break;
          }
        }
      }
      this.setSimpleGroupId(groupIdList, frameDynamicFilter);
    }
  };

  private setSimpleGroupId(
    groupIdList: number[],
    frameDynamicFilter: FrameDynamicStruct[]
  ): void {
    let simpleGroup = groupIdList.filter(groupId => {
      return groupId !== invalidGroupId && groupIdList.indexOf(groupId) === groupIdList.lastIndexOf(groupId);
    });
    frameDynamicFilter.forEach(dynamic => {
      if (simpleGroup.indexOf(dynamic.groupId!) > invalidGroupId) {
        dynamic.groupId = 0;
      }
    });
  }

  private refreshDynamicFrame(
    frameDynamicFilter: FrameDynamicStruct[],
    frame: Rect,
    startNS: number,
    endNS: number,
    totalNS: number,
    modelName: string
  ): void {
    for (let index: number = 0; index < frameDynamicFilter.length; index++) {
      let frameDynamicNode: FrameDynamicStruct = frameDynamicFilter[index];
      frameDynamicNode.frame = undefined;
      if (frameDynamicNode.appName === modelName) {
        FrameDynamicStruct.setFrameDynamic(frameDynamicNode, startNS, endNS, totalNS, frame);
      }
    }
  };

  private drawSinglePoint(
    ctx: CanvasRenderingContext2D,
    currDynamic: FrameDynamicStruct,
    row: TraceRow<FrameDynamicStruct>,
    modelType: string,
    minValue: number,
    maxValue: number
  ): void {
    let smallArcRadius: number = 2;
    // @ts-ignore
    currDynamic.typeValue = currDynamic[modelType];
    currDynamic.frame!.y = row.frame.height - padding -
      ((row.frame.height - padding * multiple) * ((currDynamic.typeValue || 0) - minValue) / (maxValue - minValue));
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.globalAlpha = 1;
    ctx.arc(currDynamic.frame!.x, currDynamic.frame!.y, smallArcRadius, 0, multiple * Math.PI);
    ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[2];
    ctx.fillStyle = ColorUtils.ANIMATION_COLOR[2];
    ctx.stroke();
    ctx.fill();
    ctx.closePath();
  }

  private refreshPointY(
    curDynamic: FrameDynamicStruct,
    row: TraceRow<FrameDynamicStruct>,
    modelType: string,
    minValue: number,
    maxValue: number
  ): void {
    // @ts-ignore
    let currDynamicValue = curDynamic[modelType];
    if (curDynamic.frame) {
      let pointY = (row.frame.height - padding * multiple) * ((currDynamicValue - minValue) / (maxValue - minValue));
      curDynamic.frame.y = row.frame.height - padding - pointY;
    }
  };

  private drawDynamicPointYStr(
    ctx: CanvasRenderingContext2D,
    frameDynamicFilter: FrameDynamicStruct[],
    frame: Rect,
    minValue: number,
    maxValue: number
  ): void {
    if (frameDynamicFilter.length > 0) {
      ctx.globalAlpha = 1;
      ctx.lineWidth = 1;
      ctx.fillStyle = ColorUtils.ANIMATION_COLOR[3];
      let minUnitValue: number = 10;
      let fixedNumber = 2;
      let totalValue = maxValue - minValue;
      let pointYInterval = totalValue / (yScaleNumber - unitIndex);
      for (let index = 0; index < yScaleNumber; index++) {
        let pointYValue = minValue + pointYInterval * index;
        let pointYHeight = (frame.height - padding * multiple) * (pointYValue - minValue) / totalValue;
        let pointY = frame.height - multiple * padding - pointYHeight;
        if (pointYValue !== 0) {
          if (maxValue - minValue <= minUnitValue) {
            ctx.fillText(`- ${pointYValue.toFixed(fixedNumber)}`, 0, pointY);
          } else {
            ctx.fillText(`- ${Math.ceil(pointYValue)}`, 0, pointY);
          }
        }
      }
    }
  };

  private getMinAndMaxData(
    frameDynamicFilter: FrameDynamicStruct[],
    modelType: string
  ): [number, number] {
    let min: number = Math.min.apply(Math, frameDynamicFilter.map(filterData => {
      // @ts-ignore
      return filterData[modelType];
    }));
    let max: number = Math.max.apply(Math, frameDynamicFilter.map(filterData => {
      // @ts-ignore
      return filterData[modelType];
    }));

    let yScaleMinValue: number = 1;
    let yScaleMinSpacing: number = 10;
    if (min === max) {
      if (max <= yScaleMinValue) {
        max = yScaleMinValue;
        min = 0;
      } else if (max <= yScaleMinSpacing) {
        max = yScaleMinSpacing;
        min = 0;
      } else {
        max += yScaleMinSpacing;
        min -= yScaleMinSpacing;
      }
    }
    return [min, max];
  };

  private refreshFilterDynamicFrame(
    frameDynamicFilter: FrameDynamicStruct[],
    currentFrameDynamic: FrameDynamicStruct,
    frame: Rect,
    startNS: number,
    endNS: number,
    totalNS: number,
    groupIdList: number[]
  ): void {
    groupIdList.push(currentFrameDynamic.groupId!);
    frameDynamicFilter.push(currentFrameDynamic);
    FrameDynamicStruct.setFrameDynamic(currentFrameDynamic, startNS, endNS, totalNS, frame);
  };
}

export class FrameDynamicStruct extends BaseStruct {
  static hoverFrameDynamicStruct: FrameDynamicStruct | undefined;
  static selectFrameDynamicStruct: FrameDynamicStruct | undefined;
  ts: number = 0;
  id: number | undefined;
  x: number | undefined;
  y: number | undefined;
  width: number | undefined;
  height: number | undefined;
  alpha: number = 0;
  appName: string | undefined;
  groupId: number | undefined;
  typeValue: number | undefined;

  static setFrameDynamic(
    dynamicNode: FrameDynamicStruct,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: Rect
  ): void {
    let pointX: number = ns2x(dynamicNode.ts || 0, startNS, endNS, totalNS, frame);
    if (!dynamicNode.frame) {
      dynamicNode.frame = new Rect(0, 0, 0, 0);
    }
    dynamicNode.frame.x = Math.floor(pointX);
    dynamicNode.frame.height = frame.height;
  }

  static draw(
    ctx: CanvasRenderingContext2D,
    preDynamicStruct: FrameDynamicStruct,
    currDynamicStruct: FrameDynamicStruct,
    row: TraceRow<FrameDynamicStruct>,
    modelType: string
  ): void {
    if (preDynamicStruct.frame && currDynamicStruct.frame) {
      // @ts-ignore
      preDynamicStruct.typeValue = preDynamicStruct[modelType];
      // @ts-ignore
      currDynamicStruct.typeValue = currDynamicStruct[modelType];
      if (preDynamicStruct.typeValue === currDynamicStruct.typeValue) {
        ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[1];
      } else {
        ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[2];
      }
      ctx.beginPath();
      ctx.globalAlpha = 1;
      ctx.lineWidth = 3;
      ctx.lineJoin = 'round';
      ctx.moveTo(preDynamicStruct.frame.x, preDynamicStruct.frame.y);
      ctx.lineTo(currDynamicStruct.frame.x, currDynamicStruct.frame.y);
      ctx.stroke();
      ctx.closePath();
      FrameDynamicStruct.drawSelect(ctx, preDynamicStruct, row);
    }
  }

  static drawSelect(
    ctx: CanvasRenderingContext2D,
    currDynamicStruct: FrameDynamicStruct,
    row: TraceRow<FrameDynamicStruct>
  ): void {
    if ((currDynamicStruct === FrameDynamicStruct.hoverFrameDynamicStruct && row.isHover) ||
      currDynamicStruct === FrameDynamicStruct.selectFrameDynamicStruct) {
      FrameDynamicStruct.drawSelectOrHoverArc(ctx, currDynamicStruct);
    }
    if (row.getAttribute('check-type') === '2' && FrameDynamicStruct.isSelect(currDynamicStruct)) {
      FrameDynamicStruct.drawSelectOrHoverArc(ctx, currDynamicStruct);
    }
  }

  static drawSelectOrHoverArc(
    ctx: CanvasRenderingContext2D,
    currDynamicStruct: FrameDynamicStruct
  ): void {
    if (currDynamicStruct.frame &&
      currDynamicStruct.ts > TraceRow.range!.startNS &&
      currDynamicStruct.ts < TraceRow.range!.endNS) {
      let bigArcRadius: number = 3;
      ctx.beginPath();
      ctx.lineWidth = 3;
      ctx.globalAlpha = 1;
      ctx.arc(currDynamicStruct.frame.x, currDynamicStruct.frame.y, bigArcRadius, 0, multiple * Math.PI);
      ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[7];
      ctx.fillStyle = ColorUtils.ANIMATION_COLOR[9];
      ctx.stroke();
      ctx.fill();
    }
  }

  static isSelect(currDynamicStruct: FrameDynamicStruct): boolean | 0 | undefined {
    return TraceRow.rangeSelectObject &&
      TraceRow.rangeSelectObject.startNS &&
      TraceRow.rangeSelectObject.endNS &&
      currDynamicStruct.ts >= TraceRow.rangeSelectObject.startNS &&
      currDynamicStruct.ts <= TraceRow.rangeSelectObject.endNS;
  }
}

const padding: number = 3;
const invalidGroupId: number = -1;
const multiple: number = 2;
const unitIndex: number = 1;
const yScaleNumber: number = 5;
