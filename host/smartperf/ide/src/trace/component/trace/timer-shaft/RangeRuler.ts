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

import { Graph } from './Graph.js';
import { Rect } from './Rect.js';
import { ns2s, ns2UnitS, TimerShaftElement } from '../TimerShaftElement.js';
import { ColorUtils, interpolateColorBrightness } from '../base/ColorUtils.js';
import { CpuStruct } from '../../../database/ui-worker/ProcedureWorkerCPU.js';
import { CurrentSlicesTime, SpSystemTrace } from '../../SpSystemTrace.js';

const MarkPadding = 5;
const FIT_TOTALX_MIN: number = 280;
const FIT_TOTALX_MAX: number = 300;
const MID_OFFSET: number = 10;

export class Mark extends Graph {
  name: string | undefined;
  inspectionFrame: Rect;
  private _isHover: boolean = false;

  constructor(canvas: HTMLCanvasElement | undefined | null, name: string, c: CanvasRenderingContext2D, frame: Rect) {
    super(canvas, c, frame);
    this.name = name;
    this.inspectionFrame = new Rect(frame.x - MarkPadding, frame.y, frame.width + MarkPadding * 2, frame.height);
  }

  get isHover(): boolean {
    return this._isHover;
  }

  set isHover(value: boolean) {
    this._isHover = value;
    if (value) {
      document.body.style.cursor = 'ew-resize';
    } else {
      document.body.style.cursor = 'default';
    }
  }

  draw(): void {
    this.context2D.beginPath();
    this.context2D.strokeStyle = '#999999';
    this.context2D.lineWidth = 7;
    this.context2D.moveTo(this.frame.x, this.frame.y);
    this.context2D.lineTo(this.frame.x, this.frame.y + this.frame.height / 3);
    this.context2D.stroke();
    this.context2D.strokeStyle = '#999999';
    this.context2D.lineWidth = 1;
    this.context2D.moveTo(this.frame.x, this.frame.y);
    this.context2D.lineTo(this.frame.x, this.frame.y + this.frame.height);
    this.context2D.stroke();
    this.context2D.closePath();
  }
}

export interface TimeRange {
  slicesTime: {
    color: string | null | undefined;
    startTime: number | null | undefined;
    endTime: number | null | undefined;
  };
  scale: number;
  totalNS: number;
  startX: number;
  endX: number;
  startNS: number;
  endNS: number;
  xs: Array<number>;
  refresh: boolean;
  xsTxt: Array<string>;
}

export class RangeRuler extends Graph {
  public rangeRect: Rect;
  public currentSlicesTime: CurrentSlicesTime;
  public markAObj: Mark;
  public markBObj: Mark;
  public drawMark: boolean = false;
  public range: TimeRange;
  private pressedKeys: Array<string> = [];
  mouseDownOffsetX = 0;
  mouseDownMovingMarkX = 0;
  movingMark: Mark | undefined | null;
  isMouseDown: boolean = false;
  isMovingRange: boolean = false;
  isNewRange: boolean = false;
  markAX: number = 0;
  markBX: number = 0;
  isPress: boolean = false;
  pressFrameIdF: number = -1;
  pressFrameIdW: number = -1;
  pressFrameIdS: number = -1;
  pressFrameIdA: number = -1;
  pressFrameIdD: number = -1;
  upFrameIdW: number = -1;
  upFrameIdS: number = -1;
  upFrameIdA: number = -1;
  upFrameIdD: number = -1;
  currentDuration: number = 0;
  cacheInterval: { interval: number; value: number; flag: boolean } = {
    interval: 200,
    value: 0,
    flag: false,
  };
  centerXPercentage: number = 0;
  animaStartTime: number | undefined;
  p: number = 1000;
  private readonly notifyHandler: (r: TimeRange) => void;
  private scale: number = 0;
  private delayTimer: any = null;
  private rulerW = 0;

  //缩放级别
  private scales: Array<number> = [
    50, 100, 200, 500, 1_000, 2_000, 5_000, 10_000, 20_000, 50_000, 100_000, 200_000, 500_000, 1_000_000, 2_000_000,
    5_000_000, 10_000_000, 20_000_000, 50_000_000, 100_000_000, 200_000_000, 500_000_000, 1_000_000_000, 2_000_000_000,
    5_000_000_000, 10_000_000_000, 20_000_000_000, 50_000_000_000, 100_000_000_000, 200_000_000_000, 500_000_000_000,
    1_000_000_000_000, 2_000_000_000_000, 5_000_000_000_000,
  ];
  private _cpuUsage: Array<{ cpu: number; ro: number; rate: number }> = [];

