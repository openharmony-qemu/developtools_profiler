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
import { TraceRow } from './base/TraceRow.js';
import { dpr } from './base/Extension.js';
import {
  drawFlagLineSegment,
  drawLines,
  drawLinkLines,
  drawLogsLineSegment,
  drawWakeUp,
  drawWakeUpList,
  PairPoint,
  Rect,
} from '../../database/ui-worker/ProcedureWorkerCommon.js';
import { Flag } from './timer-shaft/Flag.js';
import { TimerShaftElement } from './TimerShaftElement.js';
import { CpuStruct } from '../../database/ui-worker/ProcedureWorkerCPU.js';
import { WakeupBean } from '../../bean/WakeupBean.js';
import { LitIcon } from '../../../base-ui/icon/LitIcon.js';

const maxScale = 0.8; //收藏最大高度为界面最大高度的80%
const topHeight = 150; // 顶部cpu使用率部分高度固定为150px
const minHeight = 40; //泳道最低高度为40
const mouseMoveRange = 5;

@element('sp-chart-list')
export class SpChartList extends BaseElement {
  private static COLLECT_G1 = '1';
  private static COLLECT_G2 = '2';
  private collectEl1: HTMLDivElement | null | undefined;
  private collectEl2: HTMLDivElement | null | undefined;
  private groupTitle1: HTMLDivElement | null | undefined;
  private groupTitle2: HTMLDivElement | null | undefined;
  private icon1: LitIcon | null | undefined;
  private icon2: LitIcon | null | undefined;
  private removeCollectIcon1: LitIcon | null | undefined;
  private removeCollectIcon2: LitIcon | null | undefined;
  private rootEl: HTMLDivElement | null | undefined;
  private fragmentGroup1: DocumentFragment = document.createDocumentFragment();
  private fragmentGroup2: DocumentFragment = document.createDocumentFragment();
  private canvas: HTMLCanvasElement | null | undefined; //绘制收藏泳道图
  private canvasCtx: CanvasRenderingContext2D | undefined | null;
  private canResize: boolean = false;
  private isPress: boolean = false;
  private startPageY = 0;
  private startClientHeight: number = 0;
  private scrollTimer: any;
  private collect1Expand: boolean = true;
  private collect2Expand: boolean = true;
  private collectRowList1: Array<TraceRow<any>> = [];
  private collectRowList2: Array<TraceRow<any>> = [];
  private maxHeight = 0;
  private manualHeight = 0;

  initElements(): void {
    this.collectEl1 = this.shadowRoot?.querySelector<HTMLDivElement>('#collect-group-1');
    this.collectEl2 = this.shadowRoot?.querySelector<HTMLDivElement>('#collect-group-2');
    this.groupTitle1 = this.shadowRoot?.querySelector<HTMLDivElement>('#group-1-title');
    this.groupTitle2 = this.shadowRoot?.querySelector<HTMLDivElement>('#group-2-title');
    this.icon1 = this.shadowRoot?.querySelector<LitIcon>('#group_1_expand');
    this.icon2 = this.shadowRoot?.querySelector<LitIcon>('#group_2_expand');
    this.removeCollectIcon1 = this.shadowRoot?.querySelector<LitIcon>('#group_1_collect');
    this.removeCollectIcon2 = this.shadowRoot?.querySelector<LitIcon>('#group_2_collect');
    this.rootEl = this.shadowRoot?.querySelector<HTMLDivElement>('.root');
    this.canvas = this.shadowRoot?.querySelector<HTMLCanvasElement>('.panel-canvas');
    this.canvasCtx = this.canvas?.getContext('2d');
    window.subscribe(window.SmartEvent.UI.RowHeightChange, (data) => {
      this.resizeHeight();
      this.scrollTop = 0;
      this.refreshFavoriteCanvas();
    });
    this.icon1?.addEventListener('click', () => {
      this.collect1Expand = !this.collect1Expand;
      if (this.collect1Expand) {
        this.icon1!.style.transform = 'rotateZ(0deg)';
        this.collectEl1?.appendChild(this.fragmentGroup1);
        this.resizeHeight();
      } else {
        this.icon1!.style.transform = 'rotateZ(-90deg)';
        this.collectRowList1.forEach((row) => this.fragmentGroup1.appendChild(row));
        this.resizeHeight();
      }
    });
    this.icon2?.addEventListener('click', () => {
      this.collect2Expand = !this.collect2Expand;
      if (this.collect2Expand) {
        this.icon2!.style.transform = 'rotateZ(0deg)';
        this.collectEl2?.appendChild(this.fragmentGroup2);
        this.resizeHeight();
      } else {
        this.icon2!.style.transform = 'rotateZ(-90deg)';
        this.collectRowList2.forEach((row) => this.fragmentGroup2.appendChild(row));
        this.resizeHeight();
      }
    });
    this.removeCollectIcon1?.addEventListener('click', () => {
      for (let i = 0; i < this.collectRowList1.length; i++) {
        this.collectRowList1[i].collectEL?.click();
        i--;
      }
    });
    this.removeCollectIcon2?.addEventListener('click', () => {
      for (let i = 0; i < this.collectRowList2.length; i++) {
        this.collectRowList2[i].collectEL?.click();
        i--;
      }
    });
  }

