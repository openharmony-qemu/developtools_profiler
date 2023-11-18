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

import { BaseElement, element } from '../../../base-ui/BaseElement.js';
import { TimeRuler } from './timer-shaft/TimeRuler.js';
import { Rect } from './timer-shaft/Rect.js';
import { RangeRuler, TimeRange } from './timer-shaft/RangeRuler.js';
import { SlicesTime, SportRuler } from './timer-shaft/SportRuler.js';
import { procedurePool } from '../../database/Procedure.js';
import { Flag } from './timer-shaft/Flag.js';
import { info } from '../../../log/Log.js';
import { TraceSheet } from './base/TraceSheet.js';
import { SelectionParam } from '../../bean/BoxSelection.js';
import { type SpSystemTrace, CurrentSlicesTime } from '../SpSystemTrace.js';
import './timer-shaft/CollapseButton.js';
//随机生成十六位进制颜色
export function randomRgbColor() {
  const letters = '0123456789ABCDEF';
  let color = '#';
  for (let index = 0; index < 6; index++) {
    color += letters[Math.floor(Math.random() * 16)];
  }
  return color;
}

export function ns2s(ns: number): string {
  let one_second = 1_000_000_000; // 1 second
  let one_millisecond = 1_000_000; // 1 millisecond
  let one_microsecond = 1_000; // 1 microsecond
  let nanosecond1 = 1000.0;
  let result;
  if (ns >= one_second) {
    result = (ns / 1000 / 1000 / 1000).toFixed(1) + ' s';
  } else if (ns >= one_millisecond) {
    result = (ns / 1000 / 1000).toFixed(1) + ' ms';
  } else if (ns >= one_microsecond) {
    result = (ns / 1000).toFixed(1) + ' μs';
  } else if (ns > 0) {
    result = ns.toFixed(1) + ' ns';
  } else {
    result = ns.toFixed(1) + ' s';
  }
  return result;
}

export function ns2UnitS(ns: number, scale: number): string {
  let one_second = 1_000_000_000; // 1 second
  let result;
  if (scale >= 10_000_000_000) {
    result = (ns / one_second).toFixed(0) + ' s';
  } else if (scale >= 1_000_000_000) {
    result = (ns / one_second).toFixed(1) + ' s';
  } else if (scale >= 100_000_000) {
    result = (ns / one_second).toFixed(2) + ' s';
  } else if (scale >= 10_000_000) {
    result = (ns / one_second).toFixed(3) + ' s';
  } else if (scale >= 1_000_000) {
    result = (ns / one_second).toFixed(4) + ' s';
  } else if (scale >= 100_000) {
    result = (ns / one_second).toFixed(5) + ' s';
  } else {
    result = (ns / one_second).toFixed(6) + ' s';
  }
  return result;
}

export function ns2x(ns: number, startNS: number, endNS: number, duration: number, rect: Rect): number {
  if (endNS == 0) {
    endNS = duration;
  }
  let xSize: number = ((ns - startNS) * rect.width) / (endNS - startNS);
  if (xSize < 0) {
    xSize = 0;
  }
  if (xSize > rect.width) {
    xSize = rect.width;
  }
  return xSize;
}

