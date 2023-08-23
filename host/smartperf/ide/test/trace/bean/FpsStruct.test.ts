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
import { FpsStruct } from '../../../dist/trace/bean/FpsStruct.js';

describe('FpsStruct Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');

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
  const node = {
    startNS: 200,
    frame: 2,
    dur: 3,
  };
  const padding = 1;
  const startNs = 1;
  const endNS = 1;
  const totalNS = 1;
  const frame = {
    x: 20,
    y: 20,
    width: 100,
    height: 100,
  };
  const dataSource = {
    frame: {
      x: 20,
      y: 20,
      width: 100,
      height: 100,
    },
    value: 50,
    maxFps: 50,
  };

  it('FpsStructTest01', function () {
    expect(FpsStruct.draw(ctx, data)).toBeUndefined();
  });

  it('FpsStructTest04 ', function () {
    expect(FpsStruct.draw(ctx, dataSource)).toBeUndefined();
  });

  it('FpsStructTest02', function () {
    let fpsStruct = new FpsStruct();
    expect(fpsStruct).not.toBeUndefined();
  });

  it('FpsStructTest03', function () {
    expect(FpsStruct.setFrame(node, padding, startNs, endNS, totalNS, frame)).toBeUndefined();
  });
});
