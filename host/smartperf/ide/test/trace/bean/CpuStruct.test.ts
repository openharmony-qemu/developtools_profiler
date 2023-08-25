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
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    startNS: 200,
    value: 50,
  };
  const data1 = {
    frame: {
      x: 100,
      y: 100,
      width: 10,
      height: 10,
    },
    startNS: 1000,
    value: 500,
  };

  it('CpuStructTest01', function () {
    expect(CpuStruct.draw(ctx, data)).toBeUndefined();
    expect(data).toMatchInlineSnapshot(
{
  startNS: expect.any(Number),
  value: expect.any(Number) }, `
{
  "frame": {
    "height": 100,
    "width": 100,
    "x": 20,
    "y": 20,
  },
  "startNS": Any<Number>,
  "value": Any<Number>,
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
    expect(data1).toMatchInlineSnapshot(
{
  startNS: expect.any(Number),
  value: expect.any(Number) }, `
{
  "frame": {
    "height": 10,
    "width": 10,
    "x": 100,
    "y": 100,
  },
  "startNS": Any<Number>,
  "value": Any<Number>,
}
`);
  });

  it('CpuStructTest06', function () {
    expect(CpuStruct.equals({}, data)).toBeTruthy();
    expect(CpuStruct.draw(ctx, data1)).toBeUndefined();
    expect(data1).toMatchInlineSnapshot(
{
  startNS: expect.any(Number),
  value: expect.any(Number) }, `
{
  "frame": {
    "height": 10,
    "width": 10,
    "x": 100,
    "y": 100,
  },
  "startNS": Any<Number>,
  "value": Any<Number>,
}
`);
  });
});