@element('timer-shaft-element')
export class TimerShaftElement extends BaseElement {
  // @ts-ignore
  offscreen: OffscreenCanvas | undefined;
  isOffScreen: boolean = false;
  public ctx: CanvasRenderingContext2D | undefined | null;
  public canvas: HTMLCanvasElement | null | undefined;
  public totalEL: HTMLDivElement | null | undefined;
  public timeTotalEL: HTMLSpanElement | null | undefined;
  public timeOffsetEL: HTMLSpanElement | null | undefined;
  public collectGroup: HTMLDivElement | null | undefined;
  public collect1: HTMLInputElement | null | undefined;
  public loadComplete: boolean = false;
  public collecBtn: HTMLElement | null | undefined;
  rangeChangeHandler: ((timeRange: TimeRange) => void) | undefined = undefined;
  rangeClickHandler: ((sliceTime: SlicesTime | undefined | null) => void) | undefined = undefined;
  flagChangeHandler: ((hoverFlag: Flag | undefined | null, selectFlag: Flag | undefined | null) => void) | undefined =
    undefined;
  flagClickHandler: ((flag: Flag | undefined | null) => void) | undefined = undefined;
  /**
   * 离线渲染需要的变量
   */
  dpr = window.devicePixelRatio || 1;
  frame: Rect = new Rect(0, 0, 0, 0);
  must: boolean = true;
  hoverX: number = 0;
  hoverY: number = 0;
  canvasWidth: number = 0;
  canvasHeight: number = 0;
  _cpuUsage: Array<{ cpu: number; ro: number; rate: number }> = [];
  protected timeRuler: TimeRuler | undefined;
  protected _rangeRuler: RangeRuler | undefined;
  protected _sportRuler: SportRuler | undefined;
  private root: HTMLDivElement | undefined | null;
  private _totalNS: number = 10_000_000_000;
  private _startNS: number = 0;
  private _endNS: number = 10_000_000_000;
  private traceSheetEL: TraceSheet | undefined | null;
  private sliceTime: SlicesTime | undefined | null;
  public selectionList: Array<SelectionParam> = [];
  public selectionMap: Map<string, SelectionParam> = new Map<string, SelectionParam>();

  get sportRuler(): SportRuler | undefined {
    return this._sportRuler;
  }

  get rangeRuler(): RangeRuler | undefined {
    return this._rangeRuler;
  }

  set cpuUsage(value: Array<{ cpu: number; ro: number; rate: number }>) {
    info('set cpuUsage values :', value);
    this._cpuUsage = value;
    if (this._rangeRuler) {
      this._rangeRuler.cpuUsage = this._cpuUsage;
    }
  }

  get totalNS(): number {
    return this._totalNS;
  }

  set totalNS(value: number) {
    info('set totalNS values :', value);
    this._totalNS = value;
    if (this.timeRuler) this.timeRuler.totalNS = value;
    if (this._rangeRuler) this._rangeRuler.range.totalNS = value;
    if (this.timeTotalEL) this.timeTotalEL.textContent = `${ns2s(value)}`;
    requestAnimationFrame(() => this.render());
  }

  get startNS(): number {
    return this._startNS;
  }

  set startNS(value: number) {
    this._startNS = value;
  }

  get endNS(): number {
    return this._endNS;
  }

  set endNS(value: number) {
    this._endNS = value;
  }

  isScaling(): boolean {
    return this._rangeRuler?.isPress || false;
  }

  reset(): void {
    this.loadComplete = false;
    this.totalNS = 10_000_000_000;
    this.startNS = 0;
    this.endNS = 10_000_000_000;
    if (this._rangeRuler) {
      this._rangeRuler.drawMark = false;
      this._rangeRuler.range.totalNS = this.totalNS;
      this._rangeRuler.markAObj.frame.x = 0;
      this._rangeRuler.markBObj.frame.x = this._rangeRuler.frame.width;
      this._rangeRuler.cpuUsage = [];
      this.sportRuler!.flagList.length = 0;
      this.sportRuler!.slicesTimeList.length = 0;
      this.selectionList.length = 0;
      this.selectionMap.clear();
      this._rangeRuler.rangeRect = new Rect(0, 25, this.canvas?.clientWidth || 0, 75);
      this.sportRuler!.isRangeSelect = false;
      this.setSlicesMark();
    }
    this.removeTriangle('inverted');
    this.setRangeNS(0, this.endNS);
  }