  constructor(timerShaftEL: TimerShaftElement, frame: Rect, range: TimeRange, notifyHandler: (r: TimeRange) => void) {
    super(timerShaftEL.canvas, timerShaftEL.ctx!, frame);
    this.range = range;
    this.notifyHandler = notifyHandler;
    this.markAObj = new Mark(
      timerShaftEL.canvas,
      'A',
      timerShaftEL.ctx!,
      new Rect(range.startX, frame.y, 1, frame.height)
    );
    this.markBObj = new Mark(
      timerShaftEL.canvas,
      'B',
      timerShaftEL.ctx!,
      new Rect(range.endX, frame.y, 1, frame.height)
    );
    this.rangeRect = new Rect(range.startX, frame.y, range.endX - range.startX, frame.height);
    this.currentSlicesTime = new CurrentSlicesTime();
  }

  set cpuUsage(value: Array<{ cpu: number; ro: number; rate: number }>) {
    this._cpuUsage = value;
    this.draw();
  }

  drawCpuUsage() {
    this.context2D.clearRect(this.frame.x, this.frame.y, this.frame.width, this.frame.height);
    let miniHeight = Math.round(this.frame.height / CpuStruct.cpuCount); //每格高度
    let miniWidth = Math.ceil(this.frame.width / 100); //每格宽度
    for (let index = 0; index < this._cpuUsage.length; index++) {
      let cpuUsageItem = this._cpuUsage[index];
      const color = interpolateColorBrightness(ColorUtils.MD_PALETTE[cpuUsageItem.cpu], cpuUsageItem.rate);
      this.context2D.fillStyle = `rgb(${color[0]}, ${color[1]}, ${color[2]})`;
      this.context2D.globalAlpha = cpuUsageItem.rate;
      this.context2D.fillRect(
        this.frame.x + miniWidth * cpuUsageItem.ro,
        this.frame.y + cpuUsageItem.cpu * miniHeight,
        miniWidth,
        miniHeight
      );
    }
  }

  draw(discardNotify: boolean = false): void {
    this.context2D.clearRect(
      this.frame.x - MarkPadding,
      this.frame.y,
      this.frame.width + MarkPadding * 2,
      this.frame.height
    );
    this.context2D.beginPath();
    if (this._cpuUsage.length > 0) {
      this.drawCpuUsage();
      this.context2D.globalAlpha = 0;
    } else {
      this.context2D.globalAlpha = 1;
    }
    //绘制选中区域
    if (this.drawMark) {
      this.drawSelectionRange();
    }
    if (this.notifyHandler) {
      this.range.startX = this.rangeRect.x;
      this.range.endX = this.rangeRect.x + this.rangeRect.width;
      this.range.startNS = (this.range.startX * this.range.totalNS) / (this.canvas?.clientWidth || 0);
      this.range.endNS = (this.range.endX * this.range.totalNS) / (this.canvas?.clientWidth || 0);
      let l20 = (this.range.endNS - this.range.startNS) / 20;
      let minScale = 0;
      let maxScale = 0;
      let weight = 0;
      for (let scalesIndex = 0; scalesIndex < this.scales.length; scalesIndex++) {
        if (this.scales[scalesIndex] > l20) {
          if (scalesIndex > 0) {
            minScale = this.scales[scalesIndex - 1];
          } else {
            minScale = 0;
          }
          maxScale = this.scales[scalesIndex];
          weight = ((l20 - minScale) * 1.0) / (maxScale - minScale);
          if (weight > 0.243) {
            this.scale = maxScale;
          } else {
            this.scale = minScale;
          }
          break;
        }
      }
      if (this.scale === 0) {
        this.scale = this.scales[0];
      }
      let tempNs = 0;
      let rangeStartX = 0;
      let rangeYu = this.range.startNS % this.scale;
      let rangeRealW = (this.scale * this.frame.width) / (this.range.endNS - this.range.startNS);
      if (this.range.xsTxt) {
        this.range.xsTxt.length = 0;
      } else {
        this.range.xsTxt = [];
      }
      if (this.range.xs) {
        this.range.xs.length = 0;
      } else {
        this.range.xs = [];
      }
      this.range.scale = this.scale;
      if (rangeYu !== 0) {
        let first_NodeWidth = ((this.scale - rangeYu) / this.scale) * rangeRealW;
        rangeStartX += first_NodeWidth;
        tempNs += rangeYu;
        this.range.xs.push(rangeStartX);
        this.range.xsTxt.push(ns2UnitS(tempNs + this.range.startNS, this.scale));
      }
      while (tempNs < this.range.endNS - this.range.startNS) {
        rangeStartX += rangeRealW;
        tempNs += this.scale;
        this.range.xs.push(rangeStartX);
        this.range.xsTxt.push(ns2UnitS(tempNs + this.range.startNS, this.scale));
      }
      if (!discardNotify) {
        this.notifyHandler(this.range);
      }
    }
  }

