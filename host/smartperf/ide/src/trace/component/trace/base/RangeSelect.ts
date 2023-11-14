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
  // private favoriteRowsEL: HTMLDivElement | undefined | null;
  isMouseDown: boolean = false;
  public rangeTraceRow: Array<TraceRow<any>> | undefined;
  public selectHandler: ((ds: Array<TraceRow<any>>, refreshCheckBox: boolean) => void) | undefined;
  private startPageX: number = 0;
  private startPageY: number = 0;
  private endPageX: number = 0;
  private endPageY: number = 0;
  private timerShaftEL: TimerShaftElement | null | undefined;
  private timerShaftDragEL: HTMLDivElement | null | undefined;
  private isHover: boolean = false;
  private movingMark: string = '';
  private mark: { startMark: number; endMark: number } = {
    startMark: 0,
    endMark: 0,
  };
  // private readonly spacerEL: HTMLDivElement;
  private trace: SpSystemTrace | null | undefined;
  drag = false;
  constructor(trace: SpSystemTrace | null | undefined) {
    this.trace = trace;
    this.timerShaftEL = trace?.timerShaftEL;
    this.timerShaftDragEL = this.timerShaftEL?.shadowRoot?.querySelector('.total > div:nth-child(1)');
    // this.spacerEL = trace?.spacerEL!;
    this.rowsEL = trace?.rowsEL;
    this.rowsPaneEL = trace?.rowsPaneEL;
    // this.favoriteRowsEL = trace?.favoriteRowsEL;
  }

  isInRowsEl(ev: MouseEvent): boolean {
    return this.rowsPaneEL!.containPoint(ev, { left: 248 });
  }

  isInSpacerEL(ev: MouseEvent): boolean {
    return this.trace!.favoriteChartListEL!.containPoint(ev, { left: 248 });
  }

  mouseDown(eventDown: MouseEvent) {
    this.startPageX = eventDown.pageX;
    this.startPageY = eventDown.pageY;
    if (this.isHover) {
      this.isMouseDown = true;
      return;
    }
    this.rangeTraceRow = [];
    this.isMouseDown = true;
    TraceRow.rangeSelectObject = undefined;
  }

  mouseUp(mouseEventUp: MouseEvent) {
    this.endPageX = mouseEventUp.pageX;
    this.endPageY = mouseEventUp.pageY;
    if (this.drag) {
      if (this.selectHandler) {
        this.selectHandler(this.rangeTraceRow || [], !this.isHover);
      }
    }
    this.isMouseDown = false;
  }

  isDrag(): boolean {
    return this.startPageX != this.endPageX;
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
    this.endPageX = mouseEventOut.pageX;
    this.endPageY = mouseEventOut.pageY;
    if (this.drag) {
      if (this.selectHandler && this.isMouseDown) {
        this.selectHandler(this.rangeTraceRow || [], !this.isHover);
      }
    }
    document.getSelection()?.removeAllRanges();
    this.isMouseDown = false;
  }

  mouseMove(rows: Array<TraceRow<any>>, ev: MouseEvent) {
    this.endPageX = ev.pageX;
    this.endPageY = ev.pageY;
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
    let rangeSelect: RangeSelectStruct | undefined;
    this.rangeTraceRow = rows.filter((it) => {
      if (
        Rect.intersect(it.getBoundingClientRect(), {
          x: Math.min(this.startPageX, this.endPageX),
          y: Math.min(this.startPageY, this.endPageY),
          width: Math.abs(this.startPageX - this.endPageX),
          height: Math.abs(this.startPageY - this.endPageY),
        } as Rect)
      ) {
        if (!rangeSelect) {
          it.setTipLeft(0, null);
          rangeSelect = new RangeSelectStruct();
          let startX = Math.min(this.startPageX, this.endPageX) - it.describeEl!.getBoundingClientRect().right;
          let endX = Math.max(this.startPageX, this.endPageX) - it.describeEl!.getBoundingClientRect().right;
          if (startX <= 0) startX = 0;
          if (endX > it.frame.width) endX = it.frame.width;
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
