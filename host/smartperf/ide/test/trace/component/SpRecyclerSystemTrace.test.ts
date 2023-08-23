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
window.ResizeObserver = window.ResizeObserver ||
    jest.fn().mockImplementation(() => ({
        disconnect: jest.fn(),
        observe: jest.fn(),
        unobserve: jest.fn(),
    }));

jest.mock('../../../dist/trace/component/trace/TimerShaftElement.js', () => {
  return {};
});

// @ts-ignore
import { TraceRow } from '../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { SpRecyclerSystemTrace } from '../../../dist/trace/component/SpRecyclerSystemTrace.js';

describe('SpRecyclerSystemTrace Test', () => {
  let spRecyclerSystemTrace = new SpRecyclerSystemTrace();
  const newEl = 1;
  const targetEl = {
    parentNode: 1,
    nextSibling: 1,
  };

  spRecyclerSystemTrace.initElements = jest.fn(() => true);

  it('SpRecyclerSystemTraceTest01', function () {
    expect(spRecyclerSystemTrace.getScrollWidth()).toBe(1);
  });

  it('SpRecyclerSystemTraceTest02', function () {
    let resultLength = spRecyclerSystemTrace.getVisibleRows([{}]).length;
    expect(resultLength).toBe(0);
  });

  it('SpRecyclerSystemTraceTest03', function () {
    expect(spRecyclerSystemTrace.timerShaftELRangeChange('')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest04', function () {
    expect(spRecyclerSystemTrace.rowsElOnScroll('Scroll')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest05', function () {
    let htmlElement = document.createElement('div');
    spRecyclerSystemTrace.rangeSelect.rowsPaneEL = htmlElement;
    spRecyclerSystemTrace.rangeSelect.MouseDown = jest.fn(() => {});
    expect(spRecyclerSystemTrace.documentOnMouseDown('MouseDown')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest06', function () {
    expect(spRecyclerSystemTrace.documentOnMouseUp('MouseUp')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest07', function () {
    spRecyclerSystemTrace.rangeSelec = jest.fn(() => true);
    spRecyclerSystemTrace.rangeSelect.mouseMove = jest.fn(() => true);
    expect(spRecyclerSystemTrace.documentOnMouseMove('MouseMove')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest08', function () {
    expect(spRecyclerSystemTrace.hoverStructNull('')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest09', function () {
    expect(spRecyclerSystemTrace.selectStructNull('')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest10', function () {
    spRecyclerSystemTrace.rangeSelec = jest.fn(() => true);
    spRecyclerSystemTrace.rangeSelect.mouseMove = jest.fn(() => true);
    expect(spRecyclerSystemTrace.documentOnClick('OnClick')).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest11', function () {
    expect(spRecyclerSystemTrace.connectedCallback()).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest12', function () {
    expect(spRecyclerSystemTrace.disconnectedCallback()).toBeUndefined();
  });

  it('SpRecyclerSystemTraceTest13', function () {
    expect(spRecyclerSystemTrace.init).toBeTruthy();
  });

  it('SpRecyclerSystemTraceTest15', function () {
    spRecyclerSystemTrace.loadDatabaseUrl = jest.fn(() => true);
    expect(spRecyclerSystemTrace.loadDatabaseUrl()).toBeTruthy();
  });

  it('SpRecyclerSystemTraceTest16', function () {
    spRecyclerSystemTrace.loadDatabaseArrayBuffer = jest.fn(() => true);
    expect(spRecyclerSystemTrace.loadDatabaseArrayBuffer()).toBeTruthy();
  });

  it('SpRecyclerSystemTraceTest17', function () {
    expect(spRecyclerSystemTrace.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            display: block;
            width: 100%;
            height: 100%;
        }
        .recycle-timer-shaft{
            width: 100%;
            z-index: 2;
        }
        .rows{
            display: flex;
            box-sizing: border-box;
            flex-direction: column;
            overflow-y: auto;
            max-height: calc(100vh - 150px - 48px);
            flex: 1;
            width: 100%;
        }
        .container{
            width: 100%;
            box-sizing: border-box;
            height: 100%;
            display: grid;
            grid-template-columns: 1fr;
            grid-template-rows: min-content 1fr min-content;
        }

        </style>
        <div class="container">
            <timer-shaft-element class="recycle-timer-shaft">
            </timer-shaft-element>
            <trace-row-recycler-view class="rows">
            </trace-row-recycler-view>
            <trace-sheet class="recycle-trace-sheet" mode="hidden">
            </trace-sheet>
        </div>
        "
`);
  });
  it('SpRecyclerSystemTraceTest18', function () {
    const newEl = 1;
    const targetEl = {
      parentNode: {
        insertBefore: jest.fn(() => true),
      },
      nextSibling: 1,
    };

    expect(spRecyclerSystemTrace.insertAfter(newEl, targetEl)).toBeUndefined();
  });
});