  private drawSelectionRange(): void {
    this.context2D.fillStyle = window.getComputedStyle(this.canvas!, null).getPropertyValue('background-color');
    this.rangeRect.x = this.markAObj.frame.x < this.markBObj.frame.x ? this.markAObj.frame.x : this.markBObj.frame.x;
    this.rangeRect.width = Math.abs(this.markBObj.frame.x - this.markAObj.frame.x);
    this.context2D.fillRect(this.rangeRect.x, this.rangeRect.y, this.rangeRect.width, this.rangeRect.height);
    this.context2D.globalAlpha = 1;
    this.context2D.globalAlpha = 0.5;
    this.context2D.fillStyle = '#999999';
    this.context2D.fillRect(this.frame.x, this.frame.y, this.rangeRect.x, this.rangeRect.height);
    this.context2D.fillRect(
      this.rangeRect.x + this.rangeRect.width,
      this.frame.y,
      this.frame.width - this.rangeRect.width,
      this.rangeRect.height
    );
    this.context2D.globalAlpha = 1;
    this.context2D.closePath();
    this.markAObj.draw();
    this.markBObj.draw();
  }

  getScale(): number {
    return this.scale;
  }

  mouseDown(mouseEventDown: MouseEvent) {
    let mouseDown_x = mouseEventDown.offsetX - (this.canvas?.offsetLeft || 0);
    let mouseDown_y = mouseEventDown.offsetY - (this.canvas?.offsetTop || 0);
    this.isMouseDown = true;
    this.mouseDownOffsetX = mouseDown_x;
    if (this.markAObj.isHover) {
      this.movingMark = this.markAObj;
      this.mouseDownMovingMarkX = this.movingMark.frame.x || 0;
    } else if (this.markBObj.isHover) {
      this.movingMark = this.markBObj;
      this.mouseDownMovingMarkX = this.movingMark.frame.x || 0;
    } else {
      this.movingMark = null;
    }
    if (this.rangeRect.containsWithPadding(mouseDown_x, mouseDown_y, 5, 0)) {
      this.isMovingRange = true;
      this.markAX = this.markAObj.frame.x;
      this.markBX = this.markBObj.frame.x;
      document.body.style.cursor = 'move';
    } else if (
      this.frame.containsWithMargin(mouseDown_x, mouseDown_y, 20, 0, 0, 0) &&
      !this.rangeRect.containsWithMargin(mouseDown_x, mouseDown_y, 0, MarkPadding, 0, MarkPadding)
    ) {
      this.isNewRange = true;
    }
  }

  mouseUp(ev: MouseEvent) {
    this.isMouseDown = false;
    this.isMovingRange = false;
    this.isNewRange = false;
    this.movingMark = null;
  }

  mouseMove(ev: MouseEvent, trace: SpSystemTrace) {
    this.range.refresh = false;
    let move_x = ev.offsetX - (this.canvas?.offsetLeft || 0);
    let move_y = ev.offsetY - (this.canvas?.offsetTop || 0);
    this.centerXPercentage = move_x / (this.canvas?.clientWidth || 0);
    if (this.centerXPercentage <= 0) {
      this.centerXPercentage = 0;
    } else if (this.centerXPercentage >= 1) {
      this.centerXPercentage = 1;
    }
    let maxX = this.canvas?.clientWidth || 0;
    if (this.markAObj.inspectionFrame.contains(move_x, move_y)) {
      this.markAObj.isHover = true;
    } else if (this.markBObj.inspectionFrame.contains(move_x, move_y)) {
      this.markBObj.isHover = true;
    } else {
      this.markAObj.isHover = false;
      this.markBObj.isHover = false;
    }
    if (this.movingMark) {
      let result = move_x - this.mouseDownOffsetX + this.mouseDownMovingMarkX;
      if (result >= 0 && result <= maxX) {
        this.movingMark.frame.x = result;
      } else if (result < 0) {
        this.movingMark.frame.x = 0;
      } else {
        this.movingMark.frame.x = maxX;
      }
      this.movingMark.inspectionFrame.x = this.movingMark.frame.x - MarkPadding;
      this.recordMovingS();
      requestAnimationFrame(() => {
        this.draw();
        this.range.refresh = false;
        this.delayDraw();
      });
    } else if (this.rangeRect.containsWithPadding(move_x, move_y, MarkPadding, 0)) {
      trace.style.cursor = 'move';
      document.body.style.cursor = 'move';
    } else if (
      this.frame.containsWithMargin(move_x, move_y, 20, 0, 0, 0) &&
      !this.rangeRect.containsWithMargin(move_x, move_y, 0, MarkPadding, 0, MarkPadding)
    ) {
      trace.style.cursor = 'crosshair';
      document.body.style.cursor = 'crosshair';
    }
    if (this.isMovingRange && this.isMouseDown) {
      let result = move_x - this.mouseDownOffsetX;
      let mA = result + this.markAX;
      let mB = result + this.markBX;
      if (mA >= 0 && mA <= maxX) {
        this.markAObj.frame.x = mA;
      } else if (mA < 0) {
        this.markAObj.frame.x = 0;
      } else {
        this.markAObj.frame.x = maxX;
      }
      this.markAObj.inspectionFrame.x = this.markAObj.frame.x - MarkPadding;
      if (mB >= 0 && mB <= maxX) {
        this.markBObj.frame.x = mB;
      } else if (mB < 0) {
        this.markBObj.frame.x = 0;
      } else {
        this.markBObj.frame.x = maxX;
      }
      this.markBObj.inspectionFrame.x = this.markBObj.frame.x - MarkPadding;
      this.recordMovingS();
      requestAnimationFrame(() => {
        this.draw();
        this.range.refresh = false;
        this.delayDraw();
      });
    } else if (this.isNewRange) {
      this.markAObj.frame.x = this.mouseDownOffsetX;
      this.markAObj.inspectionFrame.x = this.mouseDownOffsetX - MarkPadding;
      if (move_x >= 0 && move_x <= maxX) {
        this.markBObj.frame.x = move_x;
      } else if (move_x < 0) {
        this.markBObj.frame.x = 0;
      } else {
        this.markBObj.frame.x = maxX;
      }
      this.markBObj.inspectionFrame.x = this.markBObj.frame.x - MarkPadding;
      this.recordMovingS();
      requestAnimationFrame(() => {
        this.draw();
        this.range.refresh = false;
        this.delayDraw();
      });
    }
  }

