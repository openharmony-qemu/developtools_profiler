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

import { ns2s, Rect, Render, RequestMessage } from './ProcedureWorkerCommon.js';
import { ColorUtils } from '../../component/trace/base/ColorUtils.js';
import { TraceRow } from '../../component/trace/base/TraceRow.js';
import { CpuStruct } from './ProcedureWorkerCPU.js';

//绘制时间轴
let timeRuler: TimeRuler | undefined;
let rangeRuler: RangeRuler | undefined;
let sportRuler: SportRuler | undefined;
let offsetTop: number = 0;
let offsetLeft: number = 0;

export class TimelineRender extends Render {
  renderMainThread(req: any, row: TraceRow<any>) {}
  render(req: RequestMessage, list: Array<any>, filter: Array<any>) {
    timeline(
      req.canvas,
      req.context,
      req.startNS,
      req.endNS,
      req.totalNS,
      req.frame,
      req.params.keyPressCode,
      req.params.keyUpCode,
      req.params.mouseDown,
      req.params.mouseUp,
      req.params.mouseMove,
      req.params.mouseOut,
      req.params.offsetLeft,
      req.params.offsetTop,
      (a: any) => {
        //@ts-ignore
        self.postMessage({
          id: 'timeline',
          type: 'timeline-range-changed',
          results: a,
        });
      }
    );
    // @ts-ignore
    self.postMessage({
      id: req.id,
      type: req.type,
      results: null,
    });
  }
}

export function timeline(
  // @ts-ignore
  canvas: OffscreenCanvas,
  // @ts-ignore
  ctx: OffscreenCanvasRenderingContext2D,
  startNS: number,
  endNS: number,
  totalNS: number,
  frame: Rect,
  keyPressCode: any,
  keyUpCode: any,
  mouseDown: any,
  mouseUp: any,
  mouseMove: any,
  mouseOut: any,
  _offsetLeft: number,
  _offsetTop: number,
  changeHandler: Function
) {
  offsetLeft = _offsetLeft;
  offsetTop = _offsetTop;
  if (timeRuler == undefined) {
    timeRuler = new TimeRuler(canvas, ctx, new Rect(0, 0, frame.width, 20), totalNS);
  }
  if (!sportRuler) {
    sportRuler = new SportRuler(canvas, ctx, new Rect(0, 100.5, frame.width, frame.height - 100));
  }
  if (!rangeRuler) {
    rangeRuler = new RangeRuler(
      canvas,
      ctx!,
      new Rect(0, 25, frame.width, 75),
      {
        startX: 0,
        endX: frame.width,
        startNS: 0,
        endNS: totalNS,
        totalNS: totalNS,
        xs: [],
        xsTxt: [],
      },
      (a) => {
        if (sportRuler) {
          sportRuler.range = a;
        }
        changeHandler(a);
      }
    );
  }

  rangeRuler.frame.width = frame.width;
  sportRuler.frame.width = frame.width;
  timeRuler.frame.width = frame.width;
  if (keyPressCode) {
    rangeRuler.keyPress(keyPressCode);
  } else if (keyUpCode) {
    rangeRuler.keyUp(keyUpCode);
  } else if (mouseDown) {
    rangeRuler.mouseDown(mouseDown);
  } else if (mouseUp) {
    rangeRuler.mouseUp(mouseUp);
  } else if (mouseMove) {
    rangeRuler.mouseMove(mouseMove);
  } else if (mouseOut) {
    rangeRuler.mouseOut(mouseOut);
  } else {
    timeRuler.draw();
    rangeRuler.draw();
  }
}

export abstract class Graph {
  // @ts-ignore
  c: OffscreenCanvasRenderingContext2D;
  // @ts-ignore
  canvas: OffscreenCanvas | undefined | null;
  frame: Rect;

  protected constructor(
    // @ts-ignore
    canvas: OffscreenCanvas | undefined | null,
    // @ts-ignore
    c: OffscreenCanvasRenderingContext2D,
    frame: Rect
  ) {
    this.canvas = canvas;
    this.frame = frame;
    this.c = c;
  }

