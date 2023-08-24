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
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { BaseStruct, drawString, isFrameContainPoint, ns2x, Rect, Render } from './ProcedureWorkerCommon.js';

export class FrameAnimationRender extends Render {
  renderMainThread(
    req: {
      useCache: boolean;
      context: CanvasRenderingContext2D;
      type: string;
    },
    row: TraceRow<FrameAnimationStruct>
  ): void {
    let frameAnimationList: FrameAnimationStruct[] = row.dataList;
    let frameAnimationFilter: FrameAnimationStruct[] = row.dataListCache;
    this.frameAnimation(
      frameAnimationList,
      frameAnimationFilter,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      row.frame,
      req.useCache || !TraceRow.range!.refresh
    );
    req.context.beginPath();
    let find: boolean = false;
    for (let index: number = 0; index < frameAnimationFilter.length; index++) {
      let currentAnimationStruct: FrameAnimationStruct = frameAnimationFilter[index];
      FrameAnimationStruct.draw(req.context, index, currentAnimationStruct, row);
      if (row.isHover && currentAnimationStruct.frame &&
        isFrameContainPoint(currentAnimationStruct.frame, row.hoverX, row.hoverY)) {
        FrameAnimationStruct.hoverFrameAnimationStruct = currentAnimationStruct;
        find = true;
      }
    }
    if (!find && row.isHover) {
      FrameAnimationStruct.hoverFrameAnimationStruct = undefined;
    }
    req.context.closePath();
  }

  private frameAnimation(
    frameAnimationList: FrameAnimationStruct[],
    frameAnimationFilter: FrameAnimationStruct[],
    startNS: number = 0,
    endNS: number = 0,
    totalNS: number,
    frame: Rect,
    use: boolean
  ): void {
    if (use && frameAnimationFilter.length > 0) {
      for (let index: number = 0; index < frameAnimationFilter.length; index++) {
        let frameAnimationNode: FrameAnimationStruct = frameAnimationFilter[index];
        frameAnimationNode.frame = undefined;
        FrameAnimationStruct.setFrameAnimation(frameAnimationNode, padding, startNS, endNS, totalNS, frame);
      }
      return;
    }
    frameAnimationFilter.length = 0;
    if (frameAnimationList) {
      for (let index: number = 0; index < frameAnimationList.length; index++) {
        let currentFrameAnimation: FrameAnimationStruct = frameAnimationList[index];
        let currentResponseFrame: FrameAnimationStruct = JSON.parse(JSON.stringify(currentFrameAnimation));
        currentResponseFrame.status = 'Response delay';
        currentResponseFrame.dur = currentFrameAnimation.dynamicStartTs - currentFrameAnimation.ts;
        if ((currentResponseFrame.ts || 0) + (currentResponseFrame.dur || 0) > startNS &&
          (currentResponseFrame.ts || 0) < endNS) {
          FrameAnimationStruct.setFrameAnimation(currentResponseFrame, padding, startNS,
            endNS || 0, totalNS || 0, frame);
          frameAnimationFilter.push(currentResponseFrame);
        }
        let currentCompletionFrame: FrameAnimationStruct = JSON.parse(JSON.stringify(currentFrameAnimation));
        currentCompletionFrame.status = 'Completion delay';
        currentCompletionFrame.ts = currentFrameAnimation.dynamicStartTs;
        currentCompletionFrame.dur = currentFrameAnimation.dynamicEndTs - currentFrameAnimation.dynamicStartTs;
        if ((currentCompletionFrame.ts || 0) + (currentCompletionFrame.dur || 0) > startNS &&
          (currentCompletionFrame.ts || 0) < endNS) {
          FrameAnimationStruct.setFrameAnimation(currentCompletionFrame, padding, startNS,
            endNS || 0, totalNS || 0, frame);
          frameAnimationFilter.push(currentCompletionFrame);
        }
      }
    }
  };
}

