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
import { ColorUtils } from '../../../../../dist/trace/component/trace/base/ColorUtils.js';

describe('testColorUtils Test', () => {
  beforeAll(() => {});
  it('testColorUtils01', () => {
    expect(ColorUtils.hash('mm', ColorUtils.MD_PALETTE.length)).toBe(0);
  });
  it('testColorUtils02', () => {
    // @ts-ignore
    expect(ColorUtils.colorForThread(null)).toEqual('#f0f0f0');
  });

  it('testColorUtils03', () => {
    // @ts-ignore
    let thread = { processId: 1 };
    expect(ColorUtils.colorForThread(thread)).toEqual('#9785D3');
  });

  it('testColorUtils03', () => {
    // @ts-ignore
    let thread = {
      processId: 0,
      tid: 1,
    };
    expect(ColorUtils.colorForThread(thread)).toEqual('#9785D3');
  });

  it('testColorUtils04', () => {
    expect(ColorUtils.formatNumberComma(2)).toEqual('2');
  });

  afterAll(() => {});
});
