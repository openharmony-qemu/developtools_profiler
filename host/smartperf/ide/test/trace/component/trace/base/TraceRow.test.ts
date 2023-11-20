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
import { TraceRow } from '../../../../../dist/trace/component/trace/base/TraceRow.js';
// @ts-ignore
import { Sptext } from '../../../../../dist/trace/component/Sptext.js';
// @ts-ignore
import { ThreadStruct } from '../../../../../dist/trace/database/ui-worker/ProcedureWorkerThread.js';
jest.mock('../../../../../dist/trace/database/ui-worker/ProcedureWorker.js', () => {
  return {};
});


describe('TraceRow Test', () => {
  beforeAll(() => {});
  const ctx = {
    lineWidth: 1,
    strokeStyle: true,
  };
  let traceRow = new TraceRow<any>({
    canvasNumber: 1,
    alpha: true,
    contextId: '2d',
    isOffScreen: true,
  });
  it('TraceRow Test01', () => {
    expect(traceRow).not.toBeUndefined();
  });

  it('TraceRow Test02', () => {
    expect(traceRow.sleeping).toBeFalsy();
  });

  it('TraceRow Test03', () => {
    traceRow.sleeping = true;
    expect(traceRow.sleeping).toBeTruthy();
  });

  it('TraceRow Test04', () => {
    traceRow.sleeping = false;
    expect(traceRow.sleeping).toBeFalsy();
  });

  it('TraceRow Test05', () => {
    expect(traceRow.rangeSelect).toBeFalsy();
  });

  it('TraceRow Test06', () => {
    traceRow.rangeSelect = true;
    expect(traceRow.rangeSelect).toBeTruthy();
  });

  it('TraceRow Test10', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 10;
    canvas.height = 10;
    const ctx = canvas.getContext('2d');
    let traceRow = new TraceRow<any>({
      canvasNumber: 10,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.dataList = {
      supplier: true,
      isLoading: false,
    };
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '111';
    traceRow.height = 201;
    traceRow.height = 301;
    expect(traceRow.clearCanvas(ctx)).toBeUndefined();
  });

  it('TraceRow Test11', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 11;
    canvas.height = 11;
    const ctx = canvas.getContext('2d');
    let traceRow = new TraceRow<any>({
      canvasNumber: 11,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '561';
    traceRow.height = 33;
    traceRow.height = 35;
    traceRow.dataList = {
      supplier: true,
      isLoading: false,
    };
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '111';
    traceRow.height = 202;
    traceRow.height = 302;
    expect(traceRow.drawLines(ctx)).toBeUndefined();
  });

  it('TraceRow Test12', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 12;
    canvas.height = 12;
    const ctx = canvas.getContext('2d');
    let traceRow = new TraceRow<any>({
      canvasNumber: 12,
      alpha: true,
      contextId: '2d',
      isOffScreen: true,
    });
    traceRow.dataList = {
      supplier: true,
      isLoading: false,
    };
    traceRow.supplier = true;
    traceRow.isLoading = false;
    traceRow.name = '1201';
    traceRow.height = 554;
    expect(traceRow.drawSelection(ctx)).toBeUndefined();
  });

  it('TraceRow Test13', () => {
    expect(traceRow.collect).toBeFalsy();
  });

  it('TraceRow Test14', () => {
    traceRow.collect = true;
    expect(traceRow.collect).toBeTruthy();
  });

  it('TraceRow Test15', () => {
    expect(traceRow.rowType).toBeFalsy();
  });

  it('TraceRow Test16', () => {
    traceRow.rowType = true;
    expect(traceRow.rowType).toBeTruthy();
  });

  it('TraceRow Test17', () => {
    expect(traceRow.rowId).toBeFalsy();
  });

  it('TraceRow Test18', () => {
    traceRow.rowId = true;
    expect(traceRow.rowId).toBeTruthy();
  });

  it('TraceRow Test19', () => {
    expect(traceRow.rowParentId).toBeFalsy();
  });

  it('TraceRow Test20', () => {
    traceRow.rowParentId = true;
    expect(traceRow.rowParentId).toBeTruthy();
  });

  it('TraceRow Test21', () => {
    traceRow.rowHidden = true;
    expect(traceRow.rowHidden).toBeUndefined();
  });

  it('TraceRow Test22', () => {
    expect(traceRow.name).toBeFalsy();
  });

  it('TraceRow Test23', () => {
    traceRow.folder = false;
    expect(traceRow.folder).toBeFalsy();
  });

  it('TraceRow Test24', () => {
    traceRow.folder = true;
    expect(traceRow.folder).toBeTruthy();
  });

  it('TraceRow Test25', () => {
  });

  it('TraceRow Test26', () => {
  });

  it('TraceRow Test27', () => {
    traceRow.tip = true;
    traceRow.tipEL = true;
    expect(traceRow.tip).toBeUndefined();
  });

  it('TraceRow Test28', () => {
    expect(traceRow.frame).not.toBeUndefined();
  });

  it('TraceRow Test29', () => {
    traceRow.frame = [0, 0, 0];
    expect(traceRow.frame).toBeTruthy();
  });

  it('TraceRow Test62', () => {
    expect(traceRow.folderPaddingLeft).toBeUndefined();
  });

  it('TraceRow Test30', () => {
    expect(traceRow.checkType).not.toBeUndefined();
  });

  it('TraceRow Test31', () => {
    traceRow.checkType = '-1';
    expect(traceRow.checkType).toBeTruthy();
  });

  it('TraceRow Test32', () => {
    expect(traceRow.drawType).toBe(0);
  });

  it('TraceRow Test33', () => {
    traceRow.drawType = true;
    expect(traceRow.drawType).toBeTruthy();
  });

  it('TraceRow Test34', () => {
    traceRow.args = jest.fn(() => true);
    traceRow.args.isOffScreen = jest.fn(() => null);
    expect(traceRow.updateWidth(1)).toBeUndefined();
  });

  it('TraceRow Test36', () => {
    traceRow.tipEL = jest.fn(()=>true);
    traceRow.tipEL.style = jest.fn(()=>true);
    expect(traceRow.onMouseHover()).toBeFalsy();
  });

  it('TraceRow Test37', () => {
    expect(traceRow.setTipLeft(1, null)).toBeFalsy();
  });

  it('TraceRow Test38', () => {
    expect(traceRow.onMouseLeave(1, 1)).toBeFalsy();
  });

  it('TraceRow Test39', () => {
    expect(traceRow.draw(false)).toBeFalsy();
  });

  it('TraceRow Test40', () => {
    traceRow.collect = 1;
    expect(traceRow.collect).toBeTruthy();
  });

  it('TraceRow Test41', () => {
    traceRow.collect = 0;
    expect(traceRow.collect).toBeFalsy();
  });

  it('TraceRow Test42', () => {
    traceRow.checkType = '0';
    expect(traceRow.checkType).toBe('0');
  });

  it('TraceRow Test43', () => {
    traceRow.checkType = '1';
    expect(traceRow.checkType).toBe('1');
  });

  it('TraceRow Test44', () => {
    traceRow.checkType = '2';
    expect(traceRow.checkType).toBe('2');
  });

  it('TraceRow Test45', () => {
    traceRow.checkType = 0;
    expect(traceRow.checkType).toBe('');
  });

  it('TraceRow Test46', () => {
    traceRow.rowHidden = false;
    expect(traceRow.rowHidden).toBeUndefined();
  });

  it('TraceRow Test47', () => {
    traceRow.highlight = false;
    expect(traceRow.highlight).toBeFalsy();
  });

  it('TraceRow Test48', () => {
    traceRow.highlight = true;
    expect(traceRow.highlight).toBeFalsy();
  });

  it('TraceRow Test49', () => {
    traceRow.setCheckBox = true;
    expect(traceRow.highlight).toBeFalsy();
  });

  it('TraceRow Test50', () => {
    traceRow.initCanvas = jest.fn(() => null);
    expect(traceRow.connectedCallback()).toBeUndefined();
  });

  it('TraceRow Test51', () => {
    expect(traceRow.isInTimeRange()).toBe(false);
  });

  it('TraceRow Test52', () => {
    expect(traceRow.getLineColor()).toBe('');
  });

  it('TraceRow Test53', () => {
    let value = traceRow.attributeChangedCallback('name');
    expect(value).toBe(undefined);
  });

  it('TraceRow Test54', () => {
    let value = traceRow.attributeChangedCallback('height', '1', '2');
    expect(value).toBe(undefined);
  });

  it('TraceRow Test55', () => {
    let value = traceRow.attributeChangedCallback('check-type', '1', 'check');
    expect(value).toBe(undefined);
  });

  it('TraceRow Test57', () => {
    expect(traceRow.rowDiscard).toBeFalsy();
  });
  it('TraceRow Test58', () => {
    traceRow.rowDiscard = true;
    expect(traceRow.rowDiscard).toBeTruthy();
  });
  it('TraceRow Test58', () => {
    traceRow.rowDiscard = false;
    expect(traceRow.rowDiscard).toBeFalsy();
  });
  it('TraceRow Test59', () => {
    traceRow.disabledCheck = false;
    expect(traceRow.disabledCheck).toBeFalsy();
  });
  it('TraceRow Test64', () => {
    traceRow.folderPaddingLeft = 1;
    expect(traceRow.folderPaddingLeft).toBeUndefined();
  });
  it('TraceRow Test65', () => {
    expect(traceRow.getTransferArray()).toStrictEqual([undefined]);
  });
  it('TraceRow Test67', () => {
    expect(traceRow.clearMemory()).toBeUndefined();
  });
  it('TraceRow Test68', () => {
    expect(traceRow.rowSetting).toBeTruthy();
  });
  it('TraceRow Test69', () => {
    expect(traceRow.rowSettingPopoverDirection).toBeTruthy();
  });
  it('TraceRow Test71', () => {
    traceRow.rowSettingPopoverDirection = true;
    expect(traceRow.rowSettingPopoverDirection).toBeTruthy();
  });
  it('TraceRow Test70', () => {
    expect(traceRow.rowSettingList).toBeUndefined();
  });
  it('TraceRow Test72', () => {
    traceRow.expansion = false;
    expect(traceRow.expansion).toBeFalsy();
  });
  it('TraceRow Test74', () => {
    let threadRow = TraceRow.skeleton<ThreadStruct>();
    expect(traceRow.addChildTraceRowSpecifyLocation(threadRow,0)).toBeUndefined();
  });
  it('TraceRow Test75', () => {
    expect(traceRow.drawLine(false,'top')).toBeUndefined();
  });
  it('TraceRow Test76', () => {
    let mouseChangeEvent: MouseEvent = new MouseEvent('change', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.setCheckBox = jest.fn(()=>true);
    traceRow.checkBoxEL.dispatchEvent(mouseChangeEvent);
  });
  it('TraceRow Test77', () => {
    let mouseClickEvent: MouseEvent = new MouseEvent('click', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.isComplete = true;
    traceRow.collectEL.dispatchEvent(mouseClickEvent);
  });
  it('TraceRow Test78', () => {
    let mouseChangeEvent: MouseEvent = new MouseEvent('change', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.rowSettingTree.dispatchEvent(mouseChangeEvent);
  });
  it('TraceRow Test80', () => {
    let mouseDragOverEvent: MouseEvent = new MouseEvent('dragover', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.describeEl.dispatchEvent(mouseDragOverEvent);
  });
  it('TraceRow Test81', () => {
    let mouseDragendEvent: MouseEvent = new MouseEvent('dragend', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.describeEl.dispatchEvent(mouseDragendEvent);
  });
  it('TraceRow Test82', () => {
    let mouseDragLeaveEvent: MouseEvent = new MouseEvent('dragleave', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.describeEl.dispatchEvent(mouseDragLeaveEvent);
  });
  it('TraceRow Test83', () => {
    let mouseDragStartEvent: MouseEvent = new MouseEvent('dragstart', <MouseEventInit>{ clientX: 1, clientY: 2 });
    traceRow.describeEl.dispatchEvent(mouseDragStartEvent);
  });
  it('TraceRow Test84', () => {
    traceRow.online = true;
    expect(traceRow.draw(false)).toBeFalsy();
  });
  it('TraceRow Test85', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 34;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(traceRow.canvasSave(ctx)).toBeUndefined();
  });
  it('TraceRow Test86', () => {
    const canvas = document.createElement('canvas');
    canvas.width = 8;
    canvas.height = 1;
    const ctx = canvas.getContext('2d');
    expect(traceRow.canvasRestore(ctx)).toBeUndefined();
  });
});