  initElements(): void {
    this.root = this.shadowRoot?.querySelector('.root');
    this.canvas = this.shadowRoot?.querySelector('.panel');
    this.totalEL = this.shadowRoot?.querySelector('.total');
    this.collect1 = this.shadowRoot?.querySelector('#collect1');
    this.timeTotalEL = this.shadowRoot?.querySelector('.time-total');
    this.timeOffsetEL = this.shadowRoot?.querySelector('.time-offset');
    this.collecBtn = this.shadowRoot?.querySelector('.time-collect');
    this.collectGroup = this.shadowRoot?.querySelector('.collect_group');
    this.collectGroup?.addEventListener('click', (e) => {
      // @ts-ignore
      if (e.target && e.target.tagName === 'INPUT') {
        // @ts-ignore
        window.publish(window.SmartEvent.UI.CollectGroupChange, e.target.value);
      }
    });
    procedurePool.timelineChange = (a: any) => this.rangeChangeHandler?.(a);
    window.subscribe(window.SmartEvent.UI.TimeRange, (b) => this.setRangeNS(b.startNS, b.endNS));
  }

  getRangeRuler() {
    return this._rangeRuler;
  }

  connectedCallback(): RangeRuler | undefined {
    if (this.canvas) {
      if (this.isOffScreen) {
        // @ts-ignore
        this.offscreen = this.canvas.transferControlToOffscreen();
        return;
      } else {
        this.ctx = this.canvas?.getContext('2d', { alpha: true });
      }
    }
    if (this.timeTotalEL) this.timeTotalEL.textContent = ns2s(this._totalNS);
    if (this.timeOffsetEL && this._rangeRuler)
      this.timeOffsetEL.textContent = ns2UnitS(this._startNS, this._rangeRuler.getScale());
    const width = this.canvas?.clientWidth || 0;
    const height = this.canvas?.clientHeight || 0;
    if (!this.timeRuler) {
      this.timeRuler = new TimeRuler(this, new Rect(0, 0, width, 20), this._totalNS);
    }
    if (!this._sportRuler) {
      this._sportRuler = new SportRuler(
        this,
        new Rect(0, 100, width, height - 100),
        (hoverFlag, selectFlag) => {
          this.flagChangeHandler?.(hoverFlag, selectFlag);
        },
        (flag) => {
          this.flagClickHandler?.(flag);
        },
        (slicetime) => {
          this.rangeClickHandler?.(slicetime);
        }
      );
    }
    if (!this._rangeRuler) {
      this._rangeRuler = new RangeRuler(
        this,
        new Rect(0, 25, width, 75),
        {
          slicesTime: {
            startTime: null,
            endTime: null,
            color: null,
          },
          scale: 0,
          startX: 0,
          endX: this.canvas?.clientWidth || 0,
          startNS: 0,
          endNS: this.totalNS,
          totalNS: this.totalNS,
          refresh: true,
          xs: [],
          xsTxt: [],
        },
        (a) => {
          if (this._sportRuler) {
            this._sportRuler.range = a;
          }
          if (this.timeOffsetEL && this._rangeRuler) {
            this.timeOffsetEL.textContent = ns2UnitS(a.startNS, this._rangeRuler.getScale());
          }
          if (this.loadComplete) {
            this.rangeChangeHandler?.(a);
          }
        }
      );
    }
    this._rangeRuler.frame.width = width;
    this._sportRuler.frame.width = width;
    this.timeRuler.frame.width = width;
  }

  setRangeNS(startNS: number, endNS: number): void {
    info('set startNS values :' + startNS + 'endNS values : ' + endNS);
    this._rangeRuler?.setRangeNS(startNS, endNS);
  }

  getRange(): TimeRange | undefined {
    return this._rangeRuler?.getRange();
  }

  updateWidth(width: number): void {
    this.dpr = window.devicePixelRatio || 1;
    this.canvas!.width = width - (this.totalEL?.clientWidth || 0);
    this.canvas!.height = this.shadowRoot!.host.clientHeight || 0;
    let oldWidth = this.canvas!.width;
    let oldHeight = this.canvas!.height;
    this.canvas!.width = Math.ceil(oldWidth * this.dpr);
    this.canvas!.height = Math.ceil(oldHeight * this.dpr);
    this.canvas!.style.width = oldWidth + 'px';
    this.canvas!.style.height = oldHeight + 'px';
    this.ctx?.scale(this.dpr, this.dpr);
    this.ctx?.translate(0, 0);
    this._rangeRuler!.frame.width = oldWidth;
    this._sportRuler!.frame.width = oldWidth;
    this.timeRuler!.frame.width = oldWidth;
    this._rangeRuler?.fillX();
    this.render();
  }

