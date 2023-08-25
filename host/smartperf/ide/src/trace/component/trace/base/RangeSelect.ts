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

import { RangeSelectStruct, TraceRow } from './TraceRow.js';
import { Rect } from '../timer-shaft/Rect.js';
import { ns2x, TimerShaftElement } from '../TimerShaftElement.js';
import { info } from '../../../../log/Log.js';
import './Extension.js';
import { SpSystemTrace } from '../../SpSystemTrace.js';

export class RangeSelect {
  private rowsEL: HTMLDivElement | undefined | null;
  private rowsPaneEL: HTMLDivElement | undefined | null;
  private favoriteRowsEL: HTMLDivElement | undefined | null;
  isMouseDown: boolean = false;
  public rangeTraceRow: Array<TraceRow<any>> | undefined;
  public selectHandler: ((ds: Array<TraceRow<any>>, refreshCheckBox: boolean) => void) | undefined;
  private startX: number = 0;
  private endX: number = 0;
  private startY: number = 0;
  private endY: number = 0;
  private startY2: number = 0;
  private endY2: number = 0;
  private timerShaftEL: TimerShaftElement | null | undefined;
  private timerShaftDragEL: HTMLDivElement | null | undefined;
  private isHover: boolean = false;
  private movingMark: string = '';
  private mark: { startMark: number; endMark: number } = {
    startMark: 0,
    endMark: 0,
  };
  private readonly spacerEL: HTMLDivElement;
  private trace: SpSystemTrace | null | undefined;
  drag = false;
  constructor(trace: SpSystemTrace | null | undefined) {
    this.trace = trace;
    this.timerShaftEL = trace?.timerShaftEL;
    this.timerShaftDragEL = this.timerShaftEL?.shadowRoot?.querySelector('.total > div:nth-child(1)');
    this.spacerEL = trace?.spacerEL!;
    this.rowsEL = trace?.rowsEL;
    this.rowsPaneEL = trace?.rowsPaneEL;
    this.favoriteRowsEL = trace?.favoriteRowsEL;
  }

  isInRowsEl(ev: MouseEvent): boolean {
    return this.rowsPaneEL!.containPoint(ev, { left: 248 });
  }

  isInSpacerEL(ev: MouseEvent): boolean {
    return this.spacerEL.containPoint(ev, { left: 248 });
  }

  mouseDown(eventDown: MouseEvent) {
    if (this.isHover) {
      this.isMouseDown = true;
      return;
    }
    if (this.isInRowsEl(eventDown)) {
      this.rangeTraceRow = [];
      this.isMouseDown = true;
      TraceRow.rangeSelectObject = undefined;
      this.startX = eventDown.pageX - this.rowsEL!.getBoundingClientRect().left - 248;
      if (this.isInSpacerEL(eventDown)) {
        this.startY = 0;
        this.startY2 = eventDown.pageY - this.spacerEL.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
      } else {
        this.startY =
          eventDown.pageY - this.rowsEL!.getBoundingClientRect().top + this.spacerEL.getBoundingClientRect().height;
        this.startY2 = eventDown.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
      }
    }
  }

  mouseUp(mouseEventUp: MouseEvent) {
    if (this.drag) {
      this.endX = mouseEventUp.pageX - this.rowsEL!.getBoundingClientRect().left - 248;
      if (this.isInSpacerEL(mouseEventUp)) {
        this.endY = 0;
        this.endY2 = mouseEventUp.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
      } else {
        this.endY =
          mouseEventUp.pageY - this.rowsEL!.getBoundingClientRect().top + this.spacerEL.getBoundingClientRect().height;
        this.endY2 = mouseEventUp.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
      }
      if (this.selectHandler) {
        this.selectHandler(this.rangeTraceRow || [], !this.isHover);
      }
    }
    this.isMouseDown = false;
  }

  isDrag(): boolean {
    return this.startX != this.endX;
  }

