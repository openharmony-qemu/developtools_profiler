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
import { SpAllocations } from '../../../../dist/trace/component/setting/SpAllocations.js';

describe('SpAllocations Test', () => {
  beforeAll(() => {
    document.body.innerHTML = `
            <sp-allocations id = "sp"><sp-allocations>
        `;
  });

  it(' SpAllocations get Default attrValue', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    spEle.unwindEL = jest.fn(() => true);
    spEle.unwindEL.value = jest.fn(() => true);
    spEle.shareMemory = jest.fn(() => true);
    spEle.shareMemory.value = jest.fn(() => true);
    spEle.shareMemoryUnit = jest.fn(() => true);
    spEle.shareMemoryUnit.value = jest.fn(() => true);
    spEle.filterMemory = jest.fn(() => true);
    spEle.filterMemory.value = jest.fn(() => true);
    spEle.filterMemoryUnit = jest.fn(() => true);
    spEle.filterMemoryUnit.value = jest.fn(() => true);
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toBeNaN();
    expect(spEle.shared).toBe(16384);
    expect(spEle.filter).toBeNaN();
  });

  it(' SpAllocations set  attrValue', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    spEle.processId.value = '2';
    spEle.unwindEL.value = '111';
    spEle.shareMemory.value = '222';
    spEle.shareMemoryUnit.value = 'MB';
    spEle.filterMemory.value = '111';
    spEle.filterMemoryUnit.value = 'MB';
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toEqual(111);
    expect(spEle.shared).toEqual(222);
    expect(spEle.filter).toEqual(111);
  });

  it(' SpAllocations set  attrValue2', function () {
    let spEle = document.querySelector('#sp') as SpAllocations;
    spEle.processId.value = '3';
    spEle.unwindEL.value = '1121';
    spEle.shareMemory!.value = '222';
    spEle.shareMemoryUnit.value = 'KB';
    spEle.filterMemory.value = '111';
    spEle.filterMemoryUnit.value = 'KB';
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toEqual(1121);
    expect(spEle.shared).toEqual(222);
    expect(spEle.filter).toEqual(111);
  });

  it(' SpAllocations set  attrValue03', function () {
    let spEle = new SpAllocations();
    spEle.processId.value = '3';
    spEle.unwindEL.value = '1121';
    spEle.shareMemory.value = '222';
    spEle.filterMemory.value = '111';
    expect(spEle.pid).toEqual(undefined);
    expect(spEle.unwind).toEqual(1121);
    expect(spEle.shared).toEqual(222);
    expect(spEle.filter).toEqual(111);
  });

  it('SpAllocations test05', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.appProcess).toBe('3');
  });

  it('SpAllocations test06', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.convertToValue('0', 'MB')).toBe(0);
  });

  it('SpAllocations test07', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.convertToValue('1', 'KB')).toBe(16384);
  });

  it('SpAllocations test08', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.convertToValue('1', '')).toBe(0);
  });
  it('SpAllocations test09', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.fp_unwind).toBeTruthy();
  });
  it('SpAllocations test10', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.record_accurately).toBeTruthy();
  });
  it('SpAllocations test11', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.offline_symbolization).toBeTruthy();
  });
  it('SpAllocations test12', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.record_statistics).toBeTruthy();
  });
  it('SpAllocations test13', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.statistics_interval).toBeTruthy();
  });
  it('SpAllocations test14', function () {
    let spAllocations = document.querySelector('#sp') as SpAllocations;
    expect(spAllocations.startup_mode).toBeFalsy();
  });
});
