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

// @ts-ignore
import { RangeSelect } from '../../../../../dist/trace/component/trace/base/RangeSelect.js';
// @ts-ignore
import { TraceRow } from '../../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { SpSystemTrace } from '../../../../../dist/trace/component/SpSystemTrace.js';
jest.mock('../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('RangeSelect Test', () => {
  beforeAll(() => {});

  it('Utils Test01', () => {
    let rangeSelect = new RangeSelect();
    expect(rangeSelect).not.toBeUndefined();
  });

  it('Utils Test02', () => {
    let rangeSelect = new RangeSelect();
    rangeSelect.rowsEL = document.createElement('div');
    let mouseEvent = new MouseEvent('mousedown', {
      button: 1,
      buttons: 0,
      clientX: 21,
      clientY: 100,
      screenX: 255,
      screenY: 321,
    });
    let htmlElement = document.createElement('div');
    rangeSelect.rowsPaneEL = htmlElement;
    rangeSelect.timerShaftDragEL = jest.fn(() => true);
    rangeSelect.timerShaftDragEL.timerShaftDragEL = jest.fn(() => true);
    expect(rangeSelect.isInRowsEl(mouseEvent)).toBeFalsy();
  });
  it('Utils Test09', () => {
    let rangeSelect = new RangeSelect(new SpSystemTrace());
    rangeSelect.rowsEL = document.createElement('div');
    let mouseEvent = new MouseEvent('mousedown', {
      button: 0,
      buttons: 65,
      clientX: 63,
      clientY: 1300,
      screenX: 325,
      screenY: 325,
    });
    let htmlElement = document.createElement('div');
    rangeSelect.spacerEL = htmlElement;
    expect(rangeSelect.isInSpacerEL(mouseEvent)).toBeFalsy();
  });

  it('Utils Test05', () => {
    let rangeSelect = new RangeSelect();
    rangeSelect.isInRowsEl = jest.fn(() => true);
    rangeSelect.rowsEL = {
      // offsetTop: 100,
      offsetHeight: 71,
      offsetLeft: 15,
      offsetWidth: 134,
    };
    let mouseEvent = new MouseEvent('mousedown', {
      // @ts-ignore
      offsetY: 1,
      offsetX: 1,
      button: 1,
      buttons: 0,
      clientX: 2,
      clientY: 101,
      screenX: 255,
      screenY: 321,
    });
    let divElement = document.createElement('div');
    rangeSelect.rowsPaneEL = divElement;
    rangeSelect.spacerEL = jest.fn(() => true);
    rangeSelect.spacerEL.offsetTop = jest.fn(() => true);
    rangeSelect.rowsPaneEL.scrollTop = 0;
    rangeSelect.rowsEL.getBoundingClientRect = jest.fn(() => true);
    let htmlElement = document.createElement('div');
    rangeSelect.spacerEL = htmlElement;
    expect(rangeSelect.mouseDown(mouseEvent)).toBeUndefined();
  });

  it('Utils Test07', () => {
    let rangeSelect = new RangeSelect();
    rangeSelect.isInRowsEl = jest.fn(() => true);
    rangeSelect.isDrag = jest.fn(() => true);

    rangeSelect.rowsEL = {
      offsetTop: 102,
      offsetHeight: 540,
      offsetLeft: 390,
      offsetWidth: 1102,
    };
    let mouseEvent = new MouseEvent('mousedown', {
      // @ts-ignore
      offsetY: 1,
      offsetX: 1,
      button: 2,
      buttons: 0,
      clientX: 2,
      clientY: 100,
      screenX: 252,
      screenY: 325,
    });
    rangeSelect.spacerEL = jest.fn(() => true);
    rangeSelect.spacerEL.offsetTop = jest.fn(() => 1);
    rangeSelect.drag = true;
    rangeSelect.rowsEL = jest.fn(() => true);
    rangeSelect.rowsEL.getBoundingClientRect = jest.fn(() => true);
    rangeSelect.spacerEL.containPoint = jest.fn(() => true);
    rangeSelect.spacerEL.getBoundingClientRect = jest.fn(() => true);
    rangeSelect.rowsPaneEL = jest.fn(() => true);
    rangeSelect.rowsPaneEL.scrollTop = jest.fn(() => true);
    expect(rangeSelect.mouseUp(mouseEvent)).toBeUndefined();
  });

  it('Utils Test08', () => {
    let rangeSelect = new RangeSelect(new SpSystemTrace());
    rangeSelect.isInRowsEl = jest.fn(() => true);
    rangeSelect.isDrag = jest.fn(() => true);
    rangeSelect.isMouseDown = true;
    rangeSelect.isHover  = true;
    let rowsELDiv = document.createElement('div');
    rangeSelect.rowsEL = rowsELDiv;
    let rows = [
      {
        frame: {
          x: 1,
          width: 10,
          y: 2,
          height: 10,
        },
        offsetTop: 100,
        offsetHeight: 200,
        offsetLeft: 0,
        offsetWidth: 100,
      },
    ];
    let mouseEvent = new MouseEvent('mousedown', {
      // @ts-ignore
      offsetY: 12,
      offsetX: 1,
      button: 0,
      buttons: 0,
      clientX: 3,
      clientY: 100,
      screenX: 252,
      screenY: 325,
    });
    rangeSelect.timerShaftDragEL = jest.fn(() => true);
    rangeSelect.timerShaftEL = jest.fn(() => true);
    rangeSelect.timerShaftEL.sportRuler = jest.fn(() => true);
    rangeSelect.timerShaftEL.sportRuler.isRangeSelect = jest.fn(() => true);
    rangeSelect.timerShaftEL.sportRuler.draw = jest.fn(() => true);
    rangeSelect.timerShaftDragEL.timerShaftDragEL = jest.fn(() => 0);
    rangeSelect.spacerEL = jest.fn(() => true);
    rangeSelect.spacerEL.offsetTop = jest.fn(() => 1);
    rangeSelect.ns2x = jest.fn(() => 1);
    rangeSelect.mouseX = jest.fn(() => 10);
    rangeSelect.markA = jest.fn(() => 8);
    rangeSelect.markB = jest.fn(() => 9);
    let htmlElement = document.createElement('div');
    rangeSelect.spacerEL = htmlElement;
    let rowElement = document.createElement('div');
    rangeSelect.rowsPaneEL = rowElement;
    rangeSelect.favoriteRowsEL = rowElement;
    let traceRowElement = new TraceRow()
    expect(rangeSelect.mouseMove([traceRowElement], mouseEvent)).toBeUndefined();
  });

  it('Utils Test10', () => {
    let rangeSelect = new RangeSelect();
    rangeSelect.isInRowsEl = jest.fn(() => true);
    rangeSelect.isDrag = jest.fn(() => true);

    rangeSelect.rowsEL = {
      offsetTop: 93,
      offsetHeight: 1030,
      offsetLeft: 93,
      offsetWidth: 210,
    };
    let mouseEvent = new MouseEvent('mousedown', {
      // @ts-ignore
      offsetY: 123,
      offsetX: 1,
      button: 3,
      buttons: 0,
      clientX: 22,
      clientY: 100,
      screenX: 255,
      screenY: 322,
    });
    let htmlElement = document.createElement('div');
    rangeSelect.rowsPaneEL = htmlElement;
    rangeSelect.timerShaftDragEL = jest.fn(() => true);
    rangeSelect.timerShaftDragEL.timerShaftDragEL = jest.fn(() => 0);
    expect(rangeSelect.isTouchMark(mouseEvent)).toBeFalsy();
  });

  it('Utils Test06', () => {
    let rangeSelect = new RangeSelect();
    rangeSelect.isHover = true;
    let mouseEvent = new MouseEvent('mousedown', {
      // @ts-ignore
      offsetY: 14,
      offsetX: 1,
      button: 4,
      buttons: 0,
      clientX: 2,
      clientY: 104,
      screenX: 255,
      screenY: 325,
    });
    expect(rangeSelect.mouseDown(mouseEvent)).toBeUndefined();
  });
  it('Utils Test11', () => {
    let rangeSelect = new RangeSelect();
    rangeSelect.isInRowsEl = jest.fn(() => true);
    rangeSelect.isDrag = jest.fn(() => true);

    rangeSelect.rowsEL = {
      offsetTop: 113,
      offsetHeight: 540,
      offsetLeft: 146,
      offsetWidth: 1102,
    };
    let mouseEvent = new MouseEvent('mouseout', {
      // @ts-ignore
      offsetY: 1,
      offsetX: 3,
      button: 4,
      buttons: 0,
      clientX: 8,
      clientY: 99,
      screenX: 45,
      screenY: 78,
    });
    rangeSelect.spacerEL = jest.fn(() => true);
    rangeSelect.rowsEL = jest.fn(() => true);
    rangeSelect.rowsEL.getBoundingClientRect = jest.fn(() => true);
    rangeSelect.spacerEL.containPoint = jest.fn(() => true);
    rangeSelect.spacerEL.getBoundingClientRect = jest.fn(() => true);
    rangeSelect.rowsPaneEL = jest.fn(() => true);
    rangeSelect.rowsPaneEL.scrollTop = jest.fn(() => true);
    rangeSelect.spacerEL.offsetTop = jest.fn(() => 1);
    rangeSelect.drag = true;
    expect(rangeSelect.mouseOut(mouseEvent)).toBeUndefined();
  });
  it('Utils Test12', () => {
    let rangeSelect = new RangeSelect(new SpSystemTrace());
    rangeSelect.isInRowsEl = jest.fn(() => true);
    rangeSelect.isDrag = jest.fn(() => true);
    rangeSelect.isMouseDown = false;
    let rowsELDiv = document.createElement('div');
    rangeSelect.rowsEL = rowsELDiv;
    let mouseEvent = new MouseEvent('mousedown', {
      // @ts-ignore
      offsetY: 12,
      offsetX: 1,
      button: 74,
      buttons: 0,
      clientX: 12,
      clientY: 100,
      screenX: 9,
      screenY: 325,
    });
    rangeSelect.timerShaftEL = jest.fn(() => true);
    rangeSelect.timerShaftEL.sportRuler = jest.fn(() => true);
    rangeSelect.timerShaftEL.sportRuler.isRangeSelect = jest.fn(() => true);
    rangeSelect.timerShaftEL.sportRuler.draw = jest.fn(() => true);
    rangeSelect.timerShaftDragEL = jest.fn(() => true);
    rangeSelect.timerShaftDragEL.timerShaftDragEL = jest.fn(() => 0);
    rangeSelect.spacerEL = jest.fn(() => true);
    let htmlElement = document.createElement('div');
    rangeSelect.spacerEL = htmlElement;
    let rowElement = document.createElement('div');
    rangeSelect.rowsPaneEL = rowElement;
    rangeSelect.favoriteRowsEL = rowElement;
    let traceRowElement = new TraceRow();
    rangeSelect.ns2x = jest.fn(() => 1);
    rangeSelect.mouseX = jest.fn(() => 10);
    rangeSelect.markA = jest.fn(() => 8);
    rangeSelect.markB = jest.fn(() => 9);
    expect(rangeSelect.mouseMove([traceRowElement], mouseEvent)).toBeUndefined();
  });
});