  isTouchMark(ev: MouseEvent): boolean {
    let notTimeHeight: boolean = this.rowsPaneEL!.containPoint(ev, {
      left: 248,
      top: -45,
    });
    if (!notTimeHeight) {
      return false;
    }
    if ((this.rangeTraceRow?.isEmpty() ?? false) && !this.isMouseDown) {
      this.isHover = false;
    }
    return notTimeHeight && (this.rangeTraceRow?.isNotEmpty() ?? false) && !this.isMouseDown;
  }

  mouseOut(mouseEventOut: MouseEvent) {
    if (this.drag) {
      this.endX = this.rowsEL!.getBoundingClientRect().right - this.rowsEL!.getBoundingClientRect().left - 248;
      if (this.isInSpacerEL(mouseEventOut)) {
        this.endY = 0;
        this.endY2 = mouseEventOut.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
      } else {
        this.endY =
          mouseEventOut.pageY - this.rowsEL!.getBoundingClientRect().top + this.spacerEL.getBoundingClientRect().height;
        this.endY2 = mouseEventOut.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
      }
      if (this.selectHandler && this.isMouseDown) {
        this.selectHandler(this.rangeTraceRow || [], !this.isHover);
      }
    }
    document.getSelection()?.removeAllRanges();
    this.isMouseDown = false;
  }

