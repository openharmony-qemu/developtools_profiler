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

jest.mock('../../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

// @ts-ignore
import { TabFrameSpacing } from '../../../../../../dist/trace/component/trace/sheet/frame/TabFrameSpacing.js';
// @ts-ignore
import { Rect } from '../../../../../../dist/trace/component/trace/timer-shaft/Rect.js';

describe('TabPaneFrameSpacing Test', () => {
  let tabFrameSpacing = new TabFrameSpacing();
  let frameSpacing = {
    currentFrameHeight: 0,
    currentFrameWidth: 0,
    currentTs: 4153254331,
    frame: new Rect(),
    frameSpacingResult: 0,
    groupId: 4091445476,
    id: 288,
    nameId: 'test',
    preFrameHeight: 0,
    preFrameWidth: 0,
    preTs: 0,
    x: 0,
    y: 0,
  };
  let frameData = {
    leftNs: 253,
    rightNs: 1252,
    frameSpacing: [frameSpacing],
  };

  it('TabPaneFrameSpacingTest01', function () {
    tabFrameSpacing.data = frameData;
    expect(tabFrameSpacing.data).toBeUndefined();
  });

  it('TabPaneFrameSpacingTest02', function () {
    tabFrameSpacing.setFrameSpacingData(frameSpacing);
    expect(tabFrameSpacing.range.textContent).toEqual('');
  });

  it('TabPaneFrameSpacingTest03', function () {
    expect(
      tabFrameSpacing.sortByColumn({
        key: 'property',
      })
    ).toBeUndefined();
  });
});
