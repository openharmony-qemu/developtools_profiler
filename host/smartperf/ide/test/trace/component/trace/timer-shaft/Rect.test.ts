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
import { Rect, Point } from '../../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe('Rect Test', () => {
  let rect = new Rect(20, 20, 100, 100);
  let point = new Point();

  it('RectTest01', function () {
    expect(rect.contains(4, 5)).toBeFalsy();
  });

  it('RectTest02', function () {
    expect(Rect.contains(rect, 4, 4)).toBeFalsy();
  });

  it('RectTest03', function () {
    expect(rect.containsWithPadding(4, 5, 2, 2)).toBeFalsy();
  });

  it('RectTest04', function () {
    expect(Rect.containsWithPadding(rect, 4, 4, 2, 2)).toBeFalsy();
  });

  it('RectTest05', function () {
    expect(rect.containsWithMargin(1, 2, 1, 2, 4, 5)).toBeFalsy();
  });

  it('RectTest06', function () {
    expect(Rect.containsWithMargin(1, 2, 1, 2, 4, 5)).toBeFalsy();
  });

  it('RectTest07', function () {
    expect(rect.intersect([])).toBeFalsy();
  });
});
