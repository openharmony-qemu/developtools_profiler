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
import { ProcessMemStruct } from '../../../dist/trace/bean/ProcessMemStruct.js';

describe('ProcessMemStruct Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 2;
  canvas.height = 3;
  const ctx = canvas.getContext('2d');

  const data = {
    frame: {
      x: 43,
      y: 20,
      width: 240,
      height: 140,
    },
    startNS: 243,
    value: 4,
  };
  it('ProcessMemStructTest01', function () {
    expect(ProcessMemStruct.draw(ctx, data)).toBeUndefined();
  });
});
