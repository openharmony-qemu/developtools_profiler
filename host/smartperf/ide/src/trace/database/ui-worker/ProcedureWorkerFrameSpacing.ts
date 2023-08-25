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

export class FrameSpacingRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
      frameRate: number
      animationRanges: AnimationRanges[]
    },
    row: TraceRow<FrameSpacingStruct>
  ): void {
    let frameSpacingList = row.dataList;
    let frameSpacingFilter = row.dataListCache;
    this.frameSpacing(
      frameSpacingList,
      frameSpacingFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row,
      req.animationRanges,
      req.useCache || !TraceRow.range!.refresh
    );
    this.render(req, frameSpacingFilter, row);
  }

  private render(req: { useCache: boolean; context: CanvasRenderingContext2D; type: string; frameRate: number;
  animationRanges: AnimationRanges[] }, frameSpacingFilter: Array<FrameSpacingStruct>,
  row: TraceRow<FrameSpacingStruct>) : void {
    if (req.animationRanges.length > 0 && req.animationRanges[0] && frameSpacingFilter.length > 0) {
      let preFrameSpacing: FrameSpacingStruct = frameSpacingFilter[0];
      // @ts-ignore
      let smallTickStandard = smallTick[req.frameRate];
      let [minValue, maxValue] = this.maxMinData(smallTickStandard.firstLine,
        smallTickStandard.thirdLine, frameSpacingFilter);
      let isDraw = false;
      let selectUnitWidth: number = 0;
      for (let index: number = 0; index < frameSpacingFilter.length; index++) {
        let currentStruct = frameSpacingFilter[index];
        selectUnitWidth = computeUnitWidth(preFrameSpacing.currentTs, currentStruct.currentTs,
          row.frame.width, selectUnitWidth);
        FrameSpacingStruct.refreshHoverStruct(preFrameSpacing, currentStruct, row, minValue, maxValue);
        if (currentStruct.groupId === 0) {
          if (currentStruct.currentTs > TraceRow.range!.startNS && currentStruct.currentTs < TraceRow.range!.endNS) {
            isDraw = true;
            this.drawPoint(req.context, currentStruct, row, minValue, maxValue);
          }
        } else if (currentStruct.groupId !== invalidGroupId && index > 0 &&
          currentStruct.groupId === preFrameSpacing!.groupId) {
          isDraw = true;
          FrameSpacingStruct.draw(req.context, preFrameSpacing, currentStruct, row, minValue, maxValue);
        }
        FrameSpacingStruct.drawSelect(currentStruct, req.context, row);
        preFrameSpacing = currentStruct;
      }
      if (isDraw) {
        this.drawDashedLines(Object.values(smallTickStandard), req, row, minValue, maxValue);
      }
      let findStructList = frameSpacingFilter.filter(filter =>
        row.isHover && isSurroundingPoint(row.hoverX, filter.frame!, selectUnitWidth / multiple));
      let find = false;
      if (findStructList.length > 0) {
        find = true;
        let hoverIndex: number = 0;
        if (findStructList.length > unitIndex) {
          hoverIndex = Math.ceil(findStructList.length / multiple);
        }
        FrameSpacingStruct.hoverFrameSpacingStruct = findStructList[hoverIndex];
      }
      if (!find && row.isHover) {
        FrameSpacingStruct.hoverFrameSpacingStruct = undefined;
      }
    }
  }

  private drawPoint(ctx: CanvasRenderingContext2D, currentStruct: FrameSpacingStruct,
    row: TraceRow<FrameSpacingStruct>, minValue: number, maxValue: number): void {
    let currentPointY = row.frame.height - Math.floor((currentStruct.frameSpacingResult! -
        minValue) * (row.frame.height - padding * multiple) /
      (maxValue - minValue)) - padding;
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.globalAlpha = 1;
    ctx.arc(currentStruct.frame!.x, currentPointY, multiple, 0, multiple * Math.PI);
    ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[2];
    ctx.fillStyle = ColorUtils.ANIMATION_COLOR[2];
    ctx.stroke();
    ctx.fill();
    ctx.closePath();
  }

  private drawDashedLines(
    dashedLines: number[],
    req: { useCache: boolean; context: CanvasRenderingContext2D; type: string; frameRate: number },
    row: TraceRow<FrameSpacingStruct>,
    minVale: number,
    maxValue: number
  ): void {
    for (let i = 0; i < dashedLines.length; i++) {
      FrameSpacingStruct.drawParallelLine(req.context, row.frame, dashedLines, i, minVale, maxValue);
    }
  }

  private frameSpacing(
    frameSpacingList: Array<FrameSpacingStruct>,
    frameSpacingFilter: Array<FrameSpacingStruct>,
    startNS: number,
    endNS: number,
    totalNS: number,
    row: TraceRow<FrameSpacingStruct>,
    animationRanges: AnimationRanges[],
    use: boolean
  ): void {
    let frame: Rect = row.frame;
    let modelName: string | undefined | null = row.getAttribute('model-name');
    if (use && frameSpacingFilter.length > 0) {
      for (let index = 0, len = frameSpacingFilter.length; index < len; index++) {
        if (frameSpacingFilter[index].nameId === modelName) {
          FrameSpacingStruct.setFrameSpacingFrame(frameSpacingFilter[index], startNS, endNS, totalNS, frame);
        }
      }
      return;
    }
    frameSpacingFilter.length = 0;
    if (frameSpacingList) {
      let groupIdList: number[] = [];
      for (let index = 0; index < frameSpacingList.length; index++) {
        let item = frameSpacingList[index];
        if (modelName === item.nameId) {
          item.groupId = invalidGroupId;
          for (let rangeIndex = 0; rangeIndex < animationRanges.length; rangeIndex++) {
            let currentRange = animationRanges[rangeIndex];
            if (item.currentTs >= currentRange.start && item.currentTs <= currentRange.end) {
              item.groupId = currentRange.start;
              break;
            }
          }
          if (item.currentTs < startNS && index + unitIndex < frameSpacingList.length &&
            frameSpacingList[index + unitIndex].currentTs >= startNS && item.groupId !== invalidGroupId) {
            this.refreshFrame(frameSpacingFilter, item, startNS, endNS, totalNS, frame, groupIdList);
          }
          if (item.currentTs >= startNS && item.currentTs <= endNS && item.groupId !== invalidGroupId) {
            this.refreshFrame(frameSpacingFilter, item, startNS, endNS, totalNS, frame, groupIdList);
          }
          if (item.currentTs > endNS && item.groupId !== invalidGroupId) {
            this.refreshFrame(frameSpacingFilter, item, startNS, endNS, totalNS, frame, groupIdList);
            break;
          }
        }
      }
      this.grouping(groupIdList, frameSpacingFilter);
    }
  }

  private grouping(groupIdList: number[], frameSpacingFilter: Array<FrameSpacingStruct>): void {
    let simpleGroup = groupIdList.filter(groupId => {
      return groupId !== invalidGroupId && groupIdList.indexOf(groupId) === groupIdList.lastIndexOf(groupId);
    });
    frameSpacingFilter.forEach(frameSpacing => {
      if (simpleGroup.indexOf(frameSpacing.groupId!) > invalidGroupId) {
        frameSpacing.groupId = 0;
        frameSpacing.frameSpacingResult = 0;
        frameSpacing.preFrameWidth = 0;
        frameSpacing.preFrameHeight = 0;
        frameSpacing.preTs = 0;
      }
    });
  }

  private refreshFrame(frameSpacingFilter: Array<FrameSpacingStruct>, item: FrameSpacingStruct,
    startNS: number, endNS: number, totalNS: number, frame: Rect, groupIdList: number[]): void {
    frameSpacingFilter.push(item);
    FrameSpacingStruct.setFrameSpacingFrame(item, startNS, endNS, totalNS, frame);
    groupIdList.push(item.groupId!);
  }

  private maxMinData(tickStandardMin: number, tickStandardMax: number, filter: FrameSpacingStruct[]): [number, number] {
    let min = Math.min.apply(Math, filter.map(filterData => {
      return filterData.frameSpacingResult!;
    }));
    let max = Math.max.apply(Math, filter.map(filterData => {
      return filterData.frameSpacingResult!;
    }));
    if (max < tickStandardMax) {
      max = tickStandardMax + padding;
    }
    if (min > tickStandardMin) {
      min = tickStandardMin;
    }
    return [min, max];
  }
}

