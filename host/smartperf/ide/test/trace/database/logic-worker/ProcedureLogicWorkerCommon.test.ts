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

//@ts-ignore
import {
  ChartStruct,
  Msg,
  MerageBean,
  merageBeanDataSplit,
  getByteWithUnit,
  getTimeString,
  timeMsFormat2p,
  getProbablyTime,
  convertJSON,
  //@ts-ignore
} from '../../../../dist/trace/database/logic-worker/ProcedureLogicWorkerCommon.js';
describe('ProcedureLogicWorkerCommon Test', () => {
  it('ChartStructTest', function () {
    let chartStruct = new ChartStruct();
    expect(chartStruct).not.toBeUndefined();
  });

  it('MsgTest', function () {
    let msg = new Msg();
    expect(msg).not.toBeUndefined();
  });

  it('MerageBeanTest01', function () {
    let merageBean = new MerageBean();
    expect(merageBean).not.toBeUndefined();
  });

  it('MerageBeanTest02', function () {
    let merageBean = new MerageBean();
    merageBean.parentNode = true;
    expect(merageBean.parentNode).toBeTruthy();
  });

  it('MerageBeanTest03', function () {
    let merageBean = new MerageBean();
    merageBean.parentNode = false;
    expect(merageBean.parentNode).toBeFalsy();
  });

  it('MerageBeanTest04', function () {
    let merageBean = new MerageBean();
    merageBean.total = true;
    expect(merageBean.total).toBeTruthy();
  });

  it('MerageBeanTest05', function () {
    let merageBean = new MerageBean();
    merageBean.total = false;
    expect(merageBean.total).toBeFalsy();
  });

  it('MerageBeanTest06', function () {
    merageBeanDataSplit.recursionChargeInitTree = jest.fn(() => true);
    merageBeanDataSplit.resetAllNode = jest.fn(() => true);
    expect(merageBeanDataSplit.splitTree('', [{ children: [] }], '', true, [''], '')).toBeUndefined();
  });
  it('MerageBeanTest30', function () {
    merageBeanDataSplit.recursionPruneInitTree = jest.fn(() => true);
    merageBeanDataSplit.resetAllNode = jest.fn(() => true);
    expect(merageBeanDataSplit.splitTree('', [{ children: [] }], '', false, [''], '')).toBeUndefined();
  });

  it('MerageBeanTest07', function () {
    merageBeanDataSplit.recursionChargeInitTree = jest.fn(() => true);
    let node = {
      symbolName: '',
      libName: '',
    };
    expect(merageBeanDataSplit.recursionChargeInitTree('', node, '', true)).toBeTruthy();
  });

  it('MerageBeanTest47', function () {
    merageBeanDataSplit.recursionChargeInitTree = jest.fn(() => undefined);
    merageBeanDataSplit.resetAllNode = jest.fn(() => true);
    let node = {
      symbolName: '',
      libName: '',
      initChildren: {
        length: 1,
      },
    };
    expect(merageBeanDataSplit.recursionChargeInitTree('', node, [], true)).toBeUndefined();
  });

  it('MerageBeanTest30', function () {
    expect(getByteWithUnit(-1_000_000_001)).toBe('-953.67 Mb');
  });

  it('MerageBeanTest08', function () {
    expect(getByteWithUnit(1_000_000_001)).toBe('953.67 Mb');
  });

  it('MerageBeanTest09', function () {
    expect(getByteWithUnit(1_000_001)).toBe('976.56 Kb');
  });

  it('MerageBeanTest10', function () {
    expect(getByteWithUnit(1_001)).toBe('1001 byte');
  });

  it('MerageBeanTest11', function () {
    expect(getByteWithUnit(1_000_000_000_1)).toBe('9.31 Gb');
  });

  it('MerageBeanTest12', function () {
    expect(getTimeString(3600_000_000_002)).toBe('1h 2ns ');
  });

  it('MerageBeanTest13', function () {
    expect(getTimeString(60_000_000_002)).toBe('1m 2ns ');
  });

  it('MerageBeanTest14', function () {
    expect(getTimeString(1_000_000_003)).toBe('1s 3ns ');
  });

  it('MerageBeanTest15', function () {
    expect(getTimeString(1_000_004)).toBe('1ms 4ns ');
  });

  it('MerageBeanTest16', function () {
    expect(getTimeString(1_003)).toBe('1μs 3ns ');
  });

  it('MerageBeanTest31', function () {
    expect(convertJSON('')).toBe('');
  });

  it('MerageBeanTest33', function () {
    expect(getProbablyTime('')).toBe('');
  });

  it('MerageBeanTest34', function () {
    expect(getProbablyTime(3600_000_000_000)).toBe('1.00h ');
  });

  it('MerageBeanTest35', function () {
    expect(getProbablyTime(60_000_000_002)).toBe('1.00m ');
  });

  it('MerageBeanTest36', function () {
    expect(getProbablyTime(1_000_000_000)).toBe('1.00s ');
  });

  it('MerageBeanTest37', function () {
    expect(getProbablyTime(1_000_000)).toBe('1.00ms ');
  });

  it('MerageBeanTest38', function () {
    expect(getProbablyTime(1_000)).toBe('1.00μs ');
  });

  it('MerageBeanTest44', function () {
    expect(getProbablyTime(100)).toBe('100ns ');
  });

  it('MerageBeanTest39', function () {
    expect(timeMsFormat2p('')).toBe('0s');
  });

  it('MerageBeanTest40', function () {
    expect(timeMsFormat2p(3600_000)).toBe('1.00h');
  });

  it('MerageBeanTest41', function () {
    expect(timeMsFormat2p(60_000)).toBe('1.00min');
  });

  it('MerageBeanTest42', function () {
    expect(timeMsFormat2p(1_000)).toBe('1.00s');
  });

  it('MerageBeanTest43', function () {
    expect(timeMsFormat2p(100)).toBe('100.00ms');
  });

  it('MerageBeanTest17', function () {
    merageBeanDataSplit.recursionChargeTree = jest.fn(() => true);
    let node = [
      {
        initChildren: {
          length: 1,
        },
      },
    ];
    expect(merageBeanDataSplit.recursionChargeTree(node, '', true)).toBeTruthy();
  });

  it('MerageBeanTest18', function () {
    merageBeanDataSplit.recursionPruneInitTree = jest.fn(() => true);
    let node = [
      {
        initChildren: {
          length: 1,
        },
      },
    ];
    expect(merageBeanDataSplit.recursionPruneInitTree('', node, '', true)).toBeTruthy();
  });

  it('MerageBeanTest19', function () {
    merageBeanDataSplit.recursionPruneTree = jest.fn(() => true);
    let node = [
      {
        initChildren: {
          length: 1,
        },
      },
    ];
    expect(merageBeanDataSplit.recursionPruneTree(node, '', true)).toBeTruthy();
  });

  it('MerageBeanTest20', function () {
    merageBeanDataSplit.recursionChargeByRule = jest.fn(() => true);
    let node = [
      {
        initChildren: {
          length: 1,
        },
      },
    ];
    expect(merageBeanDataSplit.recursionChargeByRule('', node, '', true)).toBeTruthy();
  });

  it('MerageBeanTest21', function () {
    merageBeanDataSplit.pruneChildren = jest.fn(() => true);
    let node = [
      {
        initChildren: {
          length: 1,
        },
      },
    ];
    expect(merageBeanDataSplit.pruneChildren('', node, '')).toBeTruthy();
  });

  it('MerageBeanTest22', function () {
    merageBeanDataSplit.hideSystemLibrary = jest.fn(() => true);
    expect(merageBeanDataSplit.hideSystemLibrary('', '')).toBeTruthy();
  });

  it('MerageBeanTest23', function () {
    merageBeanDataSplit.hideNumMaxAndMin = jest.fn(() => true);
    expect(merageBeanDataSplit.hideNumMaxAndMin('', '', 1, 1)).toBeTruthy();
  });

  it('MerageBeanTest24', function () {
    merageBeanDataSplit.resotreAllNode = jest.fn(() => true);
    expect(merageBeanDataSplit.resotreAllNode('', true)).toBeTruthy();
  });

  it('MerageBeanTest25', function () {
    merageBeanDataSplit.resetAllNode = jest.fn(() => true);
    expect(merageBeanDataSplit.resetAllNode('', [], '')).toBeTruthy();
  });

  it('MerageBeanTest26', function () {
    merageBeanDataSplit.resetNewAllNode = jest.fn(() => true);
    expect(merageBeanDataSplit.resetNewAllNode('', [])).toBeTruthy();
  });

  it('MerageBeanTest27', function () {
    merageBeanDataSplit.clearSearchNode = jest.fn(() => true);
    expect(merageBeanDataSplit.clearSearchNode('')).toBeTruthy();
  });

  it('MerageBeanTest28', function () {
    merageBeanDataSplit.splitAllProcess = jest.fn(() => true);
    expect(merageBeanDataSplit.splitAllProcess('', '', [])).toBeTruthy();
  });

  it('MerageBeanTest29', function () {
    merageBeanDataSplit.splitAllProcess = jest.fn(() => true);
    expect(merageBeanDataSplit.splitAllProcess('', '', [])).toBeTruthy();
  });
});
