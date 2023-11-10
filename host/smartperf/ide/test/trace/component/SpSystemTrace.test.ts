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
import { SpSystemTrace } from '../../../dist/trace/component/SpSystemTrace.js';
// @ts-ignore
import { TraceRow } from '../../../dist/trace/component/trace/base/TraceRow';
// @ts-ignore
import { procedurePool } from '../../../dist/trace/database/Procedure.js';
// @ts-ignore
import { LitTable } from '../../../dist/base-ui/table/lit-table.js';
jest.mock('../../../dist/base-ui/table/lit-table.js', () => {
  return {
    recycleDataSource: () => {},
  };
});
// @ts-ignore
import { HeapLoader } from '../../../dist/js-heap/logic/HeapLoader.js';
jest.mock('../../../dist/js-heap/logic/HeapLoader.js', () => {
  return {};
});
// @ts-ignore
import { NodeType } from '../../../dist/js-heap/model/DatabaseStruct.js';
jest.mock('../../../dist/js-heap/model/DatabaseStruct.js', () => {
  return {};
});
jest.mock('../../../dist/trace/component/trace/base/TraceSheet.js', () => {
  return {};
});

const intersectionObserverMock = () => ({
  observe: () => null,
});
window.IntersectionObserver = jest.fn().mockImplementation(intersectionObserverMock);

