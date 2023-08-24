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
import { isPointIsCircle, randomRgbColor } from '../../../../dist/base-ui/chart/pie/LitChartPieData.js';

window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('LitChartPieData Test', () => {
  it('LitChartPieDataTest01', function () {
    expect(randomRgbColor()).not.toBeUndefined();
  });

  it('LitChartPieDataTest02', function () {
    expect(isPointIsCircle(1, 2, 1, 2, 3)).toBe(true);
  });
  it('LitChartPieDataTest03', function () {
    expect(isPointIsCircle(1, 2, 7, 5, 3)).toBe(false);
  });
});