  abstract draw(): void;
}

export class TimeRuler extends Graph {
  totalNS: number;
  private stepSmall: number;
  private step: number;
  private stepNS: number;

  constructor(
    // @ts-ignore
    canvas: OffscreenCanvas | undefined | null,
    // @ts-ignore
    c: OffscreenCanvasRenderingContext2D,
    frame: Rect,
    totalNS: number = 10_000_000_000
  ) {
    super(canvas, c, frame);
    this.totalNS = totalNS;
    this.step = this.frame.width / 10;
    this.stepSmall = this.frame.width / 100;
    this.stepNS = this.totalNS / 10;
  }

  draw() {
    this.step = this.frame.width / 10;
    this.stepSmall = this.frame.width / 100;
    this.stepNS = this.totalNS / 10;
    this.c.clearRect(this.frame.x, this.frame.y, this.frame.width, this.frame.height);
    this.c.beginPath();
    this.c.strokeStyle = '#999';
    this.c.lineWidth = 1;
    for (let i = 0; i <= 10; i++) {
      let x = Math.floor(i * this.step) + this.frame.x;
      this.c.moveTo(x, 0);
      this.c.lineTo(x, this.frame.height);
      if (i == 10) break;
      for (let j = 1; j < 10; j++) {
        this.c.moveTo(x + Math.floor(j * this.stepSmall), 0);
        this.c.lineTo(x + Math.floor(j * this.stepSmall), this.frame.height / 4);
      }
      this.c.fillStyle = '#999';
      this.c.fillText(`${ns2s(i * this.stepNS)}`, x + 5, this.frame.height - 1);
    }
    this.c.stroke();
    this.c.closePath();
  }
}

/**
 * SportRuler
 */
export class SportRuler extends Graph {
  public static rulerFlagObj: Flag | null = null;
  public flagList: Array<Flag> = [];
  public flagListIdx: number | null = null;
  public obj = [{ x: 3 }, { x: 2 }];
  lineColor: string | null = null;
  private rangeFlag = new Flag(0, 0, 0, 0, 0);
  private ruler_w = 1022;
  private _range: TimeRange = {} as TimeRange;

  constructor(
    // @ts-ignore
    canvas: OffscreenCanvas | undefined | null,
    // @ts-ignore
    c: OffscreenCanvasRenderingContext2D,
    frame: Rect
  ) {
    super(canvas, c, frame);
  }

  get range(): TimeRange {
    return this._range;
  }

  set range(value: TimeRange) {
    this._range = value;
    this.draw();
  }

  modifyFlagList(type: string, flag: any = {}) {
    if (type == 'amend') {
      if (flag.text && this.flagListIdx !== null) {
        this.flagList[this.flagListIdx].text = flag.text;
      }
      if (flag.color && this.flagListIdx !== null) {
        this.flagList[this.flagListIdx].color = flag.color;
      }
    } else if (type == 'remove') {
      if (this.flagListIdx !== null) {
        this.flagList.splice(this.flagListIdx, 1);
      }
    }
    this.draw();
  }

  draw(): void {
    this.ruler_w = this.frame.width;
    this.c.clearRect(this.frame.x, this.frame.y, this.frame.width, this.frame.height);
    this.c.beginPath();
    this.lineColor = '#dadada';
    this.c.strokeStyle = this.lineColor; //"#dadada"
    this.c.lineWidth = 1;
    this.c.moveTo(this.frame.x, this.frame.y);
    this.c.lineTo(this.frame.x + this.frame.width, this.frame.y);
    this.c.stroke();
    this.c.closePath();
    this.c.beginPath();
    this.c.lineWidth = 3;
    this.c.strokeStyle = '#999999';
    this.c.moveTo(this.frame.x, this.frame.y);
    this.c.lineTo(this.frame.x, this.frame.y + this.frame.height);
    this.c.stroke();
    this.c.closePath();
    this.c.beginPath();
    this.c.lineWidth = 1;
    this.c.strokeStyle = this.lineColor; //"#999999"
    this.c.fillStyle = '#999999';
    this.c.font = '8px sans-serif';
    this.range.xs?.forEach((it, i) => {
      this.c.moveTo(it, this.frame.y);
      this.c.lineTo(it, this.frame.y + this.frame.height);
      this.c.fillText(`+${this.range.xsTxt[i]}`, it + 3, this.frame.y + 12);
    });

    this.c.stroke();
    this.c.closePath();
  }

