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
import { TraceRowRecyclerView } from '../../../../../dist/trace/component/trace/base/TraceRowRecyclerView.js';

jest.mock('../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});


describe('TraceRow Test', () => {
  beforeAll(() => {});

  it('Utils Test01', () => {
    let traceRow = new TraceRowRecyclerView();
    expect(traceRow);
  });

  it('Test02', function () {
    let traceRow = new TraceRowRecyclerView();
    expect(traceRow.dataSource).toBeTruthy();
  });

  it('Test03', function () {
    let traceRow = new TraceRowRecyclerView();
    traceRow.measureHeight = jest.fn(() => true);
    traceRow.dataSource = true;
    expect(traceRow.dataSource).toBeTruthy();
  });

  it('Test04', function () {
    let traceRow = new TraceRowRecyclerView();
    expect(traceRow.renderType).toBeTruthy();
  });

  it('Test05', function () {
    let traceRow = new TraceRowRecyclerView();
    traceRow.renderType = false;
    expect(traceRow.renderType).toBeFalsy();
  });

  it('Test06', function () {
    let traceRow = new TraceRowRecyclerView();
    const obj = {
      folder: false,
      top: 0,
      name: '',
      children: false,
      rowId: '',
      rowType: '',
      rowParentId: '1',
      expansion: false,
      rowHidden: false,
      rowHeight: 40,
    };
    const el = {
      obj: undefined,
      style: { top: 1, visibility: 'visible' },
      name: '',
      rowId: '',
      rowType: '',
      rowParentId: '1',
      expansion: false,
      rowHidden: false,
      setAttribute: '',
      removeAttribute: '',
    };
    expect(traceRow.refreshRow(el, !obj)).toBeUndefined();
  });

  it('Test08', function () {
    let traceRow = new TraceRowRecyclerView();
    expect(traceRow.initUI()).toBeUndefined();
  });

  it('Test09', function () {
    let traceRow = new TraceRowRecyclerView();
    expect(traceRow.initUI()).toBeUndefined();
  });
  it('Test10', function () {
    let traceRow = new TraceRowRecyclerView();
    let mouseScrollEvent: MouseEvent = new MouseEvent('scroll', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.container.dispatchEvent(mouseScrollEvent);
  });

});