  recordMovingS() {
    if (this.animaStartTime == undefined) {
      let dat = new Date();
      dat.setTime(dat.getTime() - 400);
      this.animaStartTime = dat.getTime();
    }
    this.currentDuration = new Date().getTime() - this.animaStartTime;
    this.setCacheInterval();
    this.range.refresh = this.cacheInterval.flag;
  }

  setCacheInterval() {
    if (Math.trunc(this.currentDuration / this.cacheInterval.interval) !== this.cacheInterval.value) {
      this.cacheInterval.flag = true;
      this.cacheInterval.value = Math.trunc(this.currentDuration / this.cacheInterval.interval);
    } else {
      this.cacheInterval.flag = false;
    }
  }

  delayDraw() {
    if (this.delayTimer) {
      clearTimeout(this.delayTimer);
    }
    this.delayTimer = setTimeout(() => {
      this.range.refresh = true;
      this.draw();
      this.range.refresh = false;
      this.animaStartTime = undefined;
    }, this.cacheInterval.interval + 50);
  }

  mouseOut(ev: MouseEvent) {
    this.movingMark = null;
  }

  fillX() {
    if (this.range.endNS < 0) this.range.endNS = 0;
    if (this.range.startNS < 0) this.range.startNS = 0;
    if (this.range.endNS > this.range.totalNS) this.range.endNS = this.range.totalNS;
    if (this.range.startNS > this.range.totalNS) this.range.startNS = this.range.totalNS;
    this.range.startX = (this.range.startNS * (this.canvas?.clientWidth || 0)) / this.range.totalNS;
    this.range.endX = (this.range.endNS * (this.canvas?.clientWidth || 0)) / this.range.totalNS;
    this.markAObj.frame.x = this.range.startX;
    this.markAObj.inspectionFrame.x = this.markAObj.frame.x - MarkPadding;
    this.markBObj.frame.x = this.range.endX;
    this.markBObj.inspectionFrame.x = this.markBObj.frame.x - MarkPadding;
  }

  setRangeNS(startNS: number, endNS: number) {
    this.range.startNS = startNS;
    this.range.endNS = endNS;
    this.fillX();
    this.draw();
  }

  getRange(): TimeRange {
    return this.range;
  }

  cancelPressFrame(): void {
    if (this.pressFrameIdA !== -1) cancelAnimationFrame(this.pressFrameIdA);
    if (this.pressFrameIdD !== -1) cancelAnimationFrame(this.pressFrameIdD);
    if (this.pressFrameIdW !== -1) cancelAnimationFrame(this.pressFrameIdW);
    if (this.pressFrameIdS !== -1) cancelAnimationFrame(this.pressFrameIdS);
    if (this.pressFrameIdF !== -1) cancelAnimationFrame(this.pressFrameIdF);
  }

  cancelUpFrame(): void {
    if (this.upFrameIdA !== -1) cancelAnimationFrame(this.upFrameIdA);
    if (this.upFrameIdD !== -1) cancelAnimationFrame(this.upFrameIdD);
    if (this.upFrameIdW !== -1) cancelAnimationFrame(this.upFrameIdW);
    if (this.upFrameIdS !== -1) cancelAnimationFrame(this.upFrameIdS);
  }

  cancelTimeOut: any = undefined;

