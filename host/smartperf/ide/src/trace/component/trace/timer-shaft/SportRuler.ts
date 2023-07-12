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
import { TimeRange } from './RangeRuler.js';
import { Flag } from './Flag.js';
import { ns2s, ns2x, randomRgbColor, TimerShaftElement } from '../TimerShaftElement.js';
import { TraceRow } from '../base/TraceRow.js';
import { SpApplication } from '../../../SpApplication.js';

export class SportRuler extends Graph {
  static isMouseInSportRuler = false;
  public flagList: Array<Flag> = [];
  isRangeSelect: boolean = false; //region selection
  private hoverFlag: Flag = new Flag(-1, 0, 0, 0, 0);
  private lineColor: string | null = null;
  private rulerW = 0;
  private _range: TimeRange = {} as TimeRange;
  private readonly notifyHandler:
    | ((hoverFlag: Flag | undefined | null, selectFlag: Flag | undefined | null) => void)
    | undefined;
  private readonly flagClickHandler: ((flag: Flag | undefined | null) => void) | undefined;
  private invertedTriangleTime: number | null | undefined = null;
  private slicesTime: {
    startTime: number | null | undefined;
    endTime: number | null | undefined;
    color: string | null;
  } | null = {
    startTime: null,
    endTime: null,
    color: null,
  };
  private timerShaftEL: TimerShaftElement | undefined | null;
  constructor(
    timerShaftEL: TimerShaftElement,
    frame: Rect,
    notifyHandler: (hoverFlag: Flag | undefined | null, selectFlag: Flag | undefined | null) => void,
    flagClickHandler: (flag: Flag | undefined | null) => void
  ) {
    super(timerShaftEL.canvas, timerShaftEL.ctx!, frame);
    this.notifyHandler = notifyHandler;
    this.flagClickHandler = flagClickHandler;
    this.timerShaftEL = timerShaftEL;
  }

  get range(): TimeRange {
    return this._range;
  }

  set range(value: TimeRange) {
    this._range = value;
    this.draw();
  }

  modifyFlagList(flag: Flag | null | undefined) {
    if (flag) {
      if (flag.hidden) {
        let i = this.flagList.findIndex((it) => it.time == flag.time);
        this.flagList.splice(i, 1);
      } else {
        let i = this.flagList.findIndex((it) => it.time == flag.time);
        this.flagList[i] = flag;
      }
    } else {
      this.flagList.forEach((it) => (it.selected = false));
    }
    this.draw();
  }