  private resizeHeight(): void {
    this.maxHeight = 0;
    this.collectEl1!.childNodes.forEach((item) => (this.maxHeight += (item as any).clientHeight));
    this.collectEl2!.childNodes.forEach((item) => (this.maxHeight += (item as any).clientHeight));
    if (this.groupTitle1) {
      this.maxHeight += this.groupTitle1.clientHeight;
    }
    if (this.groupTitle2) {
      this.maxHeight += this.groupTitle2.clientHeight;
    }

    this.maxHeight = Math.min(this.getMaxLimitHeight(), this.maxHeight);
    if (this.manualHeight > 0) {
      this.style.height = `${Math.min(this.maxHeight, this.manualHeight)}px`;
    } else {
      this.style.height = `${this.maxHeight}px`;
    }
  }

  private getMaxLimitHeight(): number {
    return (this.parentElement!.clientHeight - topHeight) * maxScale;
  }

  getCollectRows(condition: string): Array<TraceRow<any>> | [] {
    const result = this.rootEl?.querySelectorAll<TraceRow<any>>(condition);
    if (result) {
      return Array.from(result);
    } else {
      return [];
    }
  }

  getCollectRow(condition: string): TraceRow<any> | null {
    return this.rootEl!.querySelector<TraceRow<any>>(condition);
  }

  getAllCollectRows(): Array<TraceRow<any>> {
    return [...this.collectRowList1, ...this.collectRowList2];
  }

  getAllSelectCollectRows(): Array<TraceRow<any>> {
    const rows: Array<TraceRow<any>> = [];
    for (const row of this.collectRowList1) {
      if (row.checkType === '2') {
        rows.push(row);
      }
    }
    for (const row of this.collectRowList2) {
      if (row.checkType === '2') {
        rows.push(row);
      }
    }
    return rows;
  }

  insertRowBefore(node: Node, child: Node): void {
    if (child === null || (child as TraceRow<any>).collectGroup === (node as TraceRow<any>).collectGroup) {
      if ((node as TraceRow<any>).collectGroup === SpChartList.COLLECT_G1) {
        this.collectEl1!.insertBefore(node, child);
        this.collectRowList1 = Array.from(this.collectEl1!.children) as TraceRow<any>[];
      } else {
        this.collectEl2!.insertBefore(node, child);
        this.collectRowList2 = Array.from(this.collectEl2!.children) as TraceRow<any>[];
      }
    }
  }