  keyPress(keyboardEvent: KeyboardEvent, currentSlicesTime?: CurrentSlicesTime) {
    if (currentSlicesTime) {
      this.currentSlicesTime = currentSlicesTime;
    }
    if (
      this.animaStartTime == undefined ||
      (this.pressedKeys.length > 0 &&
        this.pressedKeys[this.pressedKeys.length - 1] !== keyboardEvent.key.toLocaleLowerCase())
    ) {
      let dat = new Date();
      dat.setTime(dat.getTime() - 400);
      this.animaStartTime = dat.getTime();
    }
    this.currentDuration = new Date().getTime() - this.animaStartTime;
    this.setCacheInterval();
    this.range.refresh = this.cacheInterval.flag;
    if (this.pressedKeys.length > 0) {
      if (this.pressedKeys[this.pressedKeys.length - 1] !== keyboardEvent.key.toLocaleLowerCase()) {
        this.cancelPressFrame();
        this.cancelUpFrame();
        this.pressedKeys.push(keyboardEvent.key.toLocaleLowerCase());
        let date = new Date();
        date.setTime(date.getTime() - 400);
        this.animaStartTime = date.getTime();
        this.keyboardKeyPressMap[this.pressedKeys[this.pressedKeys.length - 1]]?.bind(this)();
      }
    } else {
      this.cancelPressFrame();
      this.cancelUpFrame();
      this.pressedKeys.push(keyboardEvent.key.toLocaleLowerCase());
      let dat = new Date();
      dat.setTime(dat.getTime() - 400);
      this.animaStartTime = dat.getTime();
      this.keyboardKeyPressMap[this.pressedKeys[this.pressedKeys.length - 1]]?.bind(this)();
    }
    this.isPress = true;
    if (this.cancelTimeOut) {
      clearTimeout(this.cancelTimeOut);
    }
    this.cancelTimeOut = setTimeout(() => {
      this.keyUp({ key: keyboardEvent.key } as KeyboardEvent);
    }, 1000);
  }