  draw(): void {
    this.rulerW = this.canvas!.offsetWidth;
    this.c.clearRect(this.frame.x, this.frame.y, this.frame.width, this.frame.height + 1);
    this.c.beginPath();
    this.lineColor = window.getComputedStyle(this.canvas!, null).getPropertyValue('color');
    this.c.lineWidth = 1;
    this.c.strokeStyle = this.lineColor; //"#dadada"
    this.c.moveTo(this.frame.x, this.frame.y);
    this.c.lineTo(this.frame.x + this.frame.width, this.frame.y);
    this.c.stroke();
    this.c.closePath();
    this.c.beginPath();
    this.c.strokeStyle = '#999999';
    this.c.lineWidth = 3;
    this.c.moveTo(this.frame.x, this.frame.y);
    this.c.lineTo(this.frame.x, this.frame.y + this.frame.height);
    this.c.stroke();
    this.c.closePath();
    this.c.beginPath();
    this.c.strokeStyle = this.lineColor; //"#999999"
    this.c.lineWidth = 1;
    this.c.fillStyle = '#999999';
    this.c.font = '8px sans-serif';
    this.range.xs?.forEach((item, index) => {
      this.c.moveTo(item, this.frame.y);
      this.c.lineTo(item, this.frame.y + this.frame.height);
      this.c.fillText(`+${this.range.xsTxt[index]}`, item + 3, this.frame.y + 12);
    });
    this.c.stroke();
    this.c.closePath();
    //绘制旗子
    this.flagList.forEach((flagObj: Flag, b) => {
      if (flagObj.time >= this.range.startNS && flagObj.time <= this.range.endNS) {
        flagObj.x = Math.round(
          (this.rulerW * (flagObj.time - this.range.startNS)) / (this.range.endNS - this.range.startNS)
        );
        this.drawFlag(flagObj.x, flagObj.color, flagObj.selected, flagObj.text, flagObj.type);
      }
    });
    !this.hoverFlag.hidden && this.drawFlag(this.hoverFlag.x, this.hoverFlag.color, true, this.hoverFlag.text);
    //If region selection is enabled, the serial number draws a line on the axis to show the length of the box selection
    if (this.isRangeSelect) {
      let range = TraceRow.rangeSelectObject;
      this.c.beginPath();
      if (document.querySelector<SpApplication>('sp-application')!.dark) {
        this.c.strokeStyle = '#FFF';
        this.c.fillStyle = '#FFF';
      } else {
        this.c.strokeStyle = '#000';
        this.c.fillStyle = '#000';
      }
      let start_X = ns2x(range?.startNS || 0, this.range.startNS, this.range.endNS, this.range.totalNS, this.frame);
      let endX = ns2x(range?.endNS || 0, this.range.startNS, this.range.endNS, this.range.totalNS, this.frame);
      let lineWidth = endX - start_X;
      let txt = ns2s((range?.endNS || 0) - (range?.startNS || 0));
      this.c.moveTo(start_X, this.frame.y + 22);
      this.c.lineTo(endX, this.frame.y + 22);
      this.c.moveTo(start_X, this.frame.y + 22 - 5);
      this.c.lineTo(start_X, this.frame.y + 22 + 5);
      this.c.moveTo(endX, this.frame.y + 22 - 5);
      this.c.lineTo(endX, this.frame.y + 22 + 5);
      let textWidth = this.c.measureText(txt).width;
      if (lineWidth > textWidth) {
        this.c.fillText(`${txt}`, start_X + (lineWidth - textWidth) / 2, this.frame.y + 20);
      } else {
        if (endX + textWidth >= this.frame.width) {
          this.c.fillText(`${txt}`, start_X - 5 - textWidth, this.frame.y + 20);
        } else {
          this.c.fillText(`${txt}`, endX + 5, this.frame.y + 20);
        }
      }
      this.c.stroke();
      this.c.closePath();
    }
    if (this.invertedTriangleTime != null && typeof this.invertedTriangleTime != undefined) {
      this.drawInvertedTriangle(
        this.invertedTriangleTime,
        document.querySelector<SpApplication>('sp-application')!.dark ? '#FFFFFF' : '#000000'
      );
    }
    this.drawSlicesMark(this.slicesTime?.startTime, this.slicesTime?.endTime);
  }

  drawTriangle(time: number, type: string) {
    if (time != null && typeof time != undefined) {
      let i = this.flagList.findIndex((it) => it.time == time);
      if (type == 'triangle') {
        let triangle = this.flagList.findIndex((it) => it.type == type);
        if (i !== -1) {
          if (triangle !== -1) {
            this.flagList[i].type == '' ? this.flagList.splice(triangle, 1) : '';
          }
          this.flagList.forEach((it) => (it.selected = false));
          this.flagList[i].selected = true;
        } else {
          if (triangle == -1) {
            this.flagList.forEach((it) => (it.selected = false));
            this.flagList.push(new Flag(0, 125, 18, 18, time, randomRgbColor(), true, 'triangle'));
          } else {
            this.flagList.forEach((it) => (it.selected = false));
            this.flagList[triangle].time = time;
            this.flagList[triangle].selected = true;
          }
        }
      } else if (type == 'square') {
        if (i != -1) {
          this.flagList[i].type = '';
        } else {
          let triangle = this.flagList.findIndex((it) => it.type == 'triangle');
          if (triangle !== -1) {
            this.flagList[triangle].type = '';
            this.draw();
            this.notifyHandler &&
              this.notifyHandler(
                !this.hoverFlag.hidden ? this.hoverFlag : null,
                this.flagList.find((it) => it.selected) || null
              );
            return this.flagList[triangle].time;
          }
        }
      } else if (type == 'inverted') {
        this.invertedTriangleTime = time;
      }
      this.draw();
      this.notifyHandler &&
        this.notifyHandler(
          !this.hoverFlag.hidden ? this.hoverFlag : null,
          this.flagList.find((it) => it.selected) || null
        );
    }
  }