  documentOnMouseDown = (ev: MouseEvent): void => {
    if ((window as any).isSheetMove) return;
    this._rangeRuler?.mouseDown(ev);
  };

  documentOnMouseUp = (ev: MouseEvent): void => {
    if ((window as any).isSheetMove) return;
    this._rangeRuler?.mouseUp(ev);
    this.sportRuler?.mouseUp(ev);
  };

  documentOnMouseMove = (ev: MouseEvent, trace: SpSystemTrace): void => {
    trace.style.cursor = 'default';
    let x = ev.offsetX - (this.canvas?.offsetLeft || 0); // 鼠标的x轴坐标
    let y = ev.offsetY; // 鼠标的y轴坐标
    let findSlicestime = this.sportRuler?.findSlicesTime(x, y); // 查找帽子
    if (!findSlicestime) {
      // 如果在该位置没有找到一个“帽子”，则可以显示一个旗子。
      this.sportRuler?.showHoverFlag();
      this._rangeRuler?.mouseMove(ev, trace);
      if (this.sportRuler?.edgeDetection(ev)) {
        this.sportRuler?.mouseMove(ev);
      } else {
        this.sportRuler?.mouseOut(ev);
      }
    } else {
      this.sportRuler?.clearHoverFlag();
      this.sportRuler?.modifyFlagList(null); //重新绘制旗子，清除hover flag
    }
  };

  documentOnMouseOut = (ev: MouseEvent): void => {
    this._rangeRuler?.mouseOut(ev);
    this.sportRuler?.mouseOut(ev);
  };

  documentOnKeyPress = (ev: KeyboardEvent, currentSlicesTime?: CurrentSlicesTime): void => {
    if ((window as any).isSheetMove) return;
    if ((window as any).flagInputFocus) return;
    this._rangeRuler?.keyPress(ev, currentSlicesTime);
    this.sportRuler?.clearHoverFlag();
  };

  documentOnKeyUp = (ev: KeyboardEvent): void => {
    if ((window as any).isSheetMove) return;
    if ((window as any).flagInputFocus) return;
    this._rangeRuler?.keyUp(ev);
  };

  disconnectedCallback(): void { }

  firstRender = true;

  lineColor(): string {
    return window.getComputedStyle(this.canvas!, null).getPropertyValue('color');
  }

  render(): void {
    this.dpr = window.devicePixelRatio || 1;
    if (this.ctx) {
      this.ctx.fillStyle = 'transparent';
      this.ctx?.fillRect(0, 0, this.canvas?.width || 0, this.canvas?.height || 0);
      this.timeRuler?.draw();
      this._rangeRuler?.draw();
      this._sportRuler?.draw();
    } else {
      procedurePool.submitWithName(
        `timeline`,
        `timeline`,
        {
          offscreen: this.must ? this.offscreen : undefined, //是否离屏
          dpr: this.dpr, //屏幕dpr值
          hoverX: this.hoverX,
          hoverY: this.hoverY,
          canvasWidth: this.canvasWidth,
          canvasHeight: this.canvasHeight,
          keyPressCode: null,
          keyUpCode: null,
          lineColor: '#dadada',
          startNS: this.startNS,
          endNS: this.endNS,
          totalNS: this.totalNS,
          frame: this.frame,
        },
        this.must ? this.offscreen : undefined,
        (res: any) => {
          this.must = false;
        }
      );
    }
  }

  modifyFlagList(flag: Flag | null | undefined): void {
    this._sportRuler?.modifyFlagList(flag);
  }

  modifySlicesList(slicestime: SlicesTime | null | undefined): void {
    this._sportRuler?.modifySicesTimeList(slicestime);
  }
  cancelPressFrame(): void {
    this._rangeRuler?.cancelPressFrame();
  }

  cancelUpFrame(): void {
    this._rangeRuler?.cancelUpFrame();
  }

  stopWASD(ev: any): void {
    this._rangeRuler?.keyUp(ev);
  }