export class FrameSpacingStruct extends BaseStruct {
  static hoverFrameSpacingStruct: FrameSpacingStruct | undefined;
  static selectFrameSpacingStruct: FrameSpacingStruct | undefined;
  static physicalWidth: number | undefined;
  static physicalHeight: number | undefined;
  preTs: number | undefined;
  currentTs: number = 0;
  frameSpacingResult: number | undefined;
  id: number = 0;
  groupId: number | undefined;
  currentFrameWidth: number | undefined;
  preFrameWidth: number | undefined;
  currentFrameHeight: number | undefined;
  preFrameHeight: number | undefined;
  x: number | undefined;
  y: number | undefined;
  nameId: string | undefined;

  static setFrameSpacingFrame(
    frameSpacingNode: FrameSpacingStruct,
    startNS: number,
    endNS: number,
    totalNS: number,
    row: Rect
  ): void {
    let pointX = ns2x(frameSpacingNode.currentTs || 0, startNS, endNS, totalNS, row);
    if (!frameSpacingNode.frame) {
      frameSpacingNode.frame = new Rect(0, 0, 0, 0);
    }
    frameSpacingNode.frame.x = Math.floor(pointX);
  }

  static isSelect(currSpacingStruct: FrameSpacingStruct) : boolean | 0 | undefined {
    return TraceRow.rangeSelectObject &&
      TraceRow.rangeSelectObject.startNS &&
      TraceRow.rangeSelectObject.endNS &&
      currSpacingStruct.currentTs >= TraceRow.rangeSelectObject.startNS &&
      currSpacingStruct.currentTs <= TraceRow.rangeSelectObject.endNS;
  }