  removeTriangle(type: string) {
    if (type == 'inverted') {
      this.invertedTriangleTime = null;
    } else {
      let i = this.flagList.findIndex((it) => it.type == type);
      if (i !== -1) {
        this.flagList.splice(i, 1);
      }
    }
    this.draw();
    this.notifyHandler &&
      this.notifyHandler(
        !this.hoverFlag.hidden ? this.hoverFlag : null,
        this.flagList.find((it) => it.selected) || null
      );
  }

  drawInvertedTriangle(time: number, color: string = '#000000') {
    if (time != null && typeof time != undefined) {
      let x = Math.round((this.rulerW * (time - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      this.c.beginPath();
      this.c.fillStyle = color;
      this.c.strokeStyle = color;
      this.c.moveTo(x - 3, 141);
      this.c.lineTo(x + 3, 141);
      this.c.lineTo(x, 145);
      this.c.fill();
      this.c.closePath();
      this.c.stroke();
    }
  }

  setSlicesMark(startTime: number | null = null, endTime: number | null = null) {
    if (startTime != null && typeof startTime != undefined && endTime != null && typeof endTime != undefined) {
      this.slicesTime = {
        startTime: startTime <= endTime ? startTime : endTime,
        endTime: startTime <= endTime ? endTime : startTime,
        color: null,
      };
    } else {
      this.slicesTime = { startTime: null, endTime: null, color: null };
    }
    this.range.slicesTime = this.slicesTime;
    this.timerShaftEL?.render();
  }

  clearHoverFlag() {
    this.hoverFlag.hidden = true;
  }

  drawSlicesMark(startTime: number | null = null, endTime: number | null = null) {
    if (startTime != null && typeof startTime != undefined && endTime != null && typeof endTime != undefined) {
      let startX = Math.round(
        (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS)
      );
      let endX = Math.round((this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      this.c.beginPath();
      if (document.querySelector<SpApplication>('sp-application')!.dark) {
        this.c.strokeStyle = '#FFF';
        this.c.fillStyle = '#FFF';
        this.range.slicesTime.color = '#FFF';
      } else {
        this.c.strokeStyle = '#344596';
        this.c.fillStyle = '#344596';
        this.range.slicesTime.color = '#344596';
      }
      this.c.moveTo(startX + 9, 132);
      this.c.lineTo(startX, 141);
      this.c.lineTo(startX, 132);
      this.c.lineTo(startX + 9, 132);

      this.c.lineTo(endX - 9, 132);
      this.c.lineTo(endX, 132);
      this.c.lineTo(endX, 141);
      this.c.lineTo(endX - 9, 132);
      this.c.closePath();
      this.c.stroke();

      this.c.beginPath();
      if (document.querySelector<SpApplication>('sp-application')!.dark) {
        this.c.strokeStyle = '#FFF';
        this.c.fillStyle = '#FFF';
      } else {
        this.c.strokeStyle = '#000';
        this.c.fillStyle = '#000';
      }
      let lineWidth = endX - startX;
      let txt = ns2s((endTime || 0) - (startTime || 0));
      this.c.moveTo(startX, this.frame.y + 22);
      this.c.lineTo(endX, this.frame.y + 22);
      this.c.moveTo(startX, this.frame.y + 22 - 5);
      this.c.lineTo(startX, this.frame.y + 22 + 5);
      this.c.moveTo(endX, this.frame.y + 22 - 5);
      this.c.lineTo(endX, this.frame.y + 22 + 5);
      let txtWidth = this.c.measureText(txt).width;
      this.c.fillStyle = '#FFF'; //为了解决文字重叠问题。在时间刻度的文字下面绘制一个小方块
      this.c.fillRect(startX + (lineWidth - txtWidth) / 2, this.frame.y + 10, txtWidth + 2, 10);
      this.c.fillStyle = 'black';
      if (lineWidth > txtWidth) {
        this.c.fillText(`${txt}`, startX + (lineWidth - txtWidth) / 2, this.frame.y + 20);
      } else {
        if (endX + txtWidth >= this.frame.width) {
          this.c.fillText(`${txt}`, startX - 5 - txtWidth, this.frame.y + 20);
        } else {
          this.c.fillText(`${txt}`, endX + 5, this.frame.y + 20);
        }
      }
      this.c.stroke();
      this.c.closePath();
    }
  }

  //绘制旗子
  drawFlag(x: number, color: string = '#999999', isFill: boolean = false, textStr: string = '', type: string = '') {
    if (x < 0) return;
    this.c.beginPath();
    this.c.fillStyle = color;
    this.c.strokeStyle = color;
    this.c.moveTo(x, 125);
    if (type == 'triangle') {
      this.c.lineTo(x + 15, 131);
    } else {
      this.c.lineTo(x + 10, 125);
      this.c.lineTo(x + 10, 127);
      this.c.lineTo(x + 18, 127);
      this.c.lineTo(x + 18, 137);
      this.c.lineTo(x + 10, 137);
      this.c.lineTo(x + 10, 135);
    }
    this.c.lineTo(x + 2, 135);
    this.c.lineTo(x + 2, 142);
    this.c.lineTo(x, 142);
    this.c.closePath();
    isFill && this.c.fill();
    this.c.stroke();
    if (textStr !== '') {
      this.c.font = '10px Microsoft YaHei';
      const { width } = this.c.measureText(textStr);
      this.c.fillStyle = 'rgba(255, 255, 255, 0.8)'; //
      this.c.fillRect(x + 21, 132, width + 4, 12);
      this.c.fillStyle = 'black';
      this.c.fillText(textStr, x + 23, 142);
      this.c.stroke();
    }
  }

  mouseUp(ev: MouseEvent) {
    if (this.edgeDetection(ev)) {
      this.flagList.forEach((it) => (it.selected = false));
      let x = ev.offsetX - (this.canvas?.offsetLeft || 0);
      let findFlag = this.flagList.find((it) => x >= it.x && x <= it.x + 18);
      if (findFlag) {
        findFlag.selected = true;
      } else {
        let flagAtRulerTime = Math.round(((this.range.endNS - this.range.startNS) * x) / this.rulerW);
        if (flagAtRulerTime > 0 && this.range.startNS + flagAtRulerTime < this.range.endNS) {
          this.flagList.push(new Flag(x, 125, 18, 18, flagAtRulerTime + this.range.startNS, randomRgbColor(), true));
        }
      }
      this.flagClickHandler && this.flagClickHandler(this.flagList.find((it) => it.selected));
    }
  }

  mouseMove(ev: MouseEvent) {
    if (this.edgeDetection(ev)) {
      let x = ev.offsetX - (this.canvas?.offsetLeft || 0);
      let flg = this.flagList.find((it) => x >= it.x && x <= it.x + 18);
      if (flg) {
        this.hoverFlag.hidden = true;
      } else {
        this.hoverFlag.hidden = false;
        this.hoverFlag.x = x;
        this.hoverFlag.color = '#999999';
      }
    } else {
      this.hoverFlag.hidden = true;
    }
    this.draw();
    this.notifyHandler &&
      this.notifyHandler(
        !this.hoverFlag.hidden ? this.hoverFlag : null,
        this.flagList.find((it) => it.selected) || null
      );
  }

  mouseOut(ev: MouseEvent) {
    if (!this.hoverFlag.hidden) {
      this.hoverFlag.hidden = true;
      this.notifyHandler &&
        this.notifyHandler(
          !this.hoverFlag.hidden ? this.hoverFlag : null,
          this.flagList.find((it) => it.selected) || null
        );
    }
    this.draw();
  }

  edgeDetection(ev: MouseEvent): boolean {
    let x = ev.offsetX - (this.canvas?.offsetLeft || 0);
    let y = ev.offsetY - (this.canvas?.offsetTop || 0);
    SportRuler.isMouseInSportRuler =
      x > 0 &&
      x < this.canvas!.offsetWidth &&
      ev.offsetY - this.frame.y > 20 &&
      ev.offsetY - this.frame.y < this.frame.height;
    return SportRuler.isMouseInSportRuler;
  }
}
