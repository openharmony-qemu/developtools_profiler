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
import { LitMainMenu } from '../../../dist/base-ui/menu/LitMainMenu.js';
import { MenuItem } from '../../../src/base-ui/menu/LitMainMenu.js';

describe('LitMainMenu Test', () => {
  it('LitMainMenu01', () => {
    let litMainMenu = new LitMainMenu();
    expect(litMainMenu).not.toBeUndefined();
    expect(litMainMenu).not.toBeNull();
  });

  it('LitMainMenu01', () => {
    let litMainMenu = new LitMainMenu();
    expect(litMainMenu).not.toBeUndefined();
    expect(litMainMenu).not.toBeNull();
  });

  it('LitMainMenu02', () => {
    let litMainMenu = new LitMainMenu();
    litMainMenu.menus = [
      {
        collapsed: false,
        title: 'Navigation',
        describe: 'Open or record a new trace',
        children: [
          {
            title: 'Open trace file',
            icon: 'folder',
            fileChoose: true,
            fileHandler: function (ev: InputEvent) {},
          },
          {
            title: 'Record new trace',
            icon: 'copyhovered',
            clickHandler: function (item: MenuItem) {},
          },
        ],
      },
    ];
    expect(litMainMenu.menus.length).toBe(1);
  });

  it('LitMainMenu04', () => {
    let litMainMenu = new LitMainMenu();
    litMainMenu.menus = [
      {
        children: [
          {
            title: 'Record new trace Test',
            icon: 'copyhovered',
            clickHandler: function (item: MenuItem) {},
          },
          {
            title: 'Open trace file Test',
            icon: 'folder',
            fileChoose: true,
            fileHandler: function (ev: InputEvent) {},
          },
        ],
        collapsed: true,
        title: 'Navigation Test',
        describe: 'Open or record a new trace',
      },
    ];
    expect(litMainMenu.menus.length).toBe(1);
  });
});