  reset(): void {
    this.maxHeight = 0;
    this.style.height = 'auto';
    this.clearRect();
    this.collect1Expand = true;
    this.collect2Expand = true;
    this.icon1!.style.transform = 'rotateZ(0deg)';
    this.icon2!.style.transform = 'rotateZ(0deg)';
    this.collectRowList1 = [];
    this.collectRowList2 = [];
    this.updateGroupDisplay();
    this.fragmentGroup1.childNodes.forEach((node) => this.fragmentGroup1.removeChild(node));
    this.fragmentGroup2.childNodes.forEach((node) => this.fragmentGroup2.removeChild(node));
    this.collectEl1!.querySelectorAll<TraceRow<any>>(`trace-row`).forEach((row) => {
      row.clearMemory();
      this.collectEl1!.removeChild(row);
    });
    this.collectEl2!.querySelectorAll<TraceRow<any>>(`trace-row`).forEach((row) => {
      row.clearMemory();
      this.collectEl2!.removeChild(row);
    });
  }

  context(): CanvasRenderingContext2D | undefined | null {
    return this.canvasCtx;
  }

  getCanvas(): HTMLCanvasElement | null | undefined {
    return this.canvas;
  }

  connectedCallback(): void {
    super.connectedCallback();
    window.addEventListener('mousedown', this.onMouseDown);
    window.addEventListener('mouseup', this.onMouseUp);
    window.addEventListener('mousemove', this.onMouseMove);
    this.addEventListener('scroll', this.onScroll, { passive: true });
  }

  disconnectedCallback(): void {
    super.disconnectedCallback();
    window.removeEventListener('mousedown', this.onMouseDown);
    window.removeEventListener('mouseup', this.onMouseUp);
    window.removeEventListener('mousemove', this.onMouseMove);
    this.removeEventListener('scroll', this.onScroll);
  }

  onScroll = (ev: Event): void => {
    this.canvas!.style.transform = `translateY(${this.scrollTop}px)`;
    if (this.scrollTimer) {
      clearTimeout(this.scrollTimer);
    }
    this.scrollTimer = setTimeout(() => {
      TraceRow.range!.refresh = true;
      window.publish(window.SmartEvent.UI.RefreshCanvas, {});
    }, 100);
    window.publish(window.SmartEvent.UI.RefreshCanvas, {});
  };

  onMouseDown = (ev: MouseEvent): void => {
    this.isPress = true;
    this.startPageY = ev.pageY;
    this.startClientHeight = this.clientHeight;
    if (this.containPoint(ev)) {
      if (
        this.getBoundingClientRect().bottom > ev.pageY - mouseMoveRange &&
        this.getBoundingClientRect().bottom < ev.pageY + mouseMoveRange
      ) {
        this.style.cursor = 'row-resize';
        this.canResize = true;
      } else {
        this.style.cursor = 'default';
        this.canResize = false;
      }
    }
  };

  onMouseMove = (ev: MouseEvent): void => {
    if (this.containPoint(ev)) {
      if (
        this.getBoundingClientRect().bottom > ev.pageY - mouseMoveRange &&
        this.getBoundingClientRect().bottom < ev.pageY + mouseMoveRange
      ) {
        this.style.cursor = 'row-resize';
      } else {
        this.style.cursor = 'default';
      }
    }
    //防止点击触发move时间
    if (Math.abs(ev.pageY - this.startPageY) < 2) {
      return;
    }
    if (this.canResize && this.isPress) {
      (window as any).rowResize = true;
      // 拖动超过所有泳道最大高度 或小于一个泳道的高度，不支持拖动
      let newHeight = this.startClientHeight + ev.pageY - this.startPageY;
      if (newHeight > this.maxHeight || newHeight > this.getMaxLimitHeight() || newHeight < minHeight) {
        // 超出最大最小高度时触发mouseup事件
        const mouseUpEvent = new MouseEvent('mouseup', {
          bubbles: true,
          cancelable: true,
          view: window,
          button: 0,
          buttons: 0,
          clientX: ev.clientX, // 鼠标在窗口中的水平坐标
          clientY: ev.clientY, // 鼠标在窗口中的垂直坐标
        });

        // 获取需要触发事件的元素
        const element = document.getElementById('myElement');
        // 触发 mouseup 事件
        element?.dispatchEvent(mouseUpEvent);
        ev.stopPropagation();
        return;
      }
      this!.style.height = `${newHeight}px`;
    } else {
      (window as any).rowResize = false;
    }
  };