  keyPressF(): void {
    const DIS = 100;
    let clientWidth = this.canvas?.clientWidth || 0;
    let midX = Math.round(clientWidth / 2);
    let sliceMidX = 0;
    let startTime = 0;
    let endTime = 0;
    this.rulerW = this.canvas!.offsetWidth;
    if (this.currentSlicesTime.startTime) {
      startTime = this.currentSlicesTime.startTime;
    } else {
      return;
    }
    if (this.currentSlicesTime.endTime) {
      endTime = this.currentSlicesTime.endTime;
    } else {
      return;
    }
    let startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
    let endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
    let totalX = 0;
    totalX = Math.round(endX - startX);
    sliceMidX = Math.round(startX + totalX / 2);
    let count1 = 0;
    let count2 = 0;
    let animF = (): void => {
      let sliceMidXMap = new Map();
      startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
      endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
      totalX = endX - startX;
      sliceMidX = Math.round(startX + totalX / 2);
      let x = 1;
      do {
        startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
        endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
        totalX = endX - startX;
        sliceMidX = Math.round(startX + (endX - startX) / 2);
        if (totalX === 0) {
          return;
        }
        // 修正sliceMidX的值
        if (sliceMidX !== sliceMidXMap.get('tempMid1') || (0 && sliceMidX !== sliceMidXMap.get('tempMid2')) || 0) {
          if (x % 2 === 0) {
            sliceMidXMap.set('tempMid2', sliceMidX); // 偶数
          } else {
            sliceMidXMap.set('tempMid1', sliceMidX); // 奇数
          }
        } else {
          if (sliceMidX === sliceMidXMap.get('tempMid1')) {
            count1++;
          }
          if (sliceMidX === sliceMidXMap.get('tempMid2')) {
            count2++;
          }
          // 此处为了解决特殊情况下sliceMidX的值非常接近中间区域，但是又无法进入中间区域，导致死循环程序无法正常终止。
          if ((count1 >= 3 && count1 <= 5) || (count2 >= 3 && count2 <= 5)) {
            if (sliceMidX >= midX - MID_OFFSET && sliceMidX <= midX + MID_OFFSET) {
              let tempMid1 = sliceMidXMap.get('tempMid1') || 0;
              let tempMid2 = sliceMidXMap.get('tempMid2') || 0;
              if (sliceMidX === tempMid1 && tempMid1 === tempMid2) {
                sliceMidX += 11;
              }
              if (sliceMidX === tempMid1 && sliceMidX !== tempMid2) {
                if (sliceMidX < tempMid2) {
                  sliceMidX += 15;
                } else {
                  sliceMidX -= 10;
                }
              }
              if (sliceMidX === tempMid2 && sliceMidX !== tempMid1) {
                if (sliceMidX < tempMid1) {
                  sliceMidX += 15;
                } else {
                  sliceMidX -= 10;
                }
              }
            }
          }
          if (count1 > 5 || count2 > 5) {
            this.fillX();
            this.range.refresh = true;
            this.notifyHandler(this.range);
            this.range.refresh = false;
            return;
          }
        }
        x++;
        if (sliceMidX >= midX - MID_OFFSET && sliceMidX <= midX + MID_OFFSET) {
          /*  把 endNS 转换为 endX ， startNS 转化 startX，
              totalX = ( endX - startX )  280px <= totalX  <= 300px
              此时，如果slice的比例或者宽度不合适，则进行调整校正，缩放到合适的比例。
              不能使用固定的 scale， 因为调整slice的宽度时，scale、 startNS 和  endNS 都在变化,
              所以要使用  totalX 来判断slice是否缩放到合适的大小了。
          */
          if (
            (totalX < FIT_TOTALX_MIN - MID_OFFSET && totalX > 0) ||
            Math.round(totalX) > FIT_TOTALX_MAX + MID_OFFSET
          ) {
            this.zoomFit(startTime, endTime);
            this.fillX();
            this.range.refresh = true;
            this.notifyHandler(this.range);
            this.range.refresh = false;
          }
          startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
          endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
          totalX = endX - startX;
          sliceMidX = Math.round(startX + (endX - startX) / 2);
          if (
            sliceMidX >= midX - MID_OFFSET &&
            sliceMidX <= midX + MID_OFFSET &&
            Math.round(totalX) >= FIT_TOTALX_MIN - MID_OFFSET &&
            Math.round(totalX) <= FIT_TOTALX_MAX + MID_OFFSET
          ) {
            this.fillX();
            this.range.refresh = true;
            this.notifyHandler(this.range);
            this.range.refresh = false;
            return;
          }
        } else {
          // 0.2经验值，不要随便调整这个系数，该系数决定了调整后的slice中间位置是否能落在FIT_TOTALX_MIN和FIT_TOTALX_MAX之间。
          let s = (this.scale / this.p) * this.currentDuration * 0.2; // 微调
          let big_s = (this.scale / this.p) * this.currentDuration * 1.2; // 大幅度调整
          let biger = (this.scale / this.p) * this.currentDuration * 10; // 更大幅度调整
          let huge = (this.scale / this.p) * this.currentDuration * 100; // 巨大幅度调整
          startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
          endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
          totalX = endX - startX;
          sliceMidX = Math.round(startX + (endX - startX) / 2);
          // slice中间位置大于画布的中间坐标时画布右移 ，startNS + s
          if (sliceMidX > midX + MID_OFFSET) {
            let distance = sliceMidX - midX;
            if (distance >= 10000) {
              this.range.startNS += huge;
              this.range.endNS += huge;
            } else if (distance >= 1000 && distance < 10000) {
              this.range.startNS += biger;
              this.range.endNS += biger;
            } else if (distance > DIS && distance < 1000) {
              this.range.startNS += big_s;
              this.range.endNS += big_s;
            } else {
              this.range.startNS += s;
              this.range.endNS += s;
            }
          }
          // slice中间位置小于画布的中间坐标时画布左移， startNS - s
          if (sliceMidX < midX - MID_OFFSET) {
            let distance = midX - sliceMidX; // 28.5
            if (distance >= 10000) {
              this.range.startNS -= huge;
              this.range.endNS -= huge;
            } else if (distance >= 1000 && distance < 10000) {
              this.range.startNS -= biger;
              this.range.endNS -= biger;
            } else if (distance > DIS && distance < 1000) {
              this.range.startNS -= big_s;
              this.range.endNS -= big_s;
            } else {
              this.range.startNS -= s;
              this.range.endNS -= s;
            }
          }
        }
        this.fillX();
        this.draw();
        this.range.refresh = false;
      } while (
        sliceMidX < midX - MID_OFFSET ||
        sliceMidX > midX + MID_OFFSET ||
        Math.round(totalX) < FIT_TOTALX_MIN - MID_OFFSET ||
        Math.round(totalX) > FIT_TOTALX_MAX + MID_OFFSET
      );
      this.pressFrameIdF = requestAnimationFrame(animF);
    };
    if ((totalX < FIT_TOTALX_MIN - MID_OFFSET && totalX > 0) || totalX > FIT_TOTALX_MAX + MID_OFFSET) {
      this.zoomFit(startTime, endTime);
      this.pressFrameIdF = requestAnimationFrame(animF);
    }
    this.pressFrameIdF = requestAnimationFrame(animF);
    this.zoomFit(startTime, endTime);
  }
  keyPressW() {
    let animW = () => {
      if (this.scale === 50) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      this.range.startNS += (this.centerXPercentage * this.currentDuration * this.scale) / this.p;
      this.range.endNS -= ((1 - this.centerXPercentage) * this.currentDuration * this.scale) / this.p;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      this.pressFrameIdW = requestAnimationFrame(animW);
    };
    this.pressFrameIdW = requestAnimationFrame(animW);
  }

  keyPressS() {
    let animS = () => {
      if (this.range.startNS <= 0 && this.range.endNS >= this.range.totalNS) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      this.range.startNS -= ((this.centerXPercentage * this.scale) / this.p) * this.currentDuration;
      this.range.endNS += (((1 - this.centerXPercentage) * this.scale) / this.p) * this.currentDuration;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      this.pressFrameIdS = requestAnimationFrame(animS);
    };
    this.pressFrameIdS = requestAnimationFrame(animS);
  }

  keyPressA() {
    let animA = () => {
      if (this.range.startNS <= 0) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      let s = (this.scale / this.p) * this.currentDuration * 0.4;
      this.range.startNS -= s;
      this.range.endNS -= s;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      this.pressFrameIdA = requestAnimationFrame(animA);
    };
    this.pressFrameIdA = requestAnimationFrame(animA);
  }

