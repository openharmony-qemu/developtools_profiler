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

jest.mock('../../../../../dist/trace/component/trace/base/TraceRow.js', () => {
  return {};
});

// @ts-ignore
import { TabPaneFlag } from '../../../../../dist/trace/component/trace/timer-shaft/TabPaneFlag.js';

describe('TabPaneFlag Test', () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1;
  canvas.height = 1;
  const ctx = canvas.getContext('2d');
  let tabPaneFlag = new TabPaneFlag();

  it('TabPaneFlagTest01', function () {
    document.body.innerHTML = '<tabpane-flag id="remove-flag"> </tabpane-flag>';
    tabPaneFlag = document.querySelector('#remove-flag') as TabPaneFlag;
    let htmlButtonElement = document.createElement('button') as HTMLButtonElement;
    document.body.appendChild(htmlButtonElement);
    htmlButtonElement.dispatchEvent(new Event('click'));
    expect(tabPaneFlag.initElements()).toBeUndefined();
  });

  it('TabPaneFlagTest02', function () {
    expect(tabPaneFlag.initHtml()).not.toBe('');
  });

  it('TabPaneFlagTest03', function () {
    expect(
      tabPaneFlag.setFlagObj({
        x: 0,
        y: 0,
        width: 0,
        height: 0,
        time: 0,
        color: '',
        selected: false,
        text: '',
      })
    ).toBeUndefined();
  });

  it('TabPaneFlagTest04', function () {
    expect(tabPaneFlag.initHtml()).toMatchInlineSnapshot(`
"
        <style>
        :host{
            display: flex;
            flex-direction: column;
            padding: 10px 10px;
        }
        .notes-editor-panel{
        display: flex;align-items: center
        }
        .flag-text{
        font-size: 14px;color: var(--dark-color1,#363636c7);font-weight: 300;
        }
        .flag-input{
            border-radius: 4px;
            border: 1px solid var(--dark-border,#dcdcdc);
            color: var(--dark-color1,#212121);
            background: var(--dark-background5,#FFFFFF);
            padding: 3px;
            margin: 0 10px;
        }
        .flag-input:focus{
            outline: none;
            box-shadow: 1px 1px 1px var(--bark-prompt,#bebebe);
        }
        .notes-editor-panel button {
            background: var(--dark-border1,#262f3c);
            color: white;
            border-radius: 10px;
            font-size: 10px;
            height: 22px;
            line-height: 18px;
            min-width: 7em;
            margin: auto 0 auto 1rem;

            border: none;
            cursor: pointer;
            outline: inherit;
        </style>
        <div class="notes-editor-panel">
            <div class="flag-text">Annotation at <span id="flag-time"></span></div>
            <input style="flex: 1" class="flag-input" type="text" id="text-input"/>
            <span class="flag-text">Change color: <input style="background: var(--dark-background5,#FFFFFF);" type="color" id="color-input"/></span>
            <button id="remove-flag">Remove</button>
        </div>
        "
`);
  });
});