  mouseMove(rows: Array<TraceRow<any>>, ev: MouseEvent) {
    if (this.isTouchMark(ev) && TraceRow.rangeSelectObject) {
      info('isTouchMark');
      let x1 =
        ((TraceRow.rangeSelectObject!.startNS! - TraceRow.range!.startNS) *
          (this.timerShaftEL?.canvas?.clientWidth || 0)) /
        (TraceRow.range!.endNS - TraceRow.range!.startNS);
      let x2 =
        ((TraceRow.rangeSelectObject!.endNS! - TraceRow.range!.startNS) *
          (this.timerShaftEL?.canvas?.clientWidth || 0)) /
        (TraceRow.range!.endNS - TraceRow.range!.startNS);
      this.mark = { startMark: x1, endMark: x2 };
      let mouseX = ev.pageX - this.rowsPaneEL!.getBoundingClientRect().left - 248;
      if (mouseX > x1 - 5 && mouseX < x1 + 5) {
        this.isHover = true;
        document.body.style.cursor = 'ew-resize';
        this.movingMark = x1 < x2 ? 'markA' : 'markB';
      } else if (mouseX > x2 - 5 && mouseX < x2 + 5) {
        this.isHover = true;
        document.body.style.cursor = 'ew-resize';
        this.movingMark = x2 < x1 ? 'markA' : 'markB';
      } else {
        this.isHover = false;
        document.body.style.cursor = 'default';
      }
    } else {
      document.body.style.cursor = 'default';
    }
    if (this.isHover && this.isMouseDown) {
      let rangeSelect: RangeSelectStruct | undefined;
      this.rangeTraceRow = rows.filter((it) => {
        if (it.rangeSelect) {
          if (!rangeSelect) {
            rangeSelect = new RangeSelectStruct();
            let mouseX = ev.pageX - this.rowsEL!.getBoundingClientRect().left - 248;
            mouseX = mouseX < 0 ? 0 : mouseX;
            let markA = this.movingMark == 'markA' ? mouseX : this.mark.startMark;
            let markB = this.movingMark == 'markB' ? mouseX : this.mark.endMark;
            let startX = markA < markB ? markA : markB;
            let endX = markB < markA ? markA : markB;
            rangeSelect.startX = startX;
            rangeSelect.endX = endX;
            rangeSelect.startNS = RangeSelect.SetNS(it, startX);
            rangeSelect.endNS = RangeSelect.SetNS(it, endX);
            if (rangeSelect.startNS <= TraceRow.range!.startNS) {
              rangeSelect.startNS = TraceRow.range!.startNS;
            }
            if (rangeSelect.endNS >= TraceRow.range!.endNS) {
              rangeSelect.endNS = TraceRow.range!.endNS;
            }
            if (startX < 0) {
              rangeSelect.startNS = TraceRow.rangeSelectObject!.startNS!;
            }
            if (endX > it.frame.width) {
              rangeSelect.endNS = TraceRow.rangeSelectObject!.endNS!;
            }
          }
          TraceRow.rangeSelectObject = rangeSelect;
          return true;
        }
      });
      this.timerShaftEL!.sportRuler!.isRangeSelect = (this.rangeTraceRow?.length || 0) > 0;
      this.timerShaftEL!.sportRuler!.draw();
      return;
    }
    if (!this.isMouseDown) {
      this.timerShaftEL!.sportRuler!.isRangeSelect = this.rangeTraceRow?.isNotEmpty() ?? false;
      this.timerShaftEL!.sportRuler!.draw();
      return;
    }
    this.endX = ev.pageX - this.rowsEL!.getBoundingClientRect().left - 248;
    if (this.isInSpacerEL(ev)) {
      this.endY = 0;
      this.endY2 = ev.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
    } else {
      this.endY = ev.pageY - this.rowsEL!.getBoundingClientRect().top + this.spacerEL.getBoundingClientRect().height;
      this.endY2 = ev.pageY - this.spacerEL!.getBoundingClientRect().top - this.rowsPaneEL!.scrollTop;
    }
    let scrollTop = this.rowsPaneEL?.scrollTop || 0;
    let xMin = this.startX < this.endX ? this.startX : this.endX;
    let xMax = this.startX > this.endX ? this.startX : this.endX;
    let yMin = this.startY < this.endY ? this.startY : this.endY;
    let yMax = this.startY > this.endY ? this.startY : this.endY;
    let rangeSelect: RangeSelectStruct | undefined;
    let spacerRect = this.favoriteRowsEL!.getBoundingClientRect();
    let rowsRect = this.rowsPaneEL!.getBoundingClientRect();
    this.rangeTraceRow = rows.filter((it) => {
      let rt: Rect;
      let bound: DOMRect | any;
      let itRect: Rect;
      if (it.collect) {
        bound = it.getBoundingClientRect();
        itRect = Rect.getIntersect(bound, spacerRect);
        rt = new Rect(xMin, Math.min(this.startY2, this.endY2), xMax - xMin, Math.abs(this.startY2 - this.endY2));
      } else {
        bound = it.getBoundingClientRect();
        if (spacerRect.height > 0 && bound.y + bound.height < spacerRect.y + spacerRect.height) {
          it.rangeSelect = false;
          return false;
        }
        itRect = Rect.getIntersect(
          bound,
          new Rect(rowsRect.x, rowsRect.y + spacerRect.height, rowsRect.width, rowsRect.height - spacerRect.height)
        );
        rt = new Rect(xMin, yMin - scrollTop, xMax - xMin, yMax - yMin);
      }
      itRect.x -= 248;
      itRect.y -= 195;
      if (Rect.intersect(itRect, rt)) {
        if (!rangeSelect) {
          it.setTipLeft(0, null);
          rangeSelect = new RangeSelectStruct();
          let startX = Math.floor(rt.x <= 0 ? 0 : rt.x);
          let endX = Math.floor(rt.x + rt.width > it.frame.width ? it.frame.width : rt.x + rt.width);
          rangeSelect.startX = startX;
          rangeSelect.endX = endX;
          rangeSelect.startNS = RangeSelect.SetNS(it, startX);
          rangeSelect.endNS = RangeSelect.SetNS(it, endX);
        }
        TraceRow.rangeSelectObject = rangeSelect;
        it.rangeSelect = true;
        return true;
      } else {
        it.rangeSelect = false;
        return false;
      }
    });
    this.timerShaftEL!.sportRuler!.isRangeSelect = this.rangeTraceRow?.length > 0;
    this.timerShaftEL!.sportRuler!.draw();
  }

  static SetNS(row: TraceRow<any>, num: number): number {
    return Math.floor(
      ((TraceRow.range!.endNS - TraceRow.range!.startNS) * num) / row.frame.width + TraceRow.range!.startNS!
    );
  }
}