  // drawTheFlag
  drawTheFlag(x: number, color: string = '#999999', isFill: boolean = false, text: string = '') {
    this.c.beginPath();
    this.c.fillStyle = color;
    this.c.strokeStyle = color;
    this.c.moveTo(x, 125);
    this.c.lineTo(x + 10, 125);
    this.c.lineTo(x + 10, 127);
    this.c.lineTo(x + 18, 127);
    this.c.lineTo(x + 18, 137);
    this.c.lineTo(x + 10, 137);
    this.c.lineTo(x + 10, 135);
    this.c.lineTo(x + 2, 135);
    this.c.lineTo(x + 2, 143);
    this.c.lineTo(x, 143);
    this.c.closePath();
    if (isFill) {
      this.c.fill();
    }
    this.c.stroke();

    if (text !== '') {
      this.c.font = '10px Microsoft YaHei';
      const { width } = this.c.measureText(text);
      this.c.fillStyle = 'rgba(255, 255, 255, 0.8)'; //
      this.c.fillRect(x + 21, 132, width + 4, 12);
      this.c.fillStyle = 'black';
      this.c.fillText(text, x + 23, 142);
      this.c.stroke();
    }
  }

  //随机生成十六位进制颜色
  randomRgbColor() {
    const letters = '0123456789ABCDEF';
    let color = '#';
    for (let i = 0; i < 6; i++) {
      color += letters[Math.floor(Math.random() * 16)];
    }
    return color;
  }

  //鼠标点击绘画旗子、点击旗子
  mouseUp(ev: MouseEvent) {}

  //选中的旗子
  onFlagRangeEvent(flagObj: Flag, idx: number) {
    SportRuler.rulerFlagObj = flagObj;
    this.flagListIdx = idx;
  }

  //鼠标移动 绘画旗子
  mouseMove(ev: MouseEvent) {
    let x = ev.offsetX - (offsetLeft || 0);
    let y = ev.offsetY - (offsetTop || 0);
    if (y >= 50 && y < 200) {
      this.draw();
      if (y >= 123 && y < 142 && x > 0) {
        let onFlagRange = this.flagList.findIndex((flagObj: Flag) => {
          let flag_x = Math.round(
            (this.ruler_w * (flagObj.time - this.range.startNS)) / (this.range.endNS - this.range.startNS)
          );
          return x >= flag_x && x <= flag_x + 18;
        });
      }
    }
  }
}

const markPadding = 5;

export class Mark extends Graph {
  name: string | undefined;
  inspectionFrame: Rect;
  private _isHover: boolean = false;

  constructor(
    // @ts-ignore
    canvas: OffscreenCanvas | undefined | null,
    name: string,
    // @ts-ignore
    c: OffscreenCanvasRenderingContext2D,
    frame: Rect
  ) {
    super(canvas, c, frame);
    this.name = name;
    this.inspectionFrame = new Rect(frame.x - markPadding, frame.y, frame.width + markPadding * 2, frame.height);
  }

  get isHover(): boolean {
    return this._isHover;
  }

  set isHover(value: boolean) {
    this._isHover = value;
  }

  draw(): void {
    this.c.beginPath();
    this.c.lineWidth = 7;
    this.c.strokeStyle = '#999999';
    this.c.moveTo(this.frame.x, this.frame.y);
    this.c.lineTo(this.frame.x, this.frame.y + this.frame.height / 3);
    this.c.stroke();
    this.c.lineWidth = 1;
    this.c.strokeStyle = '#999999';
    this.c.moveTo(this.frame.x, this.frame.y);
    this.c.lineTo(this.frame.x, this.frame.y + this.frame.height);
    this.c.stroke();
    this.c.closePath();
  }
}