  onMouseUp = (ev: MouseEvent): void => {
    this.isPress = false;
    this.canResize = false;
    (window as any).rowResize = false;
    this.refreshFavoriteCanvas();
  };

  insertRow(row: TraceRow<any>, group: string, updateGroup: boolean): void {
    this.style.display = 'flex';
    let collectGroup = !updateGroup && row.collectGroup ? row.collectGroup : group;
    if (row.collectGroup !== SpChartList.COLLECT_G1 && row.collectGroup !== SpChartList.COLLECT_G2) {
      row.collectGroup = group;
    }
    if (updateGroup) {
      row.collectGroup = group;
    }
    if (collectGroup === SpChartList.COLLECT_G1) {
      if (!this.collect1Expand) {
        this.collect1Expand = true;
        this.icon1!.style.transform = 'rotateZ(0deg)';
      }
      if (this.collectRowList1.indexOf(row) === -1) {
        this.collectRowList1.push(row);
      }
      if (!this.fragmentGroup1.contains(row)) {
        this.fragmentGroup1.appendChild(row);
      }
      this.collectEl1?.appendChild(this.fragmentGroup1);
    } else {
      if (!this.collect2Expand) {
        this.collect2Expand = true;
        this.icon2!.style.transform = 'rotateZ(0deg)';
      }
      if (this.collectRowList2.indexOf(row) === -1) {
        this.collectRowList2.push(row);
      }
      if (!this.fragmentGroup2.contains(row)) {
        this.fragmentGroup2.appendChild(row);
      }
      this.collectEl2!.appendChild(this.fragmentGroup2);
    }
    this.updateGroupDisplay();
    this.resizeHeight();
    this.scrollTo({ top: this.scrollHeight });
    this.refreshFavoriteCanvas();
    row.currentContext = this.canvasCtx;
  }

  deleteRow(row: TraceRow<any>, clearCollectGroup: boolean): void {
    if (row.collectGroup === SpChartList.COLLECT_G1) {
      this.collectRowList1.splice(this.collectRowList1.indexOf(row), 1);
      if (!this.fragmentGroup1.contains(row)) {
        this.fragmentGroup1.appendChild(row);
      }
      this.fragmentGroup1.removeChild(row);
    } else {
      this.collectRowList2.splice(this.collectRowList2.indexOf(row), 1);
      if (!this.fragmentGroup2.contains(row)) {
        this.fragmentGroup2.appendChild(row);
      }
      this.fragmentGroup2.removeChild(row);
    }
    if (clearCollectGroup) {
      row.collectGroup = undefined;
    }
    this.updateGroupDisplay();
    this.resizeHeight();
    this.scrollTop = 0;
    this.refreshFavoriteCanvas();
    row.currentContext = undefined;
    if (this.collectRowList1.length === 0 && this.collectRowList2.length === 0) {
      this.style.height = 'auto';
      this.style.display = 'none';
      this.manualHeight = 0;
    }
  }

  updateGroupDisplay(): void {
    this.groupTitle1!.style.display = this.collectRowList1.length === 0 ? 'none' : 'flex';
    this.groupTitle2!.style.display = this.collectRowList2.length === 0 ? 'none' : 'flex';
  }

  clearRect(): void {
    this.canvasCtx?.clearRect(0, 0, this.canvas?.clientWidth ?? 0, this.canvas?.clientHeight ?? 0);
  }

  drawLines(xs: number[] | undefined, color: string): void {
    drawLines(this.canvasCtx!, xs ?? [], this.clientHeight, color);
  }

