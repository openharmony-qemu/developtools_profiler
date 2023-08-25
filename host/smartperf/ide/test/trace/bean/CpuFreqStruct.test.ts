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
import { CpuFreqStruct } from '../../../dist/trace/bean/CpuFreqStruct.js';

describe('CpuFreqStruct Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');

  CpuFreqStruct.hoverCpuFreqStruct = void 0;
  const data = {
    frame: {
      x: 244,
      y: 466,
      width: 34,
      height: 600,
    },
    startNS: 400,
    value: 43,
  };

  const dataSource = {
    frame: {
      x: 35,
      y: 66,
      width: 560,
      height: 600,
    },
    value: 60,
    maxFreq: 88,
  };

  it('CpuFreqStructTest01', function () {
    expect(CpuFreqStruct.draw(ctx, data)).toBeUndefined();
    expect(data).toMatchInlineSnapshot(`
{
  "frame": {
    "height": 600,
    "width": 34,
    "x": 244,
    "y": 466,
  },
  "startNS": 400,
  "value": 43,
}
`);
  });

  it('CpuFreqStructTest02', function () {
    expect(CpuFreqStruct.draw(ctx, { startNS: 1 })).toBeUndefined();
  });

  it('CpuFreqStructTest03 ', function () {
    expect(CpuFreqStruct.draw(ctx, dataSource)).toBeUndefined();
  });
});
