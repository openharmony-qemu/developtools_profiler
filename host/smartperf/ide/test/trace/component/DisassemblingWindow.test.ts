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
import { DisassemblingWindow, Disassembling } from '../../../dist/trace/component/DisassemblingWindow.js';

describe('DisassemblingWindow Test', () => {
  let disassemblingWindow = new DisassemblingWindow();

  it('DisassemblingWindowTest01', function () {
    expect(disassemblingWindow.getMap('', '')).not.toBeUndefined();
  });

  it('DisassemblingWindowTest02', function () {
    expect(disassemblingWindow.resetCanvas(2, 1, 1, 1)).toBeUndefined();
  });

  it('DisassemblingWindowTest03', function () {
    document.body.innerHTML =
      '<div id="ddd"><tab-native-data-modal id="ccc"></tab-native-data-modal><div id="left_table"></div></div>';
    let disassemblingWindow = document.querySelector('#ccc') as DisassemblingWindow;
    expect(disassemblingWindow.showLoading()).toBeUndefined();
  });

  it('DisassemblingWindowTest04', function () {
    expect(disassemblingWindow.removeCloseListener()).toBeUndefined();
  });

  it('DisassemblingWindowTest05', function () {
    expect(disassemblingWindow.showContent('error', '')).toBeUndefined();
  });
});
