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
import { Utils } from '../base/Utils.js';

export enum StType {
  TEMP, //临时的
  PERM, // 永久的
}

export class SlicesTime {
  private _id: string;
  startTime: number | null | undefined;
  endTime: number | null | undefined;
  startNS: number;
  endNS: number;
  color: string = '';
  startX: number;
  endX: number;
  selected: boolean = true;
  hidden: boolean = false;
  text: string = '';
  type: number = StType.PERM; // 默认类型为永久的
  constructor(
    startTime: number | null | undefined,
    endTime: number | null | undefined,
    startNS: number,
    endNS: number,
    startX: number,
    endX: number,
    color: string,
    selected: boolean = true
  ) {
    this._id = Utils.uuid();
    this.startTime = startTime;
    this.endTime = endTime;
    this.startNS = startNS;
    this.endNS = endNS;
    this.color = color;
    this.startX = startX;
    this.endX = endX;
    this.selected = selected;
  }

  get id(): string {
    return this._id;
  }
}

const TRIWIDTH = 10; // 定义三角形的边长
export class SportRuler extends Graph {
  static isMouseInSportRuler = false;
  public flagList: Array<Flag> = [];
  public slicesTimeList: Array<SlicesTime> = [];
  isRangeSelect: boolean = false; //region selection
  private hoverFlag: Flag = new Flag(-1, 0, 0, 0, 0);
  private lineColor: string | null = null;
  private rulerW = 0;
  private _range: TimeRange = {} as TimeRange;
  private readonly notifyHandler:
    | ((hoverFlag: Flag | undefined | null, selectFlag: Flag | undefined | null) => void)
    | undefined;
  private readonly flagClickHandler: ((flag: Flag | undefined | null) => void) | undefined;
  private readonly rangeClickHandler: ((sliceTime: SlicesTime | undefined | null) => void) | undefined;
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
    flagClickHandler: (flag: Flag | undefined | null) => void,
    rangeClickHandler: (sliceTime: SlicesTime | undefined | null) => void
  ) {
    super(timerShaftEL.canvas, timerShaftEL.ctx!, frame);
    this.notifyHandler = notifyHandler;
    this.flagClickHandler = flagClickHandler;
    this.rangeClickHandler = rangeClickHandler;
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

  modifySicesTimeList(slicestime: SlicesTime | null | undefined) {
    if (slicestime) {
      let i = this.slicesTimeList.findIndex((it) => it.id == slicestime.id);
      if (slicestime.hidden) {
        this.slicesTimeList.splice(i, 1);
        let selectionParam = this.timerShaftEL?.selectionMap.get(slicestime.id);
        this.timerShaftEL?.selectionMap.delete(slicestime.id);
        if (selectionParam) {
          this.timerShaftEL?.selectionList.splice(this.timerShaftEL?.selectionList.indexOf(selectionParam), 1);
        }
      } else {
        this.slicesTimeList[i] = slicestime;
      }
    } else {
      this.slicesTimeList.forEach((it) => (it.selected = false));
    }
    this.draw();
  }
  draw(): void {
    this.rulerW = this.canvas!.offsetWidth;
    this.context2D.clearRect(this.frame.x, this.frame.y, this.frame.width, this.frame.height + 1);
    this.context2D.beginPath();
    this.lineColor = window.getComputedStyle(this.canvas!, null).getPropertyValue('color');
    this.context2D.lineWidth = 1;
    this.context2D.strokeStyle = this.lineColor; //"#dadada"
    this.context2D.moveTo(this.frame.x, this.frame.y);
    this.context2D.lineTo(this.frame.x + this.frame.width, this.frame.y);
    this.context2D.stroke();
    this.context2D.closePath();
    this.context2D.beginPath();
    this.context2D.strokeStyle = '#999999';
    this.context2D.lineWidth = 3;
    this.context2D.moveTo(this.frame.x, this.frame.y);
    this.context2D.lineTo(this.frame.x, this.frame.y + this.frame.height);
    this.context2D.stroke();
    this.context2D.closePath();
    this.context2D.beginPath();
    this.context2D.strokeStyle = this.lineColor; //"#999999"
    this.context2D.lineWidth = 1;
    this.context2D.fillStyle = '#999999';
    this.context2D.font = '8px sans-serif';
    this.range.xs?.forEach((item, index) => {
      this.context2D.moveTo(item, this.frame.y);
      this.context2D.lineTo(item, this.frame.y + this.frame.height);
      this.context2D.fillText(`${this.range.xsTxt[index]}`, item + 3, this.frame.y + 12);
    });
    this.context2D.stroke();
    this.context2D.closePath();
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
      this.context2D.beginPath();
      if (document.querySelector<SpApplication>('sp-application')!.dark) {
        this.context2D.strokeStyle = '#FFF';
        this.context2D.fillStyle = '#FFF';
      } else {
        this.context2D.strokeStyle = '#000';
        this.context2D.fillStyle = '#000';
      }
      let start_X = ns2x(range?.startNS || 0, this.range.startNS, this.range.endNS, this.range.totalNS, this.frame);
      let endX = ns2x(range?.endNS || 0, this.range.startNS, this.range.endNS, this.range.totalNS, this.frame);
      let lineWidth = endX - start_X;
      let txt = ns2s((range?.endNS || 0) - (range?.startNS || 0));
      this.context2D.moveTo(start_X, this.frame.y + 22);
      this.context2D.lineTo(endX, this.frame.y + 22);
      this.context2D.moveTo(start_X, this.frame.y + 22 - 5);
      this.context2D.lineTo(start_X, this.frame.y + 22 + 5);
      this.context2D.moveTo(endX, this.frame.y + 22 - 5);
      this.context2D.lineTo(endX, this.frame.y + 22 + 5);
      let textWidth = this.context2D.measureText(txt).width;
      if (lineWidth > textWidth) {
        this.context2D.fillText(`${txt}`, start_X + (lineWidth - textWidth) / 2, this.frame.y + 20);
      } else {
        if (endX + textWidth >= this.frame.width) {
          this.context2D.fillText(`${txt}`, start_X - 5 - textWidth, this.frame.y + 20);
        } else {
          this.context2D.fillText(`${txt}`, endX + 5, this.frame.y + 20);
        }
      }
      this.context2D.stroke();
      this.context2D.closePath();
    }
    if (this.invertedTriangleTime != null && typeof this.invertedTriangleTime != undefined) {
      this.drawInvertedTriangle(
        this.invertedTriangleTime,
        document.querySelector<SpApplication>('sp-application')!.dark ? '#FFFFFF' : '#000000'
      );
    }
    this.slicesTimeList.forEach((slicesTime) => {
      this.drawSlicesMarks(slicesTime);
    });
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
      this.context2D.beginPath();
      this.context2D.fillStyle = color;
      this.context2D.strokeStyle = color;
      this.context2D.moveTo(x - 3, 141);
      this.context2D.lineTo(x + 3, 141);
      this.context2D.lineTo(x, 145);
      this.context2D.fill();
      this.context2D.closePath();
      this.context2D.stroke();
    }
  }

  setSlicesMark(
    startTime: number | null = null,
    endTime: number | null = null,
    shiftKey: boolean | null = null
  ): SlicesTime | null {
    let newSlicestime: SlicesTime | null = null;
    if (startTime != null && typeof startTime != undefined && endTime != null && typeof endTime != undefined) {
      this.slicesTime = {
        startTime: startTime <= endTime ? startTime : endTime,
        endTime: startTime <= endTime ? endTime : startTime,
        color: null,
      };
      let startX = Math.round(
        (this.rulerW * (startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS)
      );
      let endX = Math.round((this.rulerW * (endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS));
      let color = randomRgbColor();
      this.slicesTime.color = color;
      newSlicestime = new SlicesTime(
        this.slicesTime.startTime,
        this.slicesTime.endTime,
        this.range.startNS,
        this.range.endNS,
        startX,
        endX,
        color,
        true
      );
      if (!shiftKey) {
        this.clearTempSlicesTime(); // 清除临时对象

        // 如果没有按下shift键，则把当前slicestime对象的类型设为临时类型。
        newSlicestime.type = StType.TEMP;
      }
      this.slicesTimeList.forEach((slicestime) => (slicestime.selected = false));
      newSlicestime.selected = true;
      this.slicesTimeList.push(newSlicestime);
    } else {
      this.clearTempSlicesTime();// 清除临时对象
      this.slicesTime = { startTime: null, endTime: null, color: null };
    }
    this.range.slicesTime = this.slicesTime;
    this.draw();
    this.timerShaftEL?.render();
    return newSlicestime;
  }

  // 清除临时对象
  clearTempSlicesTime() {
    // 清除以前放入的临时对象
    this.slicesTimeList.forEach((slicestime, index) => {
      slicestime.selected = false;
      if (slicestime.type == StType.TEMP) {
        this.slicesTimeList.splice(index, 1);
        let selectionParam = this.timerShaftEL?.selectionMap.get(slicestime.id);
        if (selectionParam && selectionParam != undefined) {
          this.timerShaftEL?.selectionList.splice(this.timerShaftEL?.selectionList.indexOf(selectionParam), 1);
          this.timerShaftEL?.selectionMap.delete(slicestime.id);
        }
      }
    });
  }

  clearHoverFlag() {
    this.hoverFlag.hidden = true;
  }

  showHoverFlag() {
    this.hoverFlag.hidden = false;
  }

  drawSlicesMarks(slicesTime: SlicesTime) {
    if (
      slicesTime.startTime != null &&
      typeof slicesTime.startTime != undefined &&
      slicesTime.endTime != null &&
      typeof slicesTime.endTime != undefined
    )  {
      let startX = Math.round(
        (this.rulerW * (slicesTime.startTime - this.range.startNS)) / (this.range.endNS - this.range.startNS)
      );
      let endX = Math.round(
        (this.rulerW * (slicesTime.endTime - this.range.startNS)) / (this.range.endNS - this.range.startNS)
      );
      // 放大、缩小、左右移动之后重置小三角的x轴坐标
      slicesTime.startX = startX;
      slicesTime.endX = endX;

      this.context2D.beginPath();
      this.context2D.strokeStyle = slicesTime.color;
      this.context2D.fillStyle = slicesTime.color;
      this.range.slicesTime.color = slicesTime.color; //紫色

      this.context2D.moveTo(startX + TRIWIDTH, 132);
      this.context2D.lineTo(startX, 142);
      this.context2D.lineTo(startX, 132);
      this.context2D.lineTo(startX + TRIWIDTH, 132);

      this.context2D.lineTo(endX - TRIWIDTH, 132);
      this.context2D.lineTo(endX, 132);
      this.context2D.lineTo(endX, 142);
      this.context2D.lineTo(endX - TRIWIDTH, 132);
      this.context2D.closePath();
      slicesTime.selected && this.context2D.fill();
      this.context2D.stroke();

      this.context2D.beginPath();
      if (document.querySelector<SpApplication>('sp-application')!.dark) {
        this.context2D.strokeStyle = '#FFF';
        this.context2D.fillStyle = '#FFF';
      } else {
        this.context2D.strokeStyle = '#000';
        this.context2D.fillStyle = '#000';
      }
      let lineWidth = endX - startX;
      let txt = ns2s((slicesTime.endTime || 0) - (slicesTime.startTime || 0));
      this.context2D.moveTo(startX, this.frame.y + 22);
      this.context2D.lineTo(endX, this.frame.y + 22);
      this.context2D.moveTo(startX, this.frame.y + 22 - 5);
      this.context2D.lineTo(startX, this.frame.y + 22 + 5);
      this.context2D.moveTo(endX, this.frame.y + 22 - 5);
      this.context2D.lineTo(endX, this.frame.y + 22 + 5);
      let txtWidth = this.context2D.measureText(txt).width;
      this.context2D.fillStyle = '#FFF'; //为了解决文字重叠问题。在时间刻度的文字下面绘制一个小方块
      this.context2D.fillRect(startX + (lineWidth - txtWidth) / 2, this.frame.y + 10, txtWidth + 2, 10);
      this.context2D.fillStyle = 'black';
      if (lineWidth > txtWidth) {
        this.context2D.fillText(`${txt}`, startX + (lineWidth - txtWidth) / 2, this.frame.y + 20);
      } else {
        if (endX + txtWidth >= this.frame.width) {
          this.context2D.fillText(`${txt}`, startX - 5 - txtWidth, this.frame.y + 20);
        } else {
          this.context2D.fillText(`${txt}`, endX + 5, this.frame.y + 20);
        }
      }
      this.context2D.stroke();
      this.context2D.closePath();
    }
  }

  //绘制旗子
  drawFlag(x: number, color: string = '#999999', isFill: boolean = false, textStr: string = '', type: string = '') {
    if (x < 0) return;
    this.context2D.beginPath();
    this.context2D.fillStyle = color;
    this.context2D.strokeStyle = color;
    this.context2D.moveTo(x, 125);
    if (type == 'triangle') {
      this.context2D.lineTo(x + 15, 131);
    } else {
      this.context2D.lineTo(x + 10, 125);
      this.context2D.lineTo(x + 10, 127);
      this.context2D.lineTo(x + 18, 127);
      this.context2D.lineTo(x + 18, 137);
      this.context2D.lineTo(x + 10, 137);
      this.context2D.lineTo(x + 10, 135);
    }
    this.context2D.lineTo(x + 2, 135);
    this.context2D.lineTo(x + 2, 142);
    this.context2D.lineTo(x, 142);
    this.context2D.closePath();
    isFill && this.context2D.fill();
    this.context2D.stroke();
    if (textStr !== '') {
      this.context2D.font = '10px Microsoft YaHei';
      const { width } = this.context2D.measureText(textStr);
      this.context2D.fillStyle = 'rgba(255, 255, 255, 0.8)'; //
      this.context2D.fillRect(x + 21, 132, width + 4, 12);
      this.context2D.fillStyle = 'black';
      this.context2D.fillText(textStr, x + 23, 142);
      this.context2D.stroke();
    }
  }

  /**
   * 查找鼠标所在位置是否存在"帽子"对象，为了操作方便，框选时把三角形的边长宽度左右各加一个像素。
   * @param x 水平坐标值
   * @returns
   */
  findSlicesTime(x: number, y: number): SlicesTime | null {
    let slicestime = this.slicesTimeList.find((slicesTime) => {
      return (
        ((x >= slicesTime.startX - 1 && x <= slicesTime.startX + TRIWIDTH + 1) || // 选中了帽子的左边三角形区域
          (x >= slicesTime.endX - TRIWIDTH - 1 && x <= slicesTime.endX + 1)) && // 选中了帽子的右边三角形区域
        y >= 132 &&
        y <= 142
      );
    });

    if (!slicestime) {
      return null;
    }
    return slicestime;
  }

  mouseUp(ev: MouseEvent) {
    if (this.edgeDetection(ev)) {
      let x = ev.offsetX - (this.canvas?.offsetLeft || 0); // 鼠标点击的x轴坐标
      let y = ev.offsetY; // 鼠标点击的y轴坐标
      let findSlicestime = this.findSlicesTime(x, y); // 查找帽子
      if (findSlicestime) {
        // 如果找到帽子，则选中帽子。
        this.slicesTimeList.forEach((slicestime) => (slicestime.selected = false));
        findSlicestime.selected = true;
        this.rangeClickHandler && this.rangeClickHandler(findSlicestime);
      } else {
        // 如果没有找到帽子，则绘制旗子，此处避免旗子和帽子重叠。
        // 查找旗子
        let findFlag = this.flagList.find((it) => x >= it.x && x <= it.x + 18);
        this.flagList.forEach((it) => (it.selected = false));
        if (findFlag) {
          findFlag.selected = true;
        } else {
          let flagAtRulerTime = Math.round(((this.range.endNS - this.range.startNS) * x) / this.rulerW);
          if (flagAtRulerTime > 0 && this.range.startNS + flagAtRulerTime < this.range.endNS) {
            let flag = new Flag(x, 125, 18, 18, flagAtRulerTime + this.range.startNS, randomRgbColor(), true,'');
            this.flagList.push(flag);
          }
        }
        this.flagClickHandler && this.flagClickHandler(this.flagList.find((it) => it.selected)); // 绘制旗子
      }
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
