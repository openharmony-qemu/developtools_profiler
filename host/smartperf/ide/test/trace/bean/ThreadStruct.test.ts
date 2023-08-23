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
import { ThreadStruct } from '../../../dist/trace/bean/ThreadStruct.js';
jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('ThreadStruct Test', () => {
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
    startNS: 200,
    state: '',
  };
  const equalsData = {
    value: 50,
    cpu: 1,
    tid: 1,
    state: 1,
    startTime: 1,
    dur: 1,
  };
  it('ThreadStructTest01', function () {
    expect(ThreadStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('ThreadStructTest02', function () {
    dataSource.state = 'S';
    expect(ThreadStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('ThreadStructTest03', function () {
    dataSource.state = 'R';
    expect(ThreadStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('ThreadStructTest04', function () {
    dataSource.state = 'D';
    expect(ThreadStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('ThreadStructTest05', function () {
    dataSource.state = 'Running';
    expect(ThreadStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('ThreadStructTest11', function () {
    dataSource.state = 'T' || 't';
    expect(ThreadStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('ThreadStructTest08', function () {
    expect(ThreadStruct.equals(equalsData, equalsData)).toBeTruthy();
  });

  it('ThreadStructTest09', function () {
    expect(ThreadStruct.equals([], dataSource)).toBeFalsy();
  });

  it('ThreadStructTest10', function () {
      dataSource.state = 'ThreadStructTest10';
      expect(ThreadStruct.getEndState(1)).toBe('Unknown State');
  });
});