  static refreshHoverStruct(preFrameSpacing: FrameSpacingStruct, frameSpacing: FrameSpacingStruct,
    row: TraceRow<FrameSpacingStruct>, minValue: number, maxValue: number): void {
    if (frameSpacing.frame) {
      frameSpacing.frame.y = row.frame.height - Math.floor((frameSpacing.frameSpacingResult! -
        minValue) * (row.frame.height - padding * multiple) /
        (maxValue - minValue)) - padding;
    }
  };

  static draw(
    ctx: CanvasRenderingContext2D,
    preFrameSpacing: FrameSpacingStruct,
    currentStruct: FrameSpacingStruct,
    rowFrame: TraceRow<FrameSpacingStruct>,
    minValue: number,
    maxValue: number
  ): void {
    if (currentStruct.frame && preFrameSpacing.frame) {
      this.drawPolyline(ctx, preFrameSpacing, currentStruct, rowFrame, minValue, maxValue);
    }
  }

  static drawSelect(currentStruct: FrameSpacingStruct, ctx: CanvasRenderingContext2D,
    rowFrame: TraceRow<FrameSpacingStruct>): void {
    if (currentStruct.frame &&
      currentStruct.currentTs > TraceRow.range!.startNS &&
      currentStruct.currentTs < TraceRow.range!.endNS) {
      if ((currentStruct === FrameSpacingStruct.hoverFrameSpacingStruct && rowFrame.isHover) ||
        currentStruct === FrameSpacingStruct.selectFrameSpacingStruct) {
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.arc(currentStruct.frame.x, currentStruct.frame.y, selectRadius, 0, multiple * Math.PI);
        ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[7];
        ctx.lineWidth = 2;
        ctx.globalAlpha = 1;
        ctx.fillStyle = ColorUtils.ANIMATION_COLOR[9];
        ctx.stroke();
        ctx.fill();
        ctx.closePath();
      }
    }
    if (rowFrame.getAttribute('check-type') === '2' && FrameSpacingStruct.isSelect(currentStruct)) {
      ctx.beginPath();
      ctx.lineWidth = 3;
      ctx.arc(currentStruct.frame!.x, currentStruct.frame!.y, selectRadius, 0, multiple * Math.PI);
      ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[7];
      ctx.fillStyle = ColorUtils.ANIMATION_COLOR[9];
      ctx.lineWidth = 2;
      ctx.globalAlpha = 1;
      ctx.stroke();
      ctx.fill();
      ctx.closePath();
    }
  }

