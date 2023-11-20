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
//@ts-ignore
import { TabPaneFps } from '../../../../../../dist/trace/component/trace/sheet/fps/TabPaneFps.js';
const sqlit = require('../../../../../../dist/trace/database/SqlLite.js');
jest.mock('../../../../../../dist/trace/database/SqlLite.js');

jest.mock('../../../../../../dist/trace/component/trace/sheet/SheetUtils.js', () => {
  return {};
});
window.ResizeObserver =
  window.ResizeObserver ||
  jest.fn().mockImplementation(() => ({
    disconnect: jest.fn(),
    observe: jest.fn(),
    unobserve: jest.fn(),
  }));

describe('TabPaneFps Test', () => {
  let tabPaneFps = new TabPaneFps();

  it('TabPaneFpsTest01', function () {
    let mockgetTabFps = sqlit.getTabFps;
    mockgetTabFps.mockResolvedValue([{ leftNs: 10, rightNs: 10 }]);
    let a = { rightNs: 1, leftNs: 1 };
    expect((tabPaneFps.data = a)).toBeTruthy();
  });

  it('TabPaneFpsTest02', function () {
    let mockgetTabFps = sqlit.getTabFps;
    mockgetTabFps.mockResolvedValue([]);
    let a = { rightNs: 1, leftNs: 1 };
    expect((tabPaneFps.data = a)).toBeTruthy();
  });
});