export interface TimeRange {
  totalNS: number;
  startX: number;
  endX: number;
  startNS: number;
  endNS: number;
  xs: Array<number>;
  xsTxt: Array<string>;
}

export class RangeRuler extends Graph {
  public rangeRect: Rect;
  public markA: Mark;
  public markB: Mark;
  public range: TimeRange;
  mouseDownOffsetX = 0;
  mouseDownMovingMarkX = 0;
  movingMark: Mark | undefined | null;
  isMouseDown: boolean = false;
  isMovingRange: boolean = false;
  isNewRange: boolean = false;
  markAX: number = 0;
  markBX: number = 0;
  isPress: boolean = false;
  pressFrameId: number = -1;
  currentDuration: number = 0;
  centerXPercentage: number = 0;
  animaStartTime: number | undefined;
  animTime: number = 100;
  p: number = 800;
  private readonly notifyHandler: (r: TimeRange) => void;
  private scale: number = 0;
  //缩放级别
  private scales: Array<number> = [
    50, 100, 200, 500, 1_000, 2_000, 5_000, 10_000, 20_000, 50_000, 100_000, 200_000, 500_000, 1_000_000, 2_000_000,
    5_000_000, 10_000_000, 20_000_000, 50_000_000, 100_000_000, 200_000_000, 500_000_000, 1_000_000_000, 2_000_000_000,
    5_000_000_000, 10_000_000_000, 20_000_000_000, 50_000_000_000, 100_000_000_000, 200_000_000_000, 500_000_000_000,
  ];
  private _cpuUsage: Array<{ cpu: number; ro: number; rate: number }> = [];

  constructor(
    // @ts-ignore
    canvas: OffscreenCanvas | undefined | null,
    // @ts-ignore
    c: OffscreenCanvasRenderingContext2D,
    frame: Rect,
    range: TimeRange,
    notifyHandler: (r: TimeRange) => void
  ) {
    super(canvas, c, frame);
    this.range = range;
    this.notifyHandler = notifyHandler;
    this.markA = new Mark(canvas, 'A', c, new Rect(range.startX, frame.y, 1, frame.height));
    this.markB = new Mark(canvas, 'B', c, new Rect(range.endX, frame.y, 1, frame.height));
    this.rangeRect = new Rect(range.startX, frame.y, range.endX - range.startX, frame.height);
  }

  set cpuUsage(value: Array<{ cpu: number; ro: number; rate: number }>) {
    this._cpuUsage = value;
    this.draw();
  }

  drawCpuUsage() {
    let maxNum = Math.round(this._cpuUsage.length / 100);
    let miniHeight = Math.round(this.frame.height / CpuStruct.cpuCount); //每格高度
    let miniWidth = Math.ceil(this.frame.width / 100); //每格宽度
    for (let i = 0; i < this._cpuUsage.length; i++) {
      //cpu: 0, ro: 0, rate: 0.987620037556431
      let it = this._cpuUsage[i];
      this.c.fillStyle = ColorUtils.MD_PALETTE[it.cpu];
      this.c.globalAlpha = it.rate;
      this.c.fillRect(this.frame.x + miniWidth * it.ro, this.frame.y + it.cpu * miniHeight, miniWidth, miniHeight);
    }
  }