  keyPressD() {
    let animD = () => {
      if (this.range.endNS >= this.range.totalNS) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      let s = (this.scale / this.p) * this.currentDuration * 0.4;
      this.range.startNS += s;
      this.range.endNS += s;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      this.pressFrameIdD = requestAnimationFrame(animD);
    };
    this.pressFrameIdD = requestAnimationFrame(animD);
  }

  keyboardKeyPressMap: any = {
    w: this.keyPressW,
    s: this.keyPressS,
    a: this.keyPressA,
    d: this.keyPressD,
    f: this.keyPressF,
  };

  keyboardKeyUpMap: any = {
    w: this.keyUpW,
    s: this.keyUpS,
    a: this.keyUpA,
    d: this.keyUpD,
  };

  keyUp(ev: KeyboardEvent) {
    this.cacheInterval.value = 0;
    if (this.pressedKeys.length > 0) {
      let number = this.pressedKeys.findIndex((value) => value === ev.key.toLocaleLowerCase());
      if (number == this.pressedKeys.length - 1) {
        this.animaStartTime = undefined;
        this.cancelPressFrame();
        this.keyboardKeyUpMap[ev.key]?.bind(this)();
      }
      if (number != -1) {
        this.pressedKeys.splice(number, 1);
      }
    }
    this.isPress = false;
  }

  keyUpW() {
    let startTime = new Date().getTime();
    let animW = () => {
      if (this.scale === 50) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      let dur = new Date().getTime() - startTime;
      this.range.startNS += (this.centerXPercentage * 100 * this.scale) / this.p;
      this.range.endNS -= ((1 - this.centerXPercentage) * 100 * this.scale) / this.p;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      if (dur < 100) {
        this.upFrameIdW = requestAnimationFrame(animW);
      } else {
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
      }
    };
    this.upFrameIdW = requestAnimationFrame(animW);
  }

  keyUpS(): void {
    let startTime = new Date().getTime();
    let animS = () => {
      if (this.range.startNS <= 0 && this.range.endNS >= this.range.totalNS) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      let dur = new Date().getTime() - startTime;
      this.range.startNS -= (this.centerXPercentage * 100 * this.scale) / this.p;
      this.range.endNS += ((1 - this.centerXPercentage) * 100 * this.scale) / this.p;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      if (dur < 100) {
        this.upFrameIdS = requestAnimationFrame(animS);
      } else {
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
      }
    };
    this.upFrameIdS = requestAnimationFrame(animS);
  }

  keyUpA(): void {
    let startTime = new Date().getTime();
    let animA = () => {
      if (this.range.startNS <= 0) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      let dur = new Date().getTime() - startTime;
      let s = (this.scale * 80) / this.p;
      this.range.startNS -= s;
      this.range.endNS -= s;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      if (dur < 100) {
        this.upFrameIdA = requestAnimationFrame(animA);
      } else {
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
      }
    };
    this.upFrameIdA = requestAnimationFrame(animA);
  }

