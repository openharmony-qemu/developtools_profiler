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
import { CpuStruct } from '../../../dist/trace/bean/CpuStruct.js';

describe('CpuStruct Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');
  CpuStruct.selectCpuStruct = {};

  const data = {
    frame: {
      x: 653,
      y: 109,
      width: 654,
      height: 332,
    },
    startNS: 200,
    value: 50,
  };
  const data1 = {
    frame: {
      x: 23,
      y: 9,
      width: 90,
      height: 60,
    },
    startNS: 132,
    value: 980,
  };

  it('CpuStructTest01', function () {
    expect(CpuStruct.draw(ctx, data)).toBeUndefined();
    expect(data).toMatchInlineSnapshot(`
{
  "frame": {
    "height": 332,
    "width": 654,
    "x": 653,
    "y": 109,
  },
  "startNS": 200,
  "value": 50,
}
`);
  });

  it('CpuStructTest02', function () {
    expect(CpuStruct.equals({}, data)).toBeTruthy();
  });

  it('CpuStructTest03', function () {
    expect(CpuStruct.equals(data, data)).toBeTruthy();
  });

  it('CpuStructTest04', function () {
    expect(CpuStruct.equals(data, data1)).toBeTruthy();
  });

  it('CpuStructTest05', function () {
    expect(CpuStruct.draw(ctx, data1)).toBeUndefined();
    expect(data1).toMatchInlineSnapshot(`
{
  "frame": {
    "height": 60,
    "width": 90,
    "x": 23,
    "y": 9,
  },
  "startNS": 132,
  "value": 980,
}
`);
  });

  it('CpuStructTest06', function () {
    expect(CpuStruct.equals({}, data1)).toBeTruthy();
    expect(CpuStruct.draw(ctx, data)).toBeUndefined();
    expect(data).toMatchInlineSnapshot(`
{
  "frame": {
    "height": 332,
    "width": 654,
    "x": 653,
    "y": 109,
  },
  "startNS": 200,
  "value": 50,
}
`);
  });
});
