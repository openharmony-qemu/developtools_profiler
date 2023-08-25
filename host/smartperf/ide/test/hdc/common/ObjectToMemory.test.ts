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
import { objectToMemorySize } from '../../../dist/hdc/common/ObjectToMemorySize.js';
describe('ObjectToMemoryTest', () => {
  let obj = new objectToMemorySize();
  it('ObjectToMemoryTest_objectToSize_01', () => {
    expect(obj.objectToSize(12)).toEqual(8);
  });

  it('ObjectToMemoryTest_objectToSize_02', () => {
    expect(obj.objectToSize(true)).toEqual(4);
  });

  it('ObjectToMemoryTest_objectToSize_03', () => {
    expect(obj.objectToSize('abc')).toEqual(6);
  });

  it('ObjectToMemoryTest_objectToSize_04', () => {
    expect(obj.objectToSize([1, 2])).toEqual(16);
  });

  it('ObjectToMemoryTest_objectToSize_05', () => {
    expect(obj.objectToSize({ name: 'demo', age: 12 })).toEqual(30);
  });

  it('ObjectToMemoryTest_sizeOfObj_01', () => {
    expect(obj.sizeOfObj(null)).toEqual(0);
  });

  it('ObjectToMemoryTest_sizeOfObj_02', () => {
    expect(obj.sizeOfObj(12)).toEqual(0);
  });

  it('ObjectToMemoryTest_sizeOfObj_03', () => {
    expect(obj.sizeOfObj(false)).toEqual(0);
  });

  it('ObjectToMemoryTest_sizeOfObj_04', () => {
    expect(obj.sizeOfObj(false)).toEqual(0);
  });

  it('ObjectToMemoryTest_sizeOfObj_05', () => {
    expect(obj.sizeOfObj([1, 2])).toEqual(20);
  });

  it('ObjectToMemoryTest_sizeOfObj_06', () => {
    expect(obj.sizeOfObj({ name: 'demo', age: 12 })).toEqual(30);
  });

  it('ObjectToMemoryTest_objectToSize_07', () => {
    expect(obj.objectToSize(undefined)).toEqual(0);
  });

  it('ObjectToMemoryTest_sizeOfObj_08', () => {
    let object = {
      [1]: 2,
      [3]: 4,
      [5]: 6,
      [7]: 8,
    };
    expect(obj.sizeOfObj(object)).toEqual(40);
  });
});
