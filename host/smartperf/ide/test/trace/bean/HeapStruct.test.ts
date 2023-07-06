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

jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

// @ts-ignore
import { HeapStruct } from '../../../dist/trace/bean/HeapStruct.js';

describe('HeapStruct Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');

  const dataSource = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    value: 50,
    maxHeapSize: 50,
    heapsize: 10,
  };

  const reachData = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    value: 50,
    startTime: 1,
  };

  const nodeSource = {
    startTime: 10,
    dur: 10,
    endTime: 20,
    frame: {
      width: 20,
    },
  };

  const heapStruct = new HeapStruct();

  it('HeapStructTest01', function () {
    expect(HeapStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('HeapStructTest02', function () {
    expect(HeapStruct.draw(ctx, reachData)).toBeUndefined();
  });
});
