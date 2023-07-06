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
import { FuncStruct } from '../../../dist/trace/bean/FuncStruct.js';
jest.mock('../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

describe('FuncStruct Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');

  const dataResource = {
    frame: {
      x: 20,
      y: 20,
    },
  };

  const durData = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    dur: 5,
  };

  FuncStruct.isSelected = jest.fn(() => true);

  it('FuncStructTest01', function () {
    expect(FuncStruct.draw(ctx, dataResource)).toBeUndefined();
  });

  it('FuncStructTest02', function () {
    expect(FuncStruct.draw(ctx, durData)).toBeUndefined();
  });

  it('FuncStructTest04', function () {
    expect(
      FuncStruct.isSelected({
        startTs: 10,
        dur: 10,
        funName: '',
      })
    ).toBeTruthy();
  });

  it('FuncStructTest05', function () {
    expect(
      FuncStruct.isBinder({
        startTs: 10,
        dur: 10,
        funName: null,
      })
    ).toBeFalsy();
  });


  it('FuncStructTest08', function () {
    expect(
      FuncStruct.isBinderAsync({
        startTs: 10,
        dur: 10,
        funName: null,
      })
    ).toBeFalsy();
  });

  it('FuncStructTest09', function () {
    expect(
      FuncStruct.isBinderAsync({
        startTs: 20,
        dur: 20,
        funName: 'funName',
      })
    ).toBeFalsy();
  });
});