  static drawParallelLine(
    ctx: CanvasRenderingContext2D,
    rowFrame: Rect,
    dashedLines: number[],
    index: number,
    minValue: number,
    maxValue: number
  ): void {
    let pointY = rowFrame.height - Math.floor((dashedLines[index] - minValue) *
      (rowFrame.height - padding * multiple) / (maxValue - minValue)) - padding;
    let lineDash = 10;
    let textPadding = 4;
    ctx.beginPath();
    ctx.lineWidth = 2;
    ctx.setLineDash([lineDash]);
    ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[6];
    ctx.fillStyle = ColorUtils.ANIMATION_COLOR[6];
    ctx.moveTo(0, pointY);
    ctx.lineTo(rowFrame.width, pointY);
    ctx.stroke();
    ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[3];
    ctx.fillStyle = ColorUtils.ANIMATION_COLOR[3];
    if (index === 0) {
      ctx.fillText(dashedLines[index].toString(), 0, pointY + multiple * textPadding);
    } else if (index === unitIndex) {
      ctx.fillText(dashedLines[index].toString(), 0, pointY + textPadding);
    } else {
      ctx.fillText(dashedLines[index].toString(), 0, pointY - textPadding);
    }
    ctx.closePath();
  }

  static drawPolyline(
    ctx: CanvasRenderingContext2D,
    preFrameSpacing: FrameSpacingStruct,
    currentStruct: FrameSpacingStruct,
    rowFrame: TraceRow<FrameSpacingStruct>,
    minValue: number,
    maxValue: number
  ): void {
    ctx.beginPath();
    let prePointY = rowFrame.frame.height - Math.floor((preFrameSpacing.frameSpacingResult! -
        minValue) * (rowFrame.frame.height - padding * multiple) /
      (maxValue - minValue)) - padding;
    let currentPointY = rowFrame.frame.height - Math.floor((currentStruct.frameSpacingResult! -
        minValue) * (rowFrame.frame.height - padding * multiple) /
      (maxValue - minValue)) - padding;
    if (preFrameSpacing.frame && currentStruct.frame) {
      ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[2];
      ctx.fillStyle = ColorUtils.ANIMATION_COLOR[2];
      ctx.lineWidth = 2;
      ctx.setLineDash([0]);
      ctx.moveTo(preFrameSpacing.frame.x, prePointY);
      ctx.lineTo(currentStruct.frame.x, currentPointY);
      ctx.stroke();
      ctx.closePath();
      FrameSpacingStruct.drawSelect(preFrameSpacing, ctx, rowFrame);
    }
  }
}

const padding = 3;
const invalidGroupId: number = -1;
const multiple: number = 2;
const unitIndex: number = 1;
const selectRadius: number = 3;

const smallTick = {
  60: {
    firstLine: 11.4,
    secondLine: 13,
    thirdLine: 14.6
  },
  90: {
    firstLine: 13.7,
    secondLine: 19.4,
    thirdLine: 25
  },
  120: {
    firstLine: 13.7,
    secondLine: 19.4,
    thirdLine: 25
  }
};