  drawTriangle(time: number, type: string): number | undefined {
    return this._sportRuler?.drawTriangle(time, type);
  }

  removeTriangle(type: string): void {
    this._sportRuler?.removeTriangle(type);
  }

  setSlicesMark(
    startTime: null | number = null,
    endTime: null | number = null,
    shiftKey: null | boolean = false
  ): SlicesTime | null | undefined {
    let sliceTime = this._sportRuler?.setSlicesMark(startTime, endTime, shiftKey);
    if (sliceTime && sliceTime != undefined) {
      this.traceSheetEL?.displayCurrent(sliceTime); // 给当前pane准备数据

      // 取最新创建的那个selection对象
      let selection = this.selectionList[this.selectionList.length - 1];
      if (selection) {
        selection.isCurrentPane = true; // 设置当前面板为可以显示的状态
        //把刚刚创建的slicetime和selection对象关联起来，以便后面再次选中“跑道”的时候显示对应的面板。
        this.selectionMap.set(sliceTime.id, selection);
        this.traceSheetEL?.rangeSelect(selection); // 显示选中区域对应的面板
      }
    }
    return sliceTime;
  }

  displayCollect(showCollect: boolean): void {
    if (showCollect) {
      this.collecBtn!.style.display = 'flex';
    } else {
      this.collecBtn!.style.display = 'none';
    }
  }

  initHtml(): string {
    return `
        <style>
        :host{
            box-sizing: border-box;
            display: flex;
            width: 100%;
            height: 148px;
            border-bottom: 1px solid var(--dark-background,#dadada);
            border-top: 1px solid var(--dark-background,#dadada);
        }
        *{
            box-sizing: border-box;
            user-select: none;
        }
        .root{
            width: 100%;
            height: 100%;
            display: grid;
            grid-template-rows: 100%;
            grid-template-columns: 248px 1fr;
            background: var(--dark-background4,#FFFFFF);
        }
        .total{
            display: grid;
            grid-template-columns: 1fr;
            grid-template-rows: min-content 1fr;
            background-color: transparent;
        }
        .panel{
            color: var(--dark-border,#dadada);
            width: 100%;
            height: 100%;
            overflow: visible;
            background-color: var(--dark-background4,#ffffff);
        }
        .time-div{
            box-sizing: border-box;
            width: 100%;
            border-top: 1px solid var(--dark-background,#dadada);
            height: 100%;display: flex;justify-content: space-between;
            background-color: var(--dark-background1,white);
            color: var(--dark-color1,#212121);
            font-size: 0.7rem;
            border-right: 1px solid var(--dark-background,#999);
            padding: 2px 6px;
            display: flex;
            justify-content: space-between;
            user-select: none;
            position: relative;
        }
        .time-total::after{
            content: " +";
        }
        .time-collect{
            position:absolute;
            right:5px;
            bottom:5px;
            color: #5291FF;
            display: none;
        }
        .time-collect[close] > .time-collect-arrow{
            transform: rotateZ(-180deg);
        }
        .collect_group{
            position:absolute;
            right:25px;
            bottom:5px;
            display: flex;
            flex-direction: row;
        }
        .collect_div{
            display: flex;
            align-items: center;
        }

        </style>
        <div class="root">
            <div class="total">
                <div style="width: 100%;height: 100px;background: var(--dark-background4,#F6F6F6)"></div>
                <div class="time-div">
                    <span class="time-total">10</span>
                    <span class="time-offset">0</span>
                    <div class="time-collect">
                        <lit-icon class="time-collect-arrow" name="caret-down" size="17"></lit-icon>
                    </div>
                    <div class="collect_group">
                        <div class="collect_div">
                            <input id="collect1" type="radio" style="cursor: pointer" checked name="collect_group" value="1"/>
                            <label>G1</label>
                        </div>
                        <div class="collect_div">
                            <input type="radio" style="cursor: pointer" name="collect_group" value="2"/>
                            <label>G2</label>
                        </div>
                    </div>
                    <collapse-button expand>123</collapse-button>
                </div>
            </div>
            <canvas class="panel"></canvas>
        </div>
        `;
  }
}