  drawFlagLineSegment(
    hoverFlag: Flag | undefined | null,
    selectFlag: Flag | undefined | null,
    tse: TimerShaftElement
  ): void {
    drawFlagLineSegment(
      this.canvasCtx,
      hoverFlag,
      selectFlag,
      {
        x: 0,
        y: 0,
        width: TraceRow.FRAME_WIDTH,
        height: this.canvas?.clientHeight,
      },
      tse
    );
  }

  drawWakeUp(): void {
    drawWakeUp(
      this.canvasCtx,
      CpuStruct.wakeupBean,
      TraceRow.range!.startNS,
      TraceRow.range!.endNS,
      TraceRow.range!.totalNS,
      {
        x: 0,
        y: 0,
        width: TraceRow.FRAME_WIDTH,
        height: this.canvas!.clientHeight!,
      } as Rect
    );
  }

  drawWakeUpList(bean: WakeupBean): void {
    drawWakeUpList(this.canvasCtx, bean, TraceRow.range!.startNS, TraceRow.range!.endNS, TraceRow.range!.totalNS, {
      x: 0,
      y: 0,
      width: TraceRow.FRAME_WIDTH,
      height: this.canvas!.clientHeight!,
    } as Rect);
  }

  drawLogsLineSegment(bean: Flag | null | undefined, timeShaft: TimerShaftElement): void {
    drawLogsLineSegment(
      this.canvasCtx,
      bean,
      {
        x: 0,
        y: 0,
        width: TraceRow.FRAME_WIDTH,
        height: this.canvas!.clientHeight,
      },
      timeShaft
    );
  }

  drawLinkLines(nodes: PairPoint[][], tse: TimerShaftElement, isFavorite: boolean, favoriteHeight: number): void {
    drawLinkLines(this.canvasCtx!, nodes, tse, isFavorite, favoriteHeight);
  }

  refreshFavoriteCanvas(): void {
    this.canvas!.style.width = `${this.clientWidth - 248}px`;
    this.canvas!.style.left = `248px`;
    this.canvas!.width = this.canvas?.clientWidth! * dpr();
    this.canvas!.height = this.clientHeight * dpr();
    this.canvas!.getContext('2d')!.scale(dpr(), dpr());
    window.publish(window.SmartEvent.UI.RefreshCanvas, {});
  }

  initHtml(): string {
    return `
<style>
:host{
    display: none;
    width: 100%;
    height: auto;
    overflow-anchor: none;
    z-index: 1;
    /*background-color: #00a3f5;*/
    box-shadow: 0 10px 10px #00000044;
    position: relative;
    overflow: auto;
    overflow-x: hidden;
    scroll-behavior: smooth;
}
.root{
    width: 100%;
    box-sizing: border-box;
}
.panel-canvas{
    position: absolute;
    top: 0;
    right: 0px;
    bottom: 0px;
    box-sizing: border-box;
}
.icon:hover {
 color:#ecb93f;
}
.icon {
    margin-right: 10px;
    cursor: pointer;
}
</style>
<canvas id="canvas-panel" class="panel-canvas" ondragstart="return false"></canvas>
<div class="root">
    <div id="group-1-title" style="background-color: #efefef;padding: 10px;align-items: center">
        <lit-icon id="group_1_expand" class="icon" name="caret-down" size="19"></lit-icon>
        <span style="width: 184px;font-size: 10px;color: #898989">G1</span>
        <lit-icon id="group_1_collect" name="star-fill" style="color: #5291FF;cursor: pointer" size="19"></lit-icon>
    </div>
    <div id="collect-group-1"></div>
    <div id="group-2-title" style="background-color: #efefef;padding: 10px;align-items: center">
        <lit-icon id="group_2_expand" class="icon" name="caret-down" size="19"></lit-icon>
        <span style="width: 184px;font-size: 10px;color: #898989">G2</span>
        <lit-icon id="group_2_collect" name="star-fill" style="color: #f56940;cursor: pointer" size="19"></lit-icon>
    </div>
    <div id="collect-group-2"></div>
</div>
`;
  }
}
