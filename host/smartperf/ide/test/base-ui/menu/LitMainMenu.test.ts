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

  it('LitMainMenu03', () => {
    let litMainMenu = new LitMainMenu();
    expect(litMainMenu.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            width: 248px;
            height: 100vh;
            display: flex;
            flex-direction: column;
            background-color: null;
        }
        .menu-body ::-webkit-scrollbar-thumb
        {
            background-color: var(--dark-background,#FFFFFF);
            border-radius:10px;

        }
        .menu-body ::-webkit-scrollbar-track
        {
            border-radius:10px;
            background-color:#F5F5F5;
            
        }
        .header{
            display: grid;
            width: 100%;
            height: 56px;
            font-size: 1.4rem;
            padding-left: 20px;
            gap: 0 20px;
            box-sizing: border-box;
            grid-template-columns: min-content 1fr min-content;
            grid-template-rows: auto;
            color: #47A7E0;
            background-color: var(--dark-background1);
            border-bottom: 1px solid var(--dark-background1,#EFEFEF);
        }
        .bottom{
            width: 100%;
            display: flex;
            justify-content: space-between;
        }
        .header *{
            user-select: none;
            align-self: center;
        }
        .version{
            width: 15rem;
            padding: 20px 0;
            text-align: center;
            color: #94979d;
            font-size: 0.6rem;
        }
        .color{
            cursor: pointer;
            font-size: 0.6rem;
            padding: 20px;
        }
        *{
            box-sizing: border-box;
        }
        .menu-button{
            display: flex;
            align-content: center;
            justify-content: right;
            cursor: pointer;
            height: 47px;
            width: 48px;
        }
        </style>
        <div class="header" name="header">
            <img src="img/logo.png"/>
                <div class="menu-button">
                    <lit-icon name="menu" size="20" color="var(blue,#4D4D4D)"></lit-icon>
                </div>
            </div>
            <div class="menu-body" style="overflow: auto;overflow-x:hidden;height: 100%">
                <slot id="st" ></slot>
                </div>
        <div class="bottom">        
             <div class="color" style="">
                <lit-icon name="bg-colors" size="20" color="gray"></lit-icon>
             </div>
             <div class="version" style="">
             </div>
        </div>"
`);
  });

  it('LitMainMenu04', () => {
    let litMainMenu = new LitMainMenu();
    litMainMenu.menus = [
      {
        collapsed: true,
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
});