  keyUpD(): void {
    let startTime = new Date().getTime();
    let animD = () => {
      if (this.range.endNS >= this.range.totalNS) {
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      let dur = new Date().getTime() - startTime;
      let s = (this.scale * 80) / this.p;
      this.range.startNS += s;
      this.range.endNS += s;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      if (dur < 100) {
        this.upFrameIdD = requestAnimationFrame(animD);
      } else {
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
      }
    };
    this.upFrameIdD = requestAnimationFrame(animD);
  }

  translate(distance: number): void {
    const rangeDur = this.range.endNS - this.range.startNS;
    const time = (distance / this.canvas!.width) * rangeDur;
    if (
      this.range.startNS < 0 ||
      this.range.endNS < 0 ||
      this.range.startNS > this.range.totalNS ||
      this.range.endNS > this.range.totalNS
    ) {
      return;
    }
    this.range.startNS -= time;
    this.range.endNS -= time;
    this.fillX();
    this.draw();
    this.range.refresh = true;
    this.notifyHandler(this.range);
    this.range.refresh = false;
  }

  zoomFit(startTime: number, endTime: number): void {
    let startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
    let endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
    let totalX = endX - startX;
    this.centerXPercentage = 0.5;
    let clientWidth = this.canvas?.clientWidth || 0; // 1424
    let midX = Math.round(clientWidth / 2);
    this.currentDuration = 1000;

    let totalXMap = new Map();
    let count1 = 0;
    let count2 = 0;
    let animW = () => {
      startX = Math.round((this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      endX = Math.round((this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      totalX = endX - startX;
      if (Math.round(totalX) !== totalXMap.get('totalX1')) {
        totalXMap.set('totalX1', Math.round(totalX));
      } else {
        if (Math.round(totalX) === totalXMap.get('totalX1')) {
          count1++;
        }
      }
      if (count1 >= 3 && count1 <= 5) {
        if (Math.round(totalX) >= FIT_TOTALX_MIN - MID_OFFSET && Math.round(totalX) <= FIT_TOTALX_MAX + MID_OFFSET) {
          let tempTotalX1 = totalXMap.get('totalX1') || 0;
          let tempTotalX2 = totalXMap.get('totalX2') || 0;
          if (Math.round(totalX) === tempTotalX1 && tempTotalX2 === tempTotalX2) {
            totalX += 11;
          }
          if (Math.round(totalX) === tempTotalX1 && Math.round(totalX) !== tempTotalX2) {
            if (Math.round(totalX) < tempTotalX2) {
              totalX += 12;
            } else {
              totalX -= 11;
            }
          }
          if (Math.round(totalX) === tempTotalX2 && Math.round(totalX) !== tempTotalX1) {
            if (Math.round(totalX) < tempTotalX1) {
              totalX += 12;
            } else {
              totalX -= 11;
            }
          }
        }
      }
      // 此处为了解决特殊情况下sliceMidX的值非常接近中间区域，但是又无法进入中间区域，导致死循环程序无法正常终止。
      if (count1 > 5) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        // 如果出现循环缩放，则退出。
        return;
      }
      if (totalX >= FIT_TOTALX_MIN - MID_OFFSET && totalX <= FIT_TOTALX_MAX + MID_OFFSET) {
        let sliceMidX = Math.round(startX + totalX / 2);
        if (sliceMidX < midX - MID_OFFSET || sliceMidX > midX + MID_OFFSET) {
          this.keyPressF();
        } else {
          this.fillX();
          this.range.refresh = true;
          this.notifyHandler(this.range);
          this.range.refresh = false;
          return;
        }
      }
      if (this.scale === 50) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      this.range.startNS += (this.centerXPercentage * this.currentDuration * this.scale) / this.p;
      this.range.endNS -= ((1 - this.centerXPercentage) * this.currentDuration * this.scale) / this.p;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      startX = (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
      endX = (this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS);
      totalX = Math.round(endX - startX);
      if (totalX >= FIT_TOTALX_MAX + MID_OFFSET) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      this.pressFrameIdW = requestAnimationFrame(animW);
    };
    let animS = () => {
      startX = Math.round((this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      endX = Math.round((this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      totalX = endX - startX;
      if (Math.round(totalX) !== totalXMap.get('totalX2')) {
        totalXMap.set('totalX2', Math.round(totalX));
      } else {
        if (Math.round(totalX) === totalXMap.get('totalX2')) {
          count2++;
        }
      }
      if (count2 >= 3 && count2 <= 5) {
        if (Math.round(totalX) >= FIT_TOTALX_MIN - MID_OFFSET && Math.round(totalX) <= FIT_TOTALX_MAX + MID_OFFSET) {
          let tempTotalX1 = totalXMap.get('totalX1') || 0;
          let tempTotalX2 = totalXMap.get('totalX2') || 0;
          if (Math.round(totalX) === tempTotalX1 && tempTotalX2 === tempTotalX2) {
            totalX += 11;
          }
          if (Math.round(totalX) === tempTotalX1 && Math.round(totalX) !== tempTotalX2) {
            if (Math.round(totalX) < tempTotalX2) {
              totalX += 12;
            } else {
              totalX -= 11;
            }
          }
          if (Math.round(totalX) === tempTotalX2 && Math.round(totalX) !== tempTotalX1) {
            if (Math.round(totalX) < tempTotalX1) {
              totalX += 12;
            } else {
              totalX -= 11;
            }
          }
        }
      }
      if (count2 > 5) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        // 如果出现循环缩放，则退出。
        return;
      }
      if (totalX >= FIT_TOTALX_MIN - MID_OFFSET && totalX <= FIT_TOTALX_MAX + MID_OFFSET) {
        let sliceMidX = Math.round(startX + totalX / 2);
        if (sliceMidX < midX - MID_OFFSET || sliceMidX > midX + MID_OFFSET) {
          this.keyPressF();
        } else {
          this.fillX();
          this.range.refresh = true;
          this.notifyHandler(this.range);
          this.range.refresh = false;
          return;
        }
      }
      this.range.startNS -= ((this.centerXPercentage * this.scale) / this.p) * this.currentDuration;
      this.range.endNS += (((1 - this.centerXPercentage) * this.scale) / this.p) * this.currentDuration;
      this.fillX();
      this.draw();
      this.range.refresh = false;
      startX = Math.round((this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      endX = Math.round((this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      totalX = Math.round(endX - startX);
      if (totalX <= FIT_TOTALX_MIN - MID_OFFSET) {
        this.fillX();
        this.range.refresh = true;
        this.notifyHandler(this.range);
        this.range.refresh = false;
        return;
      }
      this.pressFrameIdS = requestAnimationFrame(animS);
    };
    if (Math.round(totalX) > FIT_TOTALX_MAX + MID_OFFSET) {
      this.pressFrameIdS = requestAnimationFrame(animS);
    }
    if (totalX < FIT_TOTALX_MIN - MID_OFFSET) {
      this.pressFrameIdW = requestAnimationFrame(animW);
    }
  }
}