export class FrameAnimationStruct extends BaseStruct {
  static hoverFrameAnimationStruct: FrameAnimationStruct | undefined;
  static selectFrameAnimationStruct: FrameAnimationStruct | undefined;
  ts: number = 0;
  dur: number = 0;
  status: string = '';
  animationId: number | undefined;
  dynamicStartTs: number = 0;
  dynamicEndTs: number = 0;
  fps: number | undefined;

  static setFrameAnimation(
    animationNode: FrameAnimationStruct,
    padding: number,
    startNS: number,
    endNS: number,
    totalNS: number,
    frame: Rect
  ): void {
    let stateStartPointX: number;
    let stateEndPointX: number;
    if ((animationNode.ts || 0) < startNS) {
      stateStartPointX = 0;
    } else {
      stateStartPointX = ns2x(animationNode.ts || 0, startNS, endNS, totalNS, frame);
    }
    if ((animationNode.ts || 0) + (animationNode.dur || 0) > endNS) {
      stateEndPointX = frame.width;
    } else {
      stateEndPointX = ns2x((animationNode.ts || 0) + (animationNode.dur || 0), startNS, endNS, totalNS, frame);
    }
    let frameWidth: number = stateEndPointX - stateStartPointX <= unitIndex ? unitIndex :
      stateEndPointX - stateStartPointX;
    if (!animationNode.frame) {
      animationNode.frame = new Rect(0, 0, 0, 0);
    }
    animationNode.frame.x = Math.floor(stateStartPointX);
    animationNode.frame.y = frame.y + padding;
    animationNode.frame.width = Math.ceil(frameWidth);
    animationNode.frame.height = Math.floor(frame.height - padding * multiple);
  }

  static draw(
    ctx: CanvasRenderingContext2D,
    index: number,
    frameAnimationNode: FrameAnimationStruct,
    row: TraceRow<FrameAnimationStruct>
  ): void {
    let tsFixed: number = 6;
    let isHover: boolean = row.isHover;
    if (frameAnimationNode.frame) {
      let nsToMillisecond = 1000_000;
      ctx.globalAlpha = 1.0;
      ctx.lineWidth = 1;
      ctx.lineJoin = 'round';
      ctx.fillStyle = ColorUtils.ANIMATION_COLOR[6];
      ctx.fillRect(frameAnimationNode.frame.x, frameAnimationNode.frame.y + (multiple * padding),
        frameAnimationNode.frame.width, frameAnimationNode.frame.height);
      ctx.fillStyle = ColorUtils.ANIMATION_COLOR[3];
      ctx.textBaseline = 'middle';
      ctx.font = '8px sans-serif';
      drawString(ctx, `${frameAnimationNode.status} (${(frameAnimationNode.dur / nsToMillisecond).
        toFixed(tsFixed)} ms)`, textPadding, frameAnimationNode.frame, frameAnimationNode);
      ctx.lineWidth = 2;
      if ((frameAnimationNode === FrameAnimationStruct.hoverFrameAnimationStruct && isHover) ||
        frameAnimationNode === FrameAnimationStruct.selectFrameAnimationStruct) {
        ctx.globalAlpha = 0.8;
        ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[3];
        ctx.strokeRect(frameAnimationNode.frame.x + padding, frameAnimationNode.frame.y + (multiple * padding),
          frameAnimationNode.frame.width - padding, frameAnimationNode.frame.height - (multiple * padding));
      } else {
        ctx.strokeStyle = ColorUtils.ANIMATION_COLOR[2];
        ctx.strokeRect(frameAnimationNode.frame.x + padding, frameAnimationNode.frame.y + (multiple * padding),
          frameAnimationNode.frame.width - padding, frameAnimationNode.frame.height - (multiple * padding));
      }
    }
  }
}

const padding: number = 2;
const multiple: number = 2;
const unitIndex: number = 1;
const textPadding: number = 5;