// @ts-ignore
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('SpSystemTrace Test', () => {
  let spSystemTrace = new SpSystemTrace();
  const offset = 1;
  const callback = true;
  const rowId = '';
  const rowParentId = '';
  const rowType = '';
  let smooth = true;

  spSystemTrace.initElements = jest.fn(() => true);

  it('SpSystemTraceTest01', function () {
    expect(spSystemTrace.getScrollWidth()).toBe(0);
  });

  it('SpSystemTraceTest02', function () {
    let resultLength = spSystemTrace.getRowsContentHeight();
    expect(resultLength).toBe(0);
  });

  it('SpSystemTraceTest03', function () {
    expect(spSystemTrace.timerShaftELRangeChange('')).toBeUndefined();
  });

  it('SpSystemTraceTest04', function () {
    expect(spSystemTrace.rowsElOnScroll('Scroll')).toBeUndefined();
  });

  it('SpSystemTraceTest05', function () {
    expect(spSystemTrace.documentOnMouseDown('MouseDown')).toBeUndefined();
  });

  it('SpSystemTraceTest06', function () {
    spSystemTrace.timerShaftEL = jest.fn(() => null);
    spSystemTrace.timerShaftEL.sportRuler = jest.fn(() => undefined);
    spSystemTrace.timerShaftEL.sportRuler.frame = jest.fn(() => '');
    spSystemTrace.timerShaftEL.canvas = jest.fn(() => undefined);
    spSystemTrace.timerShaftEL.canvas.offsetLeft = jest.fn(() => 1);
    spSystemTrace.timerShaftEL.sportRuler.frame.contains = jest.fn(() => true);
    spSystemTrace.documentOnMouseUp = jest.fn(() => true);
    expect(spSystemTrace.documentOnMouseUp('MouseUp')).toBeTruthy();
  });

  it('SpSystemTraceTest07', function () {
    spSystemTrace.timerShaftEL = jest.fn(() => undefined);
    spSystemTrace.timerShaftEL.isScaling = jest.fn(() => true);
    expect(spSystemTrace.documentOnMouseMove('MouseMove')).toBeUndefined();
  });

  it('SpSystemTraceTest08', function () {
    expect(spSystemTrace.hoverStructNull('')).toBeUndefined();
  });

  it('SpSystemTraceTest09', function () {
    expect(spSystemTrace.selectStructNull('')).toBeUndefined();
  });

  it('SpSystemTraceTest11', function () {
    expect(spSystemTrace.connectedCallback()).toBeUndefined();
  });

  it('SpSystemTraceTest12', function () {
    spSystemTrace.timerShaftEL.removeEventListener = jest.fn(() => true);
    expect(spSystemTrace.disconnectedCallback()).toBeUndefined();
  });

  it('SpSystemTraceTest14', function () {
    expect(spSystemTrace.loadDatabaseUrl).toBeTruthy();
  });

  it('SpSystemTraceTest15', function () {
    spSystemTrace.rowsPaneEL = jest.fn(() => true);
    spSystemTrace.rowsPaneEL.scrollTo = jest.fn(() => offset);
    spSystemTrace.rowsPaneEL.removeEventListener = jest.fn(() => true);
    spSystemTrace.rowsPaneEL.addEventListener = jest.fn(() => true);
    expect(spSystemTrace.rowScrollTo(offset, callback)).toBeUndefined();
  });

  it('SpSystemTraceTest16', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.onClickHandler()).toBeUndefined();
  });

  it('SpSystemTraceTest17', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.search()).toBeUndefined();
  });

  it('SpSystemTraceTest18', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.searchCPU()).not.toBeUndefined();
  });

  it('SpSystemTraceTest20', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    // @ts-ignore
    TraceRow.range = jest.fn(() => undefined);
    TraceRow.range.startNS = jest.fn(() => 1);
    spSystemTrace.onClickHandler = jest.fn(() => true);
    expect(spSystemTrace.showPreCpuStruct(1, [{ length: 0 }])).toBe(0);
  });

  it('SpSystemTraceTest21', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    // @ts-ignore
    TraceRow.range = jest.fn(() => undefined);
    TraceRow.range.startNS = jest.fn(() => 1);
    spSystemTrace.onClickHandler = jest.fn(() => true);
    expect(spSystemTrace.showNextCpuStruct(1, [{ length: 0 }])).toBe(0);
  });

  it('SpSystemTraceTest22', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    procedurePool.clearCache = jest.fn(() => true);
    spSystemTrace.traceSheetEL = jest.fn(() => true);
    spSystemTrace.traceSheetEL.clearMemory = jest.fn(() => true);
    spSystemTrace.traceSheetEL.setAttribute = jest.fn(() => true);
    expect(spSystemTrace.reset()).toBeUndefined();
  });
  it('SpSystemTraceTest23', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let structs = [
      {
        length: 1,
        starttime: 1,
      },
    ];
    let previous = 1;
    let currentIndex = 1;
    TraceRow.range = jest.fn(() => undefined);
    TraceRow.range.startNS = jest.fn(() => 1);
    expect(spSystemTrace.showStruct(previous, currentIndex, structs)).not.toBeUndefined();
  });
  it('SpSystemTraceTest24', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    TraceRow.range = jest.fn(() => undefined);
    TraceRow.range.startNS = jest.fn(() => 1);
    expect(spSystemTrace.closeAllExpandRows()).toBeUndefined();
  });
  it('SpSystemTraceTest25', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    spSystemTrace.rowsPaneEL = jest.fn(() => true);
    spSystemTrace.rowsPaneEL.scroll = jest.fn(() => true);
    expect(spSystemTrace.scrollToProcess()).toBeUndefined();
  });
  it('SpSystemTraceTest26', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    spSystemTrace.rowsPaneEL = jest.fn(() => true);
    spSystemTrace.rowsPaneEL.scroll = jest.fn(() => true);
    let anomalyTraceRow = TraceRow.skeleton();
    anomalyTraceRow.collect = true;
    spSystemTrace.appendChild(anomalyTraceRow);
    expect(spSystemTrace.scrollToDepth()).toBeUndefined();
  });
  it('SpSystemTraceTest27', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.searchThreadsAndProcesses()).toStrictEqual([]);
  });
  it('SpSystemTraceTest28', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.refreshFavoriteCanvas()).toBeUndefined();
  });
  it('SpSystemTraceTest29', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.expansionAllParentRow({ id: 1 })).toBeUndefined();
  });
  it('SpSystemTraceTest30', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let it = {
      name: '',
      rowType: '',
      rowId: 'FileSystemLogicalWrite',
      rowParentId: 'frameTime',
    };
    expect(spSystemTrace.createPointEvent(it)).toBe('');
  });
  it('SpSystemTraceTest31', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let a = {
      rowEL: {
        translateY: 1,
        offsetTop: 0,
      },
      y: 1,
      offsetY: 0,
    };
    let b = {
      rowEL: {
        translateY: 1,
        offsetTop: 0,
      },
      y: 1,
      offsetY: 0,
    };
    expect(spSystemTrace.addPointPair(a, b)).toBeUndefined();
  });
  it('SpSystemTraceTest32', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.setSLiceMark()).toBeUndefined();
  });
  it('SpSystemTraceTest33', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.clickEmptyArea()).toBeUndefined();
  });
  it('SpSystemTraceTest34', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.isWASDKeyPress()).toBeFalsy();
  });
  it('SpSystemTraceTest35', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let endParentRow = {
      expansion: true,
      childrenList: [],
    };
    let selectJankStruct = {
      frame_type: 'frameTime',
      type: '',
      pid: 1,
      ts: 1,
      dur: 0,
      depth: 1,
    };
    let data = {
      frame_type: 'frameTime',
      type: '',
      pid: 1,
      name: '',
      children: {
        frame_type: 'frameTime',
        pid: 1,
        length: 1,
      },
    };

    expect(spSystemTrace.drawJankLine(endParentRow, selectJankStruct, data)).toBeUndefined();
  });
  it('SpSystemTraceTest36', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let ev = {
      maxDuration: 1,
      timestamp: '',
    };
    expect(spSystemTrace.sliceMarkEventHandler(ev)).toBeUndefined();
  });
  it('SpSystemTraceTest37', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    expect(spSystemTrace.searchSdk([''], '')).toStrictEqual(['']);
  });
  it('SpSystemTraceTest38', function () {
    let spSystemTrace = new SpSystemTrace<any>({
      canvasNumber: 1,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    let funcStract = {
      tid: 1,
      pid: 0,
      cookie: '',
      funName: '',
      type: '',
      startTime: 2,
      depth: 1,
    };
    expect(spSystemTrace.scrollToActFunc(funcStract, true)).toBeUndefined();
  });
});