  draw(discardNotify: boolean = false): void {
    this.c.clearRect(this.frame.x - markPadding, this.frame.y, this.frame.width + markPadding * 2, this.frame.height);
    this.c.beginPath();
    if (this._cpuUsage.length > 0) {
      this.drawCpuUsage();
      this.c.globalAlpha = 0;
    } else {
      this.c.globalAlpha = 1;
    }
    //绘制选中区域
    this.drawRangeSelection();
    if (this.notifyHandler) {
      this.range.startX = this.rangeRect.x;
      this.range.endX = this.rangeRect.x + this.rangeRect.width;
      this.range.startNS = (this.range.startX * this.range.totalNS) / (this.frame.width || 0);
      this.range.endNS = (this.range.endX * this.range.totalNS) / (this.frame.width || 0);
      let l20 = (this.range.endNS - this.range.startNS) / 20;
      let min = 0;
      let max = 0;
      let weight = 0;
      for (let index = 0; index < this.scales.length; index++) {
        if (this.scales[index] > l20) {
          if (index > 0) {
            min = this.scales[index - 1];
          } else {
            min = 0;
          }
          max = this.scales[index];
          weight = ((l20 - min) * 1.0) / (max - min);
          if (weight > 0.243) {
            this.scale = max;
          } else {
            this.scale = min;
          }
          break;
        }
      }
      if (this.scale == 0) {
        this.scale = this.scales[0];
      }
      let tmpNs = 0;
      let timeLineYu = this.range.startNS % this.scale;
      let timeLineRealW = (this.scale * this.frame.width) / (this.range.endNS - this.range.startNS);
      let timeLineStartX = 0;
      if (this.range.xs) {
        this.range.xs.length = 0;
      } else {
        this.range.xs = [];
      }
      if (this.range.xsTxt) {
        this.range.xsTxt.length = 0;
      } else {
        this.range.xsTxt = [];
      }
      if (timeLineYu != 0) {
        let firstNodeWidth = ((this.scale - timeLineYu) / this.scale) * timeLineRealW;
        timeLineStartX += firstNodeWidth;
        tmpNs += timeLineYu;
        this.range.xs.push(timeLineStartX);
        this.range.xsTxt.push(ns2s(tmpNs));
      }
      while (tmpNs < this.range.endNS - this.range.startNS) {
        timeLineStartX += timeLineRealW;
        tmpNs += this.scale;
        this.range.xs.push(timeLineStartX);
        this.range.xsTxt.push(ns2s(tmpNs));
      }
      if (!discardNotify) {
        this.notifyHandler(this.range);
      }
    }
  }

  private drawRangeSelection() {
    this.c.fillStyle = '#ffffff';
    this.rangeRect.x = this.markA.frame.x < this.markB.frame.x ? this.markA.frame.x : this.markB.frame.x;
    this.rangeRect.width = Math.abs(this.markB.frame.x - this.markA.frame.x);
    this.c.fillRect(this.rangeRect.x, this.rangeRect.y, this.rangeRect.width, this.rangeRect.height);
    this.c.globalAlpha = 1;
    this.c.globalAlpha = 0.5;
    this.c.fillStyle = '#999999';
    this.c.fillRect(this.frame.x, this.frame.y, this.rangeRect.x, this.rangeRect.height);
    this.c.fillRect(
      this.rangeRect.x + this.rangeRect.width,
      this.frame.y,
      this.frame.width - this.rangeRect.width,
      this.rangeRect.height
    );
    this.c.globalAlpha = 1;
    this.c.closePath();
    this.markA.draw();
    this.markB.draw();
  }

