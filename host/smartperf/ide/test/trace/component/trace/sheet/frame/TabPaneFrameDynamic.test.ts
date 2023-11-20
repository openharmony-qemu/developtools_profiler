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
import { TabPaneFrameDynamic } from '../../../../../../dist/trace/component/trace/sheet/frame/TabPaneFrameDynamic.js';

jest.mock('../../../../../../dist/trace/bean/NativeHook.js', () => {
  return {};
});

describe('TabPaneFrameDynamic Test', () => {
  let frameDynamic = new TabPaneFrameDynamic();
  let frameDynamicParam = {
    leftNs: 211,
    rightNs: 1252,
    frameDynamic: [
      {
        ts: 4153254331,
        id: 288,
        x: 0,
        y: 0,
        width: 0,
        height: 0,
        alpha: 0,
        appName: 'test',
        groupId: 4153254331,
        typeValue: 2,
      },
    ],
  };

  it('TabPaneFrameDynamicTest01', function () {
    frameDynamic.data = frameDynamicParam;
    expect(frameDynamic.data).toBeUndefined();
  });

  it('TabPaneFrameDynamicTest02', function () {
    expect(
      frameDynamic.sortByColumn({
        key: 'timestamp',
      })
    ).toBeUndefined();
  });
});