  mouseDown(ev: MouseEvent) {
    let timeLineMouseDownX = ev.offsetX - (offsetLeft || 0);
    let timeLineMouseDownY = ev.offsetY - (offsetTop || 0);
    this.isMouseDown = true;
    this.mouseDownOffsetX = timeLineMouseDownX;
    if (this.markA.isHover) {
      this.movingMark = this.markA;
      this.mouseDownMovingMarkX = this.movingMark.frame.x || 0;
    } else if (this.markB.isHover) {
      this.movingMark = this.markB;
      this.mouseDownMovingMarkX = this.movingMark.frame.x || 0;
    } else {
      this.movingMark = null;
    }
    if (this.rangeRect.containsWithPadding(timeLineMouseDownX, timeLineMouseDownY, 5, 0)) {
      this.isMovingRange = true;
      this.markAX = this.markA.frame.x;
      this.markBX = this.markB.frame.x;
    } else if (
      this.frame.containsWithMargin(timeLineMouseDownX, timeLineMouseDownY, 20, 0, 0, 0) &&
      !this.rangeRect.containsWithMargin(timeLineMouseDownX, timeLineMouseDownY, 0, markPadding, 0, markPadding)
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

  mouseMove(ev: MouseEvent) {
    let x = ev.offsetX - (offsetLeft || 0);
    let y = ev.offsetY - (offsetTop || 0);
    this.centerXPercentage = x / (this.frame.width || 0);
    if (this.centerXPercentage <= 0) {
      this.centerXPercentage = 0;
    } else if (this.centerXPercentage >= 1) {
      this.centerXPercentage = 1;
    }
    let maxX = this.frame.width || 0;
    if (this.markA.inspectionFrame.contains(x, y)) {
      this.markA.isHover = true;
    } else if (this.markB.inspectionFrame.contains(x, y)) {
      this.markB.isHover = true;
    } else {
      this.markA.isHover = false;
      this.markB.isHover = false;
    }
    if (this.movingMark) {
      let result = x - this.mouseDownOffsetX + this.mouseDownMovingMarkX;
      if (result >= 0 && result <= maxX) {
        this.movingMark.frame.x = result;
      } else if (result < 0) {
        this.movingMark.frame.x = 0;
      } else {
        this.movingMark.frame.x = maxX;
      }
      this.movingMark.inspectionFrame.x = this.movingMark.frame.x - markPadding;
      requestAnimationFrame(() => this.draw());
    }
    if (this.isMovingRange && this.isMouseDown) {
      let result = x - this.mouseDownOffsetX;
      let mA = result + this.markAX;
      let mB = result + this.markBX;
      if (mA >= 0 && mA <= maxX) {
        this.markA.frame.x = mA;
      } else if (mA < 0) {
        this.markA.frame.x = 0;
      } else {
        this.markA.frame.x = maxX;
      }
      this.markA.inspectionFrame.x = this.markA.frame.x - markPadding;
      if (mB >= 0 && mB <= maxX) {
        this.markB.frame.x = mB;
      } else if (mB < 0) {
        this.markB.frame.x = 0;
      } else {
        this.markB.frame.x = maxX;
      }
      this.markB.inspectionFrame.x = this.markB.frame.x - markPadding;
      requestAnimationFrame(() => this.draw());
    } else if (this.isNewRange) {
      this.markA.frame.x = this.mouseDownOffsetX;
      this.markA.inspectionFrame.x = this.mouseDownOffsetX - markPadding;
      if (x >= 0 && x <= maxX) {
        this.markB.frame.x = x;
      } else if (x < 0) {
        this.markB.frame.x = 0;
      } else {
        this.markB.frame.x = maxX;
      }
      this.markB.inspectionFrame.x = this.markB.frame.x - markPadding;
      requestAnimationFrame(() => this.draw());
    }
  }

  mouseOut(ev: MouseEvent) {
    this.movingMark = null;
  }

  fillX() {
    if (this.range.startNS < 0) this.range.startNS = 0;
    if (this.range.endNS < 0) this.range.endNS = 0;
    if (this.range.endNS > this.range.totalNS) this.range.endNS = this.range.totalNS;
    if (this.range.startNS > this.range.totalNS) this.range.startNS = this.range.totalNS;
    this.range.startX = (this.range.startNS * (this.frame.width || 0)) / this.range.totalNS;
    this.range.endX = (this.range.endNS * (this.frame.width || 0)) / this.range.totalNS;
    this.markA.frame.x = this.range.startX;
    this.markA.inspectionFrame.x = this.markA.frame.x - markPadding;
    this.markB.frame.x = this.range.endX;
    this.markB.inspectionFrame.x = this.markB.frame.x - markPadding;
  }

  keyPress(ev: KeyboardEvent) {
    if (this.animaStartTime === undefined) {
      this.animaStartTime = new Date().getTime();
    }
    let startTime = new Date().getTime();
    let duration = startTime - this.animaStartTime;
    if (duration < this.animTime) duration = this.animTime;
    this.currentDuration = duration;
    if (this.isPress) return;
    this.isPress = true;
    switch (ev.key.toLocaleLowerCase()) {
      case 'w':
        let animW = () => {
          if (this.scale === 50) return;
          this.range.startNS += (this.centerXPercentage * this.currentDuration * 2 * this.scale) / this.p;
          this.range.endNS -= ((1 - this.centerXPercentage) * this.currentDuration * 2 * this.scale) / this.p;
          this.fillX();
          this.draw();
          this.pressFrameId = requestAnimationFrame(animW);
        };
        this.pressFrameId = requestAnimationFrame(animW);
        break;
      case 's':
        let animS = () => {
          if (this.range.startNS <= 0 && this.range.endNS >= this.range.totalNS) return;
          this.range.startNS -= (this.centerXPercentage * this.currentDuration * 2 * this.scale) / this.p;
          this.range.endNS += ((1 - this.centerXPercentage) * this.currentDuration * 2 * this.scale) / this.p;
          this.fillX();
          this.draw();
          this.pressFrameId = requestAnimationFrame(animS);
        };
        this.pressFrameId = requestAnimationFrame(animS);
        break;
      case 'a':
        let animA = () => {
          if (this.range.startNS == 0) return;
          let s = (this.scale / this.p) * this.currentDuration;
          this.range.startNS -= s;
          this.range.endNS -= s;
          this.fillX();
          this.draw();
          this.pressFrameId = requestAnimationFrame(animA);
        };
        this.pressFrameId = requestAnimationFrame(animA);
        break;
      case 'd':
        let animD = () => {
          if (this.range.endNS >= this.range.totalNS) return;
          this.range.startNS += (this.scale / this.p) * this.currentDuration;
          this.range.endNS += (this.scale / this.p) * this.currentDuration;
          this.fillX();
          this.draw();
          this.pressFrameId = requestAnimationFrame(animD);
        };
        this.pressFrameId = requestAnimationFrame(animD);
        break;
    }
  }

  keyUp(ev: KeyboardEvent) {
    this.animaStartTime = undefined;
    this.isPress = false;
    if (this.pressFrameId != -1) {
      cancelAnimationFrame(this.pressFrameId);
    }
    let startTime = new Date().getTime();
    switch (ev.key) {
      case 'w':
        let animW = () => {
          if (this.scale === 50) return;
          let dur = new Date().getTime() - startTime;
          this.range.startNS += (this.centerXPercentage * 100 * this.scale) / this.p;
          this.range.endNS -= ((1 - this.centerXPercentage) * 100 * this.scale) / this.p;
          this.fillX();
          this.draw();
          if (dur < 300) {
            requestAnimationFrame(animW);
          }
        };
        requestAnimationFrame(animW);
        break;
      case 's':
        let animS = () => {
          if (this.range.startNS <= 0 && this.range.endNS >= this.range.totalNS) return;
          let dur = new Date().getTime() - startTime;
          this.range.startNS -= (this.centerXPercentage * 100 * this.scale) / this.p;
          this.range.endNS += ((1 - this.centerXPercentage) * 100 * this.scale) / this.p;
          this.fillX();
          this.draw();
          if (dur < 300) {
            requestAnimationFrame(animS);
          }
        };
        requestAnimationFrame(animS);
        break;
      case 'a':
        let animA = () => {
          if (this.range.startNS <= 0) return;
          let dur = new Date().getTime() - startTime;
          let s = (this.scale * 80) / this.p;
          this.range.startNS -= s;
          this.range.endNS -= s;
          this.fillX();
          this.draw();
          if (dur < 300) {
            requestAnimationFrame(animA);
          }
        };
        animA();
        break;
      case 'd':
        let animD = () => {
          if (this.range.endNS >= this.range.totalNS) return;
          let dur = new Date().getTime() - startTime;
          let s = (this.scale * 80) / this.p;
          this.range.startNS += s;
          this.range.endNS += s;
          this.fillX();
          this.draw();
          if (dur < 300) {
            requestAnimationFrame(animD);
          }
        };
        animD();
        break;
    }
  }
}

export class Flag {
  x: number = 0;
  y: number = 0;
  width: number = 0;
  height: number = 0;
  time: number = 0;
  color: string = '';
  selected: boolean = false;
  text: string = '';

  constructor(
    x: number,
    y: number,
    width: number,
    height: number,
    time: number,
    color: string = '#999999',
    selected = false
  ) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
    this.time = time;
    this.color = color;
    this.selected = selected;
  }
}
